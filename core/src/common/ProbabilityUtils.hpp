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
 * @file ProbabilityUtils.hpp
 *
 * Contains some helper functions for dealing with probability related stuff.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_PROBABILITY_UTILS_HPP_
#define _ADEXPSIM_PROBABILITY_UTILS_HPP_

#include <cmath>

#include "Types.hpp"

namespace AdExpSim {

/**
 * Functional implementing the logistic function with given parameters.
 *
 * @tparam Invert if true, substracts the result from 1.0.
 */
template <bool Invert = false>
struct LogisticFunction {

	static constexpr bool invert = Invert;

	const Val tau;

	/**
	 *
	 * @param tauRange specifies the steepness of the sigmoid. It defines the
	 * value for which the the logistic function should return TauRangeVal.
	 * @param tauRangeVal is the value the logistic function should return at
	 * the range boundaries. For x = -TauRange the function will return
	 * TauRangeVal, for x = TauRange the function will return 1.0 - TauRangeVal.
	 */
	constexpr LogisticFunction(Val tauRange, Val tauRangeVal)
	    : tau(log(1.0 / tauRangeVal - 1.0) / tauRange)
	{
	}

	/**
	 * Evaluates the function.
	 *
	 * @param x is the value for which the sigmoid should be evaluated.
	 * @param center is the value at which the function will return
	 * 0.5.
	 */
	Val operator()(Val x, Val center = 0.0) const
	{
		const Val res = 1.0 / (1.0 + exp(-tau * (x - center)));
		return invert ? 1.0 - res : res;
	}
};
}

#endif /* _ADEXPSIM_PROBABILITY_UTILS_HPP_ */
