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

#define BENCHMARK 0

using namespace AdExpSim;

int main(int argc, char *argv[])
{
	// Use the default parameters
	Parameters params;

// Create the recorder
#if BENCHMARK
	NullRecorder recorder;
#else
	CsvRecorder<> recorder(params, 0.1e-3, std::cout);
#endif

	// Create a vector containing all input spikes
	SpikeVec spikes = scaleSpikes(
	    {{1e-3, 0.03e-6}, {2e-3, 0.03e-6}, {3e-3, 0.03e-6}}, params);

	std::cerr << "Max. iTh exponent: "
	          << WorkingParameters(params).maxIThExponent << std::endl;

#if BENCHMARK
	Timer t;
	for (int i = 0; i < 1000; i++) {
#endif
		Model::simulate(20e-3, spikes, recorder, 0.0, params);
#if BENCHMARK
	}
	std::cout << t;
#endif
	std::cerr << "Max. membrane potential: " << maxV + params.eL << std::endl;
	return 0;
}

