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

/**
 * @file SimplexPool.hpp
 *
 * Contains a class which extends the Simplex algorithm by randomly sampling the
 * start points. It executes the evaluations in parallel.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SIMPLEX_POOL_HPP_
#define _ADEXPSIM_SIMPLEX_POOL_HPP_

#include <mutex>
#include <atomic>
#include <thread>
#include <random>
#include <limits>

#include "Simplex.hpp"

namespace AdExpSim {

/**
 * Class which randomly pertubates the given start parameter and executes the
 * Simplex algorithm on these vectors.
 */
template <typename Vector>
class SimplexPool {
private:
	/*
	 * Parameters to be passed to the individual simplex instances.
	 */
	Vector xInit, xBest;
	std::vector<size_t> dims;
	size_t nSamples;
	Val fac, alpha, gamma, rho, sigma;

	/**
	 * Mutex protecting access to the "xBest" vector
	 */
	std::mutex bestMutex;

	/**
	 * Current cost for the best vector.
	 */
	Val costBest;

	/**
	 * Randomizes the dimensions of the given vector "vec" specified in "dims"
	 * by either multiplying or dividing by a value between 1.0 and 10.0.
	 *
	 * @param vec is the vector that should be randomized.
	 * @param dims is an vector containing the dimensions that should be
	 * affected by the randomization.
	 * @return the randomized vector.
	 */
	static Vector randomize(Vector vec, const std::vector<size_t> &dims)
	{
		static size_t seed = 1241249190;
		std::default_random_engine generator(seed++);
		std::uniform_real_distribution<Val> dFactor(1.0, 1.1);
//		std::uniform_int_distribution<int> dDim(0, dims.size() - 1);
		std::uniform_int_distribution<int> dChoice(0, 1);
//		size_t dim = dims[dDim(generator)];
		for (size_t dim: dims) {
			if (dChoice(generator)) {
				vec[dim] *= dFactor(generator);
			} else {
				vec[dim] /= dFactor(generator);
			}
		}
		return vec;
	}

	/**
	 * Actual optimization function executed in each thread.
	 *
	 * @tparam Function is the cost function type.
	 * @param pool is a reference at the SimplexPool instance.
	 * @param f is the cost function.
	 * @param max_it is the maximum number of iterations.
	 * @param max_samples is the maximum number of samples that should be drawn.
	 * @param epsilon is the minimum difference in simplex vectors which ends
	 * the optimization process.
	 * @param samples is a counter counting how many random samples have been
	 * produced until now.
	 * @param it is a counter counting the global number of iterations.
	 * @param abort is a flag which aborts the entire optimization process.
	 */
	template <typename Function>
	static void optimizationThread(SimplexPool<Vector> &pool, Function f,
	                               size_t max_it, float epsilon,
	                               std::atomic<size_t> &samples,
	                               std::atomic<size_t> &it,
	                               std::atomic<bool> &abort,
	                               std::atomic<size_t> &done)
	{
		while (!abort.load()) {
			// Abort if all samples have been processed
			const size_t sample = samples.fetch_add(1);
			if (sample >= pool.nSamples) {
				break;
			}

			// Create a randomized version of the initial vector -- with the
			// exception of this being the very first sample.
			const Vector x = (sample == 0) ? pool.xInit : randomize(pool.xInit, pool.dims);
			if (Val(f(x)) >= std::numeric_limits<Val>::max()) {
				continue;
			}

			// Initialize the simplex instance
			Simplex<Vector> simplex(x, pool.dims, f, pool.fac, pool.alpha,
			                        pool.gamma, pool.rho, pool.sigma);

			// Run, abort after max_it iterations or if the simplex indicates it
			// is done or if the process is manually aborted
			size_t localIt = 0;
			SimplexStepResult res;
			do {
				// Perform a simplex step
				res = simplex.step(f, epsilon);

				// Increment the local and global iteration counter
				localIt++;
				it++;

				// Update "costBest"
				{
					std::lock_guard<std::mutex> lock(pool.bestMutex);
					if (res.bestValue < pool.costBest) {
						pool.costBest = res.bestValue;
					}
				}
			} while (!res.done && !abort.load() && localIt < max_it);

			// If the best vector of the simplex is better than the currently
			// best vector, replace it
			{
				std::lock_guard<std::mutex> lock(pool.bestMutex);
				if (res.bestValue <= pool.costBest) {
					pool.costBest = res.bestValue;
					pool.xBest = simplex.getBest();
				}
			}
		}
		done++;
	}

public:
	struct SimplexPoolResult {
		/**
		 * Best vector.
		 */
		Vector best;

