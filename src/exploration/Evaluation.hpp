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
#include <tuple>

#include <utils/Types.hpp>
#include <core/Spike.hpp>

namespace AdExpSim {

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
	 * Implementation of the cost function.
	 *
	 * @param vMaxXi is the maximum membrane potential that has been reached for
	 * Xi input spikes.
	 * @param vMaxXiM1 is the maximum membrane potential that has been reached
	 * for Xi - 1 input spikes.
	 * @param eSpikeEff is the effective spike potential.
	 * @param sigma is the slope factor that should be used for the evaluation.
	 */
	static Val cost(Val vMaxXi, Val vMaxXiM1, Val eSpikeEff, Val sigma = 100);

	/**
	 * Returns the cost for the given parameter set.
	 *
	 * @param params is a reference at the parameter set that should be
	 * evaluated.
	 * @param sigma is the slope factor that should be used in the cost
	 * function.
	 * @param tDelta is the internally used resolution. Should be smaller than
	 * the spike delay T.
	 */
	std::tuple<Val, Val, bool> evaluate(const WorkingParameters &params,
	                                    Val sigma = 100, Val tDelta = -1);

	Val getXi() { return xi; }

	Time getT() { return T; };

	Time getLastSpikeTime() { return T * ceil(xi - 1.0); }
};
}

#endif /* _ADEXPSIM_EVALUATION_HPP_ */

