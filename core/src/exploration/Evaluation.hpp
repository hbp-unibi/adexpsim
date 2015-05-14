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
 * @file Evaluation.hpp
 *
 * Contains the implementation of the cost function used as an evaluation
 * measure for the parameter space.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_EVALUATION_HPP_
#define _ADEXPSIM_EVALUATION_HPP_

#include <cmath>
#include <limits>
#include <tuple>

#include <simulation/Model.hpp>
#include <simulation/Spike.hpp>
#include <utils/Types.hpp>

namespace AdExpSim {

/**
 * Structure containing the result of a call to the "evaluate" method of the
 * Evaluation class.
 */
struct EvaluationResult {
	/**
	 * Effective spike potential for this parameter set.
	 */
	Val eSpikeEff;

	/**
	 * Maximum membrane potential for this parameter set and Xi input spikes.
	 */
	Val eMaxXi;

	/**
	 * Maximum membrane potential for this parameter set and Xi - 1 input
	 * spikes.
	 */
	Val eMaxXiM1;

	/**
	 * Time at which the effective spike potential is reached for Xi input
	 * spikes.
	 */
	Val tSpike;

	/**
	 * Time at which the original membrane potential is reached for Xi input
	 * spikes.
	 */
	Val tReset;

	/**
	 * Default constructor, initializes all members with values indicating the
	 * invalidity of this result.
	 */
	EvaluationResult()
	    : eSpikeEff(std::numeric_limits<Val>::lowest()),
	      eMaxXi(std::numeric_limits<Val>::lowest()),
	      eMaxXiM1(std::numeric_limits<Val>::lowest()),
	      tSpike(MAX_TIME_SEC),
	      tReset(MAX_TIME_SEC)
	{
	}

	/**
	 * Creates an instance of the EvaluationResult struct and sets all member
	 * variables to the given values.
	 */
	EvaluationResult(Val eSpikeEff, Val eMaxXi, Val eMaxXiM1, Val tSpike,
	                 Val tReset)
	    : eSpikeEff(eSpikeEff),
	      eMaxXi(eMaxXi),
	      eMaxXiM1(eMaxXiM1),
	      tSpike(tSpike),
	      tReset(tReset)
	{
	}

	/**
	 * Implementation of the cost function.
	 *
	 * @param sigma is the slope factor that should be used for the evaluation.
	 */
	Val cost(Val sigma = 100) const;

	/**
	 * Returns true if the heaviside condition is fulfilled.
	 */
	bool ok() const;

	/**
	 * Returns true if the result is valid (the values are in a sane range).
	 */
	bool valid() const;
};

/**
 * The evaluation class can be used for the evalutation of the behaviour of a
 * single neuron given Xi and Xi - 1 input spikes.
 */
class Evaluation {
private:
	/**
	 * Value of xi given in the constructor.
	 */
	Val xi;

	/**
	 * Value of T given in the constructor.
	 */
	Time T;

	/**
	 * Vector containing the spikes for Xi input spikes.
	 */
	SpikeVec sXi;

	/**
	 * Vector containing the spikes for Xi - 1 input spikes.
	 */
	SpikeVec sXiM1;

public:
	/**
	 * Flags used for the AdExp simulation.
	 */
	static constexpr uint8_t SimulationFlags =
	    Model::CLAMP_ITH | Model::DISABLE_SPIKING | Model::FAST_EXP;

	/**
	 * Constructor of the evaluation class.
	 *
	 * @param xi is the number of input spikes for which the neuron should
	 * spike. Must be larger than one.
	 * @param T is the delay between two incomming input spikes.
	 */
	Evaluation(Val xi, Time T);

	/**
	 * Returns the cost for the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated. Automatically updates the derived values of the parameter set.
	 * @param tDelta is the timestep used for the ODE integration.
	 */
	EvaluationResult evaluate(const WorkingParameters &params,
	                          Val tDelta = -1) const;

	Val getXi() const { return xi; }

	Time getT() const { return T; };

	Time getLastSpikeTime() const { return T * ceil(xi - 1.0); }
};
}

#endif /* _ADEXPSIM_EVALUATION_HPP_ */

