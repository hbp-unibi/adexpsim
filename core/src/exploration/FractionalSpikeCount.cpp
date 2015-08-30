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
 * Structure used to store the results of the binary search. Contains a
 * time and a state as upper and lower boundary.
 */
struct PerturbationAnalysisResult {
	/**
	 * Result of a comparison between an instance of PerturbationAnalysisResult
	 * and a current state.
	 */
	enum class ComparisonResult {
		/**
		 * There will be n + 1 output spikes.
		 */
		NP1,

		/**
		 * There will be n output spikes.
		 */
		N,

		/**
		 * There will be at most n output spikes.
		 */
		AT_MOST_N,

		/**
		 * There will be at least n output spikes, might also be n + 1.
		 */
		AT_LEAST_N,

		/**
		 * There may be any number of output spikes following.
		 */
		UNKNOWN
	};

	/**
	 * Value used to mark invalid/not set limits.
	 */
	static constexpr Val INVALID_LIMIT = std::numeric_limits<Val>::max();

	/**
	 * Point in time for which this analysis result was generated.
	 */
	Time t;

	/**
	 * Adaptation current at this point.
	 */
	Val w;

	/**
	 * Membrane potential which -- if surpassed by the current state -- means
	 * that at least one additional spike will be generated.
	 */
	Val vUpper;

	/**
	 * Membrane potential which -- if larger than the current state -- means
	 * that no additional spikes will be produced beyond this point in time.
	 */
	Val vLower;

	/**
	 * Returns true if a valid upper limit is set.
	 */
	bool hasUpper() const {
		return vUpper < INVALID_LIMIT;
	}

	/**
	 * Returns true if a valid lower limit is set.
	 */
	bool hasLower() const {
		return vLower < INVALID_LIMIT;
	}

	/**
	 * Compares the given current state to this PerturbationAnalysisResult
	 * instance.
	 */
	ComparisonResult compare(const State &s) const {
		// First case: We have a lower boundary and the current voltage is
		// smaller than the voltage of that lower boundary
		if (hasLower() && s.v() <= vLower) {
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
			// The adapation current stayed the same. We'll get exactly N
			// output spikes
			return ComparisonResult::N;
		}

		// Second case: The current voltage is larger than the 
		if (hasUpper() && s.v() >= vUpper) {
		
		}

		// Third case: We're neither in the upper, nor the lower boundary.
		if (hasLower() && s.v() >= vLower) {
			if (s.dvW() > w) {
				
			} else {
				// Adapation current is larger than s.w(), there will be at
				// least N output spikes
				return ComparisonResult::AT_LEAST_N;
			}
		}
		return ComparisonResult::UNKNOWN;
	}
};

/**
 * Both a recorder and controller used inside the FractionalSpikeCount class.
 * Allows to abort the simulation once a certain count of output spikes have
 * been reached or a previously simulated state is reached for which the
 * remaining number of output spikes is known.
 */
class PerturbationAnalysisManager {
private:
	/**
	 * List containing the perturbation analysis results for upcomming time
	 * points. Latest results are at the beginning, earliest results are at the
	 * end of the list.
	 */
	const std::vector<PerturbationAnalysisResult> &results;

public:


	void record(Time, const State &, const AuxiliaryState &, bool)
	{
		// Discard everything
	}

	void inputSpike(Time, const State &)
	{
		// Discard everything
	}

	void outputSpike(Time t, const State &)
	{
		// Discard everything
	}

	/**
	 * Resets the recorder to its initial state. Nothing to do in this
	 * implementation.
	 */
	void reset()
	{
		// Nothing to reset.
	}

	static ControllerResult control(Time, const State &, const AuxiliaryState &,
	                                const WorkingParameters &, bool)
	{
		return ControllerResult::CONTINUE;
	}
};
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
                                               uint16_t vMin,
                                               size_t expectedSpikeCount)
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

		OutputSpikeCountRecorder recorder;
		{
			MaxValueController maxValueController;
			auto controller = createMaxOutputSpikeCountController(
			    [&recorder]() { return recorder.count(); },
			    expectedSpikeCount, maxValueController);
			DormandPrinceIntegrator integrator(eTar);
			Model::simulate<Model::PROCESS_SPECIAL | Model::FAST_EXP>(
			    useIfCondExp, input, recorder, controller, integrator, params,
			    Time(-1), MAX_TIME, spike.state, Time(0));
		}

		// Run the simulation, restrict binary search area according to the
		// result
		if (recorder.count() > expectedSpikeCount) {
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
	uint16_t vMin = SpecialSpike::encodeSpikeVoltage(eSpikeEff, params.vMin(),
	                                                 params.vMax());
	for (ssize_t i = outputCount; i >= 0; i--) {
		vMin = minPerturbation(output[i], input, params, vMin, outputCount - i);
	}

	// Convert vMin into an actual membrane potential
	const Val eReq =
	    SpecialSpike::decodeSpikeVoltage(vMin, params.vMin(), params.vMax());
	const Val eNorm = (outputCount == 0) ? 0.0 : params.eReset();
	const Val eMax = maximumRecorder.global().s.v();
	return Result(outputCount, eReq, eMax, eNorm, eSpikeEff);
}
}
