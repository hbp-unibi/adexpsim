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

#include <limits>

#include <common/ProbabilityUtils.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>

#include "SingleGroupMultiOutEvaluation.hpp"

namespace AdExpSim {

/* Class SingleGroupMultiOutEvaluation */

EvaluationResult SingleGroupMultiOutEvaluation::evaluate(
    const WorkingParameters &params, Val eTar) const
{
	// Make sure the parameters are inside the valid range, otherwise abort
	if (!params.valid()) {
		return EvaluationResult();
	}

	// Make sure all derived parameters have been calculated correctly
	params.update();

	// TODO
	return EvaluationResult();
}
}

