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

#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>

#include "Evaluation.hpp"

namespace AdExpSim {

/*
 * Class EvaluationResult
 */

Val EvaluationResult::cost(Val sigma) const
{
	return exp(sigma * (eSpikeEff - eMaxXi)) +
	       exp(sigma * (eMaxXiM1 - eSpikeEff)) + 40 * tSpike;
}

bool EvaluationResult::ok() const
{
	return (eMaxXi > eSpikeEff) && (eMaxXiM1 < eSpikeEff);
}

/*
 * Class Evaluation
 */

Evaluation::Evaluation(Val xi, Time T)
    : xi(xi),
      T(T),
      sXi(buildInputSpikes(xi, T)),
      sXiM1(buildInputSpikes(xi - 1, T))
{
}

EvaluationResult Evaluation::evaluate(const WorkingParameters &params, Val tDelta) const
{
	// Make sure the parameters are inside the valid range, otherwise abort
	if (!params.valid()) {
		return EvaluationResult();
	}

	// Make sure all derived parameters have been calculated correctly
	params.update();

	// Do not record any result
	NullRecorder n;

	// Use max value controller to track the maximum value
	MaxValueController cXi, cXiM1;

	// Simulate for both the sXi and the sXiM1 input spike train
	Model::simulate<SimulationFlags>(sXi, n, cXi, params);
	Model::simulate<SimulationFlags>(sXiM1, n, cXiM1, params);

	// Return the recorded values
	return EvaluationResult(params.eSpikeEff(), cXi.vMax, cXiM1.vMax,
	                        cXi.tSpike.toSeconds() - getLastSpikeTime().toSeconds(), 0);
}
}

