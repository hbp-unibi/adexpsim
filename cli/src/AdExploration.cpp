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

#include <exploration/Exploration.hpp>
#include <exploration/SpikeTrainEvaluation.hpp>
#include <exploration/SingleGroupSingleOutEvaluation.hpp>
#include <exploration/SingleGroupMultiOutEvaluation.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <io/SurfacePlotIo.hpp>
#include <utils/ParameterCollection.hpp>
#include <common/Timer.hpp>

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
	std::cerr << std::setw(8) << std::setprecision(4) << perc << "% [";
	for (int i = 0; i < WIDTH; i++) {
		bool cur = i * 100 / WIDTH < perc;
		bool prev = (i - 1) * 100 / WIDTH < perc;
		if (cur && prev) {
			std::cerr << "=";
		} else if (prev) {
			std::cerr << ">";
		} else {
			std::cerr << " ";
		}
	}
	std::cerr << "]   \r";
	return !cancel;
}

bool runExploration(const std::string &prefix, const SpikeTrainEnvironment &env,
                    const Parameters &params,
                    const SingleGroupMultiOutDescriptor &singleGroup,
                    ModelType model, EvaluationType evaluation,
                    const DiscreteRange &rangeX, const DiscreteRange &rangeY,
                    size_t dimX, size_t dimY, size_t spikeTrainN)
{
	// Run the exploration, write the result matrices to a file
	std::cout << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "Running exploration" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << std::endl;

	std::cout << "X-Axis: " << Parameters::names[dimX] << " from " << rangeX.min
	          << " to " << rangeX.max << " in " << rangeX.steps << " steps"
	          << std::endl;
	std::cout << "Y-Axis: " << Parameters::names[dimY] << " from " << rangeY.min
	          << " to " << rangeY.max << " in " << rangeY.steps << " steps"
	          << std::endl;
	std::cout << std::endl;

	std::cout << "Group descriptor:" << std::endl;
	std::cout << "nIn1: " << singleGroup.n << std::endl;
	std::cout << "nIn0: " << singleGroup.nM1 << std::endl;
	std::cout << "nOut: " << singleGroup.nOut << std::endl;
	std::cout << std::endl;

	std::cout << "Environment:" << std::endl;
	std::cout << "burstSize: " << env.bundleSize << std::endl;
	std::cout << "T: " << env.T << std::endl;
	std::cout << "sigmaT: " << env.sigmaT << std::endl;
	std::cout << "deltaT: " << env.deltaT << std::endl;
	std::cout << "sigmaW: " << env.sigmaW << std::endl;
	std::cout << std::endl;

	std::cout << "Model: " << ParameterCollection::modelNames[size_t(model)]
	          << std::endl;
	std::cout << "Evaluation: "
	          << ParameterCollection::evaluationNames[size_t(evaluation)]
	          << std::endl;

	const bool useIfCondExp = (model == ModelType::IF_COND_EXP);

	bool ok = false;
	Exploration exploration(true, params, dimX, dimY, rangeX, rangeY);
	Timer timer;
	switch (evaluation) {
		case EvaluationType::SPIKE_TRAIN: {
			SpikeTrain train(singleGroup, spikeTrainN, env, false);
			ok = exploration.run(SpikeTrainEvaluation(train, useIfCondExp),
			                     showProgress);
			break;
		}
		case EvaluationType::SINGLE_GROUP_SINGLE_OUT: {
			ok = exploration.run(
			    SingleGroupSingleOutEvaluation(env, singleGroup, useIfCondExp),
			    showProgress);
			break;
		}
		case EvaluationType::SINGLE_GROUP_MULTI_OUT: {
			ok = exploration.run(
			    SingleGroupMultiOutEvaluation(env, singleGroup, useIfCondExp),
			    showProgress);
			break;
		}
	}
	timer.pause();
	std::cout << std::endl;
	std::cout << "Done." << std::endl;
	std::cout << timer << std::endl;

	// Dump the results
	if (ok && !cancel) {
		const EvaluationResultDescriptor &descr = exploration.descriptor();
		for (size_t i = 0; i < descr.size(); i++) {
			const std::string filename =
			    prefix + "_X" + Parameters::nameIds[dimX] + "_Y" +
			    Parameters::nameIds[dimY] + "_" + descr.id(i) + "_" +
			    ParameterCollection::evaluationNames[size_t(evaluation)] + "_" +
			    ParameterCollection::modelNames[size_t(model)] + ".csv";
			std::cout << "Writing layer " << descr.id(i) << " to " << filename
			          << std::endl;
			std::ofstream os(filename);
			SurfacePlotIo::storeSurfacePlot(os, exploration, i, false);
		}
		return true;
	}
	return false;
}

