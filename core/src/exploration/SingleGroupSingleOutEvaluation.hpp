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

#ifndef _ADEXPSIM_SINGLE_GROUP_SINGLE_OUT_EVALUATION_HPP_
#define _ADEXPSIM_SINGLE_GROUP_SINGLE_OUT_EVALUATION_HPP_

#include <simulation/Parameters.hpp>
#include <simulation/SpikeTrain.hpp>
#include <common/Types.hpp>

#include "EvaluationResult.hpp"
#include "SingleGroupEvaluationBase.hpp"

namespace AdExpSim {
/**
 * The evaluation class can be used for the evalutation of the behaviour of a
 * single neuron for the given SpikeTrain.
 */
class SingleGroupSingleOutEvaluation
    : public SingleGroupEvaluationBase<SingleGroupSingleOutDescriptor> {
private:
	static const EvaluationResultDescriptor descr;

public:
	using SingleGroupEvaluationBase<
	    SingleGroupSingleOutDescriptor>::SingleGroupEvaluationBase;

	/**
	 * Evaluates the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 */
	EvaluationResult evaluate(const WorkingParameters &params) const;

	/**
	 * Returns the evaluation result descriptor for the SingleGroupEvaluation
	 * class.
	 */
	static const EvaluationResultDescriptor &descriptor() { return descr; }
};
}

#endif /* _ADEXPSIM_SINGLE_GROUP_SINGLE_OUT_EVALUATION_HPP_ */