		/**
		 * Initial cost value.
		 */
		Val costInit;

		/**
		 * Best cost value.
		 */
		Val costBest;

		/**
		 * Constructor of the SimplexPoolResult class.
		 */
		SimplexPoolResult(const Vector &best, Val costInit, Val costBest)
		    : best(best), costInit(costInit), costBest(costBest)
		{
		}
	};

	/**
	 * Constructor of the Simplex class. Generically implements the Downhill
	 * Simplex algorithm.
	 *
	 * @tparam Vector is the vector type to which the optimization should be
	 * applied.
	 * @tparam Function is the cost function that should be used to evaluate
	 * the vectors.
	 * @param nSamplesPerDim controls how many samples are chosen per dimension
	 * (note this number is only multiplied by the number of dimensions, not
	 * potentiated).
	 * @param xInit is the initial vector.
	 * @param f is the cost function.
	 * @param dims is a vector containing the indices of the dimensions that
	 * should be optimized.
	 * @param fac controls the creation of the initial simplex. It is the factor
	 * by which the individual dimensions are multiplied.
	 * @param alpha controls the construction of the reflected point.
	 * @param gamma controls the construction of the expanded point.
	 * @param rho controls the construction of the contracted point.
	 * @param sigma controls the reduction process.
	 */
	SimplexPool(const Vector xInit, const std::vector<size_t> &dims,
	            size_t nSamples = 100, Val fac = 1.1, Val alpha = 1.0,
	            Val gamma = 2.0, Val rho = -0.5, Val sigma = 0.5)
	    : xInit(xInit),
	      xBest(xInit),
	      dims(dims),
	      nSamples(nSamples),
	      fac(fac),
	      alpha(alpha),
	      gamma(gamma),
	      rho(rho),
	      sigma(sigma)
	{
	}

	/**
	 * The step function implements a single step in the optimization process.
	 * This function can be called multiple times (but not concurrently).
	 *
	 * @tparam Function is the cost function that should be used to evaluate
	 * the vectors.
	 * @tparam Callback is a callback function that gets called with the current
	 * number of iterations. Gets two arguments: The current number of
	 * iterations and the currently best cost value. Should return "false" if
	 * the operation is to be aborted, true otherwise.
	 * @param f is the cost function.
	 * @param max_it is the maximum number of iterations that should be
	 * performed in each individual optimization operation. Default is virtually
	 * no limit.
	 * @param epsilon controls the abort condition of the algorithm. If the
	 * mean cost for all points in the simplex minus the smallest cost is
	 * smaller than epsilon, the algorithm aborts.
	 * @return the best vector.
	 */
	template <typename Function, typename Callback>
	SimplexPoolResult run(Function f, Callback callback,
	                      size_t max_it = std::numeric_limits<size_t>::max(),
	                      Val epsilon = 1e-5)
	{
		// Calculate the initial cost and the cost of the best vector.
		const Val costInit = f(xInit);
		costBest = f(xBest);

		// Values shared by all threads
		std::atomic<size_t> samples(0);
		std::atomic<size_t> it(0);
		std::atomic<size_t> done(0);
		std::atomic<bool> abort(false);

		// Execute thread_fun once per processor
		// Fetch the number of threads to be used
		size_t nThreads =
		    std::max<size_t>(1, std::thread::hardware_concurrency());

		// Create a thread for each hardware thread
		std::vector<std::thread> threads;
		for (size_t i = 0; i < nThreads; i++) {
			threads.emplace_back(optimizationThread<Function>, std::ref(*this),
			                     f, max_it, epsilon, std::ref(samples),
			                     std::ref(it), std::ref(abort), std::ref(done));
		}

		// Wait for all threads to be finished
		while (done.load() < nThreads) {
			{
				std::lock_guard<std::mutex> lock(bestMutex);
				if (!callback(it.load(), std::min(nSamples, samples.load()),
				              costBest)) {
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

		// Return the best result vector
		return SimplexPoolResult(xBest, costInit, costBest);
	}
};
}

#endif /* _ADEXPSIM_SIMPLEX_POOL_HPP_ */

