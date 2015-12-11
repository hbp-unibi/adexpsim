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
#include <random>

#include "SpikeTrain.hpp"

namespace AdExpSim {

/* Static functions */

/**
 * Used internally to initialize a default_random_engine instance with the value
 * at the given seed.
 */
static std::default_random_engine initializeRandomEngine(size_t *seed)
{
	std::default_random_engine gen;

	static size_t internalSeed = 22294529;
	seed = seed ? seed : &internalSeed;
	gen.seed(*seed);
	*seed += 4781536;  // Advance the seed by some number

	return gen;
}

/**
 * Updates pointers at a minimum and maximum value.
 */
template <typename T>
static void updateMinMax(T val, T *min, T *max)
{
	if (min != nullptr && val < *min) {
		*min = val;
	}
	if (max != nullptr && val > *max) {
		*max = val;
	}
}

/* Global functions */

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

SpikeVec &buildSpikeGroup(SpikeVec &spikes, Val w, size_t nBursts,
                          const SpikeTrainEnvironment &env, bool equidistant,
                          Time t0, Time *tMin, Time *tMax, size_t *seed)
{
	// Calculate the deltaT for equidistant distribution
	const Time deltaTEqn = Time(2 * (env.sigmaT.t + env.sigmaTOffs.t) /
	                            std::max<size_t>(1, nBursts));

	// Random number generator
	std::default_random_engine gen = initializeRandomEngine(seed);
	std::normal_distribution<> distT(0, env.sigmaT.sec());
	std::normal_distribution<> distTOffs(0, env.sigmaTOffs.sec());
	std::normal_distribution<> distW(w, env.sigmaW);

	// Iterate over each spike in the burst
	for (size_t i = 0; i < env.burstSize; i++) {
		// Realise each spike nBursts times, either adding some jitter or
		// distributing them equidistantly
		const Time tOffs = equidistant ? 0_s : Time::sec(distTOffs(gen));
		const Time tBase = t0 + tOffs + Time(env.deltaT.t * i);
		for (size_t j = 0; j < nBursts; j++) {
			// Choose a spike time
			Time t;
			if (equidistant) {
				t = tBase + Time(deltaTEqn.t * j);
			} else {
				t = tBase + Time::sec(distT(gen));
			}
			updateMinMax(t, tMin, tMax);

			// Insert the spike into the list
			spikes.emplace_back(t, distW(gen));
		}
	}

	// Make sure the result vector is still sorted
	std::sort(spikes.begin(), spikes.end());
	return spikes;
}

SpikeVec buildSpikeGroup(Val w, size_t nBursts,
                         const SpikeTrainEnvironment &env, bool equidistant,
                         Time t0, Time *tMin, Time *tMax, size_t *seed)
{
	SpikeVec res;  // Result spike vector
	return buildSpikeGroup(res, w, nBursts, env, equidistant, t0, tMin, tMax,
	                       seed);
}

/* Class GenericGroupDescriptor */

SpikeVec &GenericGroupDescriptor::build(SpikeVec &spikes, Spike::Type type,
                                        const SpikeTrainEnvironment &env,
                                        bool equidistant, Time t0, Time *tMin,
                                        Time *tMax, size_t *seed) const
{
	switch (type) {
		case Spike::Type::EXCITATORY:
			return buildSpikeGroup(spikes, wE, nE, env, equidistant, t0, tMin,
			                       tMax, seed);
		case Spike::Type::INHIBITORY:
			return buildSpikeGroup(spikes, -wI, nI, env, equidistant, t0, tMin,
			                       tMax, seed);
	}
	return spikes;
}

/* Class SingleGroupSingleOutDescriptor */

SpikeVec &SingleGroupSingleOutDescriptor::build(
    SpikeVec &spikes, SingleGroupSingleOutDescriptor::Type type,
    const SpikeTrainEnvironment &env, Time t0, Time *tMin, Time *tMax) const
{
	return buildSpikeGroup(spikes, 1.0, type == Type::N ? n : nM1, env, true,
	                       t0, tMin, tMax);
}

/* Class SpikeTrain */

void SpikeTrain::rebuild()
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

	// Distribution used to fetch the descriptors
	std::default_random_engine gen = initializeRandomEngine(nullptr);
	std::uniform_int_distribution<> distDescr(0, nDescrs - 1);

	// Iterate over all spike trains that should be generated
	Time t, lastStart = MIN_TIME;
	size_t idx = 0;
	for (size_t i = 0; i < n; i++) {
		// Fetch a descriptor
		const size_t descrIdx = sorted ? i % nDescrs : distDescr(gen);
		GenericGroupDescriptor &descr = descrs[descrIdx];
		descr.adjust();  // Make sure the descriptor has at least one spike

		// Generate the inhibitory and the excitatory spikes
		Time tMin = MAX_TIME, tMax = MIN_TIME;
		descr.build(spikes, Spike::Type::EXCITATORY, env, equidistant, t, &tMin,
		            &tMax);
		descr.build(spikes, Spike::Type::INHIBITORY, env, equidistant, t, &tMin,
		            &tMax);

		// Remember the first spike as a "range start spike", add a range for
		// the group
		rangeStartSpikes.emplace_back(idx);
		ranges.emplace_back(std::max(lastStart, tMin), i, descrIdx,
		                    descr.nOut * env.burstSize);

		// Go to the next timestamp and increment the total spike index
		lastStart = tMin;
		t += env.T;
		idx = spikes.size();
	}

	// Add a final range at the end
	ranges.emplace_back(t, n, 0, 0);

	// Shift both spikes and ranges by "minT" to have the first spike at zero
	Time minT = spikes[0].t;
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
		res += range.nOut;
	}
	return res;
}

void SpikeTrain::fromSingleGroupSpikeData(
    const SingleGroupMultiOutDescriptor &data)
{
	descrs.clear();
	descrs.emplace_back(data.n, data.nOut);
	descrs.emplace_back(data.nM1, 0);
	rebuild();
}
}

