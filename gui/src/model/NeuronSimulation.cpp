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

void NeuronSimulation::prepare(const Parameters &parameters,
                               const SpikeTrain &train)
{
	this->parameters = parameters;
	this->evaluation = SpikeTrainEvaluation(train);
}

void NeuronSimulation::run()
{
	// Reset the recorder and first run the simulation
	recorder.reset();
	NullController controller;
	DormandPrinceIntegrator integrator;

	// Run the actual simulation until the end of the time
	Model::simulate(getTrain().getSpikes(), recorder, controller, integrator,
	                parameters, Time(-1), getTrain().getMaxT());

	// Run the evaluation to fetch the output spikes and the output groups
	outputSpikes.clear();
	outputGroups.clear();
	evaluation.evaluate(parameters, outputSpikes, outputGroups);
}
}

