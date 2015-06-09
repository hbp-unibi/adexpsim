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
 * @file ParameterCollection.hpp
 *
 * Contains the ParameterCollection class which contains parameters for both
 * the model, the exploration and the optimization process.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_PARAMETER_COLLECTION_HPP_
#define _ADEXPSIM_PARAMETER_COLLECTION_HPP_

#include <array>
#include <string>
#include <vector>

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>

namespace AdExpSim {
/**
 * The ModelType enum defines the model that should be used in the simulation.
 */
enum class ModelType: int {
	/**
	 * Represents the simple IF_COND_EXP model (integrate & fire conductance
	 * based with exponential decay).
	 */
	IF_COND_EXP = 0,

	/**
	 * Represents the simple AD_IF_COND_EXP model (integrate & fire conductance
	 * based with exponential decay and adaptation mechanism).
	 */
	AD_IF_COND_EXP = 1
};

/**
 * The EvaluationType enum defines the evaluation method which is going to be
 * used in the simulation.
 */
enum class EvaluationType: int {
	/**
	 * Uses a spike train template for the evaluation -- determines how well the
	 * actual simulation matches a predefined spike train.
	 */
	SPIKE_TRAIN = 0,

	/**
	 * Only uses a single spike gorup for the evaluation and checks it for 
	 * fulfilling the binary threshold and reset condition.
	 */
	SINGLE_GROUP = 1
};

/**
 * ParameterCollection contains parameters which describe both the neuron model
 * parameters, the input spike train and information for explorations.
 */
struct ParameterCollection {
	/**
	 * String list containing the names of the available models. The indices
	 * in the list correspond to the integer values of the ModelType enum.
	 */
	static std::vector<std::string> modelNames;

	/**
	 * String list containing the names of the available evaluation methods. The
	 * indices in the list correspond to the integer values of the
	 * EvaluationType enum.
	 */
	static std::vector<std::string> evaluationNames;

	/**
	 * Enum describing the currently used model.
	 */
	ModelType model;

	/**
	 * Enum describing the currently used evaluation model.
	 */
	EvaluationType evaluation;

	/**
	 * Holds the current SpikeTrain setup. The SpikeTrain defines the input
	 * spikes which are being sent to the 
	 */
	SpikeTrain train;

	/**
	 * Holds the current model Parameters.
	 */
	Parameters params;

	/**
	 * Minimum/maximum model Parameters as used for the sliders in the GUI and
	 * the optimization.
	 */
	WorkingParameters min, max;

	/**
	 * Flags determining whether the corresponding working parameters are 
	 * optimized in the optimization process or not.
	 */
	std::array<bool, WorkingParameters::Size> optimize;

	/**
	 * Flags determining whether the corresponding working parameters are 
	 * explored in an offline exploration run.
	 */
	std::array<bool, WorkingParameters::Size> explore;

	/**
	 * Default constructor. Initializes all variables with sane default values.
	 */
	ParameterCollection();
};
}

#endif /* _ADEXPSIM_PARAMETER_COLLECTION_HPP_ */

