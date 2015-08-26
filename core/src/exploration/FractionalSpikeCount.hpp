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
 * Tries to implement a measure which associates a neuron parameter set with a
 * fractional spike count.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_FRACTIONAL_SPIKE_COUNT_HPP_
#define _ADEXPSIM_FRACTIONAL_SPIKE_COUNT_HPP_

#include <vector>

#include <common/Types.hpp>
#include <simulation/Spike.hpp>

namespace AdExpSim {
class FractionalSpikeCount {
private:
	bool useIfCondExp;
	Val eTar;
	size_t maxSpikeCount;

public:
	struct Result {
		size_t spikeCount;
		Val eReq;
		Val eRel;

		Result(size_t spikeCount, Val eReq, Val eRel)
		    : spikeCount(spikeCount), eReq(eReq), eRel(eRel)
		{
		}

		Val fracSpikeCount() { return spikeCount + 1.0 - eRel; }
	};

	FractionalSpikeCount(bool useIfCondExp = false, Val eTar = 0.1e-3,
	                     size_t maxSpikeCount = 50)
	    : useIfCondExp(useIfCondExp), eTar(eTar), maxSpikeCount(maxSpikeCount)
	{
	}

	Result calculate(const SpikeVec &spikes, const WorkingParameters &params);
};
}

#endif /* _ADEXPSIM_FRACTIONAL_SPIKE_COUNT_HPP_ */

