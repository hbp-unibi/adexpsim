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
#include <simulation/Parameters.hpp>
#include <simulation/SpikeTrain.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace AdExpSim;

static void progress(Val x, Val min, Val max, Val step)
{
	const int w = 50;
	const Val p = std::min(
	    100.0f, std::max(.0f, (100.0f * (x - min) / (max - step - min))));
	std::cerr << std::fixed << std::setprecision(2) << std::setw(6) << p
	          << "% [";
	const int j = p * float(w) / 100.0;
	for (int i = 0; i < w; i++) {
		std::cerr << (i > j ? ' ' : (i == j ? '>' : '='));
	}
	std::cerr << "]\r";
}

static const Val gL0 = 2.00202e-07;
static const Val gL1 = 2.00602e-07;

static const SpikeVec train = buildInputSpikes(5, 5e-3_s);

static void sweep(bool useIfCondExp, size_t dim, Val min, Val max, Val step,
                  Val w = DefaultParameters::w)
{
	std::cerr << "AdExEvaluationMinApices: Performing sweep on parameter "
	          << Parameters::names[dim] << " ("
	          << (useIfCondExp ? "lif" : "adex") << ")" << std::endl;

	std::ofstream of("sweep_" + Parameters::nameIds[dim] +
	                 (useIfCondExp ? "_lif" : "_adex") + ".csv");

	Parameters params;
	params[Parameters::idx_w] = w;
	for (Val x = min; x < max; x += step) {
		progress(x, min, max, step);
		params[dim] = x;
		const WorkingParameters wp(params);
		if (wp.valid()) {
			LocalMaximumRecorder maximumRecorder;
			OutputSpikeCountRecorder spikeCountRecorder;
			auto recorder =
			    makeMultiRecorder(maximumRecorder, spikeCountRecorder);
			DefaultController controller;
			RungeKuttaIntegrator integrator;
			Model::simulate(useIfCondExp, train, recorder, controller,
			                integrator, wp, 1e-6_s);

			const Val th = wp.eSpikeEff(useIfCondExp);
			const Val pOut = 1 - (th - maximumRecorder.global().s.v()) / th;
			const Val nOut = spikeCountRecorder.count();
			const Val qOut = nOut + pOut;
			of << x << "\t" << nOut << "\t" << pOut << "\t" << qOut
			   << std::endl;
		}
	}

	std::cerr << std::endl << "Done." << std::endl;
}

static void record(bool useIfCondExp, size_t dim, Val v,
                   Val w = DefaultParameters::w)
{
	Parameters params;
	params[Parameters::idx_w] = w;
	params[dim] = v;

	std::ofstream of("record_" + Parameters::nameIds[dim] + "_" +
	                 std::to_string(v * 1000.0 * 1000.0) +
	                 (useIfCondExp ? "_lif" : "_adex") + ".csv");
	CsvRecorder<> recorder(params, Time(-1), of);
	DefaultController controller;
	RungeKuttaIntegrator integrator;
	Model::simulate(useIfCondExp, train, recorder, controller, integrator,
	                params, 1e-6_s);
}

int main()
{
//	sweep(true, Parameters::idx_gL, 10e-9, 2000e-9, 0.1e-9, 9e-8);
	record(true, Parameters::idx_gL, gL0, 9e-8);
	record(true, Parameters::idx_gL, gL1, 9e-8);
	return 0;
}

