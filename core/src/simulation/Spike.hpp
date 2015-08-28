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

namespace AdExpSim {

/**
 * Structure representing a single input spike.
 */
struct Spike {
	/**
	 * Enum describing the two possible spike types: Inhibitory and excitatory
	 * spikes. Generally spike which arrive at a synapse with a positive
	 * synaptic weight are excitatory, spikes which arrive at a synapse with
	 * negative synaptic weight are inhibitory.
	 */
	enum class Type {
		INHIBITORY, EXCITATORY
	};

	/**
	 * Time at which the spike is received by the neuron.
	 */
	Time t;

	/**
	 * Weight of the spike -- can be understood as the weight of the synaptic
	 * connection the spike arrives at.
	 */
	Val w;

	/**
	 * Constructor of the Spike class, allows to initialize all members.
	 *
	 * @param t is the time at which the spike is received by the neuron.
	 * @param w is the weight of the spike.
	 */
	constexpr Spike(Time t = Time(0), Val w = 0.0) : t(t), w(w) {}

	/**
	 * Returns whether this is an inhibitory or excitatory spike.
	 */
	Type type() const {
		return (w < 0.0) ? Type::INHIBITORY : Type::EXCITATORY;
	}

	/**
	 * Operator used to sort spikes by time. This way a vector of spikes can be
	 * easily sorted using the STL methods.
	 */
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
}

#endif /* _ADEXPSIM_SPIKE_HPP_ */

