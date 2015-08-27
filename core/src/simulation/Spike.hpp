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

#include <cmath>
#include <cstdint>
#include <vector>

#include <common/Types.hpp>

#include "Parameters.hpp"

namespace AdExpSim {

/**
 * Structure representing a single input spike.
 */
struct Spike {
	/**
	 * Time at which the spike is received by the neuron.
	 */
	Time t;

	/**
	 * Weight of the spike -- can be understood as the weight of the synaptic
	 * connection the spike came from.
	 */
	Val w;

	constexpr Spike(Time t = Time(0), Val w = 0.0) : t(t), w(w) {}

	friend bool operator<(const Spike &s1, const Spike &s2)
	{
		return s1.t < s2.t;
	}
};

/**
 * Class used to build a "SpecialSpike". Special spikes are used to inject
 * additional timed information into a neuron simulation. Instances of
 * SpecialSpike can be converted back and from the Spike class without any
 * loss of information. The spike type is hidden in the weight parameter.
 * Processing of special spikes must be enabled in Model::simulate() using
 * the corresponding template parameter.
 */
class SpecialSpike : public Spike {
public:
	/**
	 * Enum containing possible kinds of special spikes.
	 */
	enum class Kind : uint8_t {
		/**
	     * Tells the neuron simulation to generate an output spike. Triggers
	     * the usual reset mechanism, including refractory period.
	     */
		FORCE_OUTPUT_SPIKE = 0,

		/**
	     * Sets the voltage to the given value. The voltage is encoded as a
	     * 16-bit integer which ranges from eReset (= 0) to eSpikeEff (= MAX).
	     */
		SET_VOLTAGE = 1,
	};

private:
	union Single {
		float f;
		uint32_t i;

		constexpr Single(float f) : f(f) {}
		constexpr Single(uint32_t i) : i(i) {}
	};

	static constexpr uint32_t NaN32 = 0x7FC00000;
	static constexpr uint64_t NaN64 = 0x7FF8000000000000L;

	union Double {
		double f;
		uint64_t i;

		constexpr Double(double f) : f(f) {}
		constexpr Double(uint64_t i) : i(i) {}
	};

	static constexpr float encode(float, Kind kind, uint16_t payload)
	{
		return Single(uint32_t(NaN32 | uint8_t(kind) | (payload << 4))).f;
	}

	static constexpr double encode(double, Kind kind, uint16_t payload)
	{
		return Double(uint64_t(NaN64 | uint8_t(kind) | (payload << 4))).f;
	}

	static constexpr bool isSpecial(float f)
	{
		return (Single(f).i & NaN32) == NaN32;
	}

	static constexpr bool isSpecial(double f)
	{
		return (Double(f).i & NaN64) == NaN64;
	}

	static constexpr Kind decodeKind(float f)
	{
		return static_cast<Kind>(Single(f).i & 0x0F);
	}

	static constexpr Kind decodeKind(double f)
	{
		return static_cast<Kind>(Double(f).i & 0x0F);
	}

	static constexpr uint16_t decodePayload(float f)
	{
		return (Single(f).i & 0xFFFF0) >> 4;
	}

	static constexpr uint16_t decodePayload(double f)
	{
		return (Double(f).i & 0xFFFF0) >> 4;
	}

public:
	/**
	 * Creates a new "SpecialSpike".
	 *
	 * @param t time at which the SpecialSpike instance should be processed by
	 * the neuron.
	 * @param kind is the kind of SpecialSpike the new instance should
	 * represent.
	 */
	constexpr SpecialSpike(Time t, Kind kind, uint16_t payload = 0)
	    : Spike(t, encode(kind, payload)){};

	/**
	 * Returns true if the spike encodes special information. The information
	 * can be accessed via the "kind" method.
	 */
	static constexpr bool isSpecial(const Spike &spike)
	{
		return isSpecial(spike.w);
	}

	/**
	 * Returns the kind of the spike. The result is only valid if isSpecial()
	 * returns true.
	 */
	static constexpr Kind kind(const Spike &spike)
	{
		return decodeKind(spike.w);
	}

	/**
	 * Returns the payload attached to the spike data.
	 */
	static constexpr uint16_t payload(const Spike &spike)
	{
		return decodePayload(spike.w);
	}

	/**
	 * Encodes the special spike kind and its payload in a spike weight.
	 */
	static constexpr Val encode(Kind kind, uint16_t payload)
	{
		return encode(Val(), kind, payload);
	}

	static uint16_t encodeSpikeVoltage(Val v, Val vMin, Val vMax)
	{
		return (std::min(vMax, std::max(vMin, v)) - vMin) *
		       std::numeric_limits<uint16_t>::max() / (vMax - vMin);
	}

	static Val decodeSpikeVoltage(uint16_t v, Val vMin, Val vMax)
	{
		return vMin + (vMax - vMin) * v / std::numeric_limits<uint16_t>::max();
	}

