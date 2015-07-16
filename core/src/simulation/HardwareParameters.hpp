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
 * @file HardwareParameters.hpp
 *
 * Contains classes to represent a restricted parameter set and to convert
 * parameters from
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_HARDWARE_PARAMETERS_HPP_
#define _ADEXPSIM_HARDWARE_PARAMETERS_HPP_

#include <limits>
#include <vector>
#include <unordered_set>

#include <common/Types.hpp>

#include "Parameters.hpp"

namespace AdExpSim {
/**
 * The RestrictedParameters class is responsible for mapping a Parameters or
 * WorkingParameters instance to
 */
class HardwareParameters {
protected:
	/**
	 * Capacitances supported by the hardware (sorted).
	 */
	std::vector<Val> cMs;

	/**
	 * Supported synapse weights (sorted).
	 */
	std::vector<Val> ws;

	/**
	 * Voltage range.
	 */
	Range rE;

	/**
	 * Leak potential range.
	 */
	Range rEL;

	/**
	 * Excitatory reversal potential range.
	 */
	Range rEE;

	/**
	 * Inhibitory reversal potential range.
	 */
	Range rEI;

	/**
	 * Range for the leak conductance.
	 */
	Range rGL;

	/**
	 * Range for the channel time constants.
	 */
	Range rTau;

	/**
	 * Range for the adaptation current time constant.
	 */
	Range rTauW;

	/**
	 * Range for the refactory time.
	 */
	Range rTRef;

	/**
	 * Range for the adaptation mechanism subthreshold adaptation conductance.
	 */
	Range rA;

	/**
	 * Range for the adaptation mechanism spike triggered adaptation current.
	 */
	Range rB;

	/**
	 * Range for the spike slope.
	 */
	Range rDeltaTh;

	/**
	 * Range for the weight.
	 */
	Range rW;

	/**
	 * Returns a vector containing pointers at all range instances of this
	 * class.
	 */
	const std::vector<const Range *> ranges() const;

	/**
	 * Makes sure parameters with empty range do not deviate from their value
	 * due to numerical insufficiencies.
	 */
	Parameters fixParameters(const Parameters &p, bool useIfCondExp) const;

	// No public constructor
	HardwareParameters(){};

public:
	/**
	 * Returns true if the given Parameter set is valid.
	 */
	bool valid(const Parameters &params, bool useIfCondExp = false) const;

	/**
	 * Clamps all values to the valid ranges.
	 */
	bool clamp(Parameters &params, bool useIfCondExp = false) const;

	/**
	 * Returns the next valid weight values near the given weight.
	 */
	std::vector<Val> nextWeights(Val w) const;

	/**
	 * Maps the given working parameters to a list of possible hardware
	 * configurations. Returns an empty list of no matching parameters have been
	 * found.
	 *
	 * @param params is the set of WorkingParameters that should be mapped to
	 * hardware configutations.
	 * @param useIfCondExp if true, ignores the parameters that are only
	 * available in the ifCondExp model.
	 * @param strict if set to true, does not clamp parameters, but simply does
	 * not return a result. If false, clamps parameters to the range boundaries.
	 */
	std::vector<Parameters> map(const WorkingParameters &params,
	                            bool useIfCondExp = false,
	                            bool strict = true) const;

	/**
	 * Returns true if the map function returns at least one result for the
	 * given parameters.
	 */
	bool possible(const WorkingParameters &params, bool useIfCondExp = false,
	              bool strict = true) const;
};

/**
 * HardwareParameters filled with the correct limits for the BrainScaleS system.
 */
class BrainScaleSParameters : public HardwareParameters {
public:
	/**
	 * Creates an instance of the BrainScaleSParameters class.
	 *
	 * TODO: Add parameter for speedup factor.
	 */
	BrainScaleSParameters();

	static const BrainScaleSParameters inst;
};
}

#endif /* _ADEXPSIM_HARDWARE_PARAMETERS_HPP_ */

