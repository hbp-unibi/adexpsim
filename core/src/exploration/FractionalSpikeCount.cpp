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
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/Integrator.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>

#include "FractionalSpikeCount.hpp"

namespace AdExpSim {

template <typename Recorder>
static void run(const SpikeVec &spikes, const WorkingParameters &params,
                Recorder &recorder, bool useIfCondExp, Val eTar,
                size_t maxSpikeCount)
{
	MaxValueController maxValueController;
	auto controller = createMaxOutputSpikeCountController(
	    [&recorder]() { return recorder.count(); }, maxSpikeCount, maxValueController);
	DormandPrinceIntegrator integrator(eTar);
	Model::simulate<Model::FAST_EXP | Model::PROCESS_SPECIAL>(
	    useIfCondExp, spikes, recorder, controller, integrator, params);
}

static RecordedSpikeVec runCollectOutputSpikes(const SpikeVec &spikes,
                                               const WorkingParameters &params,
                                               bool useIfCondExp, Val eTar,
                                               size_t maxSpikeCount)
{
	OutputSpikeRecorder recorder;
	run(spikes, params, recorder, useIfCondExp, eTar, maxSpikeCount);
	return recorder.spikes;
}

static size_t runCollectSpikeCount(const SpikeVec &spikes,
                                   const WorkingParameters &params,
                                   bool useIfCondExp, Val eTar,
                                   size_t maxSpikeCount)
{
	OutputSpikeCountRecorder recorder;
	run(spikes, params, recorder, useIfCondExp, eTar, maxSpikeCount);
	return recorder.count();
}

/**
 * Inserts a new spike into a copy of a sorted list of spikes.
 *
 * @param spikes is a sorted list of input spikes.
 * @param newSpike is the new spike that should be sorted into the list.
 * @return the extended list and the index of the newly inserted spike in that
 * list.
 */
static std::pair<SpikeVec, int> injectSpike(const SpikeVec &spikes,
                                            const Spike &newSpike)
{
	SpikeVec res;
	res.reserve(spikes.size() + 1);
	size_t i;
	for (i = 0; i < spikes.size() && spikes[i].t < newSpike.t; i++) {
		res.emplace_back(spikes[i]);
	}
	size_t newIdx = i;
	res.emplace_back(newSpike);
	for (; i < spikes.size(); i++) {
		res.emplace_back(spikes[i]);
	}
	return std::make_pair(res, newIdx);
}

FractionalSpikeCount::Result FractionalSpikeCount::calculate(
    const SpikeVec &spikes, const WorkingParameters &params)
{
	// Perform an initial run with just the given input spikes
	RecordedSpikeVec outputSpikes = runCollectOutputSpikes(
	    spikes, params, useIfCondExp, eTar, maxSpikeCount);
	const size_t spikeCount = outputSpikes.size();
	const Val eSpikeEff = params.eSpikeEff(useIfCondExp);
	if (spikeCount == maxSpikeCount) {
		return Result(spikeCount, eSpikeEff, 1.0);
	}

	// Add an virtual output spike at -tauRefrac in order to be able to control
	// the initial membrane potential
	outputSpikes.emplace_back(Time::sec(-params.tauRef()), State());

	// Iterate over all output spikes. Increase the membrane potential after
	// each spike until a new output spike is generated. Search the minimum
	// potential which causes such an increase
	uint16_t vMin = SpecialSpike::encodeSpikeVoltage(eSpikeEff, params.vMin(),
	                                                 params.vMax());
	for (const RecordedSpike &spike : outputSpikes) {
		// Create a new input spike vector containing a new special
		// "SET_VOLTAGE" input spike
		SpikeVec inputSpikes;
		size_t vSpikeIdx;
		std::tie(inputSpikes, vSpikeIdx) =
		    injectSpike(spikes, spike.t + Time::sec(params.tauRef()));

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
			inputSpikes[vSpikeIdx].w =
			    SpecialSpike::encode(SpecialSpike::Kind::SET_VOLTAGE, curV);

			// Run the simulation, restrict binary search area according to the
			// result
			if (runCollectSpikeCount(inputSpikes, params, useIfCondExp, eTar,
			                         maxSpikeCount) > spikeCount) {
				curVMax = curV;
			} else {
				curVMin = curV;
			}
			first = false;
		}

		// Set the new minimum voltage needed to trigger a spike
		vMin = std::min(vMin, curVMax);
	}

	// Return the actual spike count and the minimum voltage needed to induce
	// a new spike
	const Val eReq =
	    SpecialSpike::decodeSpikeVoltage(vMin, params.vMin(), params.vMax());
	const Val eNorm = (spikeCount == 0) ? 0.0 : params.eReset();
	return Result(spikeCount, eReq, (eReq - eNorm) / (eSpikeEff - eNorm));
}
}
