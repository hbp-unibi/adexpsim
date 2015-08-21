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
#include <iostream>

#include <common/Timer.hpp>

#include "Exploration.hpp"
#include "SingleGroupEvaluation.hpp"
#include "SpikeTrainEvaluation.hpp"

namespace AdExpSim {

Exploration::Exploration(std::shared_ptr<ExplorationMemory> mem,
                         const WorkingParameters &params, size_t dimX, Val minX,
                         Val maxX, size_t dimY, Val minY, Val maxY)
    : mem(mem),
      params(params),
      rangeX(minX, maxX, mem->resX),
      rangeY(minY, maxY, mem->resY),
      dimX(dimX),
      dimY(dimY)
{
}

template <typename Evaluation>
bool Exploration::run(const Evaluation &evaluation,
                      const ProgressCallback &progress)
{
	// Fetch the total number of evaluations and the number of cores
	const size_t N = mem->resX * mem->resY;
	size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

	// Function containing the actual exploration task
	auto fun = [&](std::shared_ptr<ExplorationMemory> mem,
	               std::atomic<size_t> &counter, std::atomic<bool> &abort,
	               size_t idx) -> void {
		// Copy the parameters
		WorkingParameters p = params;

		// Iterate over all elements
		size_t i = idx;
		while (i < N && !abort.load()) {
			// Calculate the x and y coordinate from the index and update the
			// parameters according to the given range.
			const size_t x = i % mem->resX;
			const size_t y = i / mem->resX;
			p[dimX] = rangeX.value(x);
			p[dimY] = rangeY.value(y);

			// Do nothing if the given parameters are invalid
			EvaluationResult result;
			if (params.valid()) {
				// Update the parameters
				params.update();

				// Run the evaluation for these parameters
				result = evaluation.evaluate(p);
			}

			// Store the evaluation result in the matrices
			mem->pBinary(x, y) = result.pBinary;
			mem->pFalsePositive(x, y) = result.pFalsePositive;
			mem->pFalseNegative(x, y) = result.pFalseNegative;
			mem->pSoft(x, y) = result.pSoft;

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
		threads.emplace_back(fun, mem, std::ref(counter), std::ref(abort), idx);
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

Exploration Exploration::clone() const
{
	// Clone this instance
	Exploration res(*this);

	// Create a new instance of the ExplorationMemory -- due to CoW semantics
	// the memory is now decoupled
	res.mem = std::make_shared<ExplorationMemory>(*mem);
	return res;
}

/* Specializations of the "run" method. */
template bool Exploration::run<SpikeTrainEvaluation>(
    const SpikeTrainEvaluation &evaluation, const ProgressCallback &progress);
template bool Exploration::run<SingleGroupEvaluation>(
    const SingleGroupEvaluation &evaluation, const ProgressCallback &progress);
}

