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

/**
 * @file DormandPrinceIntegrator.hpp
 *
 * Implementation of a fifth-order embedded Runge-Kutta method with adaptive
 * stepsize control. Note that this implementation is particularly tailored for
 * autonomous differential equations (df does not depend on t). Inspired by the
 * algorithm presented in Numerical Recipes (NR), 3rd Edition chapter 17.2.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_DORMAND_PRINCE_HPP_
#define _ADEXPSIM_DORMAND_PRINCE_HPP_

#include <utility>

#include <utils/Types.hpp>

#include "State.hpp"

namespace AdExpSim {

namespace DormandPrinceInternal {

/**
 * Dormand-Prince Parameters for Embedded Runge-Kutta coefficients. See table in
 * chapter 17.2 of Numerical Recipes, 3rd Edition.
 */
static constexpr double COEFF_A[7][6] = {
    {0, 0, 0, 0, 0, 0},
    {1.0 / 5.0, 0, 0, 0, 0, 0},
    {3.0 / 40.0, 9.0 / 40.0, 0, 0, 0, 0},
    {44.0 / 45.0, -56.0 / 15.0, 32.0 / 9.0},
    {19372.0 / 6561.0, -25360.0 / 2187.0, 64448.0 / 6561.0, -212.0 / 729.0, 0,
     0},
    {9017.0 / 3168.0, -355.0 / 33.0, 46732.0 / 5247.0, 49.0 / 176.0,
     -5103.0 / 18656.0, 0},
    {35.0 / 384.0, 0, 500.0 / 1113.0, 125.0 / 192.0, -2187.0 / 6784.0,
     11.0 / 84.0}};

/**
 * Coefficients used to estimate the error vector (b_i - b_i^* in the
 * afforementioned Table in NR).
 */
static constexpr double COEFF_E[7] = {
    71.0 / 576000.0,     -71.0 / 16695.0, 0,          71.0 / 1920.0,
    -17253.0 / 339200.0, 22.0 / 525.0,    -1.0 / 40.0};

/**
 * Returns the Fifth-order Runge-Kutta coefficients.
 */
constexpr Val a(size_t i, size_t j) { return COEFF_A[i - 1][j - 1]; }

/**
 * Returns the error-vector coefficients.
 */
constexpr Val e(size_t i) { return COEFF_E[i - 1]; }

/*
 * The RungeKuttaEval function is used to evaluate the inner sum of a
 * Runge-Kutta step. The variadic template construct is used to calculate
 * for (i = 1; i < N; i++) c(i) * k_i without runtime overhead.
 */

template <typename Coeffs, typename K>
static inline K RungeKuttaEval(Coeffs c, size_t i, const K &k)
{
	return c(i) * k;
}

template <typename Coeffs, typename K, typename... KS>
static inline K RungeKuttaEval(Coeffs c, size_t i, const K &k, const KS &... ks)
{
	return c(i) * k + RungeKuttaEval(c, i + 1, ks...);
}

/*
 * The RungeKuttaStepInner function is used to evaluate a single RungeKutta step
 * without passing the result to the function calculating the derivative.
 */

template <typename Vector, typename... KS>
static inline Vector RungeKuttaStepInner(Val h, const Vector &y, size_t step,
                                         const KS &... ks)
{
	// Curry the coefficient function a to a function c with a fixed step
	auto c = [step](size_t j) { return a(step, j); };

	// Fold over all ks
	return y + h * RungeKuttaEval(c, 1, ks...);
}

/*
 * The RungeKuttaStep function is used to evaluate a single step of the
 * Runge-Kutta method. It calculates
 *     k_step = y + df(\sum_{i = 1}^{step - 1} a(step, i) * k_i)
 */

template <typename Vector, typename Deriv, typename... KS>
static inline Vector RungeKuttaStep(Val h, const Vector &y, Deriv df,
                                    size_t step, const KS &... ks)
{
	return df(RungeKuttaStepInner(h, y, step, ks...));
}

template <typename Vector, typename Deriv>
static inline Vector RungeKuttaStep(Val, const Vector &y, Deriv df, size_t)
{
	return df(y);
}
/**
 * Implements the fifth-order embedded RungeKutta method. Returns the value of
 * the function
 */
template <typename Vector, typename Deriv>
static std::pair<Vector, Vector> RungeKutta5(Val h, const Vector &y, Deriv df)
{
	// Execute the five Runge-Kutta steps
	const Vector k1 = RungeKuttaStep(h, y, df, 1);
	const Vector k2 = RungeKuttaStep(h, y, df, 2, k1);
	const Vector k3 = RungeKuttaStep(h, y, df, 3, k1, k2);
	const Vector k4 = RungeKuttaStep(h, y, df, 4, k1, k2, k3);
	const Vector k5 = RungeKuttaStep(h, y, df, 5, k1, k2, k3, k4);
	const Vector k6 = RungeKuttaStep(h, y, df, 6, k1, k2, k3, k4, k5);

	// Calculate the new value for y
	const Vector yN = RungeKuttaStepInner(h, y, 7, k1, k2, k3, k4, k5, k6);

	// Estimate the error
	const Vector k7 = df(yN);
	const Vector yErr = h * RungeKuttaEval(e, 1, k1, k2, k3, k4, k5, k6, k7);

	// Return both the result vector and the error vector
	return std::pair<Vector, Vector>(yN, yErr);
}
}

