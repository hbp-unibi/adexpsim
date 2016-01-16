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

/**
 * Simulates a single neuron in both the LIF and AdEx mode and records the
 * result to file.
 */

#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/SpikeTrain.hpp>

#include <fstream>
#include <iostream>

using namespace AdExpSim;

int main()
{
	// Use the default parameters
	Parameters params;
	NullController controller;
	RungeKuttaIntegrator integrator;

	params.b() = 2e-9;
	params.tauW() = 30e-3;
	params.tauRef() = 1e-3;

	// Create the recorder
	std::ofstream fLif("demo_lif.csv");
	std::ofstream fAdEx("demo_adex.csv");

	CsvRecorder<> recorderLif(params, 0_s, fLif);
	CsvRecorder<> recorderAdEx(params, 0_s, fAdEx);

	// Create a vector containing all input spikes
	Val w = 2.5;
	SpikeVec spikes = {
		Spike(5_ms, w), Spike(10_ms, w), Spike(15_ms, w),
		Spike(20_ms, w), Spike(25_ms, w), Spike(30_ms, w),
		Spike(35_ms, w), Spike(40_ms, w), Spike(45_ms, w),
		Spike(50_ms, w), Spike(55_ms, w), Spike(60_ms, w), Spike(65_ms, w),
	};

	Time tEnd = 100_ms;
	Time tDelta = 0.01_ms;

	Model::simulate<Model::IF_COND_EXP>(spikes, recorderLif, controller,
	                                    integrator, params, tDelta, tEnd);
	Model::simulate<>(spikes, recorderAdEx, controller, integrator, params,
	                  tDelta, tEnd);

	return 0;
}

