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
 * @file SingleGroupMultiOutEvaluation.hpp
 *
 * Variant of the SingleGroupEvaluation method which additionally allows the
 * specification of the number of output spikes.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SINGLE_GROUP_MULTI_OUT_EVALUATION_HPP_
#define _ADEXPSIM_SINGLE_GROUP_MULTI_OUT_EVALUATION_HPP_

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <simulation/State.hpp>
#include <common/Types.hpp>

#include "EvaluationResult.hpp"
#include "SingleGroupEvaluation.hpp"

namespace AdExpSim {

/**
 * The evaluation class can be used for the evalutation of the behaviour of a
 * single neuron for the given SpikeTrain.
 */
class SingleGroupMultiOutEvaluation
    : public SingleGroupEvaluationBase<SingleGroupMultiOutSpikeData> {
public:
	using SingleGroupEvaluationBase<
	    SingleGroupMultiOutSpikeData>::SingleGroupEvaluationBase;

	/**
	 * Structure used internally by the spikeCount method.
	 */
	struct SpikeCountResult {
		size_t spikeCount;
		Val fracSpikeCount;
		Val vMax0;
		Val vMax1;

		SpikeCountResult(size_t spikeCount)
		    : spikeCount(spikeCount),
		      fracSpikeCount(0.0),
		      vMax0(0.0),
		      vMax1(0.0)
		{
		}

		Val totalSpikeCount() { return spikeCount + fracSpikeCount; }
	};

	/**
	 * Calculates the maximum potential for the given parameters starting from
	 * the specified state with disabled spiking.
	 */
	Val maximumPotential(const SpikeVec &spikes,
	                     const WorkingParameters &params,
	                     const State &state = State(), const Time t0 = Time(0),
	                     const Time lastSpikeTime = Time(-1)) const;

	/**
	 * Calculates the fractional number of output spikes for the given
	 * parameters.
	 */
	SpikeCountResult spikeCount(const SpikeVec &spikes,
	                            const WorkingParameters &params) const;

	/**
	 * Evaluates the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 * @param eTar is the target error used in the adaptive stepsize controller.
	 */
	EvaluationResult evaluate(const WorkingParameters &params) const;
};
}

#endif /* _ADEXPSIM_SINGLE_GROUP_MULTI_OUT_EVALUATION_HPP_ */

