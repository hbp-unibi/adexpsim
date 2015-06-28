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

#include <simulation/HardwareParameters.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <exploration/SingleGroupEvaluation.hpp>
#include <exploration/Optimization.hpp>
#include <common/Terminal.hpp>
#include <io/JsonIo.hpp>

#include <csignal>
#include <limits>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

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

int main(int argc, char *argv[])
{
	signal(SIGINT, int_handler);

	// Create the initial parameters and the evaluation
	WorkingParameters params;
	ModelType model = ModelType::IF_COND_EXP;
	SingleGroupSpikeData data;  // Default SingleGroupSpikeData (n = 3)
	SingleGroupEvaluation eval(data, model == ModelType::IF_COND_EXP);

	// Parameter dimensions to optimize (exclude the inhibitory channel
	// parameters)
	std::vector<size_t> dims({0, 1, 3, 4, 6, 7, 8, 9, 10, 11, 12});

	Optimization opt(model, EvaluationResultDimension::SOFT, dims,
	                 BrainScaleSParameters::inst);
	std::vector<OptimizationResult> res =
	    opt.optimize({params}, eval,
	                 [](size_t nIt, size_t nInput,
	                    const std::vector<OptimizationResult> &output) -> bool {
		    std::cout << "nIt: " << nIt << " nInput: " << nInput
		              << " nOutput: " << output.size();
		    if (!output.empty()) {
			    std::cout << " eval: " << output.back().eval;
		    }
		    std::cout <<  "        \r";
		    return !cancel;
		});

	std::cout << std::endl;
	if (!res.empty()) {
		std::cout << "Final pSoft: " << res.back().eval << std::endl;
		JsonIo::storePyNNModel(
		    std::cout, res.back().params.toParameters(DefaultParameters::cM,
		                                          DefaultParameters::eL),
		    model);
	}

	return 0;
}

