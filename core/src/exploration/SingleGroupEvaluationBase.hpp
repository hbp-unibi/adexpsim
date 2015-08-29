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
 * @file SingleGroupEvaluationBase.hpp
 *
 * Contains the base class for SingleGroupMultiOutEvaluation and
 * SingleGroupSingleOutEvaluation.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SINGLE_GROUP_EVALUATION_BASE_HPP_
#define _ADEXPSIM_SINGLE_GROUP_EVALUATION_BASE_HPP_

#include <simulation/SpikeTrain.hpp>

namespace AdExpSim {

template <typename SpikeData>
class SingleGroupEvaluationBase {
protected:
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
	 * Common SpikeTrain parameters.
	 */
	SpikeTrainEnvironment env;

	/**
	 * Arguments passed to the evaluation describing
	 */
	SpikeData spikeData;

	/**
	 * Target error to be used for the adaptive stepsize controller.
	 */
	Val eTar;

public:
	/**
	 * Constructor of the evaluation class.
	 *
	 * @param spikeData contains the information about the spikes used for the
	 * single group experiment.
	 * @param useIfCondExp allows to degrade the simulation to the simpler
	 * linear integrate and fire model with conductive synapses.
	 * @param eTar is the target error used in the adaptive stepsize controller.
	 */
	SingleGroupEvaluationBase(const SpikeTrainEnvironment &env,
	                          const SpikeData &spikeData,
	                          bool useIfCondExp = false, Val eTar = 0.1e-3)
	    : sN(spikeData.build(SpikeData::Type::N, env)),
	      sNM1(spikeData.build(SpikeData::Type::NM1, env)),
	      useIfCondExp(useIfCondExp),
	      env(env),
	      spikeData(spikeData),
	      eTar(eTar)
	{
	}
};
}

#endif /* _ADEXPSIM_SINGLE_GROUP_EVALUATION_BASE_HPP_ */

