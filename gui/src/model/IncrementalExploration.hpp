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

#include <exploration/Exploration.hpp>
#include <common/Types.hpp>

#include <QRunnable>
#include <QObject>

class QTimer;
class QThreadPool;

namespace AdExpSim {

class ParameterCollection;

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
	 * Reference at the current neuron parameters.
	 */
	std::shared_ptr<ParameterCollection> params;

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
	 * @param params contains the params the exploration instance should be fed
	 * with.
	 */
	IncrementalExplorationRunner(Exploration &exploration,
	                             std::shared_ptr<ParameterCollection> params);

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
	static constexpr int MIN_LEVEL = 4;  // 16x16

	/**
	 * Maximum resolution level as power of two.
	 */
	static constexpr int MAX_LEVEL = 11;  // 2048x2048

	/**
	 * Initial maximum resolution level as power of two.
	 */
	static constexpr int MAX_LEVEL_INITIAL = 8;  // 256x256

	/**
	 * QThreadPool used to execute the runner.
	 */
	QThreadPool *pool;

	/**
	 * Timer used to defer the calls to "update".
	 */
	QTimer *updateTimer;

	/**
	 * Current maximum resolution level.
	 */
	int maxLevel;

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
	std::shared_ptr<ParameterCollection> params;

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
	 * The exploration instance.
	 */
	Exploration exploration;

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
	IncrementalExploration(std::shared_ptr<ParameterCollection> params,
	                       QObject *parent = nullptr);

	~IncrementalExploration() override;

	/**
	 * Returns true if the IncrementalExploration is currently calculating
	 * something or preparing to do so, false if it currently idles.
	 */
	bool isActive() const;

	/**
	 * Used to set the current maximum resolution as exponent to the base of
	 * two.
	 */
	void setMaxLevel(int maxLevel);

	/**
	 * Returns the current maximum resolution level.
	 */
	int getMaxLevel() { return maxLevel; }

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
	void data(Exploration exploration);
};
}

#endif /* _ADEXPSIM_INCREMENTRAL_EXPLORATION_HPP_ */

