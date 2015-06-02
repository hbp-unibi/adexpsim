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
 * @file Integrator.hpp
 *
 * Contains basic integrator classes which implement the Euler, Midpoint and
 * fourth-order Runge-Kutta method.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_INTEGRATOR_HPP_
#define _ADEXPSIM_INTEGRATOR_HPP_

#include <utils/Types.hpp>

#include "Parameters.hpp"
#include "State.hpp"

namespace AdExpSim {
/**
 * The EulerIntegrator class represents euler's method for integrating ODEs. Do
 * not use this. For debugging only.
 */
class EulerIntegrator {
public:
	/**
	 * Implements the second-order Runge-Kutta method (Midpoint method).
	 *
	 * @param tDelta is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	static std::pair<State, Time> integrate(Time tDelta, Time, const State &s,
	                                        Deriv df)
	{
		const Val h = tDelta.sec();
		return std::pair<State, Time>(s + h * df(s), tDelta);
	}
};

/**
 * The MidpointIntegrator class implements the second-order Runge-Kutta
 * method.
 */
class MidpointIntegrator {
public:
	/**
	 * Implements the second-order Runge-Kutta method (Midpoint method).
	 *
	 * @param tDelta is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	static std::pair<State, Time> integrate(Time tDelta, Time, const State &s,
	                                        Deriv df)
	{
		const Val h = tDelta.sec();
		const State k1 = h * df(s);
		const State k2 = h * df(s + 0.5f * k1);

		return std::pair<State, Time>(s + k2, tDelta);
	}
};

/**
 * The RungeKuttaIntegrator class implements the fourth-order Runge-Kutta
 * method.
 */
class RungeKuttaIntegrator {
public:
	/**
	 * Implements the fourth-order Runge-Kutta method.
	 *
	 * @param tDelta is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	static std::pair<State, Time> integrate(Time tDelta, Time, const State &s,
	                                        Deriv df)
	{
		const Val h = tDelta.sec();
		const State k1 = h * df(s);
		const State k2 = h * df(s + 0.5f * k1);
		const State k3 = h * df(s + 0.5f * k2);
		const State k4 = h * df(s + k3);

		return std::pair<State, Time>(s + (k1 + 2.0f * (k2 + k3) + k4) / 6.0f,
		                              tDelta);
	}
};
}

#endif /* _ADEXPSIM_INTEGRATOR_HPP_ */

