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

/**
 * @file OptimizationJob.hpp
 *
 * The OptimizationJob class implements a Qt wrapper around the Optimization
 * class.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_OPTIMIZATION_JOB_HPP_
#define _ADEXPSIM_OPTIMIZATION_JOB_HPP_

#include <atomic>
#include <memory>
#include <vector>

#include <exploration/Optimization.hpp>

#include <QRunnable>
#include <QObject>

class QThreadPool;

namespace AdExpSim {

class ParameterCollection;

/**
 * The IncrementalExplorationRunner runs a single exploration process in the
 * background.
 */
class OptimizationJobRunner : public QObject, public QRunnable {
	Q_OBJECT
private:
	/**
	 * Optimization instance.
	 */
	Optimization optimization;

	/**
	 * Flag used to abort the exploration process.
	 */
	std::atomic<bool> aborted;

	/**
	 * Reference at the current neuron parameters.
	 */
	std::shared_ptr<ParameterCollection> params;

	/**
	 * Task code, runs the exploration, triggers the done and progress signals.
	 */
	void run() override;

public:
	/**
	 * Constructor of the OptimizationJobRunner class.
	 *
	 * @param limitToHw is set to true if the optimizer should try to optimize
	 * according to the hardware constraints.
	 * @param params contains the params the exploration instance should be fed
	 * with.
	 */
	OptimizationJobRunner(bool limitToHw,
	                      std::shared_ptr<ParameterCollection> params);

	~OptimizationJobRunner() override;

	/**
	 * Cancels the current exploration process.
	 */
	void abort();

signals:
	/**
	 * Signal emitted whenever the progress should be updated or when the
	 * optimization is done.
	 */
	void progress(bool done, size_t nIt, size_t nInput, float eval,
	              std::vector<OptimizationResult> output);
};

/**
 * The OptimizationJob class manages a single optimization job run in the
 * background.
 */
class OptimizationJob : public QObject {
	Q_OBJECT
private:
	/**
	 * Thread pool used to run the OptimizationJobRunner instance.
	 */
	QThreadPool *pool;

	/**
	 * Current neuron parameters.
	 */
	std::shared_ptr<ParameterCollection> params;

	/**
	 * The current OptimizationJobRunner instance.
	 */
	std::unique_ptr<OptimizationJobRunner> currentRunner;

private slots:
	void handleProgress(bool done, size_t nIt, size_t nInput, float eval,
	              std::vector<OptimizationResult> output);

public:
	/**
	 * Constructor of the IncrementalExploration class. Initializes the
	 * parameters and dimensions with some sane values.
	 *
	 * @param parent is the owner of this object.
	 */
	OptimizationJob(std::shared_ptr<ParameterCollection> params,
	                QObject *parent = nullptr);

	~OptimizationJob() override;

	/**
	 * Returns true if the job is currently active.
	 */
	bool isActive() const;

	/**
	 * Aborts the optimization process (if an optimization is in progress).
	 */
	void abort();

	/**
	 * Starts a new optimization.
	 */
	void start(bool limitToHw);

signals:
	/**
	 * Signal emitted whenever the progress should be updated.
	 *
	 * @param p is the progress between 0 and 1.
	 * @param show is set to true, if the progress bar should be shown, false
	 * otherwise.
	 */
	void progress(bool done, size_t nIt, size_t nInput, float eval,
	              std::vector<OptimizationResult> output);
};
}

#endif /* _ADEXPSIM_OPTIMIZATION_JOB_HPP_ */

