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

#include <iostream>

#include <QTimer>
#include <QThreadPool>

#include <exploration/Exploration.hpp>

#include "IncrementalExploration.hpp"

namespace AdExpSim {

/*
 * Class IncrementalExplorationRunner
 */

IncrementalExplorationRunner::IncrementalExplorationRunner(
    Exploration &exploration)
    : aborted(false), exploration(exploration)
{
	setAutoDelete(false);
	std::cout << "Create IncrementalExplorationRunner" << std::endl;
}

IncrementalExplorationRunner::~IncrementalExplorationRunner()
{
	std::cout << "Destroy IncrementalExplorationRunner" << std::endl;
}

void IncrementalExplorationRunner::run()
{
	// Run the exploration process
	const bool ok = exploration.run([&](float p) -> bool {
		emit progress(p);
		return !aborted.load();
	});

	// Emit the done event
	emit done(ok && !aborted.load());
}

void IncrementalExplorationRunner::abort() { aborted.store(true); }

/*
 * Class IncrementalExploration
 */

IncrementalExploration::IncrementalExploration(QObject *parent)
    : QObject(parent),
      dimX(0),
      dimY(1),
      minX(1),
      maxX(100),
      minY(1),
      maxY(100),
      Xi(3),
      T(1e-3),
      level(MIN_LEVEL),
      restart(false),
      inEmitData(false),
      currentExploration(nullptr),
      currentRunner(nullptr)
{
	// Choose an estimate for w
	params.wSpike() = params.estimateW(Xi);

	// Create the exploration memory instances
	for (int level = MIN_LEVEL; level <= MAX_LEVEL; level++) {
		mem.emplace_back(
		    std::make_shared<ExplorationMemory>(1 << level, 1 << level));
	}

	// Create the timer
	updateTimer = new QTimer(this);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateTimeout()));
}

IncrementalExploration::~IncrementalExploration()
{
	if (currentRunner != nullptr) {
		currentRunner->abort();
		QThreadPool::globalInstance()->waitForDone();
	}
}

void IncrementalExploration::start()
{
	// Delete the currentExploration instance
	if (currentExploration != nullptr) {
		delete currentExploration;
	}

	// Create a new Exploration instance
	currentExploration =
	    new Exploration(mem[level - MIN_LEVEL], params, Xi, Time::sec(T), dimX,
	                    minX, maxX, dimY, minY, maxY);

	// Create a new IncrementExplorationRunner and connect all signals
	currentRunner = new IncrementalExplorationRunner(*currentExploration);
	connect(currentRunner, SIGNAL(progress(float)), this,
	        SLOT(runnerProgress(float)));
	connect(currentRunner, SIGNAL(done(bool)), this, SLOT(runnerDone(bool)));

	// Reset the restart flag and increment the level counter
	restart = false;
	level++;

	// Start the runner
	QThreadPool::globalInstance()->start(currentRunner);
}

void IncrementalExploration::update()
{
	// Delay the update action by 250msec, however, we can abort the current job
	updateTimer->start(250);
	if (currentRunner != nullptr) {
		currentRunner->abort();
	}
}

void IncrementalExploration::updateTimeout()
{
	// Kill the update timer
	updateTimer->stop();

	// Schedule a restart and reset the current level
	restart = true;
	level = MIN_LEVEL;

	// If there currently is no runner, start a new one, otherwise abort the
	// current runner
	if (currentRunner == nullptr) {
		start();
	} else {
		currentRunner->abort();
	}
}

void IncrementalExploration::runnerProgress(float p)
{
	// l is always set ot the next level, so we have to decrement it by one
	const int L = level - MIN_LEVEL - 1;
	if (L < 0 || restart) {
		emit progress(0.0, false);
		return;
	}

	// Calculate the normalization value (sum over 2^i for i=0..(MAX-MIN))
	const int norm = (1 << (MAX_LEVEL - MIN_LEVEL + 1)) - 1;

	// Calculate the previous progress (sum over 2^i for i = 0..L-1))
	const int previous = (1 << L) - 1;

	const Val pTotal = (Val(previous) + Val(1 << L) * p) / Val(norm);
	// std::cout << "L " << L << " norm " << norm << " previous " << previous <<
	// " p " << p << " res " << pTotal << std::endl;

	// Progress is (previous + 2^L * p) / norm
	emit progress(pTotal, true);
}

void IncrementalExploration::runnerDone(bool ok)
{
	std::cout << "Runner done, ok = " << ok << std::endl;
	// If the result is ok, emit the data
	if (ok) {
		// If the last level is reached, emit a last progress event
		if (level > MAX_LEVEL) {
			emit progress(1.0, false);
		}
		inEmitData = true;
		emit data(*currentExploration);
		inEmitData = false;
	} else {
		emit progress(0.0, false);
	}

	// Delete the runner object and the exploration object
	delete currentRunner;
	delete currentExploration;
	currentRunner = nullptr;
	currentExploration = nullptr;

	// Start the next iteration
	if ((level <= MAX_LEVEL && ok) || restart) {
		start();
	}
}

void IncrementalExploration::updateParameters(Val Xi, Val T,
                                              const WorkingParameters &params)
{
	// Copy the given parameters and schedule an update
	this->Xi = Xi;
	this->T = T;
	this->params = params;
	update();
}

void IncrementalExploration::updateRange(size_t dimX, size_t dimY, Val minX,
                                         Val maxX, Val minY, Val maxY)
{
	if ((dimX != this->dimX || dimY != this->dimY || minX != this->minX ||
	     maxX != this->maxX || minY != this->minY || maxY != this->maxY) &&
	    !inEmitData) {
		// Copy the range and schedule an update
		this->dimX = dimX;
		this->dimY = dimY;
		this->minX = minX;
		this->maxX = maxX;
		this->minY = minY;
		this->maxY = maxY;
		update();
	}
}
}