bool runExplorations(const std::string &prefix,
                     const SpikeTrainEnvironment &env, const Parameters &params,
                     const SingleGroupMultiOutDescriptor &singleGroup,
                     const DiscreteRange &rangeX, const DiscreteRange &rangeY,
                     size_t dimX, size_t dimY, size_t spikeTrainN = 100)
{
	for (size_t model = 0; model < 1; model++) {
		for (size_t evaluation = 0; evaluation < 1; evaluation++) {
			if (evaluation == 1 && singleGroup.nOut > 1) {
				continue;
			}
			if (!runExploration(prefix, env, params, singleGroup,
			                    ModelType(model), EvaluationType(evaluation),
			                    rangeX, rangeY, dimX, dimY, spikeTrainN)) {
				return false;
			}
		}
	}
	return true;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, int_handler);

	// Setup the parameters, set an initial value for w
	Parameters params;

	// Setup the exploration
	const size_t resolution = 2048;

	// Scenario 1: single output spike
	Parameters paramsSc1;  // Just use the default parameters for this scenario
	runExplorations("ex_sc1", SpikeTrainEnvironment(1, 200_ms, 5_ms, 2_ms),
	                paramsSc1, SingleGroupMultiOutDescriptor(3, 2, 1),
	                DiscreteRange(0.01e-6, 0.6e-6, resolution),
	                DiscreteRange(1e-3, 100e-3, resolution), Parameters::idx_gL,
	                Parameters::idx_tauE);
	runExplorations("ex_sc1", SpikeTrainEnvironment(1, 200_ms, 5_ms, 2_ms),
	                paramsSc1, SingleGroupMultiOutDescriptor(3, 2, 1),
	                DiscreteRange(DefaultParameters::eL + 2.1 / 1000.0,
	                              DefaultParameters::eE, resolution),
	                DiscreteRange(0.0e-6, 1.0e-6, resolution),
	                Parameters::idx_eTh, Parameters::idx_w);

	// Scenario 2: Burst
	//	Parameters paramsSc2; // Use some better starting parameters
	//	paramsSc2.gL() = 0.525e-6;
	//	paramsSc2.tauE() = 25.0e-3;
	//		runExplorations("ex_sc2", SpikeTrainEnvironment(3, 200_ms, 5_ms,
	//2_ms), paramsSc2,
	//		                SingleGroupMultiOutDescriptor(3, 2, 1),
	//		                DiscreteRange(0.01e-6, 0.6e-6, resolution),
	//		                DiscreteRange(1e-3, 100e-3, resolution),
	//		                Parameters::idx_gL, Parameters::idx_tauE);
	//	runExplorations("ex_sc2", SpikeTrainEnvironment(3, 200_ms, 5_ms, 2_ms),
	//paramsSc2,
	//	                SingleGroupMultiOutDescriptor(3, 2, 1),
	//	                DiscreteRange(DefaultParameters::eL + 2.1 / 1000.0,
	//	                              DefaultParameters::eE, resolution),
	//	                DiscreteRange(0.0e-6, 1.0e-6, resolution),
	//	                Parameters::idx_eTh, Parameters::idx_w);

	if (cancel) {
		std::cout << "Manually aborted exploration" << std::endl;
	}
}

