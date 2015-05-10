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
 * @file Exploration.hpp
 *
 * Implements the Exploration process, iterates over a two dimensional parameter
 * space.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_EXPLORATION_
#define _ADEXPSIM_EXPLORATION_

#include <utils/Types.hpp>

namespace AdExpSim {

/**
 * The Exploration class performs a two dimensional exploration on 
 */
class Exploration {
private:
	WorkingParameters params;
	Val Xi;
	Val T;
	size_t dimX;
	size_t dimY;
	Range rangeX;
	Range rangeY;

	Matrix eSpikeEff;
	Matrix eMaxXi;
	Matrix eMaxXiM1;
	Matrix tMaxXi;
	Matrix tMaxXiM1;

public:
	Exploration(const WorkingParameters &params, Val Xi, Val T, size_t dimX,
	            size_t dimY, const Range &rangeX, const Range &rangeY);
};
};

#endif /* _ADEXPSIM_EXPLORATION_OPENCL_ */

