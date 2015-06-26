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
 * @file Spike.hpp
 *
 * Contains the data type describing a spike and auxiliary functions for
 * generating and scaling input spikes.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SPIKE_HPP_
#define _ADEXPSIM_SPIKE_HPP_

#include <cstdint>
#include <vector>

#include <common/Types.hpp>

#include "Parameters.hpp"

namespace AdExpSim {

/**
 * Structure representing a single spike.
 */
struct Spike {
	Time t;
	Val w;

	Spike() : t(0), w(0.0) {}

	Spike(Time t, Val w) : t(t), w(w) {}

	Spike(Time t) : t(t) {}

	friend bool operator<(const Spike &s1, const Spike &s2)
	{
		return s1.t < s2.t;
	}
};

/**
 * Vector of Spike instances.
 */
using SpikeVec = std::vector<Spike>;

/**
 * Method used to build a set of n input spikes with uniform spacing.
 *
 * @param n is the number of input spikes. If a fractional number is given, an
 * additional input spike is generated which is scaled by the fractional part
 * of the number.
 * @param t is the delay between the spikes.
 * @param t0 is the time at which the first spike should be generated.
 * @param w is the weight factor with which the spike weights are multiplied.
 */
SpikeVec buildInputSpikes(Val n, Time t, Time t0 = Time(0), Val w = 1);

/**
 * The SingleGroupSpikeData class represents the data needed for the
 * SingleGroupEvaluation.
 */
struct SingleGroupSpikeData {
	/**
	 * n is the number of input spikes that should cause an output spike.
	 * Should be larger or equal to one.
	 */
	Val n;

	/**
	 * nM1 is the number of input spikes that should not cause an output spike.
	 * Usually set to n - 1.
	 */
	Val nM1;

	/**
	 * t is the delay between the input spikes.
	 */
	Time deltaT;

	/**
	 * T is the time after which the neuron should have resetted.
	 */
	Time T;

	/**
	 * Default constructor. Initializes all members with sane values.
	 */
	SingleGroupSpikeData() : n(3), nM1(2), deltaT(1e-3_s), T(33e-3_s) {}

	/**
	 * Constructor, sets each member to the given value.
	 */
	SingleGroupSpikeData(Val n, Time deltaT = 1e-3_s, Time T = 33e-3_s)
	    : SingleGroupSpikeData(n, n - 1, deltaT, T)
	{
	}

	/**
	 * Constructor, sets each member to the given value.
	 */
	SingleGroupSpikeData(Val n, Val nM1, Time deltaT = 1e-3_s, Time T = 33e-3_s)
	    : n(n), nM1(nM1), deltaT(deltaT), T(T)
	{
	}

	SpikeVec spikes(Val n) const
	{
		return buildInputSpikes(n, deltaT, 0_s, 1.0);
	}
};

/**
 * The SpikeTrain class is used to construct input spikes as a set of random
 * spike trains which are generated from spike train descriptirs. The SpikeTrain
 * class furthermore generates the ranges in which an output is (or is not)
 * expected and implements a distance measure which compares the spike
 */
class SpikeTrain {
public:
	/**
	 * The Descriptor structure describes a group of input spikes (with
	 * both inhibitory and excitatory spikes). For technical reasons, a
	 * descriptor must contain at least one spike. If this condition is not
	 * fulfilled in the constructor, the descriptor describes an additional
	 * excitatory dummy spike with no weight attached to it.
	 */
	class Descriptor {
	private:
		/**
		 * Function used internally to make sure that the Descriptor contains
		 * at least one spike -- set the number of excitatory spikes to 1 if
		 * there is no spike.
		 */
		static constexpr uint16_t chooseNE(uint16_t nE, uint16_t nI = 0)
		{
			return (nE + nI == 0) ? 1 : nE;
		}

		/**
		 * Function used internally to make sure that the additionally created
		 * input spike (see chooseNE) has no weight and thus does not affect
		 * the result.
		 */
		static constexpr uint16_t chooseWE(Val wE, uint16_t nE, uint16_t nI = 0)
		{
			return (nE + nI == 0) ? 0.0 : wE;
		}

	public:
		/**
		 * Number of excitatory spikes in the spike group.
		 */
		uint16_t nE;

		/**
		 * Number of inhibitory spikes in the spike group.
		 */
		uint16_t nI;

		/**
		 * Weight factor of the excitatory spikes.
		 */
		Val wE;

		/**
		 * Weight factor of the inhibitory spikes.
		 */
		Val wI;

		/**
		 * Standard deviation of the input spikes in seconds.
		 */
		Val sigmaT;

		/**
		 * Standard deviation of the input weights.
		 */
		Val sigmaW;

		/**
		 * Number of expected output spikes.
		 */
		uint16_t nOut;

		/**
		 * Default constructor.
		 */
		Descriptor()
		    : nE(0), nI(0), wE(1.0), wI(1.0), sigmaT(0.0), sigmaW(0.0), nOut(0)
		{
		}

		/**
		 * Constructor for a default excitatory spike group.
		 */
		Descriptor(uint16_t nE, uint16_t nOut, Val sigmaT, Val wE = 1.0,
		           Val sigmaW = 0.0)
		    : nE(chooseNE(nE)),
		      nI(0),
		      wE(chooseWE(wE, nE)),
		      wI(-1.0),
		      sigmaT(sigmaT),
		      sigmaW(sigmaW),
		      nOut(nOut)
		{
		}

