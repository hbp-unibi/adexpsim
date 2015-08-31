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
 * @file FractionalSpikeCount.hpp
 *
 * Implements a measure which associates a neuron parameter set with a
 * fractional spike count. This evaluation measure lies at the heart of the
 * SingleGroupMultiOutEvaluation class.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_FRACTIONAL_SPIKE_COUNT_HPP_
#define _ADEXPSIM_FRACTIONAL_SPIKE_COUNT_HPP_

#include <limits>
#include <vector>

#include <common/Types.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <simulation/Recorder.hpp>

namespace AdExpSim {

/**
 * Used internally by the FractionalSpikeCount measure to perform the min
 * perturbation analysis.
 */
struct PerturbationAnalysisResult {
	/**
	 * Result of a comparison between an instance of PerturbationAnalysisResult
	 * and a current state.
	 */
	enum class ComparisonResult {
		/**
	     * There will be at most n + 1 output spikes. Might also be less than
	     * n.
	     */
		AT_MOST_NP1,

		/**
	     * There will be at least n + 1 output spikes.
	     */
		AT_LEAST_NP1,

		/**
	     * There will be n output spikes.
	     */
		N,

		/**
	     * There will be at most n output spikes.
	     */
		AT_MOST_N,

		/**
	     * There will be at least n output spikes, might also be n + 1.
	     */
		AT_LEAST_N
	};

	/**
	 * Point in time for which this analysis result was generated.
	 */
	Time t;

	/**
	 * Membrane potential which -- if surpassed by the current state -- means
	 * that at least one additional spike will be generated.
	 */
	Val vUpper;

	/**
	 * Membrane potential which -- if the current voltage is smaller -- means
	 * that no additional spikes will be produced beyond this point in time.
	 */
	Val vLower;

	/**
	 * Adaptation current at this point.
	 */
	Val w;

	/**
	 * Default constructor, makes sure "compare" always returns AT_MOST_NP1,
	 * which is the most general result.
	 */
	PerturbationAnalysisResult()
	    : vUpper(std::numeric_limits<Val>::max()),
	      vLower(std::numeric_limits<Val>::lowest()),
	      w(std::numeric_limits<Val>::lowest())
	{
	}

	/**
	 * Constructor of PerturbationAnalysisResult, allows to set all values.
	 */
	PerturbationAnalysisResult(Time t, Val vUpper, Val vLower, Val w)
	    : t(t), vUpper(vUpper), vLower(vLower), w(w)
	{
	}

	/**
	 * Compares the given current state to this PerturbationAnalysisResult
	 * instance.
	 */
	ComparisonResult compare(const State &s) const;
};

/**
 * Can be used to measure the number of output spikes of a neuron given a
 * certain parameter set and an input spike train. In contrast to just counting
 * the output spikes, FractionalSpikeCount is capable of estimating a fractional
 * number which inidcates how near the current parameter set is to causing
 * an additional output spike. This is important when trying to optimize the
 * neuron parameters with respect to a certain output spike count.
 */
class FractionalSpikeCount {
private:
	/**
	 * Set to true if the simple IF_COND_EXP model should be used.
	 */
	bool useIfCondExp;

	/**
	 * Target error used in the DormandPrinceIntegrator.
	 */
	Val eTar;

	/**
	 * Maximum output spike count before the method aborts.
	 */
	size_t maxSpikeCount;

	/**
	 * Function performing a binary search in order ot find the minimum
	 * perturbation membrane potential which causes another spike.
	 */
	uint16_t minPerturbation(const RecordedSpike &spike, const SpikeVec &spikes,
	                         const WorkingParameters &params, uint16_t vMin,
	                         size_t spikeCount,
	                         std::vector<PerturbationAnalysisResult> &results);

public:
	/**
	 * Result structure containing both the fractional spike counter and other
	 * intermediate results which are of interest for analyzing the method
	 * itself. Simply call its "fracSpikeCount" method in order to obtain the
	 * actual fractional spike count.
	 */
	struct Result {
		/**
		 * Spike count as integer.
		 */
		size_t spikeCount;

		/**
		 * Required voltage boost to cause another spike.
		 */
		Val eReq;

		/**
		 * Required voltage boost relative to the available voltage range.
		 */
		Val pReq;

		/**
		 * Largest local maximum (aside from spikes).
		 */
		Val eMax;

		/**
		 * Largest local maximum relative to the available voltage range.
		 */
		Val pMax;

		/**
		 * Constructor for a non-fractional spike count.
		 */
		Result(size_t spikeCount)
		    : spikeCount(spikeCount), eReq(0.0), pReq(1.0), eMax(0.0), pMax(0.0)
		{
		}

		/**
		 * Constructor of the Result struct, allows to set all members.
		 */
		Result(size_t spikeCount, Val eReq, Val eMax, Val eNorm, Val eSpikeEff)
		    : spikeCount(spikeCount),
		      eReq(eReq),
		      pReq((eReq - eNorm) / (eSpikeEff - eNorm)),
		      eMax(eMax),
		      pMax((eMax - eNorm) / (eSpikeEff - eNorm))
		{
		}

		/**
		 * Converts the internal values into the actual fractional spike count.
		 */
		Val fracSpikeCount()
		{
			return (spikeCount == 0) ? pMax : (spikeCount + 1.0 - pReq);
		}
	};

	/**
	 * Constructor of the FractionalSpikeCount class.
	 *
	 * @param useIfCondExp set to true to use the more simple IF_COND_EXP neuron
	 * model.
	 * @param eTar is the target error to be used in the adaptive stepsize
	 * controller.
	 * @param maxSpikeCount is the maximum number of output spike to occur
	 * before aborting. This is important, as a huge number of output spikes as
	 * they may occur during parameter space exploration may cause significant
	 * slowdowns.
	 */
	FractionalSpikeCount(bool useIfCondExp = false, Val eTar = 0.1e-3,
	                     size_t maxSpikeCount = 50)
	    : useIfCondExp(useIfCondExp), eTar(eTar), maxSpikeCount(maxSpikeCount)
	{
	}

	/**
	 * Calculates the FractionalSpikeCount for the given input spike vector
	 * and the given set of WorkingParameters.
	 */
	Result calculate(const SpikeVec &spikes, const WorkingParameters &params);
};
}

#endif /* _ADEXPSIM_FRACTIONAL_SPIKE_COUNT_HPP_ */

