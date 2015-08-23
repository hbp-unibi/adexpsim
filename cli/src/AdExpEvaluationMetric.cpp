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

#include <exploration/SingleGroupMultiOutEvaluation.hpp>
#include <simulation/Spike.hpp>

#include <iostream>

using namespace AdExpSim;

int main(int argc, char *argv[])
{
	// Use the default parameters
	Parameters params;

	// Setup the input spike train
	SingleGroupMultiOutSpikeData spikeData(5, 1, 5e-3_s, 100e-3_s);

	bool useIfCondExp = false;

	SingleGroupMultiOutEvaluation eval(spikeData, useIfCondExp, 0.1e-3);

	std::cerr << "AdExpEvaluationMetric: Performing sweep..." << std::endl;

	// Vary the eTh parameter and record the results
	for (Val eTh = params.eL() + 2.1e-3; eTh < 0.0; eTh += 0.01e-3) {
	    params.eTh() = eTh;
	    const WorkingParameters p(params);
	    if (p.valid()) {
	        auto res = eval.spikeCount(eval.getSpikesN(), p);
	        Val th = p.eSpikeEff(useIfCondExp);
			std::cout << params.eTh() << "\t" << res.spikeCount << "\t"
			          << res.eDelta << "\t" << res.eDeltaNorm << "\t" << th << std::endl;
	    }
	}
/*	for (Val tauE = 1e-3; tauE < 100e-3; tauE += 0.01e-3) {
		params.tauE() = tauE;
		const WorkingParameters p(params);
		if (p.valid()) {
			auto res = eval.spikeCount(eval.getSpikesN(), p);
			Val th = p.eSpikeEff(useIfCondExp);
			std::cout << params.tauE() << "\t" << res.spikeCount << "\t"
			          << res.eDelta << "\t" << res.eDeltaNorm << "\t" << th << std::endl;
		}
	}*/
/*	params.w() = 9e-8;
	for (Val gL = 1e-9; gL < 1e-6; gL += 1e-9) {
		params.gL() = gL;
		const WorkingParameters p(params);
		if (p.valid()) {
			auto res = eval.spikeCount(eval.getSpikesN(), p);
			Val th = p.eSpikeEff(useIfCondExp);
			std::cout << params.gL() << "\t" << res.spikeCount << "\t"
			          << res.eDelta << "\t" << res.eDeltaNorm << "\t" << th << std::endl;
		}
	}*/
/*	params.w() = 1e-6;
	for (Val tauRef = 0; tauRef < 2e-3; tauRef += 0.001e-3) {
		params.tauRef() = tauRef;
		const WorkingParameters p(params);
		if (p.valid()) {
			auto res = eval.spikeCount(eval.getSpikesN(), p);
			Val th = p.eSpikeEff(useIfCondExp);
			std::cout << params.tauRef() << "\t" << res.spikeCount << "\t"
			          << res.eDelta << "\t" << res.eDeltaNorm << "\t" << th << std::endl;
		}
	}*/


	std::cerr << "Done!" << std::endl;
	return 0;
}

