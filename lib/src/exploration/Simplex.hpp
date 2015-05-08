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
 * @file Simplex.hpp
 *
 * Contains a class implementing the Downhill Simplex algorithm by Nelder und
 * Mead.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SIMPLEX_HPP_
#define _ADEXPSIM_SIMPLEX_CPP_

#include <cstdlib>

#include <algorithm>
#include <limits>
#include <vector>

#include <utils/Types.hpp>

namespace AdExpSim {

/**
 * Struct used to describe what happened in a single optimization step.
 */
struct SimplexStepResult {
	/**
	 * Contains the cost of the currently best parameter set.
	 */
	Val bestValue;

	/**
	 * Contains the mean cost of the current simplex.
	 */
	Val meanValue;

	/**
	 * Set to true if the optimization is done.
	 */
	bool done : 1;

	/**
	 * Set to true if this is a new best value.
	 */
	bool hasNewBest : 1;

	/**
	 * Set to true if the simplex is currently contracting or reducing.
	 */
	bool reducing : 1;

	SimplexStepResult()
	    : bestValue(std::numeric_limits<Val>::max()),
	      meanValue(std::numeric_limits<Val>::max()),
	      done(false),
	      hasNewBest(false),
	      reducing(false)
	{
	}

	SimplexStepResult(Val bestValue, Val meanValue, bool done, bool hasNewBest,
	                  bool reducing)
	    : bestValue(bestValue),
	      meanValue(meanValue),
	      done(done),
	      hasNewBest(hasNewBest),
	      reducing(reducing)
	{
	}
};

/**
 * This class implements the Nelder–Mead method also known as Downhill Simplex
 * method. See http://en.wikipedia.org/wiki/Nelder%E2%80%93Mead_method
 */
template <typename Vector>
class Simplex {
public:
	/**
	 * The ValueVector struct is used to extend the given Vector type with a
	 * value y which is calculated from a function f at the time the vector is
	 * created.
	 */
	struct ValueVector {
		/**
		 * The actual vector that is being stored.
		 */
		Vector x;

		/**
		 * The associated value y.
		 */
		Val y;

		/**
		 * Constructor of the ValueVector struct.
		 *
		 * @tparam Function is the type of the function that is used to
		 * calculate the value y from x.
		 * @param x is the vector.
		 * @param f is the function calculating y from x.
		 */
		template <typename Function>
		ValueVector(const Vector &x, Function f)
		    : x(x), y(f(x))
		{
		}

		/**
		 * Operator used to sort the vectors.
		 *
		 * @param v1 is the first element of the comparison.
		 * @param v2 is the second element of the comparison.
		 * @return true if v1 has a smaller value than v2.
		 */
		friend bool operator<(const ValueVector &v1, const ValueVector &v2)
		{
			return v1.y < v2.y;
		}
	};

private:
	/**
	 * Function used internally to build the initial vector set by varying a
	 * single vector dimension.
	 *
	 * @param x is the vector that should be varried.
	 * @param dim is the dimension of the vector that should be varried.
	 * @param fac is the factor by which the specified dimension should be
	 * multiplied to produce the variation.
	 */
	static Vector vary(const Vector &x, size_t dim, Val fac)
	{
		Vector res = x;
		res[dim] *= fac;
		return res;
	}

	/**
	 * The restart function preserves the currently best simplex point but
	 * randomly distributes all other simplex points along the axes of the
	 * parameter dimensions.
	 *
	 * @param f is the function used to evaluate the simplex.
	 */
	template <typename Function>
	void restart(Function f)
	{
		for (size_t i = 0; i < N; i++) {
			// Randomly draw a scale factor between 0.5 and 1.5
			const int r = std::rand();
			const Val range = 0.5;
			const Val fac = 1.0 - range + 2.0 * range * (Val(r) / Val(RAND_MAX));

			// Create a version of the x-Vector with the corresponding dimension
			// scaled
			simplex[i + 1] = ValueVector(vary(simplex[0].x, dims[i], fac), f);
		}
	}

	/**
	 * Counter used to control how many times the simplex has been restarted in
	 * the last time.
	 */
	size_t restartCount = 0;

	/**
	 * Counter used to abort after a certain number of iterations.
	 */
	size_t iterationCount = 0;

	/**
	 * Number of dimensions that should be optimized.
	 */
	const size_t N;

	/**
	 * Indices of the dimensions that are being optimized.
	 */
	const std::vector<size_t> dims;

	/**
	 * Alpha controls the construction of the reflected point.
	 */
	const Val alpha;

