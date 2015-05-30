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
 * @file SimulationWindow.hpp
 *
 * Contains the SimulationWindow class, which is used to control the single
 * neuron visualization.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SIMULATION_WINDOW_HPP_
#define _ADEXPSIM_SIMULATION_WINDOW_HPP_

#include "AbstractViewerWindow.hpp"

namespace AdExpSim {

class NeuronSimulationWidget;

/**
 * The SimulationWindow class is the controller object, containing a simulation
 * model and the corresponding view widget. The SimulationWindow is responsible
 * for exchanging events between these classes.
 */
class SimulationWindow : public AbstractViewerWindow {
	Q_OBJECT

private:
	NeuronSimulationWidget *simulationWidget;

	void createWidgets();

public:
	/**
	 * Constructor of the SimulationWindow class.
	 */
	SimulationWindow(std::shared_ptr<Parameters> params,
	                 std::shared_ptr<SpikeTrain> train,
	                 QWidget *parent = nullptr);

	/**
	 * Destructor of the SimulationWindow class.
	 */
	~SimulationWindow();

	/**
	 * Should be called whenever the neuron parameters or the spike train
	 * have been updated.
	 *
	 * @param dims contains the indices of the dimensions that have been
	 * updated. If empty, this indicates that "everything" has changed.
	 */
	void handleUpdateParameters(std::set<size_t>) override;
};
}

#endif /* _ADEXPSIM_SIMULATION_WINDOW_HPP_ */
