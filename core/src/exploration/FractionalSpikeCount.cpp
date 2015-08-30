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

#include <cmath>
#include <utility>
#include <tuple>

#include <simulation/Controller.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Integrator.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/SpikeTrain.hpp>

#include "FractionalSpikeCount.hpp"

namespace AdExpSim {

template <uint8_t Flags, typename Recorder, typename CountFun>
static void run(const SpikeVec &spikes, const WorkingParameters &params,
                Recorder &recorder, bool useIfCondExp, Val eTar,
                size_t maxSpikeCount, CountFun countFun,
                const State &s0 = State())
{
	MaxValueController maxValueController;
	auto controller = createMaxOutputSpikeCountController(
	    countFun, maxSpikeCount, maxValueController);
	DormandPrinceIntegrator integrator(eTar);
	Model::simulate<Flags>(useIfCondExp, spikes, recorder, controller,
	                       integrator, params, Time(-1), MAX_TIME, s0);
}

/**
 * Copies all spikes from "spikes" to the result which occur later than "t" and
 * inserts a control spike at tCtrl, returning its index.
 */
static std::pair<SpikeVec, size_t> rebuildInput(const SpikeVec &spikes, Time t,
                                                Time tCtrl)
{
	// Index of the control spike -- not inserted yet
	size_t iCtrl = std::numeric_limits<size_t>::max();

	// Construct a new SpikeVec with shifted input spikes, insert the control
	// spike before any spike with greater timestamp occurs.
	SpikeVec res;
	for (const Spike &spike : spikes) {
		const Time ts = spike.t - t;
		if (ts > tCtrl && iCtrl > res.size()) {
			iCtrl = res.size();
			res.emplace_back(tCtrl);
		}
		if (ts > Time(0)) {
			res.emplace_back(ts, spike.w);
		}
	}

	// Control spike has not been inserted yet, insert it at the end
	if (iCtrl > res.size()) {
		iCtrl = res.size();
		res.emplace_back(tCtrl);
	}

	// Return both the new list and the index of the constrol spike
	return std::make_pair(res, iCtrl);
}

uint16_t FractionalSpikeCount::minPerturbation(const RecordedSpike &spike,
                                               const SpikeVec &spikes,
                                               const WorkingParameters &params,
                                               uint16_t vMin, size_t spikeCount)
{
	// Create a new input spike vector containing a new special
	// "SET_VOLTAGE" input spike
	SpikeVec input;
	size_t iCtrl;
	std::tie(input, iCtrl) =
	    rebuildInput(spikes, spike.t, Time::sec(params.tauRef()));

	// Perform a new binary search between curVMin and curVMax. The first
	// binary search point should be vMin -- in this case we can abort early
	// if the output will not spike with a lower voltage than the current
	// minimum
	bool first = true;
	uint16_t curVMin = SpecialSpike::encodeSpikeVoltage(
	    spike.state.v(), params.vMin(), params.vMax());
	uint16_t curVMax = vMin;
	while (int(curVMax) - int(curVMin) > 1) {
		// Set the voltage of the virtual spike
		const uint16_t curV =
		    first ? curVMax : (curVMin + (curVMax - curVMin) / 2);

		// Store the new voltage in the control spike
		input[iCtrl].w =
		    SpecialSpike::encode(SpecialSpike::Kind::SET_VOLTAGE, curV);

		// Run the simulation, restrict binary search area according to the
		// result
		OutputSpikeCountRecorder recorder;
		run<Model::PROCESS_SPECIAL | Model::FAST_EXP>(
		    input, params, recorder, useIfCondExp, eTar, spikeCount + 1,
		    [&recorder]() { return recorder.count(); }, spike.state);
		if (recorder.count() > spikeCount) {
			curVMax = curV;
		} else {
			curVMin = curV;
		}
		first = false;
	}

	// Return the new minimum voltage needed to trigger a spike
	return std::min(vMin, curVMax);
}

FractionalSpikeCount::Result FractionalSpikeCount::calculate(
    const SpikeVec &spikes, const WorkingParameters &params)
{
	// Perform an initial run with just the given input spikes
	LocalMaximumRecorder maximumRecorder;
	OutputSpikeRecorder spikeRecorder;
	auto recorder = makeMultiRecorder(maximumRecorder, spikeRecorder);
	run<Model::FAST_EXP>(spikes, params, recorder, useIfCondExp, eTar,
	                     maxSpikeCount,
	                     [&spikeRecorder] { return spikeRecorder.count(); });
	RecordedSpikeVec outputSpikes = spikeRecorder.spikes;

	const size_t spikeCount = outputSpikes.size();
	const Val eSpikeEff = params.eSpikeEff(useIfCondExp);
	if (spikeCount == maxSpikeCount) {
		return Result(spikeCount);
	}

	// Add an virtual output spike at -tauRefrac in order to be able to control
	// the initial membrane potential
	outputSpikes.insert(
	    outputSpikes.begin(),
	    RecordedSpike(Time::sec(-params.tauRef()) - Time(1), State()));

	uint16_t vMin = SpecialSpike::encodeSpikeVoltage(eSpikeEff, params.vMin(),
	                                                 params.vMax());

	// Note: outputSpikes.size() = spikeCount + 1
	for (ssize_t i = spikeCount; i >= 0; i--) {
		vMin = minPerturbation(outputSpikes[i], spikes, params, vMin,
		                       spikeCount - i);
	}

	// Return the actual spike count and the minimum voltage needed to induce
	// a new spike
	const Val eReq =
	    SpecialSpike::decodeSpikeVoltage(vMin, params.vMin(), params.vMax());
	const Val eNorm = (spikeCount == 0) ? 0.0 : params.eReset();
	const Val eMax = maximumRecorder.global().s.v();

	return Result(spikeCount, eReq, eMax, eNorm, eSpikeEff);
}
}