	/**
	 * Gamma controls the construction of the expanded point.
	 */
	const Val gamma;

	/**
	 * Rho controls the construction of the contracted point.
	 */
	const Val rho;

	/**
	 * Sigma controls the reduction process.
	 */
	const Val sigma;

	/**
	 * Simplex is a list of N + 1 vectors, where N is the number of dimensions
	 * that should be optimized.
	 */
	std::vector<ValueVector> simplex;

public:
	/**
	 * Constructor of the Simplex class. Generically implements the Downhill
	 * Simplex algorithm.
	 *
	 * @tparam Vector is the vector type to which the optimization should be
	 * applied.
	 * @tparam Function is the cost function that should be used to evaluate
	 * the vectors.
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
	template <typename Function>
	Simplex(const Vector xInit, const std::vector<size_t> &dims, Function f,
	        Val fac = 1.1, Val alpha = 1.0, Val gamma = 2.0, Val rho = -0.5,
	        Val sigma = 0.5)
	    : N(dims.size()),
	      dims(dims),
	      alpha(alpha),
	      gamma(gamma),
	      rho(rho),
	      sigma(sigma)
	{
		// Create the initial simplex
		simplex.reserve(N + 1);
		simplex.emplace_back(xInit, f);
		for (size_t i = 0; i < N; i++) {
			simplex.emplace_back(vary(xInit, dims[i], fac), f);
		}
	}

	/**
	 * The step function implements a single step in the optimization process.
	 *
	 * @tparam Function is the cost function that should be used to evaluate
	 * the vectors.
	 * @param f is the cost function.
	 * @param epsilon controls the abort condition of the algorithm. If the
	 * mean cost for all points in the simplex minus the smallest cost is
	 * smaller than epsilon, the algorithm aborts.
	 */
	template <typename Function>
	SimplexStepResult step(Function f, Val epsilon = 1e-5)
	{
		iterationCount++;

		// (1) Order
		// Sort the vectors asscending by function value
		std::sort(simplex.begin(), simplex.end());

		// Calculate the mean function value, abort once all points have
		// settled (the difference in the function values is smaller than
		// epsilon)
		Val mean = 0;
		for (auto &v : simplex) {
			mean += v.y;
		}
		mean = mean / (N + 1);
		if (mean - simplex[0].y < epsilon || iterationCount > 100) {
			if (restartCount < N * 10) {
				restart(f);
				restartCount++;
				iterationCount = 0;
			} else {
				return SimplexStepResult(simplex[0].y, mean, true, false,
				                         false);
			}
		}

		// (2) Centroid
		Vector x0(simplex[0].x);
		for (size_t i = 1; i < N; i++) {
			x0 = x0 + simplex[i].x;
		}
		x0 = x0 / N;

		// (3) Reflection
		// Compute the reflected point and evaluate it
		ValueVector vr(x0 + alpha * (x0 - simplex[N].x), f);

		// If the reflected point is worse than the best point but better
		// than the second-worst point, replace the worst point with the
		// new point and abort
		if (vr.y > simplex[0].y && vr.y < simplex[N - 1].y) {
			simplex[N] = vr;
			return SimplexStepResult(simplex[0].y, mean, false, false, false);
		}

		// (4) Expansion
		// If vr is the best point so far, calculate an expanded point ve
		if (vr.y < simplex[0].y) {
			// If we got a significantly better point, reset restartCount
			if (simplex[0].y - vr.y > epsilon) {
				restartCount = 0;
				iterationCount = 0;
			}
			ValueVector ve(x0 + gamma * (x0 - simplex[N - 1].x), f);
			if (ve.y < vr.y) {
				simplex[N] = ve;
			} else {
				simplex[N] = vr;
			}
			return SimplexStepResult(simplex[0].y, mean, false, true, false);
		}

		// (5) Contraction
		ValueVector vc(x0 + rho * (x0 - simplex[N].x), f);
		if (vc.y < simplex[N].y) {
			simplex[N] = vc;
			return SimplexStepResult(simplex[0].y, mean, false, false, true);
		}

		// (6) Reduction
		for (size_t i = 1; i < N + 1; i++) {
			simplex[i] = ValueVector(
			    simplex[0].x + sigma * (simplex[i].x - simplex[0].x), f);
		}
		return SimplexStepResult(simplex[0].y, mean, false, false, true);
	}

	/**
	 * Returns a reference at the internally used simplex.
	 */
	const std::vector<ValueVector> &getSimplex() const { return simplex; }
};
}

#endif /* _ADEXPSIM_SIMPLEX_HPP_ */

