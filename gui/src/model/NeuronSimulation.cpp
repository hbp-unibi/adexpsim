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

#include <simulation/DormandPrinceIntegrator.hpp>

#include "NeuronSimulation.hpp"

namespace AdExpSim {

void NeuronSimulation::run(std::shared_ptr<ParameterCollection> sharedParams)
{
	// Copy the parameters from the global parameters instance
	params = *sharedParams;
	evaluation = SpikeTrainEvaluation(
	    params.train, params.model == ModelType::IF_COND_EXP);

	// Reset all output
	mValid = false;
	vectorRecorder.reset();
	maximumRecorder.reset();
	outputSpikes.clear();
	outputGroups.clear();

	// Abort if the current parameters are not valid
	WorkingParameters wp(params.params);
	if (!wp.valid()) {
		return;
	}

	// Create a new controller -- make sure to abort after a certain count of
	// output spikes is superceeded.
	auto controller = createMaxOutputSpikeCountController<false>(
	    [this]() -> size_t {
		    return vectorRecorder.getData().outputSpikeTimes.size();
		},
	    getTrain().getExpectedOutputSpikeCount() * 20);

	// Use a DormandPrinceIntegrator with default parameters
	DormandPrinceIntegrator integrator(0.1e-3);
//	RungeKuttaIntegrator integrator;
//	const Time deltaT = Time(-1);
	const Time deltaT = 0.1e-3_s;

	// Run the actual simulation until the end of the time
	auto multiRecorder = makeMultiRecorder(vectorRecorder, maximumRecorder);
	if (params.model == ModelType::IF_COND_EXP) {
		Model::simulate<Model::IF_COND_EXP>(getTrain().getSpikes(), multiRecorder,
		                                    controller, integrator, wp,
		                                    deltaT, getTrain().getMaxT());
	} else {
		Model::simulate<Model::FAST_EXP>(getTrain().getSpikes(), multiRecorder,
		                                 controller, integrator, wp, deltaT,
		                                 getTrain().getMaxT());
	}

	// Run the evaluation to fetch the output spikes and the output groups
	evaluation.evaluate(wp, outputSpikes, outputGroups);

	// The result is not valid, if the controller has triggered
	mValid = !controller.tripped();
}
}

