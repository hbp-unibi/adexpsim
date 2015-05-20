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
 * @file IncrementalExploration.hpp
 *
 * The IncrementalExploration class implements a background exploration with
 * multiple resolution levels.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_INCREMENTRAL_EXPLORATION_HPP_
#define _ADEXPSIM_INCREMENTRAL_EXPLORATION_HPP_

#include <atomic>
#include <memory>
#include <vector>

#include <utils/Types.hpp>

#include <QRunnable>
#include <QObject>

class QTimer;

namespace AdExpSim {

class SpikeTrain;
class Parameters;
class Exploration;
class ExplorationMemory;

/**
 * The IncrementalExplorationRunner runs a single exploration process in the
 * background.
 */
class IncrementalExplorationRunner : public QObject, public QRunnable {
	Q_OBJECT
private:
	/**
	 * Flag used to abort the exploration process.
	 */
	std::atomic<bool> aborted;

	/**
	 * Reference at the exploration code.
	 */
	Exploration &exploration;

	/**
	 * Task code, runs the exploration, triggers the done and progress signals.
	 */
	void run() override;

public:
	/**
	 * Constructor of the IncrementalExplorationRunner class.
	 *
	 * @param exploration is a reference at the exploration instance that should
	 * run.
	 */
	IncrementalExplorationRunner(Exploration &exploration);

	~IncrementalExplorationRunner() override;

	/**
	 * Cancels the current exploration process.
	 */
	void abort();

signals:
	/**
	 * Signal emitted whenever the progress should be updated.
	 *
	 * @param p is the current progress as a value from 0 to 1.
	 */
	void progress(float p);

	/**
	 * Signal emitted whenever the exploration has finished.
	 *
	 * @param ok is set to true if the exploration was successful, false
	 * otherwise.
	 */
	void done(bool ok);
};

/**
 * The IncrementalExploration class is used for the exploration of the
 * incremental update of the
 */
class IncrementalExploration : public QObject {
	Q_OBJECT

private:
	/**
	 * Minimum resolution level as power of two.
	 */
	static constexpr int MIN_LEVEL = 5;  // 32x32

	/**
	 * Minimum resolution level as power of two.
	 */
	static constexpr int MAX_LEVEL = 8;  // 256x256

	/**
	 * Memories for the resolution levels.
	 */
	std::vector<std::shared_ptr<ExplorationMemory>> mem;

private:
	/**
	 * Timer used to defer the calls to "update".
	 */
	QTimer *updateTimer;

	/**
	 * Dimensions x and y for the exploration.
	 */
	size_t dimX, dimY;

	/**
	 * Range for the dimensions.
	 */
	Val minX, maxX, minY, maxY;

	/**
	 * Current neuron parameters.
	 */
	std::shared_ptr<Parameters> params;

	/**
	 * Spike train used for the exploration.
	 */
	std::shared_ptr<SpikeTrain> train;

	/**
	 * Contains the resolution level (as power of two) of the currently running
	 * exploration runner.
	 */
	int level;

	/**
	 * Flag set to true if a new IncrementalExplorationRunner instance should be
	 * started in the moment the last IncrementalExplorationRunner finishes.
	 */
	bool restart;

	/**
	 * Flag set to true if were currently emitting data -- in this case range
	 * updates should be discarded.
	 */
	bool inEmitData;

	/**
	 * The current exploration instance.
	 */
	Exploration *currentExploration;

	/**
	 * The current IncrementalExplorationRunner instance.
	 */
	IncrementalExplorationRunner *currentRunner;

	/**
	 * Starts a new IncrementalExplorationRunner instance.
	 */
	void start();

private slots:
	/**
	 * Slot used to relay the progress to the corresponding signal of this
	 * instance, while rescaling the progress according to the current
	 * resolution level.
	 */
	void runnerProgress(float p);

	/**
	 * Slot used to detect the runner being done.
	 *
	 * @param ok is set to true if the runner finished its calculation.
	 */
	void runnerDone(bool ok);

	/**
	 * Method call whenever the update timer fires.
	 */
	void updateTimeout();

public:
	/**
	 * Constructor of the IncrementalExploration class. Initializes the
	 * parameters and dimensions with some sane values.
	 *
	 * @param parent is the owner of this object.
	 */
	IncrementalExploration(QObject *parent, std::shared_ptr<Parameters> params,
	                       std::shared_ptr<SpikeTrain> train);

	~IncrementalExploration() override;

public slots:
	/**
	 * Should be called whenever the range of the exploration or the exploration
	 * dimensions changes.
	 */
	void updateRange(size_t dimX, size_t dimY, Val minX, Val maxX, Val minY,
	                 Val maxY);

	/**
	 * Prepares the start of a new IncrementalExplorationRunner process with the
	 * current parameters and directly starts the runner if no other
	 * IncrementalExplorationRunner is active.
	 */
	void update();

signals:
	/**
	 * Signal emitted whenever the progress should be updated.
	 *
	 * @param p is the progress between 0 and 1.
	 * @param show is set to true, if the progress bar should be shown, false
	 * otherwise.
	 */
	void progress(float p, bool show);

	/**
	 * Signal emitted whenever the exploration for one resolution level has
	 * finished.
	 */
	void data(const Exploration &exploration);
};
}

#endif /* _ADEXPSIM_INCREMENTRAL_EXPLORATION_HPP_ */

