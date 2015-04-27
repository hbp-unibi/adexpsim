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
 * @file Parameters.hpp
 *
 * Contains the "Parameters" structure holding all parameters of the AdExp
 * model.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_PARAMETERS_HPP_
#define _ADEXPSIM_PARAMETERS_HPP_

#include "Types.hpp"

namespace AdExpSim {

/**
 * Structure holding the parameters of a single AdExp neuron.
 */
struct Parameters {
	Val cM = 1e-9;        // Membrane capacitance [F]
	Val gL = 0.05e-6;     // Membrane leak conductance [S]
	Val eL = -70e-3;      // Membrane leak channel reversal potential [V]
	Val eI = -70e-3;      // Membrane inhibitory channel reversal potential [V]
	Val eE = 0e-3;        // Membrane excitatory channel reversal potential [V]
	Val eTh = -54.0e-3;   // Threshold potential [V]
	Val eSpike = 20e-3;   // Spike potential [V]
	Val eReset = -80e-3;  // Reset potential [V]
	Val deltaTh = 2e-3;   // Slope factor [V]
	Val tauI = 5e-3;      // Time constant for exponential decay of gI
	Val tauE = 5e-3;      // Time constant for exponential decay of gE
	Val tauW = 144e-3;    // Time constant for exponential decay of w
	Val a = 4e-9;         // Subthreshold adaptation [S]
	Val b = 0.0805e-9;    // Spike triggered adaptation [A]
};
}

#endif /* _ADEXPSIM_PARAMETERS_HPP_ */

