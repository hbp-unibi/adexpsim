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

#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>

#include "SpikeTrainEvaluation.hpp"

namespace AdExpSim {
/**
 * Spike as recorded by the SpikeRecorder class. Contains the state of the
 * neuron at the time the spike has been issued.
 */
struct RecordedSpike {
	/**
	 * Time at which the spike was recorded.
	 */
	Time t;

	/**
	 * State of the neuron at the time the spike was captured.
	 */
	State state;

	/**
	 * Constructor of the RecordedSpike descriptor.
	 *
	 * @param t is the time at which the Spike was captured.
	 * @param state is the state of the neuron at the time of capture.
	 * @param isOutputSpike is set to true if the captured spike is an
	 * output spike, otherwise to true.
	 */
	RecordedSpike(Time t, const State &state = State()) : t(t), state(state) {}

	/**
	 * Comperator used to allow binary search within a list of RecordedSpike
	 * instances.
	 */
	friend bool operator<(const RecordedSpike &s1, const RecordedSpike &s2)
	{
		return s1.t < s2.t;
	}
};

/**
 * The SpikeRecorder class is used internally be the evaluation algorithm to
 * track input and output spikes. Input spikes are recorded, because they may
 * indicate the start of a range.
 */
class SpikeRecorder {
private:
	/**
	 * Recorded input spikes.
	 */
	std::vector<RecordedSpike> inputSpikes;

	/**
	 * Recorded output spikes.
	 */
	std::vector<RecordedSpike> outputSpikes;

	/**
	 * Indices of the input spikes that should be recorded.
	 */
	const std::vector<size_t> &rangeStartSpikes;

	/**
	 * Current input spike index.
	 */
	size_t inputSpikeIdx;

public:
	/**
	 * Iterator type used to access the elements in the spike iterator.
	 */
	using RecordedSpikeIt = std::vector<RecordedSpike>::const_iterator;

	/**
	 * Constructor of the SpikeRecorder class.
	 */
	SpikeRecorder(const std::vector<size_t> &rangeStartSpikes)
	    : rangeStartSpikes(rangeStartSpikes), inputSpikeIdx(0)
	{
	}

	/**
	 * Actually called by the simulation to record the internal state, however
	 * this class just acts as a null sink for this data.
	 */
	void record(Time, const State &, const AuxiliaryState &, bool)
	{
		// Discard all in-between data
	}

	/**
	 * Called whenever an input spike is consumed by the model
	 *
	 * @param t is the time at which the input spike has been consumed.
	 * @param s is the state after the input spike has been consumed.
	 */
	void inputSpike(Time t, const State &s)
	{
		// Check whether this spike is one of the spikes that should be recorded
		const size_t rangeStartSpikesIdx = inputSpikes.size();
		if (rangeStartSpikesIdx < rangeStartSpikes.size() &&
		    rangeStartSpikes[rangeStartSpikesIdx] == inputSpikeIdx) {
			inputSpikes.emplace_back(t, s);
		}

		// Increment the input spike index
		inputSpikeIdx++;
	}

	/**
	 * Called whenever an output spike is produced by the model.
	 *
	 * @param t is the time at which the output spike has been produced.
	 * @param s is the neuron state after the spike has been issued (the neuron
	 * has already been reset).
	 */
	void outputSpike(Time t, const State &s)
	{
		outputSpikes.emplace_back(t, s);
	}

	/**
	 * Returns the recorded input spikes (one for each range start).
	 */
	const std::vector<RecordedSpike> &getInputSpikes() const
	{
		return inputSpikes;
	}

