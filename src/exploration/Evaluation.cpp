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

#include <cmath>
#include <limits>

#include <core/Model.hpp>
#include <core/Recorder.hpp>

#include "Evaluation.hpp"

namespace AdExpSim {

Evaluation::Evaluation(Val xi, Time T)
    : xi(xi),
      T(T),
      sXi(buildInputSpikes(xi, T)),
      sXiM1(buildInputSpikes(xi - 1, T))
{
}

Val Evaluation::cost(Val vMaxXi, Val vMaxXiM1, Val eSpikeEff, Val sigma)
{
	return exp(sigma * (eSpikeEff - vMaxXi)) +
	       exp(sigma * (vMaxXiM1 - eSpikeEff));
}

std::tuple<Val, Val, bool, Val, Val> Evaluation::evaluate(
    const WorkingParameters &params, Val sigma, Val tDelta)
{
	// Make sure the parameters are inside the valid range, otherwise abort
	if (!params.valid()) {
		return std::tuple<Val, Val, bool, Val, Val>(
		    std::numeric_limits<Val>::max(), std::numeric_limits<Val>::max(),
		    false, 0, 0);
	}

	// Make sure all derived parameters have been calculated correctly
	params.update();

	// Do not record any result
	NullRecorder n;

	// Use max value controller to track the maximum value
	MaxValueController cXi, cXiM1;

	// Simulate for both the sXi and the sXiM1 input spike train
	Model::simulate<SimulationFlags>(sXi, n, cXi, params, tDelta);
	Model::simulate<SimulationFlags>(sXiM1, n, cXiM1, params, tDelta);

	// Return cost, time to spike and whether the parameters generally fulfill
	// the condition
	return std::tuple<Val, Val, bool, Val, Val>(
	    cost(cXi.vMax, cXiM1.vMax, params.eSpikeEff(), sigma),
	    std::min(cXi.tSpike, cXi.tVMax).toSeconds(),
	    cXi.vMax > params.eSpikeEff() && cXiM1.vMax < params.eSpikeEff(),
	    cXi.vMax - params.eSpikeEff(), cXiM1.vMax - params.eSpikeEff());
}
}

