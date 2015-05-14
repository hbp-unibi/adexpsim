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

#include <atomic>
#include <thread>
#include <vector>

#include <utils/Timer.hpp>

#include "Exploration.hpp"

namespace AdExpSim {

Exploration::Exploration(const ExplorationMemory &mem,
                         WorkingParameters &params, Val Xi, Val T, size_t dimX,
                         Val minX, Val maxX, size_t dimY, Val minY, Val maxY)
    : mem(mem),
      params(params),
      rangeX(minX, maxX, mem.resX),
      rangeY(minY, maxY, mem.resY),
      dimX(dimX),
      dimY(dimY),
      evaluation(Xi, T)
{
}

bool Exploration::run(const ProgressCallback &progress)
{
	// Fetch the total number of evaluations and the number of cores
	const size_t N = mem.resX * mem.resY;
	size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

	// Function containing the actual exploration task
	auto fun = [&](ExplorationMemory &mem, std::atomic<size_t> &counter,
	               std::atomic<bool> &abort, size_t idx) -> void {
		// Copy the parameters
		WorkingParameters p = params;

		// Iterate over all elements
		size_t i = idx;
		while (i < N) {
			// Calculate the x and y coordinate from the index and update the
			// parameters according to the given range.
			const size_t x = i % mem.resX;
			const size_t y = i / mem.resX;
			p[dimX] = rangeX.value(x);
			p[dimY] = rangeY.value(y);

			// Run the evaluation for these parameters
			EvaluationResult result = evaluation.evaluate(p);

			// Store the evaluation result in the matrices
			mem.eSpikeEff(x, y) = result.eSpikeEff;
			mem.eMaxXi(x, y) = result.eMaxXi;
			mem.eMaxXiM1(x, y) = result.eMaxXiM1;
			mem.tSpike(x, y) = result.tSpike;
			mem.tReset(x, y) = result.tReset;

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
		threads.emplace_back(fun, std::ref(mem), std::ref(counter),
		                   std::ref(abort), idx);
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
}

