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
 * @file Optimization.hpp
 *
 * Contains a threaded implementation of the Parameter optimization
 * implementation. Uses a simplex from multiple starting points. Can respect
 * the hardware parameter boundaries.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_OPTIMIZATION_HPP_
#define _ADEXPSIM_OPTIMIZATION_HPP_

#include <vector>

#include <exploration/Optimization.hpp>
#include <simulation/Parameters.hpp>

namespace AdExpSim {
/**
 * Contains a single result returned by the optimizer.
 */
struct OptimizationResult {
	/**
	 * The actual parameter vector returned by the optimizer.
	 */
	WorkingParameters params:

	/**
	 * Evaluation result
	 */
	EvaluationResult eval;
};

class Optimization {

public:
	std

};

}

#endif /* _ADEXPSIM_OPTIMIZATION_HPP_ */

