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
 * @file NeuronSimulationResult.hpp
 *
 * The NeuronSimulationResult class is used to store the result of a series of
 * single-neuron simulations of different parameters and different spike
 * patterns.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_NEURON_SIMULATION_HPP_
#define _ADEXPSIM_NEURON_SIMULATION_HPP_

#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/Spike.hpp>
#include <simulation/Model.hpp>

#include <QVector>

namespace AdExpSim {

/**
 * NeuronSimulationResult contains the results for a single single-neuron
 * simulation run. It stores the parameters the experiment ran with, the input
 * spikes and the recorded model data.
 */
class NeuronSimulation {
private:
	/**
	 * Parameters the experiment ran with.
	 */
	Parameters parameters;

	/**
	 * Input spikes sent to the neuron.
	 */
	SpikeVec spikes;

	/**
	 * Recorder used to record the data from the neuron simulation.
	 */
	VectorRecorder<QVector<double>, SIPrefixTransformation> recorder;

	/**
	 * Controller used for tracking the maximum value.
	 */
	MaxValueController controller;

	/**
	 * Internally used integrator.
	 */
	DormandPrinceIntegrator integrator;

public:
	/**
	 * Creates a new NeuronSimulation instance.
	 */
	NeuronSimulation(Time interval = Time(0)) : recorder(parameters, interval){};

	/**
	 * Sets the parameters and input spikes.
	 */
	void prepare(const Parameters &parameters, const SpikeVec &spikes);

	/**
	 * Runs the simulation with the parameters given in the prepare method.
	 */
	void run(Time tDelta = Time(-1));

	/**
	 * Returns the parameters.
	 */
	const Parameters &getParameters() const { return parameters; }

	/**
	 * Returns the input spikes.
	 */
	const SpikeVec &getSpikes() const { return spikes; }

	/**
	 * Returns the recorded data.
	 */
	const VectorRecorderData<QVector<double>> &getData() const
	{
		return recorder.getData();
	}

	/**
	 * Returns the controller.
	 */
	const MaxValueController &getController() const { return controller; }
};

using NeuronSimulationResultVec = QVector<NeuronSimulation>;
}

#endif /* _ADEXPSIM_NEURON_SIMULATION_HPP_ */

