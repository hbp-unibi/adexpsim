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

#include <QDockWidget>
#include <QTimer>
#include <QLabel>

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <exploration/Exploration.hpp>
#include <model/IncrementalExploration.hpp>
#include <model/NeuronSimulation.hpp>

#include "ExplorationWidget.hpp"
#include "MainWindow.hpp"
#include "NeuronSimulationWidget.hpp"

namespace AdExpSim {

static QDockWidget *createDockWidget(const QString &name, QMainWindow *parent)
{
	QDockWidget *res = new QDockWidget(name, parent);
	res->setAllowedAreas(Qt::AllDockWidgetAreas);
	res->setFeatures(QDockWidget::DockWidgetMovable |
	                 QDockWidget::DockWidgetFloatable);
	parent->addDockWidget(Qt::TopDockWidgetArea, res);
	return res;
}

MainWindow::MainWindow()
    : fitExploration(true),
      params(std::make_shared<Parameters>()),
      train(std::make_shared<SpikeTrain>(
          SpikeTrain({{4, 1, 1e-3}, {3, 0, 1e-3}}, 20, false, 0.033_s)))
{
	// Create the incremental exploration object
	exploration = new IncrementalExploration(this, params, train);

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
	simulationDockWidget->setWidget(simulationWidget);


	// We have no central widget
	setCentralWidget(nullptr);

	resize(1024, 768);

	// Update the simulation according to the current parameters
	updateSimulation();
}

MainWindow::~MainWindow() {}

void MainWindow::data(const Exploration &exploration)
{
	explorationWidget->show(exploration, fitExploration);
	fitExploration = false;
}

void MainWindow::explorationWidgetUpdateParameters() { updateSimulation(); }

void MainWindow::updateSimulation()
{
	NeuronSimulation sim;
	sim.prepare(*params, train->getSpikes());
	sim.run();
	simulationWidget->show({&sim});
}
}

