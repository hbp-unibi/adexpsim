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

	// Convert the fractional spike counts to a value between 0 and 1
	static constexpr Val nu = 100;
	const Val pN = dist(resN.fracSpikeCount(), nOut + 0.5, nu);
	const Val pNM1 = dist(resNM1.fracSpikeCount(), 0, nu);
	const Val pBin = ((resN.spikeCount == nOut) && (resNM1.spikeCount == 0));
	return EvaluationResult(pBin, (1.0 - pNM1), (1.0 - pN), pN * pNM1);
}
}
