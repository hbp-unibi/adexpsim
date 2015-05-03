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
#include "exploration/Evaluation.hpp"

#include <iostream>
#include <fstream>

using namespace AdExpSim;

int main(int argc, char *argv[])
{

	std::ofstream fTime("exploration_time.csv");
	std::ofstream fCost("exploration_cost.csv");
	std::ofstream fOk("exploration_ok.csv");

	WorkingParameters params;

	Evaluation evaluation(3, 1e-3);

	float wOrig = params.wSpike();
	size_t nY = 0;
	params.wSpike() = wOrig * 0.03e-6;
//	for (Val w = 0; w < 0.1e-6; w += 0.001e-6) {
	for (Val tauL = 0.1e-3; tauL < 10e-3; tauL += 0.1e-3) {
		bool first = true;
		size_t cOk = 0;
//		for (Val eTh = 0.01; eTh < 0.05; eTh += 0.0001) {
//		for (Val deltaTh = 0.001e-3; deltaTh < 0.1e-3; deltaTh += 0.001e-3) {
		for (Val tauE = 0.1e-3; tauE < 200e-3; tauE += 1e-3) {
			// Set the new parameters
//			params.wSpike = wOrig * w;
			params.lL() = 1.0 / tauL;
			params.lE() = 1.0 / tauE;
//			params.eTh = eTh;
//			params.deltaTh = deltaTh;
			params.update();

			// Call the evaluation function
			if (!first) {
				fTime << ",";
				fCost << ",";
				fOk << ",";
			}
			auto res = evaluation.evaluate(params);
			fCost << std::get<0>(res);
			fTime << std::get<1>(res);
			fOk << std::get<2>(res);
			first = false;
			cOk = cOk + std::get<2>(res);
		}
		fCost << std::endl;
		fTime << std::endl;
		fOk << std::endl;
		if (nY == 0) {
			std::cerr << std::endl << " - ";
		}
		nY = (nY + 1) % 100;
		std::cerr << (cOk == 0 ? ' ' : (cOk < 10 ? '.' : (cOk < 100 ? ':' : '|')));
	}
	std::cerr << std::endl;
}

