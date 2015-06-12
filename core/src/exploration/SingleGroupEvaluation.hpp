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
 * @file SingleGroupEvaluation.hpp
 *
 * Contains the implementation of the SingleGroupEvaluation which evaluates the
 * ability of the neuron to fulfill the heaviside condition and the reset 
 * condition.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SINGLE_GROUP_EVALUATION_HPP_
#define _ADEXPSIM_SINGLE_GROUP_EVALUATION_HPP_

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <common/Types.hpp>

#include "EvaluationResult.hpp"

namespace AdExpSim {
/**
 * The evaluation class can be used for the evalutation of the behaviour of a
 * single neuron for the given SpikeTrain.
 */
class SingleGroupEvaluation {
private:
	/**
	 * Vector containing the spikes for n input spikes.
	 */
	SpikeVec sN;

	/**
	 * Vector containing the spikes for n - 1 input spikes.
	 */
	SpikeVec sNM1;

	/**
	 * Flag used to indicate whether to use the reduced IF_COND_EXP model.
	 */
	bool useIfCondExp;

	/**
	 * SpikeTrain instance on which the evaluation is tested.
	 */
	SingleGroupSpikeData spikeData;

	/**
	 * Sigmoid function translating a membrane potential to a
	 * pseudo-probability.
	 */
	Val sigmaV(Val x, Val th,
	          bool invert = false) const;

public:
	/**
	 * Constructor of the evaluation class.
	 *
	 * @param spikeData contains the information about the spikes used for the
	 * single group experiment.
	 */
	SingleGroupEvaluation(const SingleGroupSpikeData &spikeData,
	                      bool useIfCondExp = false);

	/**
	 * Evaluates the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 * @param eTar is the target error used in the adaptive stepsize controller.
	 */
	EvaluationResult evaluate(const WorkingParameters &params,
	                          Val eTar = 1e-3) const;
};
}

#endif /* _ADEXPSIM_SINGLE_GROUP_EVALUATION_HPP_ */

