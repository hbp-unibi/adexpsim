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

#include "core/Model.hpp"
#include "core/Recorder.hpp"
#include "utils/Timer.hpp"

#include <iostream>

//#define BENCHMARK

using namespace AdExpSim;

int main(int argc, char *argv[])
{
	// Use the default parameters
	Parameters params;
	MaxValueController controller;

// Create the recorder
#ifdef BENCHMARK
	NullRecorder recorder;
#else
	CsvRecorder<> recorder(params, 0.1e-3, std::cout);
#endif

	// Create a vector containing all input spikes
	SpikeVec spikes = scaleSpikes(
	    {{1e-3, 0.03e-6}, {2e-3, 0.03e-6}, {3e-3, 0.03e-6}}, params);

	WorkingParameters wParams(params);
	std::cerr << "Max. iTh exponent: " << wParams.maxIThExponent << std::endl;
	std::cerr << "Effective spike potential: " << wParams.eSpikeEff + params.eL
	          << std::endl;

#ifdef BENCHMARK
	Timer t;
	for (int i = 0; i < 1000; i++) {
#endif
		Model::simulate<Model::CLAMP_ITH | Model::FAST_EXP>(
		    spikes, recorder, controller, 0.0, wParams, 0.01e-3);
#ifdef BENCHMARK
	}
	std::cout << t;
#endif
	std::cerr << "Max. membrane potential: " << controller.maxV + params.eL
	          << std::endl;
	return 0;
}

