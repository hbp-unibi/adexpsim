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
}

void IncrementalExplorationRunner::run()
{
	// Run the exploration process
	const bool ok = exploration.run([&](float p) -> bool {
		emit progress(p);
		return !abort.load();
	});

	// Emit the done event
	emit done(ok && aborted.load());
}

void IncrementalExplorationRunner::abort() { abort.store(true); }

/*
 * Class IncrementalExploration
 */

IncrementalExploration::IncrementalExploration()
    : dimX(0),
      dimY(1),
      minX(1),
      maxX(100),
      minY(1),
      maxY(100),
      Xi(3),
      T(1e-3),
      level(MIN_LEVEL),
      restart(false),
      currentExploration(nullptr),
      currentRunner(nullptr)
{
	// Choose an estimate for w
	params.wSpike = params.estimateW(Xi);

	// Create the exploration memory instances
	for (int level = MIN_LEVEL; level < MAX_LEVEL; level++) {
		mem.emplace_back(1 << level, 1 << level);
	}
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
	currentExploration = new Exploration(mem[level - MIN_LEVEL], Xi, T, dimX,
	                                     minX, maxX, dimY, minY, maxY);

	// Create a new IncrementExplorationRunner and connect all signals
	currentRunner = new IncrementalExplorationRunner(*currentExploration);
	connect(currentRunner, SIGNAL(progress(float)), this, SLOT(runnerProgress));
	connect(currentDone, SIGNAL(done(bool)), this, SLOT(runnerDone));

	// Reset the restart flag and increment the level counter
	restart = false;
	level++;

	// Start the runner
	QThreadPool::globalInstance()->start(currentRunner);
}

void IncrementalExploration::update()
{
	// Schedule a restart and reset the current level
	restart = true;
	level = MIN_LEVEL;

	// If there currently is no runner, start a new one
	if (currentRunner == nullptr) {
		start();
	}
}

void IncrementalExploration::runnerProgress(float p)
{
	// Calculate the current progress scale factor -- note that level is always
	// one larger than the current level. So for the last level f is set to 0.5.
	Val offs = 0.0;
	Val norm = 0.0;
	for (int l = MIN_LEVEL; l < level - 1; l++) {
		offs += 1.0 / Val(1 << (l - MAX_LEVEL));
	}
	Val f = 1.0 / Val(1 << (level - MAX_LEVEL))

	emit progress((1.0 + p) * f)
}

void IncrementalExploration::runnerDone(bool ok)
{
	// If the result is ok, emit the data
	if (ok) {
		// If the last level is reached, emit a last progress event
		if (level >= MAX_LEVEL) {
			emit progress(1.0, false);
		}
		emit data(*currentExploration);
	}

	// Delete the runner object and the exploration object
	delete currentRunner;
	delete currentExploration;
	currentRunner = nullptr;
	currentExploration = nullptr;

	// Start the next iteration
	if ((level < MAX_LEVEL && ok) || restart) {
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

