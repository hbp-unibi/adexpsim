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

#include <simulation/Controller.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>

#include "FractionalSpikeCount.hpp"
#include "SingleGroupMultiOutEvaluation.hpp"

namespace AdExpSim {
/**
 * Student-t like long-tail distribution, no normalization. In contrast to a
 * gauss-like distribution has a long tail which makes it more suitable for
 * optimization.
 */
static Val dist(Val x, Val mu, Val nu)
{
	Val d = x - mu;
	return std::pow(1.0 + d * d / nu, -(nu + 1.0) * 0.5);
}

EvaluationResult SingleGroupMultiOutEvaluation::evaluate(
    const WorkingParameters &params) const
{
	// Calculate the fractional spike count
	const Val nOut = spikeData.nOut;
	FractionalSpikeCount eval(useIfCondExp, eTar, nOut * 10);
	auto resN = eval.calculate(sN, params);
	auto resNM1 = eval.calculate(sNM1, params);

	// Run a short simulation to get the state the neuron is in at time T
	NullController controller;
	DormandPrinceIntegrator integrator(eTar);
	LastStateRecorder recorder;
	Model::simulate<Model::FAST_EXP | Model::DISABLE_SPIKING |
	                Model::CLAMP_ITH>(useIfCondExp, sN, recorder, controller,
	                                  integrator, params, Time(-1),
	                                  spikeData.T);

	// Calculate the
	const State sRescale = State(100.0, 0.1, 0.1, 0.1);
	const Val delta = ((State() - recorder.state()) * sRescale).sqrL2Norm();
	const Val pReset = exp(-delta);

	// Convert the fractional spike counts to a value between 0 and 1
	static constexpr Val nu = 1;
	const Val pN = dist(resN.fracSpikeCount(), nOut + 0.3, nu);
	const Val pNM1 = dist(resNM1.fracSpikeCount(), 0, nu);
	const Val pBin = ((resN.spikeCount == nOut) && (resNM1.spikeCount == 0));
	return EvaluationResult(pReset, (1.0 - pNM1), (1.0 - pN),
	                        pN * pNM1 * pReset);
}
}
