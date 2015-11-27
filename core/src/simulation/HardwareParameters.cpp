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

#include <algorithm>
#include <limits>

#include "HardwareParameters.hpp"

namespace AdExpSim {

// Maps from index in the ranges list at corresponding parameter indices
static const std::vector<std::vector<size_t>> adExpRangeParamMap = {
    {Parameters::idx_eTh, Parameters::idx_eSpike, Parameters::idx_eReset},
    {Parameters::idx_eL},
    {Parameters::idx_eE},
    {Parameters::idx_eI},
    {Parameters::idx_gL},
    {Parameters::idx_tauE, Parameters::idx_tauI},
    {Parameters::idx_tauW},
    {Parameters::idx_tauRef},
    {Parameters::idx_a},
    {Parameters::idx_b},
    {Parameters::idx_deltaTh}};
static const std::vector<std::vector<size_t>> ifCondExpRangeParamMap = {
    {Parameters::idx_eTh, Parameters::idx_eReset},
    {Parameters::idx_eL},
    {Parameters::idx_eE},
    {Parameters::idx_eI},
    {Parameters::idx_gL},
    {Parameters::idx_tauE, Parameters::idx_tauI},
    {Parameters::idx_tauW},
    {Parameters::idx_tauRef}};

// Maps containing the indices of the parameters storing potentials in the
// WorkingParameters vector.
static const std::vector<size_t> eDimAdExp = {
    WorkingParameters::idx_eE, WorkingParameters::idx_eI,
    WorkingParameters::idx_eTh, WorkingParameters::idx_eSpike,
    WorkingParameters::idx_eReset};
static const std::vector<size_t> eDimIfCondExp = {
    WorkingParameters::idx_eE, WorkingParameters::idx_eI,
    WorkingParameters::idx_eTh, WorkingParameters::idx_eReset};

template <typename T>
static Val nearest(Val v, const T &vs)
{
	// Search the nearest value
	Val minDist = std::numeric_limits<Val>::max();
	Val nearest = 0;
	for (Val elem : vs) {
		const Val dist = fabs(elem - v);
		if (dist < minDist) {
			nearest = elem;
			minDist = dist;
		}
	}
	return nearest;
}

template <typename T>
static bool contains(Val v, const T &vs)
{
	for (Val elem : vs) {
		if (v == elem) {
			return true;
		}
	}
	return false;
}

const std::vector<const Range *> HardwareParameters::ranges() const
{
	return {&rE,    &rEL,   &rEE, &rEI, &rGL,      &rTau,
	        &rTauW, &rTauRef, &rA,  &rB,  &rDeltaTh, &rW};
}

bool HardwareParameters::valid(const Parameters &params,
                               bool useIfCondExp) const
{
	// Check whether the discrete parameters are ok
	bool ok = contains(params.cM(), cMs) && contains(params.w(), ws);

	// Fetch pointers at all ranged available in this class and the correct
	// mapping between these ranges and the parameter indices
	const std::vector<const Range *> rs = ranges();
	const std::vector<std::vector<size_t>> &rangeParamMap =
	    useIfCondExp ? ifCondExpRangeParamMap : adExpRangeParamMap;

	// Iterate over all ranges to check whether the parameters match
	for (size_t i = 0; i < rangeParamMap.size(); i++) {
		for (size_t j = 0; j < rangeParamMap[i].size(); j++) {
			ok = ok && rs[i]->contains(params[rangeParamMap[i][j]]);
		}
	}
	return ok;
}

bool HardwareParameters::clamp(Parameters &params, bool useIfCondExp) const
{
	// Select the next best cM and w
	params.cM() = nearest(params.cM(), cMs);
	params.w() = nearest(params.w(), ws);

	// Fetch pointers at all ranged available in this class and the correct
	// mapping between these ranges and the parameter indices
	const std::vector<const Range *> rs = ranges();
	const std::vector<std::vector<size_t>> &rangeParamMap =
	    useIfCondExp ? ifCondExpRangeParamMap : adExpRangeParamMap;

	// Iterate over all ranges and clamp the parameters
	for (size_t i = 0; i < rangeParamMap.size(); i++) {
		for (size_t j = 0; j < rangeParamMap[i].size(); j++) {
			params[rangeParamMap[i][j]] =
			    rs[i]->clamp(params[rangeParamMap[i][j]]);
		}
	}

	// Clamping always works
	return true;
}

std::vector<Val> HardwareParameters::nextWeights(Val w) const
{
	// Search the two nearest weights
	std::vector<Val>::const_iterator it1 =
	    std::lower_bound(ws.begin(), ws.end(), w);
	std::vector<Val>::const_iterator it2 = ws.end();
	if (it1 != ws.begin()) {
		it2 = it1 - 1;
	}

	// Add them to the result list (if they are valid)
	std::vector<Val> res;
	if (it1 != ws.end()) {
		res.push_back(*it1);
	}
	if (it2 != ws.end()) {
		res.push_back(*it2);
	}
	return res;
}

Parameters HardwareParameters::fixParameters(const Parameters &p,
                                             bool useIfCondExp) const
{
	const std::vector<const Range *> rs = ranges();
	const std::vector<std::vector<size_t>> &rangeParamMap =
	    useIfCondExp ? ifCondExpRangeParamMap : adExpRangeParamMap;

	Parameters res = p;
	for (size_t i = 0; i < rangeParamMap.size(); i++) {
		if (rs[i]->max == rs[i]->min) {
			for (size_t j = 0; j < rangeParamMap[i].size(); j++) {
				res[rangeParamMap[i][j]] = rs[i]->min;
			}
		}
	}
	return res;
}

std::vector<Parameters> HardwareParameters::map(const WorkingParameters &params,
                                                bool useIfCondExp,
                                                bool strict) const
{
	// Result vector
	std::vector<Parameters> res;

	// First step: Set the voltage offset correctly
	/*	Val eMin = std::numeric_limits<Val>::max(),
	        eMax = std::numeric_limits<Val>::lowest();

	    // List containing the relevant parameters for the IfCondExp/AdExp model
	    const std::vector<size_t> &eDims = useIfCondExp ? eDimIfCondExp :
	   eDimAdExp;
	    for (size_t i = 0; i < eDims.size(); i++) {
	        eMin = std::min(eMin, params[eDims[i]]);
	        eMax = std::max(eMax, params[eDims[i]]);
	    }

	    // Check for voltage range being too large -- abort if in strict mode
	    const Val lRE1 = rE.max - rE.min;
	    const Val lRE2 = eMax - eMin;
	    if (lRE2 > lRE1 && strict) {
	        return res;
	    }

	    // Center the leak potential in the middle of the available range
	    const Val cRE1 = (rE.max + rE.min) / 2.0;
	    const Val cRE2 = (eMax + eMin) / 2.0;
	    const Val eL = cRE1 - cRE2;*/
	const Val eL = -50e-3;

	// Calculate parameters for the membrane potentials and select the two
	// nearest available weights
	for (Val cM : cMs) {
		// Convert from the reduced parameter space to the parameter space with
		// the selected cM and eL
		Parameters p = fixParameters(params.toParameters(cM, eL), useIfCondExp);

		// Check whether the weight is in range -- abort if it is too large
		if (strict && !rW.contains(p.w())) {
			continue;
		}

		// Fetch the next possible weights and store the two parameter variants
		// in the result list if they are valid
		for (Val w : nextWeights(p.w())) {
			Parameters cp = p;
			cp.w() = w;
			if (valid(cp, useIfCondExp) ||
			    (!strict && clamp(cp, useIfCondExp))) {
				res.emplace_back(cp);
			}
		}
	}
	return res;
}

bool HardwareParameters::possible(const WorkingParameters &params,
                                  bool useIfCondExp, bool strict) const
{
	return map(params, useIfCondExp, strict).size() > 0;
}

const BrainScaleSParameters BrainScaleSParameters::inst;

BrainScaleSParameters::BrainScaleSParameters()
{
	// Possible capacities
	cMs = {0.2e-9};

	// Possible weights
	for (Val i = 0; i < 16; i++) {
		ws.push_back(0.3e-6 / 15.0 * i);
	}

	// Copy the ranges
	rE = {-100e-3, 0e-3};          // Voltage range
	rEL = {-50e-3, -50e-3};        // Fixed leak potential
	rEE = {0.0e-3, 0.0e-3};        // Fixed excitatory reversal potential
	rEI = {-100.0e-3, -100.0e-3};  // Fixed inhibitory reversal potential
	rGL = {cMs[0] / 110.001e-3f,
	       cMs[0] / 9e-3f};        // Membrane leak conductance range
	rTau = {0.5e-3, 5e-3};         // Time constant range
	rTauW = {20e-3, 780e-3};       // w decay time constant range
	rTauRef = {0.0e-3, 20e-3};     // Refractory time range
	rA = {0e-6, 0.108228e-9};      // Subthreshold adaptation range
	rB = {0e-12, 86e-12};          // Spike triggered adaptation range
	rDeltaTh = {0.0e-3, 1.35e-3};  // Slope range
	rW = {0e-6, 0.3e-6};           // Weight range
}
}

