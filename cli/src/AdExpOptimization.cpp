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
#include <exploration/SpikeTrainEvaluation.hpp>
#include <exploration/SingleGroupSingleOutEvaluation.hpp>
#include <exploration/SingleGroupMultiOutEvaluation.hpp>
#include <exploration/Optimization.hpp>
#include <common/Timer.hpp>
#include <utils/ParameterCollection.hpp>

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

static WorkingParameters run_optimisation(
    const std::vector<size_t> &dims, const WorkingParameters &params,
    const SpikeTrainEnvironment &env,
    const SingleGroupMultiOutDescriptor &group,
    const EvaluationType evaluationType,
    const ModelType modelType = ModelType::IF_COND_EXP)
{
	// Prepare the input vector
	std::vector<WorkingParameters> input{params};

	// Progress callback, print the number of iterations and the current
	// result on std::cerr
	auto progressCallback =
	    [&](size_t nIt, size_t nInput, float eval,
	          const std::vector<OptimizationResult> &output)->bool
	{
		std::cerr << "nIt: " << nIt << " nInput: " << nInput
		          << " nOutput: " << output.size();
		std::cerr << " eval: " << eval;
		std::cerr << "        \r";
		return !cancel;
	};

	// Prepare the evaluation measures
	SpikeTrain train(group, 100, env, false);
	const bool useIfCondExp = modelType == ModelType::IF_COND_EXP;
	SpikeTrainEvaluation st100(train, useIfCondExp);
	SingleGroupSingleOutEvaluation sgso(env, group, useIfCondExp);
	SingleGroupMultiOutEvaluation sgmo(env, group, useIfCondExp);

	// Optimisation dimension
	Optimization optimization(modelType, dims);

	std::vector<OptimizationResult> res;

	std::cout << "Starting evaluation..." << std::endl;
	Timer timer;
	switch (evaluationType) {
		case EvaluationType::SPIKE_TRAIN: {
			res = optimization.optimize(input, st100, progressCallback);
			break;
		}
		case EvaluationType::SINGLE_GROUP_SINGLE_OUT: {
			res = optimization.optimize(input, sgso, progressCallback);
			break;
		}
		case EvaluationType::SINGLE_GROUP_MULTI_OUT: {
			res = optimization.optimize(input, sgmo, progressCallback);
			break;
		}
	}
	timer.pause();
	std::cerr << std::endl;
	std::cout << "Done." << std::endl;
	std::cout << timer << std::endl;

	WorkingParameters wpOut(params);
	if (res.empty() || !res[0].params.valid()) {
		std::cout << "WARNING, EMPTY RESULTS!!!!!!" << std::endl;
	} else {
		wpOut = res[0].params;
	}

	std::cout << "Final parameters: " << std::endl;
	Parameters pOpt =
	    params.toParameters(DefaultParameters::cM, DefaultParameters::eL);
	for (size_t i = 0; i < pOpt.size(); i++) {
		std::cout << Parameters::names[i] << ": " << pOpt[i] << std::endl;
	}

	std::cout << "Results for ST100: "
	          << st100.evaluate(wpOut)[st100.descriptor().optimizationDim()]
	          << std::endl;
	if (env.burstSize == 1 && group.nOut == 1) {
		std::cout << "Results for SGSO: "
		          << sgso.evaluate(wpOut)[sgso.descriptor().optimizationDim()]
		          << std::endl;
	}
	std::cout << "Results for SGMO: "
	          << sgmo.evaluate(wpOut)[sgmo.descriptor().optimizationDim()]
	          << std::endl;

	return params;
}

static void optimise_scenario(const SpikeTrainEnvironment &env,
                              const SingleGroupMultiOutDescriptor &group)
{
	std::cout << "Base neuron parameters:" << std::endl;
	Parameters params;
	for (size_t i = 0; i < params.size(); i++) {
		std::cout << Parameters::names[i] << ": " << params[i] << std::endl;
	}
	std::cout << std::endl;

	std::vector<size_t> dims{
	    WorkingParameters::idx_lL, WorkingParameters::idx_lE,
	    WorkingParameters::idx_eTh, WorkingParameters::idx_w};

	std::cout << "Optimising the following dimensions:" << std::endl;
	for (size_t dim : dims) {
		std::cout << WorkingParameters::names[dim] << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Group descriptor:" << std::endl;
	std::cout << "nIn1: " << group.n << std::endl;
	std::cout << "nIn0: " << group.nM1 << std::endl;
	std::cout << "nOut: " << group.nOut << std::endl;
	std::cout << std::endl;

	std::cout << "Environment:" << std::endl;
	std::cout << "burstSize: " << env.burstSize << std::endl;
	std::cout << "T: " << env.T << std::endl;
	std::cout << "sigmaT: " << env.sigmaT << std::endl;
	std::cout << "deltaT: " << env.deltaT << std::endl;
	std::cout << "sigmaW: " << env.sigmaW << std::endl;
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << "=====================" << std::endl;
	std::cout << "Optimising with ST100" << std::endl;
	std::cout << "=====================" << std::endl;
	std::cout << std::endl;

	run_optimisation(dims, params, env, group, EvaluationType::SPIKE_TRAIN);

	if (env.burstSize == 1 && group.nOut == 1) {
		std::cout << std::endl;
		std::cout << "====================" << std::endl;
		std::cout << "Optimising with SGSO" << std::endl;
		std::cout << "====================" << std::endl;
		std::cout << std::endl;
		run_optimisation(dims, params, env, group,
		                 EvaluationType::SINGLE_GROUP_MULTI_OUT);
	}

	std::cout << std::endl;
	std::cout << "====================" << std::endl;
	std::cout << "Optimising with SGMO" << std::endl;
	std::cout << "====================" << std::endl;
	std::cout << std::endl;

	run_optimisation(dims, params, env, group,
	                 EvaluationType::SINGLE_GROUP_MULTI_OUT);
	std::cout << std::endl;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, int_handler);

	// Run the exploration, write the result matrices to a file
	std::cout << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "SCENARIO I" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << std::endl;

	optimise_scenario(SpikeTrainEnvironment(1, 200_ms, 5_ms, 10_ms),
	                  SingleGroupMultiOutDescriptor(3, 2, 1));

	// Run the exploration, write the result matrices to a file
	std::cout << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "SCENARIO II" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << std::endl;

	optimise_scenario(SpikeTrainEnvironment(3, 200_ms, 5_ms, 10_ms),
	                  SingleGroupMultiOutDescriptor(3, 2, 1));

	// Run the exploration, write the result matrices to a file
	std::cout << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "SCENARIO III" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << std::endl;

	optimise_scenario(SpikeTrainEnvironment(3, 200_ms, 5_ms, 10_ms),
	                  SingleGroupMultiOutDescriptor(9, 6, 1));

	return 0;
}

