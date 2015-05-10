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
#include "exploration/Evaluation.hpp"
#include "utils/Terminal.hpp"

#include <limits>
#include <iostream>
#include <fstream>

using namespace AdExpSim;

int main(int argc, char *argv[])
{
	Terminal term(true);

	std::ofstream fTime("exploration_time.csv");
	std::ofstream fCost("exploration_cost.csv");
	std::ofstream fOk("exploration_ok.csv");
	std::ofstream fvMax("exploration_vMax.csv");
	std::ofstream fvMaxM1("exploration_vMaxM1.csv");
	std::ofstream fvExtr("exploration_vExtr.csv");
	std::ofstream fvExtrM1("exploration_vExtrM1.csv");

	WorkingParameters params;

	Evaluation evaluation(3, 1e-3);

	float wOrig = params.wSpike();
	size_t nY = 0;
	params.wSpike() = wOrig * 0.04e-6;
	for (Val w = 0; w < 0.1e-6; w += 0.0001e-6) {
		//	for (Val tauL = 0.1e-3; tauL < 10e-3; tauL += 0.1e-3) {
		// for (Val eTh = 0.0; eTh < 0.05; eTh += 0.0001) {
		bool first = true;
		size_t cOk = 0;
		size_t cTotal = 0;
		Val minCost = std::numeric_limits<Val>::max();
		//		for (Val eTh = 0.01; eTh < 0.05; eTh += 0.0001) {
		// for (Val deltaTh = 0.0; deltaTh < 10e-3; deltaTh += 0.01e-3) {
		//		for (Val tauE = 1e-3; tauE < 50e-3; tauE += 0.1e-3) {
		for (Val tauL = 0.1e-3; tauL < 20e-3; tauL += 0.1e-3) {
			//		for (Val eE = 0.0; eE < 0.4; eE += 0.0002) {
			// Set the new parameters
			params.wSpike() = wOrig * w;
			// params.deltaTh() = deltaTh;
			params.lL() = 1.0 / tauL;
			//			params.lE() = 1.0 / tauE;
			//			params.eE() = eE;
			// params.eTh() = eTh;
			params.update();

			// Call the evaluation function
			if (!first) {
				fTime << ",";
				fCost << ",";
				fOk << ",";
				fvMax << ",";
				fvMaxM1 << ",";
				fvExtr << ",";
				fvExtrM1 << ",";
			}
			auto res = evaluation.evaluate(params);
			fCost << std::get<0>(res);
			fTime << std::get<1>(res);
			fOk << std::get<2>(res);
			fvMax << std::get<3>(res);
			fvMaxM1 << std::get<4>(res);
			fvExtr << params.calculateEExtr(evaluation.getXi() *
			                                params.wSpike()) -
			              params.eSpikeEff();
			fvExtrM1 << params.calculateEExtr((evaluation.getXi() - 1) *
			                                  params.wSpike()) -
			                params.eSpikeEff();
			minCost = std::min(minCost, std::get<0>(res));
			first = false;
			cOk = cOk + std::get<2>(res);
			cTotal++;
		}
		fCost << std::endl;
		fTime << std::endl;
		fOk << std::endl;
		fvMax << std::endl;
		fvMaxM1 << std::endl;
		fvExtr << std::endl;
		fvExtrM1 << std::endl;

		// Print some fancy progress bar
		if (nY == 0) {
			std::cerr << std::endl << " - ";
		}
		nY = (nY + 1) % 100;
		Val f1 = Val(cOk) / Val(cTotal);
		Val f2 = std::max(0.0, (1.0 - std::min(1.0, 0.2 * minCost)) - f1);
		std::cerr << term.rgb(255 * (1 - f1),
		                      255 * std::min(1.0, (20.0 * f1 + 0.75 * f2)),
		                      64 * f2, true) << term.rgb(0, 0, 0, false)
		          << (cOk > 0 ? "-" : " ") << term.reset();
	}
	std::cerr << std::endl;
}

