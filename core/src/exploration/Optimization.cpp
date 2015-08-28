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
 *  GNU General Public License for more details.d.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <limits>
#include <mutex>
#include <deque>
#include <thread>
#include <iostream>

#include "Optimization.hpp"
#include "Simplex.hpp"
#include "SingleGroupMultiOutEvaluation.hpp"
#include "SingleGroupSingleOutEvaluation.hpp"
#include "SpikeTrainEvaluation.hpp"

namespace AdExpSim {

// Minimum vector distance for two vectors to be considered disimilar
static constexpr Val MIN_DIST_INPUT = 0.1;
static constexpr Val MIN_DIST_OUTPUT = 0.1;

// Maximum value that to-be-optimized inputs may be worse than the currently
// best output
static constexpr Val MAX_WORSE = 0.02;

// Maximum change that may occur in an optimization run to consider a parameter
// set as stable
static constexpr Val MIN_DIFF = 0.1;

// Maximum number of iterations in one pass
static constexpr size_t MAX_IT = 10000;

// Step size of the mixFactor
static constexpr Val MIX_STEP = 0.2;

/**
 * The Pool class holds the input and output parameter pool and manages thread-
 * safe access to these pools.
 */
class Pool {
private:
	mutable std::mutex mutex;

	/**
	 * Tries to find a duplicate of p in the given container, returns the
	 * index of the duplicate if one is found, or -1 otherwise.
	 */
	template <typename T1, typename T2>
	static ssize_t findDuplicate(const T1 &list, const T2 &p, const Val thr)
	{
		// Calculate the minimum distance to any element already in the list
		Val minDist = std::numeric_limits<Val>::max();
		size_t minIdx = -1;
		for (size_t i = 0; i < list.size(); i++) {
			const T2 &q = list[i];
			const Val dist = (p.params - q.params).L2Norm();
			if (dist < minDist) {
				minDist = dist;
				minIdx = i;
			}
		}

		// Only add the element if the distance is larger than some small
		// threshold
		if (minDist < thr) {
			return minIdx;
		}
		return -1;
	}

	/**
	 * Returns the currently best evaluation result or zero of no such
	 * evaluation
	 * result exists.
	 */
	Val bestEval() const { return output.empty() ? 0.0 : output.back().eval; }

public:
	/**
	 * Type used for storing the parameters on the input
	 */
	struct InputParameters {
		WorkingParameters params;
		Val mixFactor;

		InputParameters() {}

		InputParameters(const WorkingParameters &params, Val mixFactor = 0.0)
		    : params(params), mixFactor(mixFactor)
		{
		}
	};

	std::deque<InputParameters> input;
	std::vector<OptimizationResult> output;

	/**
	 * Creates a new Pool instance and copies the given parameters onto the
	 * input pool.
	 */
	Pool(const std::vector<WorkingParameters> &params)
	{
		for (const auto &param : params) {
			input.emplace_back(param, false);
		}
	}

	/**
	 * Returns a lock_guard instance which synchronizes access to the input and
	 * output parameters.
	 */
	std::unique_lock<std::mutex> lock()
	{
		return std::unique_lock<std::mutex>(mutex);
	}

	/**
	 * Pops an input parameter from the input pool. This operation atomically
	 * checks whether an element is available and -- if yes -- returns it.
	 *
	 * @return a pair containing a "valid" flag as first value and the actual
	 * parameter as second value.
	 */
	std::pair<bool, InputParameters> popInput()
	{
		auto poolLock = lock();
		if (input.empty()) {
			return std::make_pair(false, InputParameters());
		}
		auto res = std::make_pair(true, input.front());
		input.pop_front();
		return res;
	}

	/**
	 * Pushes a new parameter onto the input pool, makes sure there are no
	 * duplicated parameter pairs.
	 */
	void pushInput(const WorkingParameters &p, Val eval, Val nextMf)
	{
		auto poolLock = lock();
		if (bestEval() - eval < MAX_WORSE) {
			const InputParameters ip(p, nextMf);
			if (findDuplicate(input, ip, MIN_DIST_INPUT) == -1) {
				input.emplace_back(ip);
			}
		}
	}

