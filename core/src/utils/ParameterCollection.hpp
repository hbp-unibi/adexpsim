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

#include <exploration/EvaluationResult.hpp>
#include <simulation/Model.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/SpikeTrain.hpp>

namespace AdExpSim {
/**
 * ParameterCollection contains parameters which describe both the neuron model
 * parameters, the input spike train and information for explorations.
 */
struct ParameterCollection {
	static constexpr Val MIN_HZ = 1;
	static constexpr Val MAX_HZ = 1000;
	static constexpr Val MIN_SEC = 0e-3;
	static constexpr Val MAX_SEC = 100e-3;
	static constexpr Val MIN_S = 0;
	static constexpr Val MAX_S = 1e-7;
	static constexpr Val MIN_A = 0;
	static constexpr Val MAX_A = 1e-9;
	static constexpr Val MIN_V = -0.5;
	static constexpr Val MAX_V = 0.5;

	/**
	 * String list containing the names of the available models. The indices
	 * in the list correspond to the integer values of the ModelType enum.
	 */
	static const std::vector<std::string> modelNames;

	/**
	 * String list containing the names of the available evaluation methods. The
	 * indices in the list correspond to the integer values of the
	 * EvaluationType enum.
	 */
	static const std::vector<std::string> evaluationNames;

	/**
	 * Enum describing the currently used model.
	 */
	ModelType model;

	/**
	 * Enum describing the currently used evaluation model.
	 */
	EvaluationType evaluation;

	/**
	 * Additional data used by all evaluations. Specifies basic properties of
	 * the spike train.
	 */
	SpikeTrainEnvironment environment;

	/**
	 * Holds the setup used in the SingleGroup evaluations.
	 */
	SingleGroupMultiOutDescriptor singleGroup;

	/**
	 * Holds the current SpikeTrain setup. The SpikeTrain defines the input
	 * spikes which are being sent to the neuron. Furthermore it is used in the
	 * input in the Simulation window.
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

	/**
	 * Returns the currently active optimization dimensions.
	 */
	std::vector<size_t> optimizationDims() const;

	/**
	 * Returns the currently active exploration dimensions.
	 */
	std::vector<size_t> explorationDims() const;
};
}

#endif /* _ADEXPSIM_PARAMETER_COLLECTION_HPP_ */

