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

namespace {
/**
 * Both a recorder and controller used inside the FractionalSpikeCount class.
 * Allows to abort the simulation once a certain count of output spikes have
 * been reached or a previously simulated state is reached for which the
 * remaining number of output spikes is known.
 */
class PerturbationAnalysisManager : public NullRecorder {
private:
	/**
	 * List containing the perturbation analysis results for upcomming time
	 * points. Latest results are at the beginning, earliest results are at the
	 * end of the list.
	 */
	const std::vector<PerturbationAnalysisResult> &results;

	/**
	 * Current index within the "results" list.
	 */
	ssize_t idx;

	/**
	 * Time offset relative to the values stored in the "results" list.
	 */
	Time offs;

	/**
	 * Original number of output spikes beyond the point at which the simulation
	 * was started. As soon as one of the elements in "results" is passed, this
	 * value has to be decreased.
	 */
	size_t maxSpikeCount;

	/**
	 * Number of recorded output spikes.
	 */
	size_t outputSpikeCount;

public:
	PerturbationAnalysisManager(
	    const std::vector<PerturbationAnalysisResult> &results, Time offs,
	    size_t maxSpikeCount)
	    : results(results), offs(offs), maxSpikeCount(maxSpikeCount)
	{
		reset();
	}

	void outputSpike(Time t, const State &) { outputSpikeCount++; }

	void reset()
	{
		idx = results.size() - 1;
		outputSpikeCount = 0;
	}

	/**
	 * Returns the number of recorded output spike counts.
	 */
	size_t count() const { return outputSpikeCount; }

	/**
	 * Actual control function. Tries to abort the simulation as early as
	 * possible, whenever it is clear that maxSpikeCount will not be surpassed
	 * or will be surpassed.
	 */
	ControllerResult control(Time t, const State &s, const AuxiliaryState &as,
	                         const WorkingParameters &, bool inRefrac)
	{
		if (idx >= 0 && t + offs >= results[idx].t) {
			// Number of spikes that have been expected up to this point in time
			const size_t expectedSpikeCount = results.size() - idx;

			// Number of spikes that are to be expected beyond this point in
			// time
			const size_t n = idx;

			// Compare the current state to the value stored in results[idx].
			switch (results[idx].compare(s)) {
				case PerturbationAnalysisResult::ComparisonResult::AT_LEAST_NP1:
					// There will be more than n + 1 spikes following. This is
					// all we need to know. Abort.
					outputSpikeCount += n + 1;
					return ControllerResult::ABORT;
				case PerturbationAnalysisResult::ComparisonResult::AT_LEAST_N:
					// If the current spike count already exceeds the expected
					// spike count, then we can abort, as we know that at least
					// n spikes will follow. Otherwise we cannot abort, since we
					// don't know whether n or n + 1 spikes will follow.
					if (outputSpikeCount > expectedSpikeCount) {
						outputSpikeCount += n;
						return ControllerResult::ABORT;
					}
					break;
				case PerturbationAnalysisResult::ComparisonResult::N:
					// There will be exactly N spikes following. Abort.
					outputSpikeCount += n;
					return ControllerResult::ABORT;
				case PerturbationAnalysisResult::ComparisonResult::AT_MOST_N:
					// There will be at most N spikes following. If the current
					// spike count is smaller or equal to the expected spike
					// count, then there will be no chance of getting more than
					// maxSpikeCount. Abort.
					if (outputSpikeCount == expectedSpikeCount) {
						outputSpikeCount += n;
						return ControllerResult::ABORT;
					}
					// In the other case there could still be a chance of
					// getting maxSpikeCount output spikes.
					break;
				case PerturbationAnalysisResult::ComparisonResult::AT_MOST_NP1:
					// Well, this case is stupid. It means that there could
					// still be n + 1 spikes comming, but it might also be less.
					// Nothing we can do about that. Continue simulating.
					break;
			}

			// Go to the next element in the list
			idx--;
		}

		// Abort if the maximum spike count has been surpassed.
		if (outputSpikeCount > maxSpikeCount) {
			return ControllerResult::ABORT;
		}

		// Use the MaxValueController to abort whenever it is clear that no
		// larger values can be reached.
		return MaxValueController::control(s, as, inRefrac);
	}
};
}

