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

#include <memory>

#include <exploration/SpikeTrainEvaluation.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/Model.hpp>
#include <utils/ParameterCollection.hpp>

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
	 * Flag indicating whether a valid simulation result is present.
	 */
	bool mValid;

	/**
	 * Parameters the experiment ran with.
	 */
	ParameterCollection params;

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
	VectorRecorder<QVector<double>, SIPrefixTrafo> recorder;

public:
	/**
	 * Creates a new NeuronSimulation instance.
	 */
	NeuronSimulation(Time interval = Time(0))
	    : mValid(false), recorder(params.params, interval){};

	/**
	 * Runs the simulation with the parameters given in the prepare method.
	 *
	 * @params sharedParams is a reference at the shared ParameterCollection
	 * instance.
	 */
	void run(std::shared_ptr<ParameterCollection> sharedParams);

	/**
	 * Returns the parameters.
	 */
	const ParameterCollection &getParameters() const { return params; }

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

	/**
	 * Returns true if a valid result is present.
	 */
	const bool valid() const { return mValid; }
};

using NeuronSimulationResultVec = QVector<NeuronSimulation>;
}

#endif /* _ADEXPSIM_NEURON_SIMULATION_HPP_ */

