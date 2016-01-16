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

#include <exploration/FractionalSpikeCount.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/SpikeTrain.hpp>

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

static void sweep(bool useIfCondExp, size_t dim, Val min, Val max, Val step,
                  Val w = DefaultParameters::w)
{
	std::cerr << "AdExpEvaluationMetric: Performing sweep on parameter "
	          << Parameters::names[dim] << " ("
	          << (useIfCondExp ? "lif" : "adex") << ")" << std::endl;

	SpikeVec train = buildInputSpikes(5, 5e-3_s);
	FractionalSpikeCount eval(useIfCondExp);

	std::ofstream of("sweep_" + Parameters::nameIds[dim] +
	                 (useIfCondExp ? "_lif" : "_adex") + ".csv");

	Parameters params;
	params[Parameters::idx_w] = w;
	for (Val x = min; x < max; x += step) {
		progress(x, min, max, step);
		params[dim] = x;
		const WorkingParameters wp(params);
		if (wp.valid()) {
			Val th = wp.eSpikeEff(useIfCondExp);
			auto res = eval.calculate(train, wp);
			of << x << "\t" << res.fracSpikeCount() << "\t" << res.spikeCount
			   << "\t" << res.eReq << "\t" << res.pReq << "\t" << res.eMax
			   << "\t" << res.pMax << "\t" << th << std::endl;
		}
	}

	std::cerr << std::endl << "Done." << std::endl;
}

int main()
{
//	sweep(false, Parameters::idx_gL, 10e-9, 1000e-9, 0.1e-9, 9e-8);
//	sweep(true, Parameters::idx_gL, 10e-9, 1000e-9, 0.1e-9, 9e-8);
//	sweep(false, Parameters::idx_eTh, DefaultParameters::eL + 2.1e-3, -0.03,
//	      0.1e-3);
//	sweep(true, Parameters::idx_eTh, DefaultParameters::eL + 2.1e-3, -0.03,
//	      0.1e-3);
	sweep(false, Parameters::idx_tauRef, 0, 4e-3, 10e-6, 0.25e-6);
	sweep(true, Parameters::idx_tauRef, 0, 4e-3, 10e-6, 0.25e-6);
	return 0;
}