	/**
	 * Pushes a new output parameter onto the output pool, makes sure there are
	 * no duplicated parameter pairs.
	 */
	void pushOutput(const WorkingParameters &p, Val eval)
	{
		auto poolLock = lock();

		// Make sure the evaluation measure is positive
		if (eval > MIN_DIFF) {
			// Push it onto the output list without duplicates
			const OptimizationResult opr(p, eval);
			const ssize_t dupIdx = findDuplicate(output, opr, MIN_DIST_OUTPUT);
			if (eval > bestEval() || dupIdx == -1) {
				if (dupIdx == -1) {
					output.push_back(opr);
				} else {
					output[dupIdx] = opr;
				}
				std::sort(output.begin(), output.end());
			}
		}
	}
};

Optimization::Optimization() : model(ModelType::IF_COND_EXP), hw(nullptr) {}

Optimization::Optimization(ModelType model, const std::vector<size_t> &dims,
                           const HardwareParameters &hw)
    : model(model), dims(filterDims(model, dims)), hw(&hw)
{
}

Optimization::Optimization(ModelType model, const std::vector<size_t> &dims)
    : model(model), dims(filterDims(model, dims)), hw(nullptr)
{
}

std::vector<size_t> Optimization::filterDims(ModelType model,
                                             const std::vector<size_t> &dims)
{
	// If the general AdExp model is given, do nothing
	if (model == ModelType::AD_IF_COND_EXP) {
		return dims;
	}

	// Otherwise filter out the dimensions which are not in the IF_COND_EXP
	// model
	std::vector<size_t> res;
	for (size_t dim : dims) {
		if (WorkingParameters::inIfCondExp[dim]) {
			res.push_back(dim);
		}
	}
	return res;
}

template <typename Evaluation>
void Optimization::optimizationThread(const Optimization &optimization,
                                      const Evaluation &eval, Pool &pool,
                                      std::atomic<bool> &abort,
                                      std::atomic<size_t> &nIdle,
                                      std::atomic<size_t> &nIt)
{
	// Flag to be passed to the hardware constraints
	const bool hasHw = optimization.hw;
	const bool useIfCondExp = optimization.model == ModelType::IF_COND_EXP;

	// Define the cost function f
	auto f = [&eval, &optimization, hasHw, useIfCondExp](
	             const WorkingParameters &p) -> Val {
		// Return the worst possible cost (zero, as all other costs are
		// negative) if the parameters are not realisable
		if (!p.valid() ||
		    (hasHw && !optimization.hw->possible(p, useIfCondExp))) {
			return 0.0;
		}

		// Evaluate the parameters, return the negative of the selected
		// target dimension (the optimization needs a cost and the
		// evaluation returns a success rate)
		return -eval.evaluate(p)[eval.descriptor().optimizationDim()];
	};

	// Repeat until the "abort" flag has been set by the calling code
	while (!abort.load()) {
		// Fetch an input WorkingParameters set, if no input data is available
		// just sleep
		const auto in = pool.popInput();
		if (!in.first) {
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			continue;
		}

		// We have work to do!
		nIdle--;

		// Copy the current WorkingParameters and get the current evaluation
		// measure
		const WorkingParameters params = in.second.params;
		const Val initialEval = f(params);

		// Fetch the current and the next mix factor -- the mix factor is used
		// to interploate between the forced hardware setup and the
		// current parameters
		Val curMf = hasHw ? in.second.mixFactor : 0.0;
		Val nextMf = hasHw ? curMf + MIX_STEP : 0.0;
		if (nextMf > 1.0f) {
			curMf = 1.0f;
			nextMf = 0.0f;
		}

		// Create the simplex algorithm instance, fetch the to-be-optimized
		// dimensions
		SimplexStepResult res;
		Simplex<WorkingParameters> simplex(
		    params, optimization.getDims(curMf != 0.0), f);

		// Run, abort after MAX_IT iterations or if the simplex indicates it is
		// done or if the process is manually aborted
		size_t it = 0;
		do {
			// Perform a simplex step
			res = simplex.step(f);

			// Increment the local and global iteration counter
			it++;
			nIt++;
		} while (!res.done && !abort.load() && it < MAX_IT);

		// If a hardware limitation is present, map the optimized values to the
		// hardware -- then remap them to WorkingParameters. If there is no
		// HW limitation just add the optimized params.
		const WorkingParameters optimizedParams = simplex.getBest();
		std::vector<WorkingParameters> finalParams;
		if (hasHw) {
			std::vector<Parameters> mapped =
			    optimization.hw->map(optimizedParams, useIfCondExp);
			for (const Parameters &p : mapped) {
				finalParams.push_back((optimizedParams * (1.0f - curMf)) +
				                      (WorkingParameters(p) * curMf));
			}
		} else {
			finalParams.push_back(optimizedParams);
		}

		// Check whether the parameters should be added to the output or
		// sent through the pipeline for a second round
		for (const WorkingParameters &p : finalParams) {
			// If there has been no substantial change in this optimization run
			// add the parameters to the output -- otherwise push the optimized
			// and (possibly mapped) parameters back to the input and use the
			// next mix factor.
			const Val eval = f(p);
			const bool hasSubstantialChange =
			    fabs(initialEval - eval) > MIN_DIFF;
			if (!hasSubstantialChange && nextMf == 0.0f) {
				pool.pushOutput(p, -eval);
			} else {
				pool.pushInput(p, -eval, nextMf);
			}
		}

		// We're done working, increment the "nIdle" counter
		nIdle++;
	}
}

template <typename Evaluation>
std::vector<OptimizationResult> Optimization::optimize(
    const std::vector<WorkingParameters> &params, const Evaluation &eval,
    ProgressCallback callback) const
{
	// Abort if dims is empty or params is empty
	if (dims.empty() || params.empty()) {
		return std::vector<OptimizationResult>();
	}

	// Fetch the number of threads to be used
	size_t nThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

	// Copy the given parameters into the parameter pool
	Pool pool(params);

	std::atomic<bool> abort(false);       // Flag used to abort all threads
	std::atomic<size_t> nIdle(nThreads);  // Number of threads idling
	std::atomic<size_t> nIt(0);           // Number of iterations performed

	// Create a thread for each hardware thread
	std::vector<std::thread> threads;
	for (size_t i = 0; i < nThreads; i++) {
		threads.emplace_back(optimizationThread<Evaluation>, *this, eval,
		                     std::ref(pool), std::ref(abort), std::ref(nIdle),
		                     std::ref(nIt));
	}

	// Wait for all threads to be finished
	while (true) {
		{
			// Abort if all threads idle and the input parameter array is empty
			// or if the callback returns false
			auto poolLock = pool.lock();
			if ((nIdle.load() == nThreads && pool.input.empty()) ||
			    !callback(nIt.load(),
			              pool.input.size() + nThreads - nIdle.load(),
			              pool.output)) {
				abort.store(true);
				break;
			}
		}

		// Sleep some time before checking again
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	// Wait for all threads to be finished
	for (auto &thread : threads) {
		thread.join();
	}

	// Return the final output parameters
	return pool.output;
}

std::vector<size_t> Optimization::getDims(bool clampDiscrete) const
{
	if (clampDiscrete) {
		std::vector<size_t> res;
		for (size_t dim : dims) {
			// Do not add the discrete "w" dimension to the result
			if (dim != WorkingParameters::idx_w) {
				res.push_back(dim);
			}
		}
		return res;
	}
	return dims;
}

/* Specializations of the "optimize" method. */
template std::vector<OptimizationResult> Optimization::optimize<
    SpikeTrainEvaluation>(const std::vector<WorkingParameters> &params,
                          const SpikeTrainEvaluation &eval,
                          ProgressCallback callback) const;
template std::vector<OptimizationResult>
Optimization::optimize<SingleGroupSingleOutEvaluation>(
    const std::vector<WorkingParameters> &params,
    const SingleGroupSingleOutEvaluation &eval,
    ProgressCallback callback) const;
template std::vector<OptimizationResult> Optimization::optimize<
    SingleGroupMultiOutEvaluation>(const std::vector<WorkingParameters> &params,
                                   const SingleGroupMultiOutEvaluation &eval,
                                   ProgressCallback callback) const;
}

