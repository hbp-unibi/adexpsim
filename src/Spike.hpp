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
 * Contains the data type describing a spike.
 */

#ifndef _ADEXPSIM_SPIKE_HPP_
#define _ADEXPSIM_SPIKE_HPP_

#include <vector>

#include "Types.hpp"

namespace AdExpSim {

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

}

#endif /* _ADEXPSIM_SPIKE_HPP_ */

