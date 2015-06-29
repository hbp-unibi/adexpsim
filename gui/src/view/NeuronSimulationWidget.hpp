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
 * @file NeuronSimulationWidget.hpp
 *
 * The NeuronSimulationWidget is used to display the result of a 
 * AdExp model simulation run in the form of a set of line plots.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_NEURON_SIMULATION_WIDGET_HPP_
#define _ADEXPSIM_NEURON_SIMULATION_WIDGET_HPP_

#include <QWidget>

#include <model/NeuronSimulation.hpp>

class QVBoxLayout;
class QCustomPlot;
class QTimer;

namespace AdExpSim {

class SpikeWidget;

/**
 * The SimulationResultWidget shows the line plot for a single-neuron
 * simulation and provides a user interface to interact with these plots.
 */
class NeuronSimulationWidget: public QWidget {
Q_OBJECT
private:
	QVBoxLayout *layout;
	SpikeWidget *spikeWidget;
	QCustomPlot *pltVolt;
	QCustomPlot *pltCond;
	QCustomPlot *pltCurr;
	QTimer *updateTimer;
	NeuronSimulation sim;

private slots:
	/**
	 * Redraws the plot windows according to the current range and simulation.
	 */
	void updatePlot();

	/**
	 * Called whenever the NeuronSimulationWidget indicates a range change.
	 */
	void rangeChange();

public:
	/**
	 * Constructor of the NeuronSimulationWidget class.
	 */
	NeuronSimulationWidget(QWidget *parent);

	/**
	 * Displays the given simulation result instance.
	 *
	 * @param sims is a list of references at NeuronSimulation instances that
	 * should be displayed. The given references are not stored internally, all
	 * important data is implicitly copied by the plot widgets.
	 */
	void show(const NeuronSimulation &sim);
};

}

#endif /* _ADEXPSIM_NEURON_SIMULATION_WIDGET_HPP_ */

