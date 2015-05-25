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
#include <utils/Types.hpp>

namespace AdExpSim {
// Forward declaration
struct RecordedSpike;

/**
 * Structure containing the result of a call to the "evaluate" method of the
 * Evaluation class.
 */
struct SpikeTrainEvaluationResult {
	/**
	 * Percentage of spike train groups for which all conditions were fulfilled.
	 * Ranges between 0.0 and 1.0.
	 */
	Val pBinary;

	/**
	 * Soft expectation ratio -- same as the binary ratio, but takes the maximum
	 * voltage that is theoretically reached in each range into account,
	 * creating a smooth function. Ranges between 0.0 and 1.0.
	 */
	Val pSoft;

	/**
	 * Default constructor. Initializes the values with zeros.
	 */
	SpikeTrainEvaluationResult() : pBinary(0.0), pSoft(0.0) {}

	/**
	 * Constructor of the EvaluationResult class. Initializes all members with
	 * the given values.
	 */
	SpikeTrainEvaluationResult(Val pBinary, Val pSoft)
	    : pBinary(pBinary), pSoft(pSoft)
	{
	}
};

/**
 * The evaluation class can be used for the evalutation of the behaviour of a
 * single neuron for the given SpikeTrain.
 */
class SpikeTrainEvaluation {
private:
	/**
	 * SpikeTrain instance on which the evaluation is tested.
	 */
	SpikeTrain train;

	/**
	 * Sigmoid function translating a membrane potential to a
	 * pseudo-probability.
	 */
	static Val sigma(Val x, const WorkingParameters &params,
	                 bool invert = false);

	/**
	 * Measures the theoretically reached, maximum mebrance potential for the
	 * given range. This measurement deactivates the spiking mechanism.
	 */
	std::pair<Val, Time> trackMaxPotential(const WorkingParameters &params,
	                                       const RecordedSpike &s0, Time tEnd,
	                                       Val eTar) const;

	template <typename Function>
	SpikeTrainEvaluationResult evaluateInternal(
	    const WorkingParameters &params, Val eTar,
	    Function recordOutputSpike) const;

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
		Time start, end;
		bool ok;

		OutputGroup(Time start, Time end, bool ok) : start(start), end(end), ok(ok) {}
	};

	/**
	 * Constructor of the evaluation class.
	 *
	 * @param train is the spike train that should be used for the evaluation.
	 */
	SpikeTrainEvaluation(const SpikeTrain &train);

	/**
	 * Evaluates the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 * @param eTar is the target error used in the adaptive stepsize controller.
	 */
	SpikeTrainEvaluationResult evaluate(const WorkingParameters &params,
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
	SpikeTrainEvaluationResult evaluate(const WorkingParameters &params,
	                                    std::vector<OutputSpike> &outputSpikes,
	                                    std::vector<OutputGroup> &outputGroups,
	                                    Val eTar = 1e-3) const;

	/**
	 * Returns a reference at the internally used spike train instance.
	 */
	const SpikeTrain &getTrain() { return train; }
};
}

#endif /* _ADEXPSIM_SPIKE_TRAIN_EVALUATION_HPP_ */

