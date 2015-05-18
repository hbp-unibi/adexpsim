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

#include "simulation/Model.hpp"
#include "simulation/Recorder.hpp"
#include "utils/Timer.hpp"

#include <iostream>

using namespace AdExpSim;

int main(int argc, char *argv[])
{
	// Use the default parameters
	Parameters params;
	MaxValueController controller;

	// Create the recorder
	CsvRecorder<> recorder(params, 0.1e-3, std::cout);

	// Create a vector containing all input spikes
	SpikeTrain train({{4, 1, 1e-3, 0.03175e-6}, {1, 0, 1e-3, 0.03175e-6}}, 1000,
	                 0.1, 0.01);

	WorkingParameters wParams(params);
	std::cerr << "Max. iTh exponent: " << wParams.maxIThExponent() << std::endl;
	std::cerr << "Effective spike potential: "
	          << wParams.eSpikeEff() + params.eL << std::endl;

	Model::simulate<Model::CLAMP_ITH | Model::FAST_EXP | Model::DISABLE_SPIKING,
	                AdaptiveRungeKuttaIntegrator>(train.getSpikes(), recorder,
	                                              controller, wParams);
	std::cerr << "Max. membrane potential: " << controller.vMax + params.eL
	          << std::endl;
	return 0;
}

