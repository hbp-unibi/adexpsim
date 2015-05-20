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
    : fitExploration(true), parameters(std::make_shared<Parameters>())
{
	// Create the incremental exploration object
	exploration = new IncrementalExploration(this);

	// Create a new ExplorationWidget and add it as one dock widget
	explorationDockWidget = createDockWidget("Exploration", this);
	explorationWidget = new ExplorationWidget(explorationDockWidget, parameters);
	explorationDockWidget->setWidget(explorationWidget);

	// Connect the exploration events with the exploration widget and vice versa
	connect(exploration, SIGNAL(progress(float, bool)), explorationWidget,
	        SLOT(progress(float, bool)));
	connect(exploration, SIGNAL(data(const Exploration &)), this,
	        SLOT(data(const Exploration &)));
	connect(explorationWidget,
	        SIGNAL(updateRange(size_t, size_t, Val, Val, Val, Val)),
	        exploration, SLOT(updateRange(size_t, size_t, Val, Val, Val, Val)));

	explorationWidget->centerView();

	// Create a new NeuronSimulationWidget and add it as one dock widget
	simulationDockWidget = createDockWidget("Simulation", this);
	simulationWidget = new NeuronSimulationWidget(simulationDockWidget);
	simulationDockWidget->setWidget(simulationWidget);

	// We have no central widget
	setCentralWidget(nullptr);

	resize(1024, 768);

	// Create and show a simulation
	NeuronSimulation sim(Time::sec(0.1e-3));
	SpikeTrain train({{4, 1, 1e-3}, {1, 0, 1e-3}}, 5,
	                 true, Time::sec(0.1), 0.01);
	sim.prepare(*parameters, train.getSpikes());
	sim.run();
	simulationWidget->show({&sim});
}

MainWindow::~MainWindow() {}

void MainWindow::data(const Exploration &exploration)
{
	std::cout << "Receive data" << std::endl;
	explorationWidget->show(exploration, fitExploration);
	fitExploration = false;
}
}

