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

#include <exploration/SpikeTrainEvaluation.hpp>
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
public:
	using OutputSpikeVec = std::vector<SpikeTrainEvaluation::OutputSpike>;
	using OutputGroupVec = std::vector<SpikeTrainEvaluation::OutputGroup>;

private:
	/**
	 * Parameters the experiment ran with.
	 */
	Parameters parameters;

	/**
	 * Evaluation used for storing the spike train containing the input
	 * descriptor.
	 */
	SpikeTrainEvaluation evaluation;

	/**
	 * Output spikes.
	 */
	OutputSpikeVec outputSpikes;

	/**
	 * Output groups.
	 */
	OutputGroupVec outputGroups;

	/**
	 * Recorder used to record and hold the data from the neuron simulation.
	 */
	VectorRecorder<QVector<double>, SIPrefixTransformation> recorder;

public:
	/**
	 * Creates a new NeuronSimulation instance.
	 */
	NeuronSimulation(Time interval = Time(0))
	    : recorder(parameters, interval){};

	/**
	 * Sets the parameters and input spikes.
	 */
	void prepare(const Parameters &parameters, const SpikeTrain &train);

	/**
	 * Runs the simulation with the parameters given in the prepare method.
	 */
	void run();

	/**
	 * Returns the parameters.
	 */
	const Parameters &getParameters() const { return parameters; }

	/**
	 * Returns the input spikes.
	 */
	const SpikeVec &getInputSpikes() const { return getTrain().getSpikes(); }

	/**
	 * Returns the output spikes.
	 */
	const OutputSpikeVec &getOutputSpikes() const { return outputSpikes; }

	/**
	 * Returns the output groups.
	 */
	const OutputGroupVec &getOutputGroups() const { return outputGroups; }

	/**
	 * Returns the input spike train descriptor.
	 */
	const SpikeTrain &getTrain() const { return evaluation.getTrain(); }

	/**
	 * Returns the maximum timestamp of the recorded data.
	 */
	const double getMaxT() const { return getData().maxTime; }

	/**
	 * Returns the recorded data.
	 */
	const VectorRecorderData<QVector<double>> &getData() const
	{
		return recorder.getData();
	}
};

using NeuronSimulationResultVec = QVector<NeuronSimulation>;
}

#endif /* _ADEXPSIM_NEURON_SIMULATION_HPP_ */

