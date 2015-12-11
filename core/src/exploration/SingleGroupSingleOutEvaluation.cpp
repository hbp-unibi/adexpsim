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

#include <limits>

#include <common/ProbabilityUtils.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>

#include "SingleGroupSingleOutEvaluation.hpp"

namespace AdExpSim {

namespace {

/**
 * Controller used to track the maximum potential and the last state.
 */
struct SingleGroupEvaluationController {
	State state;
	Val vMax;

	SingleGroupEvaluationController()
	    : state(0.0, 0.0, 0.0, 0.0), vMax(std::numeric_limits<Val>::min())
	{
	}

	ControllerResult control(Time, const State &s, const AuxiliaryState &,
	                         const WorkingParameters &, bool inRefrac)
	{
		vMax = std::max(s.v(), vMax);
		state = s;
		return ControllerResult::CONTINUE;
	}
};
}

/* Class SingleGroupEvaluation */

// Roughly spoken TAU_RANGE is the voltage difference from eSpikeEff
// needed to get a "good" result in the SOFT measurement
static constexpr Val TAU_RANGE = 0.002;    // 2mV
static constexpr Val TAU_RANGE_VAL = 0.1;  // sigma(eEff - TAU_RANGE)
static const LogisticFunction<true> sigmaV(TAU_RANGE, TAU_RANGE_VAL);

EvaluationResult SingleGroupSingleOutEvaluation::evaluate(
    const WorkingParameters &params) const
{
	// Do not record any result
	NullRecorder n;

	// Use max value controller to track the maximum value
	SingleGroupEvaluationController cN, cNM1, cNS;

	// Use the DormandPrinceIntegrator
	DormandPrinceIntegrator iN(eTar), iNM1(eTar), iNS(eTar);

	// Simulate for both the sXi and the sXiM1 input spike train
	if (useIfCondExp) {
		Model::simulate<Model::IF_COND_EXP | Model::DISABLE_SPIKING>(
		    sN, n, cN, iN, params, Time(-1), env.T);
		Model::simulate<Model::IF_COND_EXP | Model::DISABLE_SPIKING>(
		    sNM1, n, cNM1, iNM1, params, Time(-1), env.T);
		Model::simulate<Model::IF_COND_EXP | Model::DISABLE_SPIKING>(
		    sN, n, cNS, iNS, params, Time(-1), env.T,
		    State(params.eReset()), Time(0));
	} else {
		Model::simulate<Model::CLAMP_ITH | Model::DISABLE_SPIKING |
		                Model::FAST_EXP>(sN, n, cN, iN, params, Time(-1),
		                                 env.T);
		Model::simulate<Model::CLAMP_ITH | Model::DISABLE_SPIKING |
		                Model::FAST_EXP>(sNM1, n, cNM1, iNM1, params, Time(-1),
		                                 env.T);
		Model::simulate<Model::CLAMP_ITH | Model::DISABLE_SPIKING |
		                Model::FAST_EXP>(sN, n, cNS, iNS, params, Time(-1),
		                                 env.T, State(params.eReset()));
	}

	const Val th = params.eSpikeEff(useIfCondExp);
	const bool ok = cN.vMax > th && cNM1.vMax < th && cNS.vMax < th;
	const Val pOk = ok ? 1.0 : 0.0f;
	const Val pTrueNegative = 1.0 - sigmaV(cN.vMax, th);
	const Val pTruePositive = sigmaV(cNM1.vMax, th) * sigmaV(cNS.vMax, th);
	const State sInit = State(0.0, 0.0, 0.0, 0.0);
	const State sRescale = State(100.0, 0.1, 0.1, 0.1);
	const Val eDiff = ((sInit - cN.state) * sRescale).sqrL2Norm();
	const Val eDiffM1 = ((sInit - cNM1.state) * sRescale).sqrL2Norm();
	const Val eDiffS = ((sInit - cNS.state) * sRescale).sqrL2Norm();
	const Val pReset = exp(-((eDiff + eDiffM1 + eDiffS) * 0.333333f));
	const Val pSoft = pTrueNegative * pTruePositive * pReset;
	return EvaluationResult({pSoft, pOk, pTruePositive, pTrueNegative, pReset});
}

const EvaluationResultDescriptor SingleGroupSingleOutEvaluation::descr =
    EvaluationResultDescriptor(EvaluationType::SINGLE_GROUP_SINGLE_OUT)
        .add("Soft", "pSoft", "", 0.0, Range(0.0, 1.0), true)
        .add("Binary", "pBin", "", 0.0, Range(0.0, 1.0))
        .add("True Pos.", "pTPos", "", 0.0, Range(0.0, 1.0))
        .add("True Neg.", "pTNeg", "", 0.0, Range(0.0, 1.0))
        .add("Reset", "pReset", "", 0.0, Range(0.0, 1.0));
}