PerturbationAnalysisResult::ComparisonResult
PerturbationAnalysisResult::compare(const State &s) const
{
	// First case: We have a lower boundary and the current voltage is
	// smaller than the voltage of that lower boundary
	if (s.v() <= vLower) {
		if (s.dvW() > w) {
			// The adaptation current is larger than last time, preventing
			// any new spikes. So we're surly not going to see more than N
			// spikes.
			return ComparisonResult::AT_MOST_N;
		} else if (s.dvW() < w) {
			// The adapation current is smaller. We might also get more
			// than N spikes!
			return ComparisonResult::AT_LEAST_N;
		}
		// We'll get N spikes as output
		return ComparisonResult::N;
	}

	// Second case: The current voltage is larger than the uper boundary,
	// there can be potentially n + 1 spikes. Same cases as above.
	if (s.v() >= vUpper) {
		if (s.dvW() > w) {
			return ComparisonResult::AT_MOST_NP1;
		}
		return ComparisonResult::AT_LEAST_NP1;
	}

	// Third case: s.v() > vLower && s.v() < vUpper
	if (s.dvW() > w) {
		// s.v > vLower, but the adapation current is larger. So there
		// might be n + 1 spikes, but more likely fewer.
		return ComparisonResult::AT_MOST_NP1;
	}
	// The current adapation current is smaller than s.dvW there
	// will be at least N output spikes
	return ComparisonResult::AT_LEAST_N;
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

uint16_t FractionalSpikeCount::minPerturbation(
    const RecordedSpike &spike, const SpikeVec &spikes,
    const WorkingParameters &params, uint16_t vMin, size_t expectedSpikeCount,
    std::vector<PerturbationAnalysisResult> &results)
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

		// Run the actual simulation
		PerturbationAnalysisManager manager(results, spike.t,
		                                    expectedSpikeCount);
		DormandPrinceIntegrator integrator(eTar);
		Model::simulate<Model::PROCESS_SPECIAL | Model::FAST_EXP>(
		    useIfCondExp, input, manager, manager, integrator, params, Time(-1),
		    MAX_TIME, spike.state, Time(0));

		// Run the simulation, restrict binary search area according to the
		// result
		if (manager.count() > expectedSpikeCount) {
			curVMax = curV;
		} else {
			curVMin = curV;
		}
		first = false;
	}

	// Add a new entry to the results list. In case curVMin <= curVMax (e.g.
	// because in the first step vMin was not found to produce another spike)
	// set the upper bound to some large value.
	results.emplace_back(
	    spike.t,
	    (curVMax > curVMin) ? SpecialSpike::decodeSpikeVoltage(
	                              curVMax, params.vMin(), params.vMax())
	                        : params.vMax(),
	    SpecialSpike::decodeSpikeVoltage(curVMin, params.vMin(), params.vMax()),
	    spike.state.dvW());

	// Return the new minimum voltage needed to trigger a spike
	return std::min(vMin, curVMax);
}

FractionalSpikeCount::Result FractionalSpikeCount::calculate(
    const SpikeVec &input, const WorkingParameters &params)
{
	// Fetch some required constants from the parameters
	const Val eSpikeEff = params.eSpikeEff(useIfCondExp);
	const Time tRef = Time::sec(params.tauRef()) + Time(1);

	// Initial run with the given input spikes. Record both local maxima and the
	// output spikes. Use the MaxValueController to abort once the the neuron
	// has processed all input spikes and settled down on its last maximum.
	LocalMaximumRecorder maximumRecorder;
	OutputSpikeRecorder spikeRecorder;
	{
		MaxValueController maxValueController;
		auto recorder = makeMultiRecorder(maximumRecorder, spikeRecorder);
		auto controller = createMaxOutputSpikeCountController(
		    [&spikeRecorder]() { return spikeRecorder.count(); }, maxSpikeCount,
		    maxValueController);
		DormandPrinceIntegrator integrator(eTar);
		Model::simulate<Model::FAST_EXP>(useIfCondExp, input, recorder,
		                                 controller, integrator, params);

		// Abort if the MaxOutputSpikeCount controller has tripped
		if (controller.tripped()) {
			return Result(spikeRecorder.count());
		}
	}

	// Fetch the recorded output spikes and add an virtual output spike at -
	// tauRefrac in order to be able to control the initial membrane potential
	RecordedSpikeVec output = spikeRecorder.spikes;
	const size_t outputCount = output.size();
	output.insert(output.begin(), RecordedSpike(tRef));

	// Note: outputSpikes.size() = spikeCount + 1
	std::vector<PerturbationAnalysisResult> results;
	uint16_t vMin = SpecialSpike::encodeSpikeVoltage(eSpikeEff, params.vMin(),
	                                                 params.vMax());
	for (ssize_t i = outputCount; i >= 0; i--) {
		vMin = minPerturbation(output[i], input, params, vMin, outputCount - i,
		                       results);
	}

	// Convert vMin into an actual membrane potential
	const Val eReq =
	    SpecialSpike::decodeSpikeVoltage(vMin, params.vMin(), params.vMax());
	const Val eNorm = (outputCount == 0) ? 0.0 : params.eReset();
	const Val eMax = maximumRecorder.global().s.v();
	return Result(outputCount, eReq, eMax, eNorm, eSpikeEff);
}
}
