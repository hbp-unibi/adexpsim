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

#include <csignal>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>

#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <exploration/Exploration.hpp>
#include <utils/Terminal.hpp>

using namespace AdExpSim;

/**
 * SIGINT handler. Sets the global "cancel" flag to true when called once,
 * terminates the program if called twice. This allows to terminate the program,
 * even if it is not responsive (the cancel flag is not checked).
 */
static bool cancel = false;
void int_handler(int x)
{
	if (cancel) {
		exit(1);
	}
	cancel = true;
}

bool showProgress(Val progress)
{
	const int WIDTH = 50;
	float perc = progress * 100.0;
	std::cout << std::setw(8) << std::setprecision(4) << perc << "% [";
	for (int i = 0; i < WIDTH; i++) {
		bool cur = i * 100 / WIDTH < perc;
		bool prev = (i - 1) * 100 / WIDTH < perc;
		if (cur && prev) {
			std::cout << "=";
		} else if (prev) {
			std::cout << ">";
		} else {
			std::cout << " ";
		}
	}
	std::cout << "]   \r";
	return !cancel;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, int_handler);

	Terminal term(true);

	// Setup the parameters, set an initial value for w
	WorkingParameters params;
	float wOrig = params.wSpike();
	params.wSpike() = wOrig * 0.04e-6;

	// Exploration meta-parameters
	SpikeTrain train({{4, 1, 1e-3}, {3, 0, 1e-3}}, 4, true, 0.1_s);

	// Setup the exploration
	auto mem = std::make_shared<ExplorationMemory>(512, 512);
	Exploration exploration(mem, params, train,
	                        0,    // dimX: lL
	                        100,  // minX
	                        300,  // maxX
	                        1,    // dimY: lE
	                        100,  // minY
	                        300   // maxY
	                        );

	// Run the exploration, write the result matrices to a file
	std::cout << "Running exploration..." << std::endl;
	if (exploration.run(showProgress)) {
		std::cout << std::endl;
		std::cout << "Writing result to disk..." << std::endl;

		// Dump the matrices
		std::ofstream("exploration_pBinary.csv") << mem->pBinary;
		std::ofstream("exploration_pSoft.csv") << mem->pSoft;
	} else {
		std::cout << std::endl;
	}

	if (cancel) {
		std::cout << "Manually aborted exploration" << std::endl;
	}
}

