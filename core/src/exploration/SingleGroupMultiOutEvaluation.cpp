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
#include <vector>

#include <common/ProbabilityUtils.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>

#include "SingleGroupMultiOutEvaluation.hpp"

namespace AdExpSim {

namespace {
/**
 * Class used internally to record the number of output spikes and the state
 * after the spikes occured.
 */
class SpikeRecorder : public NullRecorder {
public:
	std::vector<std::pair<Time, State>> spikes;

	void outputSpike(Time t, const State &s) { spikes.emplace_back(t, s); }
};
}

/* Class SingleGroupMultiOutEvaluation */

Val SingleGroupMultiOutEvaluation::maximumPotential(const SpikeVec &spikes,
    const WorkingParameters &params, const State &state) const
{
	NullRecorder recorder;
	DormandPrinceIntegrator integrator(eTar);
	MaxValueController controller;

	if (useIfCondExp) {
		Model::simulate<Model::IF_COND_EXP>(spikes, recorder, controller,
		                                    integrator, params, Time(-1),
		                                    spikeData.T);
	} else {
		Model::simulate<Model::FAST_EXP>(spikes, recorder, controller, integrator,
		                                 params, Time(-1), spikeData.T);
	}

	
}

Val SingleGroupMultiOutEvaluation::fractionalNumberOfOutputSpikes(const SpikeVec &spikes,
    const WorkingParameters &params) const
{
	// First pass: Record all output spikes
	SpikeRecorder recorder;
	DormandPrinceIntegrator integrator(eTar);
	NullController controller;

	if (useIfCondExp) {
		Model::simulate<Model::IF_COND_EXP>(spikes, recorder, controller,
		                                    integrator, params, Time(-1),
		                                    spikeData.T);
	} else {
		Model::simulate<Model::FAST_EXP>(spikes, recorder, controller, integrator,
		                                 params, Time(-1), spikeData.T);
	}

	// Second pass: Record the potentially reachable potential after the last
	// the reachable potential before the last spike when spiking is disabled
}

EvaluationResult SingleGroupMultiOutEvaluation::evaluate(
    const WorkingParameters &params, Val eTar) const
{
	// TODO
	return EvaluationResult();
}
}