	/**
	 * Returns true if the Spike encodes any special information.
	 */
	bool isSpecial() const { return isSpecial(*this); }

	/**
	 * Returns the kind of the special spike. Only valid if isSpecial() returns
	 * true.
	 */
	Kind kind() const { return kind(*this); }

	/**
	 * Returns the payload attached to the spike data.
	 */
	uint16_t payload() const { return payload(*this); }
};

/**
 * Vector of Spike instances.
 */
using SpikeVec = std::vector<Spike>;

/**
 * Returns a subset of spikes from the given spikes list containing those whose
 * spike time is larger than the given t. The spike times in the resulting list
 * are shifted back by t.
 *
 * @param spikes is the list from which a subset of spikes should be extracted.
 * @param t is the lower, non-inclusive boundary the spikes have to surpass and
 * the time by which the spike times are shifted back in the resulting list.
 */
SpikeVec extractSpikesFrom(const SpikeVec &spikes, Time t);

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
 * The SingleGroupSpikeData class contains the arguments needed for the
 * SingleGroupEvaluation.
 */
struct SingleGroupSpikeData {
	/**
	 * n is the number of input patches that should cause an output spike.
	 * Should be larger or equal to one.
	 */
	Val n;

	/**
	 * nM1 is the number of input patches that should not cause an output spike.
	 * Usually set to n - 1.
	 */
	Val nM1;

	/**
	 * Number of spikes in a single patch.
	 */
	Val nPatch;

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
	SingleGroupSpikeData() : n(3), nM1(2), nPatch(1), deltaT(1e-3_s), T(33e-3_s)
	{
	}

	/**
	 * Constructor, sets each member to the given value.
	 */
	SingleGroupSpikeData(Val n, Val nPatch = 1, Time deltaT = 1e-3_s,
	                     Time T = 33e-3_s)
	    : SingleGroupSpikeData(n, n - 1, nPatch, deltaT, T)
	{
	}

	/**
	 * Constructor, sets each member to the given value.
	 */
	SingleGroupSpikeData(Val n, Val nM1, Val nPatch = 1, Time deltaT = 1e-3_s,
	                     Time T = 33e-3_s)
	    : n(n), nM1(nM1), nPatch(nPatch), deltaT(deltaT), T(T)
	{
	}

	/**
	 * Constructs an input spike vector according to the parameters stored in
	 * this class.
	 *
	 * @param n is the number of input spikes. Should be set to n or nM1.
	 */
	SpikeVec spikes(Val n) const
	{
		return buildInputSpikes(n, deltaT, 0_s, nPatch);
	}
};

/**
 * The SingleGroupMultiOutSpikeData class contains the arguments required by the
 * SingleGroupMultiOutEvaluation class.
 */
struct SingleGroupMultiOutSpikeData : public SingleGroupSpikeData {
	/**
	 * Number of expected output spikes.
	 */
	Val nOut;

	/**
	 * Default constructor. Initializes all members with sane values, expects
	 * a single output spike.
	 */
	SingleGroupMultiOutSpikeData() : SingleGroupSpikeData(), nOut(1) {}

	/**
	 * Constructor, sets each member to the given value.
	 */
	SingleGroupMultiOutSpikeData(Val n, Val nOut, Time deltaT = 1e-3_s,
	                             Time T = 33e-3_s)
	    : SingleGroupMultiOutSpikeData(n, n - 1, nOut, deltaT, T)
	{
	}

	/**
	 * Constructor, sets each member to the given value.
	 */
	SingleGroupMultiOutSpikeData(Val n, Val nM1, Val nOut, Time deltaT = 1e-3_s,
	                             Time T = 33e-3_s)
	    : SingleGroupSpikeData(n, nM1, deltaT, T), nOut(nOut)
	{
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
	 * Set to true if equal spacing should be used, just as in the SingleGroup
	 * simulation.
	 */
	bool equidistant;

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
	 * @param equidistant if true, creates equidistant input spikes just as in
	 * the SingleGroup simulation.
	 */
	SpikeTrain(const std::vector<Descriptor> &descrs, size_t n = 0,
	           bool sorted = true, Time T = 0.0334_s, Val sigmaT = 0.0,
	           bool equidistant = false);

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
	 * Returns whether the input spikes are drawn equidistant.
	 */
	bool isEquidistant() const { return equidistant; }

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
	 * Sets the "sorted" flag.
	 */
	void setSorted(bool sorted) { this->sorted = sorted; }

	/**
	 * Sets the "equidistant" flag.
	 */
	void setEquidistant(bool equidistant) { this->equidistant = equidistant; }

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
	SingleGroupMultiOutSpikeData toSingleGroupSpikeData() const;

	/**
	 * Adapts this instance to the parameters found in the SingleGroupSpikeData
	 * instance.
	 */
	void fromSingleGroupSpikeData(const SingleGroupMultiOutSpikeData &data);
};
}

#endif /* _ADEXPSIM_SPIKE_HPP_ */
