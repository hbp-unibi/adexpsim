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

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBox>
#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <view/ParametersWidget.hpp>

#include "MainWindow.hpp"
#include "ExplorationWindow.hpp"
#include "SimulationWindow.hpp"

namespace AdExpSim {

// Create the incremental exploration object
/*	exploration = new IncrementalExploration(this, params, train);

    // Create a new ExplorationWidget and add it as one dock widget
    explorationDockWidget = createDockWidget("Exploration", this);
    explorationWidget = new ExplorationWidget(explorationDockWidget, params);
    explorationDockWidget->setWidget(explorationWidget);

    // Connect the exploration events with the exploration widget and vice versa
    connect(exploration, SIGNAL(progress(float, bool)), explorationWidget,
            SLOT(progress(float, bool)));
    connect(exploration, SIGNAL(data(const Exploration &)), this,
            SLOT(data(const Exploration &)));
    connect(explorationWidget,
            SIGNAL(updateRange(size_t, size_t, Val, Val, Val, Val)),
            exploration, SLOT(updateRange(size_t, size_t, Val, Val, Val, Val)));
    connect(explorationWidget, SIGNAL(updateParameters()), this,
            SLOT(explorationWidgetUpdateParameters()));

    explorationWidget->centerView();

    // Create a new NeuronSimulationWidget and add it as one dock widget
    simulationDockWidget = createDockWidget("Simulation", this);
    simulationWidget = new NeuronSimulationWidget(simulationDockWidget);
    simulationWidget->setMinimumWidth(300);
    simulationDockWidget->setWidget(simulationWidget);*/

MainWindow::MainWindow()
    : /*fitExploration(true),*/
      params(std::make_shared<Parameters>()),
      train(std::make_shared<SpikeTrain>(
          SpikeTrain({
                      {4, 1, 1e-3, 1.0, 0.1},
                      {3, 0, 1e-3, 1.0, 0.1},
                     },
                     2, true, 0.033_s)))
{
	// Create all actions and menus
	createActions();
	createMenus();
	createWidgets();

	// Resize the window
	resize(400, 600);

	// Open a new exploration and simulation window
	newExploration();
	newSimulation();
}

MainWindow::~MainWindow() {}

void MainWindow::createActions()
{
	actNewExplorationWnd = new QAction(tr("New Exploration Window..."), this);
	connect(actNewExplorationWnd, SIGNAL(triggered()), this,
	        SLOT(newExploration()));

	actNewSimulationWnd = new QAction(tr("New Simulation Window..."), this);
	connect(actNewExplorationWnd, SIGNAL(triggered()), this,
	        SLOT(newSimulation()));

	actOpenExploration = new QAction(QIcon::fromTheme("document-open"),
	                                 tr("Open Exploration..."), this);

	actSaveExploration = new QAction(QIcon::fromTheme("document-save"),
	                                 tr("Save Current Exploration..."), this);

	actExit = new QAction(tr("Exit"), this);
	connect(actExit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMenus()
{
	QMenu *fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(actNewExplorationWnd);
	fileMenu->addAction(actNewSimulationWnd);
	fileMenu->addSeparator();
	fileMenu->addAction(actOpenExploration);
	fileMenu->addAction(actSaveExploration);
	fileMenu->addSeparator();
	fileMenu->addAction(actExit);

	menuBar()->addMenu(fileMenu);
}

void MainWindow::createWidgets()
{
	// Create the tool box
	QToolBox *tools = new QToolBox(this);

	// Create the parameters panel and add it to the tool box
	ParametersWidget *parametersWidget = new ParametersWidget(this, params);
	connect(parametersWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(parametersWidget, "Parameters");

	// Set the tool box as central widget
	setCentralWidget(tools);
}

void MainWindow::newExploration()
{
	ExplorationWindow *wnd = new ExplorationWindow(params, train, this);
	connect(wnd, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	explorations.push_back(wnd);
	wnd->show();
}

void MainWindow::newSimulation()
{
//	simulations.push_back(new SimulationWindow(params, train));
}

void MainWindow::handleUpdateParameters(std::set<size_t> dims)
{
	// Forward the event to all exploration and simulation windows
	for (auto wnd : explorations) {
		if (wnd != nullptr) {
			wnd->handleUpdateParameters(dims);
		}
	}
/*	for (auto wnd : simulations) {
		if (wnd != nullptr) {
			wnd->handleUpdateParameters(dims);
		}
	}*/
}
}

