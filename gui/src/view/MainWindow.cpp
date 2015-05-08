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

#include <core/Spike.hpp>
#include <model/NeuronSimulation.hpp>

#include "MainWindow.hpp"
#include "NeuronSimulationWidget.hpp"

namespace AdExpSim {

MainWindow::MainWindow() : sim1(new NeuronSimulation())
{
	// Create a new NeuronSimulationWidget and set it as the central 
	// widget
	neuronSimulationWidget = new NeuronSimulationWidget();
	setCentralWidget(neuronSimulationWidget);

	resize(1024, 768);

	sim1->prepare(Parameters(), buildInputSpikes(4, 1e-3, 0, 0.03175e-6));
	sim1->run(0.1e-3);

	neuronSimulationWidget->show({sim1.get()});
}

MainWindow::~MainWindow() {}


}

