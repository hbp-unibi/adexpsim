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
 *  GNU General Public License for more details.d.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file Simplex.hpp
 *
 * Contains the Downhill Simplex algorithm by Nelder und Mead.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SIMPLEX_HPP_
#define _ADEXPSIM_SIMPLEX_CPP_

#include <vector>

#include <utils/Types.hpp>

namespace AdExpSim {

class Simplex {
	template <typename Vector, typename Function>
	static Vector optimize(Vector v0, Function f, Val epsilon = 1e-6,
	                       Val alpha = 1.0, Val gamma = 2.0, Val rho = -0.5,
	                       Val sigma = 0.5)
	{
		
	}
};
};

#endif /* _ADEXPSIM_SIMPLEX_HPP_ */

