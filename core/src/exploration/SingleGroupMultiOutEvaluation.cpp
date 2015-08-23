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
#include <vector>

#include <common/ProbabilityUtils.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>

#include "SingleGroupMultiOutEvaluation.hpp"

namespace AdExpSim {

// static Time tDelta(-1);
static Time tDelta = 0.1e-3_s;

/* Class SingleGroupMultiOutEvaluation */

static LocalMaximumRecorder::Maximum const *nextLocalMaximum(
    const LocalMaximumRecorder &recorder, const SpikeVec &inputSpikes)
{
	static constexpr Time MIN_TIME_DISTANCE = 0.001e-3_s;
	LocalMaximumRecorder::Maximum const *res = nullptr;
	for (auto const &max : recorder.maxima) {
		// Compute the distance of the maximum to any already inserted special
		// spike
		Time dist = MAX_TIME;
		for (auto const &spike : inputSpikes) {
			if (SpecialSpike::isSpecial(spike)) {
				dist = Time(std::min(std::abs(spike.t.t - max.t.t), dist.t));
			}
		}

		// Check whether the membrane potential of this maximum is larger than
		// the membrane potential of the current result
		if (dist > MIN_TIME_DISTANCE &&
		    (res == nullptr || max.s.v() > res->s.v())) {
			res = &max;
		}
	}
	return res;
}

SingleGroupMultiOutEvaluation::SpikeCountResult
SingleGroupMultiOutEvaluation::spikeCount(const SpikeVec &spikes,
                                          const WorkingParameters &params) const
{
	// Construct the recorder -- record both output spikes and local maxima
	OutputSpikeCountRecorder spikeRecorder;
	LocalMaximumRecorder maximumRecorder;
	auto recorder = makeMultiRecorder(spikeRecorder, maximumRecorder);

	// Copy the input spikes
	SpikeVec inputSpikes = spikes;

	// Spike count counter for the next iteration
	size_t lastSpikeCount = std::numeric_limits<size_t>::max();

	const Val eSpikeEff = params.eSpikeEff(useIfCondExp);  // Spike threshold
	Val eDelta = 0.0;  // Total potential that needs to be bridged
	while (true) {
		// Reset the recorder
		recorder.reset();

		// Run a simulation
		// RungeKuttaIntegrator integrator;
		DormandPrinceIntegrator integrator(eTar);
		DefaultController controller;
		if (useIfCondExp) {
			Model::simulate<Model::IF_COND_EXP | Model::PROCESS_SPECIAL>(
			    inputSpikes, recorder, controller, integrator, params, tDelta);
		} else {
			Model::simulate<Model::FAST_EXP | Model::PROCESS_SPECIAL>(
			    inputSpikes, recorder, controller, integrator, params, tDelta);
		}

		// Count the number of output spikes, if the number of output spikes
		// is larger than the last number of output spikes, we're done
		if (spikeRecorder.count > lastSpikeCount) {
			break;
		}

		// Make the current spike count the last spike count
		lastSpikeCount = spikeRecorder.count;

		// Search the next local maximum which is clear appart from the already
		// created special output spikes
		auto max = nextLocalMaximum(maximumRecorder, inputSpikes);
		if (max != nullptr) {
//			std::cout << "-> " << max->t << ", " << max->s.v() << std::endl;
			// Insert a forced output spike at largest maximum
			inputSpikes.push_back(
			    SpecialSpike(max->t, SpecialSpike::Kind::FORCE_OUTPUT_SPIKE));
			std::sort(inputSpikes.begin(), inputSpikes.end());

			// Increase eDelta by the distance of the maximum to the effective
			// spike potential -- this represents the "effort" that is needed
			// to produce the forced spike.
			eDelta += eSpikeEff - max->s.v();
		} else {
			// Just use the the last state of the maximumRecorder if no local
			// maximum exists. This might be the case if the membrane slowly
			// approaches its resting potential from the rest potential.
			if (maximumRecorder.hasLastState()) {
				eDelta += 2.0 * (eSpikeEff - maximumRecorder.getLastState().second.v());
			}
			break;
		}
	}

	return SpikeCountResult(lastSpikeCount, eDelta, eDelta / eSpikeEff);
}

EvaluationResult SingleGroupMultiOutEvaluation::evaluate(
    const WorkingParameters &params) const
{
	// TODO
	return EvaluationResult();
}
}
