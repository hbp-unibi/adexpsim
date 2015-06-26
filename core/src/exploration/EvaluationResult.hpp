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

/**
 * @file EvaluationResult.hpp
 *
 * Contains the EvaluationResult structure which is shared between all
 * implemented evaluation methods.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_EVALUATION_RESULT_HPP_
#define _ADEXPSIM_EVALUATION_RESULT_HPP_

namespace AdExpSim {

/**
 * Available evaluation result dimensions.
 */
enum class EvaluationResultDimension {
	 SOFT = 0, BINARY = 1, FALSE_POSITIVE = 2, FALSE_NEGATIVE = 3
};

/**
 * Structure containing the result of a call to the "evaluate" method of either
 * the SingleSpikeEvaluation class or the SpikeGroupEvaluation class.
 */
struct EvaluationResult {
	/**
	 * Percentage of spike train groups for which all conditions were fulfilled.
	 * Ranges between 0.0 and 1.0.
	 */
	Val pBinary;

	/**
	 * Percentage of spike train groups where the number of output spikes was
	 * larger than the expected number of output spikes.
	 */
	Val pFalsePositive;

	/**
	 * Percentage of spike train groups where the number of output spikes was
	 * smaller than the expected number of output spikes.
	 */
	Val pFalseNegative;

	/**
	 * Soft expectation ratio -- same as the binary ratio, but takes the maximum
	 * voltage that is theoretically reached in each range into account,
	 * creating a smooth function. Ranges between 0.0 and 1.0.
	 */
	Val pSoft;

	/**
	 * Default constructor. Initializes the values with the worst possible
	 * result.
	 */
	EvaluationResult()
	    : pBinary(0.0), pFalsePositive(1.0), pFalseNegative(1.0), pSoft(0.0)
	{
	}

	/**
	 * Constructor of the EvaluationResult class. Initializes all members with
	 * the given values.
	 */
	EvaluationResult(Val pBinary, Val pFalsePositive, Val pFalseNegative,
	                 Val pSoft)
	    : pBinary(pBinary),
	      pFalsePositive(pFalsePositive),
	      pFalseNegative(pFalseNegative),
	      pSoft(pSoft)
	{
	}
};
}

#endif /* _ADEXPSIM_EVALUATION_RESULT_HPP_ */
