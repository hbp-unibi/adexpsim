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

#include <simulation/Spike.hpp>
#include <exploration/Exploration.hpp>
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
{
	// Create a new ExplorationWidget and add it as one dock widget
	explorationDockWidget = createDockWidget("Exploration", this);
	explorationWidget = new ExplorationWidget(explorationDockWidget);
	explorationDockWidget->setWidget(explorationWidget);

	// Create a new NeuronSimulationWidget and add it as one dock widget
	simulationDockWidget = createDockWidget("Simulation", this);
	simulationWidget = new NeuronSimulationWidget(simulationDockWidget);
	simulationDockWidget->setWidget(simulationWidget);

	// We have no central widget
	setCentralWidget(nullptr);

	resize(1024, 768);

	// Create and show a simulation
	NeuronSimulation sim;
	sim.prepare(Parameters(), buildInputSpikes(4, 1e-3, 0, 0.03175e-6));
	sim.run(0.1e-3);
	simulationWidget->show({&sim});

	// Setup the parameters, set an initial value for w
	WorkingParameters params;
	float wOrig = params.wSpike();
	params.wSpike() = wOrig * 0.04e-6;

	// Exploration meta-parameters
	const Val Xi = 3;
	const Val T = 1e-3;

	// Setup the exploration
	ExplorationMemory mem(256, 256);
	Exploration exploration(mem, params, Xi, T,
	                        0,    // dimX lL
	                        1,    // minX
	                        100,  // maxX
	                        1,    // dimY lE
	                        1,    // minY
	                        100   // maxY
	                        );
	exploration.run();
	explorationWidget->show(exploration);
}

MainWindow::~MainWindow() {}
}

