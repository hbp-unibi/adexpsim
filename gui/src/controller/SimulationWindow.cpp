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

#include <model/NeuronSimulation.hpp>
#include <view/NeuronSimulationWidget.hpp>

#include "SimulationWindow.hpp"

namespace AdExpSim {

SimulationWindow::SimulationWindow(std::shared_ptr<Parameters> params,
                                   std::shared_ptr<SpikeTrain> train,
                                   QWidget *parent)
    : AbstractViewerWindow(params, train, parent)
{
	// Create the underlying Simulation class and assemble the view
	createWidgets();

	// Set the window size and title
	resize(800, 900);
	setWindowTitle("AdExpSim ‒ Neuron Simulation");

	// Show the plotted data
	handleUpdateParameters(std::set<size_t>{});
}

SimulationWindow::~SimulationWindow()
{
	// Do nothing here
}

void SimulationWindow::createWidgets()
{
	simulationWidget = new NeuronSimulationWidget(this);
	setCentralWidget(simulationWidget);
}

void SimulationWindow::handleUpdateParameters(std::set<size_t>)
{
	NeuronSimulation sim(0.1e-3_s);
	sim.prepare(*params, *train);
	sim.run();
	simulationWidget->show(sim);
}
}