		/**
		 * Constructor allowing to initialize all members of the Descriptor
		 * structure.
		 */
		Descriptor(uint16_t nE, uint16_t nI, uint16_t nOut, Val sigmaT, Val wE,
		           Val wI, Val sigmaW)
		    : nE(chooseNE(nE, nI)),
		      nI(nI),
		      wE(chooseWE(wE, nE, nI)),
		      wI(wI),
		      sigmaT(sigmaT),
		      sigmaW(sigmaW),
		      nOut(nOut)
		{
		}
	};

	/**
	 * The Range descriptor describes how many spikes are expected within a
	 * range starting at the given time point.
	 */
	struct Range {
		/**
		 * Start time of this range element.
		 */
		Time start;

		/**
		 * Spike group to which this range belongs.
		 */
		size_t group;

		/**
		 * Spike train descriptor index this range belongs to.
		 */
		size_t descrIdx;

		/**
		 * Number of spikes to occur inside this range.
		 */
		uint16_t nSpikes;

		Range() : group(0), nSpikes(0) {}

		Range(Time start, size_t group, size_t descrIdx, uint16_t nSpikes)
		    : start(start), group(group), descrIdx(descrIdx), nSpikes(nSpikes)
		{
		}

		friend bool operator<(const Range &r1, const Range &r2)
		{
			return r1.start < r2.start;
		}
	};

private:
	/**
	 * Generated input spikes.
	 */
	SpikeVec spikes;

	/**
	 * Range descriptor.
	 */
	std::vector<Range> ranges;

	/**
	 * Indices of the spikes in the spike list that coincide with the start of a
	 * new range.
	 */
	std::vector<size_t> rangeStartSpikes;

	/**
	 * Descriptors given in the constructor.
	 */
	std::vector<Descriptor> descrs;

	/**
	 * Number of spike train groups as given in the constructor.
	 */
	size_t n;

	/**
	 * Set to true, if the spike train groups were drawn sorted.
	 */
	bool sorted;

	/**
	 * Time between the occurance of two spike train groups.
	 */
	Time T;

	/**
	 * Standard deviation of the T parameter.
	 */
	Val sigmaT;

public:
	/**
	 * Default constructor. Creates an empty spike train.
	 */
	SpikeTrain() : SpikeTrain(std::vector<Descriptor>{}) {}

	/**
	 * Constructor of the SpikeTrain class -- calculates a new set of input
	 * spikes trains.
	 *
	 * @param descrs is the set of spike train descriptors from which to
	 * (randomly) choose.
	 * @param n is the number of spike train groups the should be created. If
	 * set to zero, the number of groups is set to the number of descriptors.
	 * @param sorted if set to true, the descriptors are not chosen randomly,
	 * but repeated in the order in which they were given in the descriptor
	 * list.
	 * @param T is the time between two spike train groups.
	 * @param sigmaT is the standard deviation that should be added to the inter
	 * spike train group delay T.
	 */
	SpikeTrain(const std::vector<Descriptor> &descrs, size_t n = 0,
	           bool sorted = true, Time T = 0.0334_s, Val sigmaT = 0.0);

	/**
	 * Builds a new spike train using the parameters given in the constructor.
	 */
	void rebuild(bool randomSeed = false);

	/**
	 * Returns the spike train group descriptors.
	 */
	const std::vector<Descriptor> &getDescrs() const { return descrs; }

	/**
	 * Returns the number of spike train groups.
	 */
	size_t getN() const { return n; }

	/**
	 * Returns whether the spike train was sorted.
	 */
	bool isSorted() const { return sorted; }

	/**
	 * Returns the time between the occurance of two spike train groups.
	 */
	Time getT() const { return T; }

	/**
	 * Returns the standard deviation of the T parameter.
	 */
	Val getSigmaT() const { return sigmaT; }

	/**
	 * Sets the spike train group descriptors.
	 */
	void setDescrs(const std::vector<Descriptor> &descrs)
	{
		this->descrs = descrs;
	}

	/**
	 * Returns the number of spike train groups.
	 */
	void setN(size_t n) { this->n = n; }

	/**
	 * Returns whether the spike train was sorted.
	 */
	void setSorted(bool sorted) { this->sorted = sorted; }

	/**
	 * Returns the time between the occurance of two spike train groups.
	 */
	void setT(Time T) { this->T = T; }

	/**
	 * Returns the standard deviation of the T parameter.
	 */
	void setSigmaT(Val sigmaT) { this->sigmaT = sigmaT; }

	/**
	 * Returns the simulation end time, which is set to the end of the last
	 * spike train group (approximately n * T, depending on sigmaT).
	 */
	Time getMaxT() const
	{
		return ranges.empty() ? Time(0) : ranges.back().start;
	}

	/**
	 * Returns the generated input spikes.
	 */
	const SpikeVec &getSpikes() const { return spikes; }

	/**
	 * Returns the spike ranges, the last element marks the end time.
	 */
	const std::vector<Range> &getRanges() const { return ranges; }

	/**
	 * Returns the indices of the spikes that coincide with the start of a new
	 * range.
	 */
	const std::vector<size_t> &getRangeStartSpikes() const
	{
		return rangeStartSpikes;
	}

	/**
	 * Returns the number of expected output spikes.
	 */
	size_t getExpectedOutputSpikeCount() const;

	/**
	 * Constructs a new SingleGroupSpikeData instance according to the
	 * parameters found in the SpikeTrain.
	 */
	SingleGroupSpikeData toSingleGroupSpikeData() const;

	/**
	 * Adapts this instance to the parameters found in the SingleGroupSpikeData
	 * instance.
	 */
	void fromSingleGroupSpikeData(const SingleGroupSpikeData &data);
};
}

#endif /* _ADEXPSIM_SPIKE_HPP_ */
