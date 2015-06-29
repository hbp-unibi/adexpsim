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
 * @file Evaluation.hpp
 *
 * Contains the implementation of the cost function used as an evaluation
 * measure for the parameter space.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SPIKE_TRAIN_EVALUATION_HPP_
#define _ADEXPSIM_SPIKE_TRAIN_EVALUATION_HPP_

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <common/Types.hpp>

#include "EvaluationResult.hpp"

namespace AdExpSim {
// Forward declaration
struct RecordedSpike;

/**
 * The evaluation class can be used for the evalutation of the behaviour of a
 * single neuron for the given SpikeTrain.
 */
class SpikeTrainEvaluation {
private:
	/**
	 * Used internally as a result of the trackMaxPotential function.
	 */
	struct MaxPotentialResult {
		Val vMax;
		Time tMax;
		Time tLen;

		MaxPotentialResult(Val vMax, Time tMax, Time tLen)
		    : vMax(vMax), tMax(tMax), tLen(tLen){};

		Val tMaxRel() const
		{
			return std::max(0.0, std::min(1.0, 1.0 - tMax.sec() / tLen.sec()));
		}
	};

	/**
	 * Flag used to indicate whether to use the reduced IF_COND_EXP model.
	 */
	bool useIfCondExp;

	/**
	 * SpikeTrain instance on which the evaluation is tested.
	 */
	SpikeTrain train;

	/**
	 * Sigmoid function translating a membrane potential to a
	 * pseudo-probability.
	 */
	Val sigma(Val x, const WorkingParameters &params,
	          bool invert = false) const;

	/**
	 * Measures the theoretically reached, maximum mebrance potential for the
	 * given range. This measurement deactivates the spiking mechanism.
	 */
	MaxPotentialResult trackMaxPotential(const WorkingParameters &params,
	                                     const RecordedSpike &s0, Time tEnd,
	                                     Val eTar) const;

	template <typename F1, typename F2>
	EvaluationResult evaluateInternal(const WorkingParameters &params, Val eTar,
	                                  F1 recordOutputSpike,
	                                  F2 recordOutputGroup) const;

public:
	/**
	 * Structure describing a recorded output spike.
	 */
	struct OutputSpike {
		/**
		 * Time at which the spike was recorded.
		 */
		Time t;

		/**
		 * Index of the group to which the spike belongs.
		 */
		size_t group;

		/**
		 * Flag indicating whether the spike was ok or not.
		 */
		bool ok;

		/**
		 * Default constructor.
		 */
		OutputSpike() {}

		/**
		 * Creates a new OutputSpike instance and fills it with the given
		 * parameters.
		 */
		OutputSpike(Time t, size_t group, bool ok) : t(t), group(group), ok(ok)
		{
		}
	};

	/**
	 * Structure describing the time range of a group of input/output spikes
	 * and whether the spikes in this range fulfilled the binary condition.
	 */
	struct OutputGroup {
		/**
		 * Start time of the group.
		 */
		Time start;

		/**
		 * End time of the group.
		 */
		Time end;

		/**
		 * Index of the spike train template this group belongs to.
		 */
		size_t descrIdx;

		/**
		 * Flag indicating whether all conditions were fulfilled for this group.
		 */
		bool ok;

		/**
		 * Default constructor.
		 */
		OutputGroup(){};

		/**
		 * Creates a new OutputGroup instance and fills it with the given
		 * parameters.
		 *
		 * @param start is the start time.
		 * @param end is the end time.
		 * @param descrIdx is the index of the corresponding
		 *SpikeTrainDescriptor.
		 * @param ok specifies whether all conditions were fulfilled for this
		 * group.
		 */
		OutputGroup(Time start, Time end, size_t descrIdx, bool ok)
		    : start(start), end(end), descrIdx(descrIdx), ok(ok)
		{
		}
	};

	/**
	 * Default constructor.
	 */
	SpikeTrainEvaluation(bool useIfCondExp = false) : useIfCondExp(useIfCondExp)
	{
	}

	/**
	 * Constructor of the evaluation class.
	 *
	 * @param train is the spike train that should be used for the evaluation.
	 */
	SpikeTrainEvaluation(const SpikeTrain &train, bool useIfCondExp = false);

	/**
	 * Evaluates the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 * @param eTar is the target error used in the adaptive stepsize controller.
	 */
	EvaluationResult evaluate(const WorkingParameters &params,
	                          Val eTar = 1e-3) const;

	/**
	 * Evaluates the given parameter set and writes information about the
	 * encountered output spikes to the corresponding list.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 * @param eTar is the target error used in the adaptive stepsize controller.
	 * @param outputSpikes is a list to which the captured output spikes should
	 * be added.
	 * @param outputGroups is a list to which information about the groups
	 * should be written.
	 */
	EvaluationResult evaluate(const WorkingParameters &params,
	                          std::vector<OutputSpike> &outputSpikes,
	                          std::vector<OutputGroup> &outputGroups,
	                          Val eTar = 1e-3) const;

	/**
	 * Returns a reference at the internally used spike train instance.
	 */
	const SpikeTrain &getTrain() const { return train; }
};
}

#endif /* _ADEXPSIM_SPIKE_TRAIN_EVALUATION_HPP_ */

