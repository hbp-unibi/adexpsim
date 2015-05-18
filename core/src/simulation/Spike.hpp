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

#include <utils/Types.hpp>

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

	friend bool operator<(const Spike &s1, const Spike &s2)
	{
		return s1.t < s2.t;
	}
};

using SpikeVec = std::vector<Spike>;

/**
 * Method used to build a set of N input spikes with uniform spacing.
 *
 * @param N is the number of input spikes. If a fractional number is given, an
 * additional input spike is generated which is scaled by the fractional part
 * of the number.
 * @param T is the delay between the spikes.
 * @param t0 is the time at which the first spike should be generated.
 * @param w is the weight factor with which the spike weights are multiplied.
 */
SpikeVec buildInputSpikes(Val N, Time T, Time t0 = 0, Val w = 1);

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
	 * both inhibitory and excitatory spikes).
	 */
	struct Descriptor {
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
		Val sigma;

		/**
		 * Number of expected output spikes.
		 */
		uint16_t nOut;

		/**
		 * Default constructor.
		 */
		Descriptor() : nE(0), nI(0), wE(1.0), wI(1.0), sigma(1.0), nOut(0) {}

		/**
		 * Constructor for a default excitatory spike group.
		 */
		Descriptor(uint16_t nE, uint16_t nOut, Val sigma, Val wE = 1.0)
		    : nE(nE), nI(0), wE(wE), wI(1.0), sigma(sigma), nOut(nOut)
		{
		}

		/**
		 * Constructor allowing to initialize all members of the Descriptor
		 * structure.
		 */
		Descriptor(uint16_t nE, uint16_t nI, uint16_t nOut, Val sigma, Val wE,
		           Val wI)
		    : nE(nE), nI(nI), wE(wE), wI(wI), sigma(sigma), nOut(nOut)
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
		 * Number of spikes to occur inside this range.
		 */
		uint16_t nSpikes;

		Range() : nSpikes(0) {}

		Range(Time start, uint16_t nSpikes) : start(start), nSpikes(nSpikes) {}

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

public:
	/**
	 * Constructor of the SpikeTrain class -- calculates a new set of input
	 * spikes trains.
	 *
	 * @param descrs is the set of spike train descriptors from which to
	 * (randomly) choose.
	 * @param
	 */
	SpikeTrain(const std::vector<Descriptor> &descrs, size_t n, Time T,
	           Val sigmaT);

	const Time getMaxT() const { return getRanges().back().start; }

	/**
	 * Returns the generated input spikes.
	 */
	const SpikeVec &getSpikes() const { return spikes; }

	/**
	 * Returns the spike ranges, the last element marks the end time.
	 */
	const std::vector<Range> &getRanges() const { return ranges; }
};
}

#endif /* _ADEXPSIM_SPIKE_HPP_ */
