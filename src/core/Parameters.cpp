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

#include "Parameters.hpp"

namespace AdExpSim {

Val WorkingParameters::calculateESpikeEff(double eTh, double deltaTh)
{
	constexpr double EPS = 1e-9;

	// Abort if deltaTh is near zero. Additionally,  as shown (TM), threshold
	// potentials smaller or equal to deltaTh will result in a instable neuron
	// state and cause spikes no matter the current voltage is.
	if (deltaTh < EPS || eTh <= deltaTh) {
		return std::numeric_limits<Val>::lowest();
	}

	// Copy some variables for convenient access
	const double invDeltaTh = 1.0 / deltaTh;
	const double logDeltaTh = log(deltaTh);

	// Start a newton iteration
	double x = eTh + EPS;
	while (true) {
		const double dx = (logDeltaTh + (x - eTh) * invDeltaTh - log(x)) *
		               (x * deltaTh) / (x - deltaTh);
		x -= dx;
		if (fabs(dx) < EPS) {
			break;
		}
	}
	return x;
}
}

