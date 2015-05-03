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

#include <cmath>

#include <utils/Types.hpp>

namespace AdExpSim {

namespace DefaultParameters {
constexpr Val cM = 1e-9;        // Membrane capacitance [F]
constexpr Val gL = 0.05e-6;     // Membrane leak conductance [S]
constexpr Val eL = -70e-3;      // Leak channel reversal potential [V]
constexpr Val eE = 0e-3;        // Excitatory channel reversal potential [V]
constexpr Val eI = -70e-3;      // Inhibitory channel reversal potential [V]
constexpr Val eTh = -54.0e-3;   // Threshold potential [V]
constexpr Val eSpike = 20e-3;   // Spike potential [V]
constexpr Val eReset = -80e-3;  // Reset potential [V]
constexpr Val deltaTh = 2e-3;   // Slope factor [V]
constexpr Val tauI = 5e-3;      // Time constant for exponential decay of gI
constexpr Val tauE = 5e-3;      // Time constant for exponential decay of gE
constexpr Val tauW = 144e-3;    // Time constant for exponential decay of w
constexpr Val a = 4e-9;         // Subthreshold adaptation [S]
constexpr Val b = 0.0805e-9;    // Spike triggered adaptation [A]
}

/**
 * Structure holding the parameters of a single AdExp neuron.
 */
struct Parameters {
	Val cM = DefaultParameters::cM;    // Membrane capacitance [F]
	Val gL = DefaultParameters::gL;    // Membrane leak conductance [S]
	Val eL = DefaultParameters::eL;    // Leak channel reversal potential [V]
	Val eE = DefaultParameters::eE;    // Excitatory reversal potential [V]
	Val eI = DefaultParameters::eI;    // Inhibitory reversal potential [V]
	Val eTh = DefaultParameters::eTh;  // Threshold potential [V]
	Val eSpike = DefaultParameters::eSpike;    // Spike potential [V]
	Val eReset = DefaultParameters::eReset;    // Reset potential [V]
	Val deltaTh = DefaultParameters::deltaTh;  // Slope factor [V]
	Val tauI = DefaultParameters::tauI;        // Time constant for decay of gI
	Val tauE = DefaultParameters::tauE;        // Time constant for decay of gE
	Val tauW = DefaultParameters::tauW;        // Time constant for decay of w
	Val a = DefaultParameters::a;              // Subthreshold adaptation [S]
	Val b = DefaultParameters::b;              // Spike triggered adaptation [A]
};

/**
 * Structure holding the reduced parameter set which is actually used for the
 * calculations as well as some pre-calculated values
 */
class WorkingParameters {
private:
	/**
	 * Function used to calculate the effective spike potential.
	 */
	Val calculateESpikeEff();

public:
	Val lL;       // Membrane leak rate [Hz]
	Val lE;       // Excitatory decay rate [Hz]
	Val lI;       // Inhibitory decay rate [Hz]
	Val lW;       // Adaptation cur. decay rate [Hz]
	Val eE;       // Excitatory channel reversal potential [V]
	Val eI;       // Inhibitory channel reversal potential [V]
	Val eTh;      // Spike threshold potential [V]
	Val eSpike;   // Spike generation potential [V]
	Val eReset;   // Membrane reset potential [V]
	Val deltaTh;  // Spike slope factor [V]
	Val lA;       // Subthreshold adaptation [Hz]
	Val lB;       // Spike triggered adaptation current [V/s]
	Val wSpike;   // Weight with which the spikes should be multiplied [V/A*s]

	Val invDeltaTh;  // Reverse spike slope factor [1/V]

	Val maxIThExponent;  // Value to which the exponent is clamped
	Val eSpikeEff;       // Effective spike potential
	Val eSpikeEffRed;  // Reduced effective spike potential (used for clamping)

	static constexpr Val MIN_DELTA_T =
	    0.1e-6;  // 0.1 µS -- used for the calculation of maxIThExponent

	/**
	 * Creates working parameters for the given Parameters instance.
	 *
	 * @param p is the Parameters instance for which the WorkingParameters set
	 * should be created.
	 */
	WorkingParameters(const Parameters &p = Parameters())
	    : lL(p.gL / p.cM),
	      lE(1.0 / p.tauE),
	      lI(1.0 / p.tauI),
	      lW(1.0 / p.tauW),
	      eE(p.eE - p.eL),
	      eI(p.eI - p.eL),
	      eTh(p.eTh - p.eL),
	      eSpike(p.eSpike - p.eL),
	      eReset(p.eReset - p.eL),
	      deltaTh(p.deltaTh),
	      lA(p.a / p.cM),
	      lB(p.b / p.cM),
	      wSpike(1.0 / p.cM),
	      invDeltaTh(1.0 / p.deltaTh)
	{
		update();
	}

	/**
	 * Updates some derived values. This method must be called whenever the
	 * parameters have been changed from the outside.
	 */
	void update()
	{
		maxIThExponent = log((eSpike - eReset) / (MIN_DELTA_T * deltaTh * lL));
		eSpikeEff = calculateESpikeEff();
		eSpikeEffRed = eSpikeEff - 1e-4;
	}
};
}

#endif /* _ADEXPSIM_PARAMETERS_HPP_ */

