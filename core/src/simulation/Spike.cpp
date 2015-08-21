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
#include <limits>
#include <random>
#include <vector>

#include "Spike.hpp"

namespace AdExpSim {

SpikeVec extractSpikesFrom(const SpikeVec &spikes, Time t)
{
	SpikeVec res;
	for (const Spike &spike: spikes) {
		if (spike.t > t) {
			res.emplace_back(spike.t - t, spike.w);
		}
	}
	return res;
}

SpikeVec buildInputSpikes(Val xi, Time T, Time t0, Val w)
{
	// Calculate the number of spikes
	const size_t c = static_cast<size_t>(ceil(std::max(0.0f, xi)));

	// Create the spikes and return them
	SpikeVec res;
	res.reserve(c);
	for (size_t i = 0; i < c; i++) {
		res.emplace_back(t0 + Time(T.t * i), std::min(1.0f, xi - i) * w);
	}
	return res;
}

SpikeTrain::SpikeTrain(const std::vector<Descriptor> &descrs, size_t n,
                       bool sorted, Time T, Val sigmaT)
    : descrs(descrs), n(n), sorted(sorted), T(T), sigmaT(sigmaT)
{
	rebuild();
}

void SpikeTrain::rebuild(bool randomSeed)
{
	// Clear all internal lists
	spikes.clear();
	ranges.clear();
	rangeStartSpikes.clear();

	// Use the number of descriptors if n is zero
	const size_t nDescrs = descrs.size();
	if (nDescrs == 0) {
		return;
	}
	if (n == 0) {
		n = nDescrs;
	}

	// Random number generator
	std::default_random_engine gen;
	if (randomSeed) {
		static int seed = 22294529;  // Initial seed
		gen.seed(seed);
		seed += 4781536;  // Advance the seed by some number
	}

	// Distribution used to fetch the descriptors
	std::uniform_int_distribution<> distDescr(0, nDescrs - 1);

	// Iterate over all spike trains that should be generated
	Time t;
	Time minT = MAX_TIME;
	size_t idx = 0;
	for (size_t i = 0; i < n; i++) {
		// Fetch a descriptor
		const size_t descrIdx = sorted ? i % nDescrs : distDescr(gen);
		const Descriptor &descr = descrs[descrIdx];

		// Generate nE + nI spikes
		std::vector<Spike> spikeGroup;
		std::normal_distribution<> distT(t.sec(), descr.sigmaT);
		std::normal_distribution<> distW(0, descr.sigmaW);
		for (size_t j = 0; j < descr.nE; j++) {
			spikeGroup.emplace_back(Time::sec(distT(gen)),
			                        descr.wE + distW(gen));
		}
		for (size_t j = 0; j < descr.nI; j++) {
			spikeGroup.emplace_back(Time::sec(distT(gen)),
			                        descr.wI + distW(gen));
		}
		for (const Spike &spike : spikeGroup) {
			minT = std::min(minT, spike.t);
		}

		// Sort the spike group by spike time
		std::sort(spikeGroup.begin(), spikeGroup.end());

		// Allow no spikes during the spike group itself
		if (spikeGroup.size() > 1) {
			rangeStartSpikes.emplace_back(idx);
			ranges.emplace_back(spikeGroup.front().t, i, descrIdx, 0);
		}

		// Remember the number of spikes expected after the spike group
		rangeStartSpikes.emplace_back(idx + spikeGroup.size() - 1);
		ranges.emplace_back(spikeGroup.back().t, i, descrIdx, descr.nOut);

		// Add the spikes to the global spike train
		spikes.insert(spikes.end(), spikeGroup.begin(), spikeGroup.end());

		// Go to the next timestamp and increment the total spike index
		t += Time::sec(fabs(std::normal_distribution<>(T.sec(), sigmaT)(gen)));
		idx += spikeGroup.size();
	}

	// Add a final range at the end
	ranges.emplace_back(t, n, 0, 0);

	// Shift both spikes and ranges by "minT" to have the first spike at zero
	for (Spike &spike : spikes) {
		spike.t -= minT;
	}
	for (Range &range : ranges) {
		range.start -= minT;
	}
}

size_t SpikeTrain::getExpectedOutputSpikeCount() const
{
	size_t res = 0;
	for (const auto &range : ranges) {
		res += range.nSpikes;
	}
	return res;
}

SingleGroupSpikeData SpikeTrain::toSingleGroupSpikeData() const
{
	static constexpr uint16_t NInit = std::numeric_limits<uint16_t>::max();
	static constexpr uint16_t NM1Init = std::numeric_limits<uint16_t>::lowest();

	// Find the threshold values n and nM1 in the descriptor list (note: this
	// is not guaranteed to work -- the descriptors may not contain a clear
	// threshold)
	uint16_t n = std::numeric_limits<uint16_t>::max();
	uint16_t nM1 = std::numeric_limits<uint16_t>::lowest();
	Val sigmaT = 0;
	Val sigmaTM1 = 0;
	for (const auto &descr : descrs) {
		if (descr.nOut == 0) {
			if (descr.nE > nM1) {
				nM1 = descr.nE;
				sigmaTM1 = descr.sigmaT;
			}
		} else if (descr.nOut > 0) {
			if (descr.nE < n) {
				n = descr.nE;
				sigmaT = descr.sigmaT;
			}
		}
	}

	// Make sure some sane values are set
	if (n == NInit) {
		if (nM1 != NM1Init) {
			n = nM1 - 1;
			sigmaT = sigmaTM1;
		} else {
			n = 3;
			nM1 = 2;
			sigmaT = 1e-3;
			sigmaTM1 = 1e-3;
		}
	} else if (nM1 == NM1Init) {
		nM1 = n;
		sigmaTM1 = sigmaT;
	}

	// Create a new SingleGroupSpikeData instance
	return SingleGroupSpikeData(
	    n, nM1, Time::sec(2 * (sigmaT + sigmaTM1) / (n + nM1)), T);
}

void SpikeTrain::fromSingleGroupSpikeData(const SingleGroupSpikeData &data)
{
	// Copy T
	T = data.T;

	// Clear the current descriptor list and create three new descriptors
	descrs.clear();
	Val sigmaT = 0.25 * data.deltaT.sec() * (data.n + data.nM1);
	descrs.emplace_back(data.n, 1, sigmaT);
	descrs.emplace_back(data.nM1, 0, sigmaT);

	// Rebuild
	rebuild(false);
}
}