/**
 * Class implementing the stepsize control for a generic integrator. The actual
 * integrator must provide the integrated solution as well as an error vector
 * containing the estimated error the integrator made as a feedback for the
 * stepsize controller.
 */
template <typename Impl>
class AdaptiveIntegratorBase {
private:
	/**
	 * Inverse target error.
	 */
	const Val invETar;

	/**
	 * Last stepsize.
	 */
	Val hOld;

	/**
	 * Calculates a single error vector form the error vector. Calculates the
	 * L2-norm of the vector.
	 */
	Val error(State errVec) const
	{
		errVec = errVec * invETar;
		errVec = errVec * errVec * 0.25;
		Val err = 0.0f;
		for (size_t k = 0; k < 4; k++) {
			err += errVec[k];
		}
		err = sqrtf(err);
		return err;
	}

public:
	/**
	 * Constructor of the AdaptiveIntegratorBase class. Initializes the scale
	 * vector depending on the given target error.
	 *
	 * @param err is the target integration error.
	 */
	AdaptiveIntegratorBase(Val eTar = 1e-3) : invETar(1.0 / eTar) { reset(); }

	/**
	 * Resets the integrator to its initial state.
	 */
	void reset() { hOld = 0.0f; }

	/**
	 * Implements an integrator with adaptive step size.
	 *
	 * @param tDelta is the timestep width.
	 * @param tDeltaMax is the maximum step size that can be used.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	std::pair<State, Time> integrate(Time tDelta, Time tDeltaMax,
	                                 const State &s, Deriv df)
	{
		static constexpr Val S = 0.9; // Safety factor
		static constexpr Val MIN_H = 1e-6; // Absolute minimum for h.
		static constexpr Val MIN_SCALE = 0.2; // Minimum scale factor.
		static constexpr Val MAX_SCALE = 10.0; // Maximum scale factor.

		// Fetch the step size as floating point number
		const Val MAX_H = std::min(10e-3, tDeltaMax.sec());
		Val h = hOld == 0.0f ? MAX_H : std::min(hOld, MAX_H);

		// Integrator result storage. First element is the integrated value,
		// second value is the estimated error vector
		std::pair<State, State> res;

		// Stepsize for the next iteration
		Val hNew;

		// Flags used for infinite loop prevention
		bool reachedMinH = false;
		bool reachedMaxH = false;
		while (true) {
			// Run the actual integrator
			res = static_cast<const Impl *>(this)->doIntegrate(h, s, df);

			// Calculate the normalized error
			const Val e = error(res.second);

			// Calculate the timestep scale factor, limit it to the minimum and
			// maximum scale. We're neither using the PI controller proposed in
			// NR here and approximate S * err^{-1/5} with S / err. Works better
			// and faster.
			Val scale = (e == 0.0)
			                ? MAX_SCALE
			                : std::min(MAX_SCALE, std::max(MIN_SCALE, S / e));

			// Adjust the stepsize, make sure the stepsize is not smaller than
			// the maximum/minimum stepsize
			hNew = h * scale;
			if (hNew < MIN_H) {
				hNew = MIN_H;
				if (reachedMinH) {
					break;  // Abort to prevent infinite loops
				}
			}
			if (hNew > MAX_H) {
				hNew = MAX_H;
				if (reachedMaxH) {
					break;  // Abort to prevent infinite loops
				}
			}
			reachedMinH = hNew == MIN_H;
			reachedMaxH = hNew == MAX_H;

			// If the error is smaller than 1.0 the operation was successful,
			// abort
			if (e < 1.0) {
				break;
			}

			// Use the new h in the next iteration
			h = hNew;
		}

		// Copy current stepsize
		hOld = hNew;

		// Return the solution and the time h actually used time h.
		return std::pair<State, Time>(res.first, Time::sec(h));
	}
};

/**
 * The DormandPrinceIntegrator class implements the fifth-order embedded
 * Runge-Kutta method with step-size control. This allows the integrator to
 * skip over regions in which nothing happens.
 */
class DormandPrinceIntegrator
    : public AdaptiveIntegratorBase<DormandPrinceIntegrator> {
public:
	using Base = AdaptiveIntegratorBase<DormandPrinceIntegrator>;
	friend Base;

private:
	/**
	 * Implements the fourth-order Runge-Kutta method.
	 *
	 * @param h is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep.
	 */
	template <typename Deriv>
	std::pair<State, State> doIntegrate(Val h, const State &s, Deriv df) const
	{
		return DormandPrinceInternal::RungeKutta5(h, s, df);
	}

public:
	using Base::Base;
};
}

#endif /* _ADEXPSIM_ADAPTIVE_STEPSIZE_RUNGE_KUTTA_HPP_ */

