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
 * @file AdaptiveStepsizeRungeKutta.hpp
 *
 * Implementation of the fifth-order embedded Runge-Kutta method with adaptive
 * stepsize control. Note that this implementation is for autonomous
 * differential equations (df does not depend on t!). Based on the algorithm
 * presented in Numerical Recipes chapter 17.2.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_ADAPTIVE_STEPSIZE_RUNGE_KUTTA_HPP_
#define _ADEXPSIM_ADAPTIVE_STEPSIZE_RUNGE_KUTTA_HPP_

#include <utility>

#include <utils/Types.hpp>

namespace AdExpSim {

namespace AdaptiveStepsizeRungeKuttaInternal {

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
 * Returns the Fifth-order Runge-Kutta coefficients. This function is likely
 * to be evaluated at compile time.
 */
constexpr Val a(size_t i, size_t j)
{
	return COEFF_A[i - 1][j - 1];
}

/**
 * Coefficients used to estimate the error vector (b_i - b_i^* in the
 * afforementioned Table in NR).
 */
static constexpr double COEFF_E[7] = {
    71.0 / 576000.0,     -71.0 / 16695.0, 0,          71.0 / 1920.0,
    -17253.0 / 339200.0, 22.0 / 525.0,    -1.0 / 40.0};

/**
 * Returns the error-vector coefficients with range check (evaluated at
 * compile-time).
 */
constexpr Val e(size_t i)
{
	return COEFF_E[i - 1];
}

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
static inline K RungeKuttaEval(Coeffs c, size_t i, const K &k,
                                    const KS &... ks)
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
}

/**
 * Implements the fifth-order embedded RungeKutta method. Returns the value of
 * the function
 */
template <typename Vector, typename Deriv>
static std::pair<Vector, Vector> RungeKutta5(Val h, const Vector &y, Deriv df)
{
	// Import the coefficient namespace (supplies the c, a, b)
	using namespace AdaptiveStepsizeRungeKuttaInternal;

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

#endif /* _ADEXPSIM_ADAPTIVE_STEPSIZE_RUNGE_KUTTA_HPP_ */

