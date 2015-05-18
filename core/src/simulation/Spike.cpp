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

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "Spike.hpp"

namespace AdExpSim {

SpikeVec buildInputSpikes(Val xi, Time T, Time t0, Val w)
{
	// Calculate the number of spikes
	const size_t c = static_cast<size_t>(ceil(std::max(0.0f, xi)));

	// Create the spikes and return them
	SpikeVec res;
	res.reserve(c);
	for (size_t i = 0; i < c; i++) {
		res.emplace_back(t0 + TimeType(T.t * i), std::min(1.0f, xi - i) * w);
	}
	return res;
}

SpikeTrain::SpikeTrain(const std::vector<Descriptor> &descrs, size_t n, Time T,
                       Val sigmaT)
{
	std::default_random_engine gen;

	// Distribution used to fetch the descriptors
	std::uniform_int_distribution<> distDescr(0, descrs.size() - 1) ;

	// Iterate over all spike trains that should be generated
	Time t(TimeType(0));
	for (size_t i = 0; i < n; i++) {
		// Fetch a descriptor
		const Descriptor &descr = descrs[distDescr(gen)];

		// Generate nE + nI spikes
		std::vector<Spike> spikeGroup;
		std::normal_distribution<> distT(t.toSeconds(), descr.sigma);
		for (size_t i = 0; i < descr.nE; i++) {
			spikeGroup.emplace_back(distT(gen), descr.wE);
		}
		for (size_t i = 0; i < descr.nI; i++) {
			spikeGroup.emplace_back(distT(gen), descr.wI);
		}

		// Sort the spike group by spike time
		std::sort(spikeGroup.begin(), spikeGroup.end());

		// Allow no spikes during the spike group itself
		if (spikeGroup.size() > 1) {
			ranges.emplace_back(spikeGroup.front().t, 0);
		}

		// Remember the number of spikes expected after the spike group
		ranges.emplace_back(spikeGroup.back().t, descr.nOut);

		// Add the spikes to the global spike train
		spikes.insert(spikes.end(), spikeGroup.begin(), spikeGroup.end());

		// Go to the next timestamp
		t += std::normal_distribution<>(T.toSeconds(), sigmaT)(gen);
	}

	// Add a final range at the end
	ranges.emplace_back(t + T, 0);
}
}