	/**
	 * Returns the recorded output spikes.
	 */
	const std::vector<RecordedSpike> &getOutputSpikes() const
	{
		return outputSpikes;
	}
};

SpikeTrainEvaluation::SpikeTrainEvaluation(const SpikeTrain &train)
    : train(train)
{
}

// Roughly spoken TAU_RANGE is the voltage difference from eSpikeEff
// needed to get a "good" result in the SOFT measurement
static constexpr double TAU_RANGE = 0.01;     // 10mV
static constexpr double TAU_RANGE_VAL = 0.1;  // sigma(eEff - TAU_RANGE)
static constexpr double TAU = log(1.0 / TAU_RANGE_VAL - 1.0) / TAU_RANGE;

Val SpikeTrainEvaluation::sigma(Val x, const WorkingParameters &params,
                                bool invert)
{
	const double res = 1.0 / (1.0 + exp(-TAU * (x - params.eSpikeEff())));
	return invert ? 1.0 - res : res;
}

SpikeTrainEvaluation::MaxPotentialResult
SpikeTrainEvaluation::trackMaxPotential(const WorkingParameters &params,
                                        const RecordedSpike &s0, Time tEnd,
                                        Val eTar) const
{
	// Fetch the time range
	const Time tStart = s0.t;
	const Time tLen = tEnd - tStart;

	// Abort if the range is invalid, return the voltage at the start time point
	if (tLen <= Time(0)) {
		return MaxPotentialResult(s0.state.v(), Time(0), Time(1));
	}

	// Fetch all the input spikes that occured in this period and move their
	// start time back by tStart
	const auto &spikes = train.getSpikes();
	SpikeVec inputSpikes(
	    std::upper_bound(spikes.begin(), spikes.end(), Spike(tStart)),
	    std::lower_bound(spikes.begin(), spikes.end(), Spike(tEnd)));
	for (Spike &spike : inputSpikes) {
		spike.t -= tStart;
	}

	// Run the simulation with the maximum value controller, record nothing
	NullRecorder recorder;
	MaxValueController controller;
	DormandPrinceIntegrator integrator(eTar);
	Model::simulate<Model::FAST_EXP | Model::CLAMP_ITH |
	                Model::DISABLE_SPIKING>(inputSpikes, recorder, controller,
	                                        integrator, params, Time(-1), tLen,
	                                        s0.state);

	// Return the tracked maximum membrane potential
	return MaxPotentialResult(
	    controller.vMax, std::min(controller.tVMax, controller.tSpike), tLen);
}

template <typename F1, typename F2>
SpikeTrainEvaluationResult SpikeTrainEvaluation::evaluateInternal(
    const WorkingParameters &params, Val eTar, F1 recordOutputSpike,
    F2 recordOutputGroup) const
{
	// Abort if the given parameters are invalid
	if (!params.valid() || train.getRanges().empty()) {
		return SpikeTrainEvaluationResult();
	}

	// Run the simulation on the spike train with the given parameters and
	// collect all spikes
	const Time T = train.getMaxT();
	SpikeRecorder recorder(train.getRangeStartSpikes());
	NullController controller;
	DormandPrinceIntegrator integrator(eTar);
	Model::simulate<Model::FAST_EXP>(train.getSpikes(), recorder, controller,
	                                 integrator, params, Time(-1), T);

	// Iterate over all ranges described in the spike train and adapt the result
	// according to whether how well the range condition (number of expected
	// output spikes) has been fulfilled.
	const std::vector<RecordedSpike> &inputSpikes = recorder.getInputSpikes();
	const std::vector<RecordedSpike> &outputSpikes = recorder.getOutputSpikes();
	const std::vector<SpikeTrain::Range> &ranges = train.getRanges();

	SpikeTrainEvaluationResult res;
	bool groupOk = true;
	size_t group = 0;
	size_t groupDescrIdx = ranges.front().descrIdx;
	Time groupStart;
	size_t nGroups = 1;
	for (size_t rangeIdx = 0; rangeIdx < ranges.size() - 1; rangeIdx++) {
		// Fetch some information about the current spike
		size_t nSpikesExpected = ranges[rangeIdx].nSpikes;
		const Time rangeStart = ranges[rangeIdx].start;
		const Time rangeEnd = ranges[rangeIdx + 1].start;
		const size_t rangeGroup = ranges[rangeIdx].group;
		const size_t rangeDescrIdx = ranges[rangeIdx].descrIdx;
		const Time rangeLen = rangeEnd - rangeStart;

		// Skip this range if it has a non-positive length
		if (rangeLen <= Time(0)) {
			continue;
		}

		// Fetch the input spike that started this range
		const RecordedSpike &inputSpike = inputSpikes[rangeIdx];

		// Fetch all output spikes that fall into the range
		const auto firstSpike =
		    std::lower_bound(outputSpikes.begin(), outputSpikes.end(),
		                     RecordedSpike(rangeStart));
		const auto lastSpike = std::lower_bound(firstSpike, outputSpikes.end(),
		                                        RecordedSpike(rangeEnd));

		// Calculate the number of encountered spikes
		const size_t nSpikesReceived = std::distance(firstSpike, lastSpike);

		// If this range belongs to another group than the previous, update the
		// binaryExpectationRatio
		if (group != rangeGroup) {
			// Record the current output group
			recordOutputGroup(
			    OutputGroup(groupStart, rangeStart, groupDescrIdx, groupOk));

			// Update the output
			res.pBinary += groupOk ? 1.0 : 0.0;

			// Increase the group cpunt
			nGroups++;

			// Reset the current group
			groupOk = true;
			groupStart = rangeStart;
			group = rangeGroup;
			groupDescrIdx = rangeDescrIdx;
		}

		// Update the "groupOk" flag
		groupOk = groupOk && (nSpikesReceived == nSpikesExpected);

		// Iterate over all output spikes and call the "output" function for
		// each of them
		size_t i = 0;
		for (auto it = firstSpike; it != lastSpike; it++, i++) {
			recordOutputSpike(
			    OutputSpike(it->t, rangeGroup, i < nSpikesExpected));
		}

		// Update the softExpectationRatio: Iterate over all output spikes while
		// there are expected output spikes and measure the maximum potential
		RecordedSpike const *curSpike = &inputSpike;
		for (auto it = firstSpike; it != lastSpike && nSpikesExpected > 0;
		     it++, nSpikesExpected--) {
			// Track the maximum potential between the current spike and the
			// next output spike
			const auto simRes =
			    trackMaxPotential(params, *curSpike, it->t, eTar);

			// Adapt the softExpectationRatio
			res.pSoft += sigma(simRes.vMax, params) * simRes.tLen.sec()/* *
			             simRes.tMaxRel()*/;

			// Advance the curSpike pointer to the last processed output spike
			curSpike = &(*it);
		}

		// Now there are either no more expected output spikes, or no more
		// received output spikes for this range. Run the simulation for the
		// rest of the range period and adapt the softExpectationRatio. If no
		// spikes were expected, the sigma function has to be inverted (because
		// lower potentials are better).
		const auto simRes =
		    trackMaxPotential(params, *curSpike, rangeEnd, eTar);
		res.pSoft += sigma(simRes.vMax, params, nSpikesExpected == 0) *
		             simRes.tLen.sec()/* * simRes.tMaxRel()*/;
	}

	// Record the last output group
	recordOutputGroup(
	    OutputGroup(groupStart, ranges.back().start, groupDescrIdx, groupOk));

	// Normalize the result by the total simulation time in seconds
	res.pBinary = (res.pBinary + (groupOk ? 1.0 : 0.0)) / Val(nGroups);
	res.pSoft = /*res.pBinary **/ res.pSoft / T.sec();

	return res;
}

SpikeTrainEvaluationResult SpikeTrainEvaluation::evaluate(
    const WorkingParameters &params, Val eTar) const
{
	// Call the evaluateInternal template with two empty functions, thus
	// removing all of the recording code.
	return evaluateInternal(params, eTar, [](const OutputSpike &) -> void {},
	                        [](const OutputGroup &) -> void {});
}

SpikeTrainEvaluationResult SpikeTrainEvaluation::evaluate(
    const WorkingParameters &params, std::vector<OutputSpike> &outputSpikes,
    std::vector<OutputGroup> &outputGroups, Val eTar) const
{
	// Call the evaluateInternal template with record callbacks storing the
	// to be recorded objects in the given lists.
	return evaluateInternal(params, eTar,
	                        [&outputSpikes](const OutputSpike &spike)
	                            -> void { outputSpikes.emplace_back(spike); },
	                        [&outputGroups](const OutputGroup &group)
	                            -> void { outputGroups.emplace_back(group); });
}
}

