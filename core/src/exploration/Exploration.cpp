/*
 *  AdExpSim -- Simulator for the AdExp model
 *  Copyright (C) 2015  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define PTHREAD_SET_PRIORITY
#ifdef PTHREAD_SET_PRIORITY
#include <pthread.h>
#include <sched.h>
#endif

#include <atomic>
#include <thread>
#include <vector>

#include "Exploration.hpp"

#include "SingleGroupSingleOutEvaluation.hpp"
#include "SingleGroupMultiOutEvaluation.hpp"
#include "SpikeTrainEvaluation.hpp"

namespace AdExpSim {

template <typename Evaluation>
bool Exploration::run(const Evaluation &evaluation,
                      const ProgressCallback &progress)
{
	// Create the ExplorationMemory instance
	// Note: It might seem somewhat wasteful to throw away any existing memory
	// instance and not to reuse it. However, exploration takes significantly
	// longer than memory allocation.
	mMem = ExplorationMemory(evaluation.descriptor(), resX(), resY());

	// Fetch the total number of evaluations and the number of cores
	const size_t N = resX() * resY();
	size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

	// Function containing the actual exploration task
	auto fun = [&](ExplorationMemory &mem, std::atomic<size_t> &counter,
	               std::atomic<bool> &abort, size_t idx) -> void {
		// Copy the parameters
		WorkingParameters p = params();

		// Variable containing the evaluation result
		EvaluationResult result(mem.descriptor.size());

		// Iterate over all elements
		size_t i = idx;
		while (i < N && !abort.load()) {
			// Calculate the x and y coordinate from the index and update the
			// parameters according to the given range.
			const size_t x = i % resX();
			const size_t y = i / resX();
			p[dimX()] = rangeX().value(x);
			p[dimY()] = rangeY().value(y);

			// Check whether the parameters are valid, if not use the default
			// evaluation result
			if (p.valid()) {
				p.update();
				result = evaluation.evaluate(p);
			} else {
				result = descriptor().defaultResult();
			}

			// Store the evaluation result in the matrices
			mem.store(x, y, result);

			// Increment the counter
			counter++;

			// Go to the next work item
			i += nThreads;
		}
	};

	// Create a thread for each hardware thread
	std::vector<std::thread> threads;
	std::atomic<size_t> counter(0);
	std::atomic<bool> abort(false);
	for (size_t idx = 0; idx < nThreads; idx++) {
		threads.emplace_back(fun, std::ref(mMem), std::ref(counter),
		                     std::ref(abort), idx);
#ifdef PTHREAD_SET_PRIORITY
		// Fetch the native pthread handle
		auto handle = threads.back().native_handle();

		// Let's be nice and reduce the thread priority
		sched_param sch;
		int policy;
		if (pthread_getschedparam(handle, &policy, &sch) == 0) {
			sch.sched_priority = sched_get_priority_min(policy);
			pthread_setschedparam(handle, policy, &sch);
		}
#endif
	}

	// Wait for all threads to be finished
	while (true) {
		// Fetch the current progress
		size_t totalCount = counter.load();

		// Call the progress function
		if (!progress(Val(totalCount) / Val(N))) {
			abort.store(true);
			break;
		}

		// Sleep some time before checking again
		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		// Abort if the threads are finished
		if (totalCount >= N) {
			break;
		}
	}

	// Wait for all threads to be finished
	for (auto &thread : threads) {
		thread.join();
	}
	return !abort.load();
}

/* Specializations of the "run" method. */
template bool Exploration::run<SpikeTrainEvaluation>(
    const SpikeTrainEvaluation &evaluation, const ProgressCallback &progress);
template bool Exploration::run<SingleGroupSingleOutEvaluation>(
    const SingleGroupSingleOutEvaluation &evaluation,
    const ProgressCallback &progress);
template bool Exploration::run<SingleGroupMultiOutEvaluation>(
    const SingleGroupMultiOutEvaluation &evaluation,
    const ProgressCallback &progress);
}

