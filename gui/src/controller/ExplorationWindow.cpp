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

#include <QHBoxLayout>
#include <QWidget>

#include <view/ExplorationWidget.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <model/IncrementalExploration.hpp>

#include "ExplorationWindow.hpp"

namespace AdExpSim {

ExplorationWindow::ExplorationWindow(std::shared_ptr<Parameters> params,
                                     std::shared_ptr<SpikeTrain> train,
                                     QWidget *parent)
    : AbstractViewerWindow(params, train, parent),
      fitView(true),
      exploration(std::make_shared<Exploration>())
{
	// Create all elements
	createModel();
	createWidgets();

	// Resize the window to a proper size
	resize(800, 600);
}

ExplorationWindow::~ExplorationWindow()
{
	// Only needed for the shared_ptr
}

void ExplorationWindow::createModel()
{
	// Create the incrementalExploration model and connect its data singal to
	// the local handleExplorationData
	incrementalExploration = new IncrementalExploration(params, train, this);
	connect(incrementalExploration, SIGNAL(data(Exploration)), this,
	        SLOT(handleExplorationData(Exploration)));
}

void ExplorationWindow::createWidgets()
{
	// Create the exploration widget and connect its signals/slots
	explorationWidget = new ExplorationWidget(params, exploration, this);
	connect(explorationWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleInternalUpdateParameters(std::set<size_t>)));
	connect(explorationWidget,
	        SIGNAL(updateRange(size_t, size_t, Val, Val, Val, Val)),
	        incrementalExploration,
	        SLOT(updateRange(size_t, size_t, Val, Val, Val, Val)));
	connect(incrementalExploration, SIGNAL(progress(float, bool)),
	        explorationWidget, SLOT(progress(float, bool)));

	// Center the view of the ExplorationWidget to trigger an initial
	// exploration
	explorationWidget->centerView();

	// Create a layout and a container widget, add the created widgets
	QWidget *container = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(container);
	layout->addWidget(explorationWidget);
	layout->setSpacing(0);
	layout->setMargin(0);
	container->setLayout(layout);

	setCentralWidget(container);
}

void ExplorationWindow::handleExplorationData(Exploration data)
{
	// Create a clone of the given exploration instance (including the memory)
	*exploration = data.clone();

	// Update the view, fit it if the corresponding flag has been set
	explorationWidget->refresh();
	if (fitView) {
		explorationWidget->fitView();
		fitView = false;
	}
}

void ExplorationWindow::handleInternalUpdateParameters(std::set<size_t> dims)
{
	emit updateParameters(dims);
}

void ExplorationWindow::handleUpdateParameters(std::set<size_t> dims)
{
	// Determine whether a recalculation is needed. This is the case if dims
	// is empty (then everything should be updated) or one of the changed
	// dimensions is not currently being displayed
	bool needRecalc = dims.empty();
	dims.erase(explorationWidget->getDimX());
	dims.erase(explorationWidget->getDimY());
	needRecalc = needRecalc || !dims.empty();

	// If the dimensions that changed are not the dimensions that are currently
	// viewed, we need to run a new exploration in the background
	if (needRecalc) {
		incrementalExploration->update();
	} else {
		// Otherwise just refresh in order to move the crosshair position
		explorationWidget->refresh();
	}
}
}

