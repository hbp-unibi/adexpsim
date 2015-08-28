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

#include <QThreadPool>

#include <exploration/SingleGroupSingleOutEvaluation.hpp>
#include <exploration/SingleGroupMultiOutEvaluation.hpp>
#include <exploration/SpikeTrainEvaluation.hpp>
#include <utils/ParameterCollection.hpp>
#include <simulation/HardwareParameters.hpp>

#include "OptimizationJob.hpp"

namespace AdExpSim {

/*
 * Class OptimizationJobRunner
 */

OptimizationJobRunner::OptimizationJobRunner(
    bool limitToHw, std::shared_ptr<ParameterCollection> params)
    : aborted(false), params(params)
{
	// Fetch the to-be-optimized dimensions
	const std::vector<size_t> dims = params->optimizationDims();

	// Create the actual optimization object, do not pass the hw limits if
	// the limitToHw flag is not set
	if (limitToHw) {
		optimization =
		    Optimization(params->model, dims, BrainScaleSParameters::inst);
	} else {
		optimization = Optimization(params->model, dims);
	}

	// Do not automatically free this object once it is done
	setAutoDelete(false);
}

OptimizationJobRunner::~OptimizationJobRunner()
{
	// Destructor needed here for shared pointers
}

void OptimizationJobRunner::run()
{
	// Prepare the input vector
	std::vector<WorkingParameters> input{params->params};

	// Create the progress callback -- relay the information via the progress
	// signal
	size_t it = 0;
	auto progressCallback =
	    [&](size_t nIt, size_t nInput,
	        const std::vector<OptimizationResult> &output) -> bool {
		it = nIt;
		emit progress(false, nIt, nInput, output);
		return !aborted.load();
	};

	// Optimize either using the SPIKE_TRAIN or the SINGLE_GROUP evaluation
	std::vector<OptimizationResult> res;
	switch (params->evaluation) {
		case EvaluationType::SPIKE_TRAIN:
			res = optimization.optimize(
			    input,
			    SpikeTrainEvaluation(params->train,
			                         params->model == ModelType::IF_COND_EXP),
			    progressCallback);
			break;
		case EvaluationType::SINGLE_GROUP_SINGLE_OUT:
			res = optimization.optimize(
			    input, SingleGroupSingleOutEvaluation(
			               params->environment, params->singleGroup,
			               params->model == ModelType::IF_COND_EXP),
			    progressCallback);
		case EvaluationType::SINGLE_GROUP_MULTI_OUT:
			res = optimization.optimize(
			    input, SingleGroupMultiOutEvaluation(
			               params->environment, params->singleGroup,
			               params->model == ModelType::IF_COND_EXP),
			    progressCallback);
	}

	// Emit the done event
	emit progress(true, it, 0, res);
}

void OptimizationJobRunner::abort() { aborted.store(true); }

/*
 * Class OptimizationJob
 */

OptimizationJob::OptimizationJob(std::shared_ptr<ParameterCollection> params,
                                 QObject *parent)
    : QObject(parent),
      pool(new QThreadPool(this)),
      params(params),
      currentRunner(nullptr)
{
	qRegisterMetaType<size_t>("size_t");
	qRegisterMetaType<std::vector<OptimizationResult>>(
	    "std::vector<OptimizationResult>");
}

OptimizationJob::~OptimizationJob() { abort(); }

void OptimizationJob::handleProgress(bool done, size_t nIt, size_t nInput,
                                     std::vector<OptimizationResult> output)
{
	// Reset the currentRunner once the operation is done
	if (done) {
		pool->waitForDone();
		currentRunner = nullptr;
	}

	// Relay the progress
	emit progress(done, nIt, nInput, output);
}

bool OptimizationJob::isActive() const { return currentRunner != nullptr; }

void OptimizationJob::abort()
{
	// Call the abort function of the runner
	if (currentRunner != nullptr) {
		currentRunner->abort();
	}

	// Wait for the thread to be finished
	pool->waitForDone();

	// Free the runner
	currentRunner = nullptr;
}

void OptimizationJob::start(bool limitToHw)
{
	// Cancel any running optimization first
	abort();

	// Start a new optimization, pass the progress signal through
	currentRunner = std::unique_ptr<OptimizationJobRunner>(
	    new OptimizationJobRunner(limitToHw, params));
	connect(
	    currentRunner.get(),
	    SIGNAL(progress(bool, size_t, size_t, std::vector<OptimizationResult>)),
	    this, SLOT(handleProgress(bool, size_t, size_t,
	                              std::vector<OptimizationResult>)));
	pool->start(currentRunner.get());
}
}

