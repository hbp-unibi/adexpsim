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

	friend bool operator<(const Spike &s1, const Spike &s2) {
		return s1.t < s2.t;
	}
};

using SpikeVec = std::vector<Spike>;

/**
 * Method used to build a set of xi input spikes.
 *
 * @param xi is the number of input spikes. If a fractional number is given, an
 * additional input spike is generated which is scaled by the fractional part
 * of the number.
 * @param T is the delay between the spikes.
 * @param t0 is the time at which the first spike should be generated.
 * @param w is the weight factor with which the spike weights are multiplied.
 */
SpikeVec buildInputSpikes(Val xi, Time T, Time t0 = 0, Val w = 1);

}

#endif /* _ADEXPSIM_SPIKE_HPP_ */

