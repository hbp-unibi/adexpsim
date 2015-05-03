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
 * @file State.hpp
 *
 * Contains structures used to represent the current state of a neuron.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_STATE_HPP_
#define _ADEXPSIM_STATE_HPP_

#include <utils/Types.hpp>
#include <utils/Vector.hpp>

#include "Parameters.hpp"

namespace AdExpSim {

/**
 * The State class contains the state of a single neuron. This state consists
 * of the membrane voltage v, the excitatory channel rate lE, the inhibitory
 * channel rate lI and the adaptive current induced voltage change rate dvW.
 */
class State : public Vec4<State> {
public:
	/**
	 * Inherit the base class constructors.
	 */
	using Vec4::Vec4;

	/**
	 * Constructor of the State class which initializes all members.
	 *
	 * @param v is the initial membrane potential in [V].
	 * @param lE is the initial excitatory leak rate in [1/s].
	 * @param lI is the initial inhibitory leak rate in [1/s].
	 * @param dvW is the initial adaptive voltage change rate in [V/s].
	 */
	State(Val v = 0.0, Val lE = 0.0, Val lI = 0.0, Val dvW = 0.0)
	    : Vec4(vec4_t{v, lE, lI, dvW})
	{
	}

	void v(Val v) { arr[0] = v; }

	void lE(Val lE) { arr[1] = lE; }

	void lI(Val lI) { arr[2] = lI; }

	void dvW(Val dvW) { arr[3] = dvW; }

	Val &v() { return arr[0]; }

	Val &lE() { return arr[1]; }

	Val &lI() { return arr[2]; }

	Val &dvW() { return arr[3]; }

	const Val &v() const { return arr[0]; }

	const Val &lE() const { return arr[1]; }

	const Val &lI() const { return arr[2]; }

	const Val &dvW() const { return arr[3]; }
};

/**
 * The AuxiliaryState class contains auxiliary variables used to calculate the
 * actual state. The auxiliary state consists of the voltage change rates. The
 * unit of all auxiliary variables is [V/s].
 */
class AuxiliaryState: public Vec4<AuxiliaryState> {
public:
	/**
	 * Inherit the base class constructors.
	 */
	using Vec4::Vec4;

	/**
	 * Constructor of the AuxiliaryState class which initializes all members
	 * with the given values.
	 *
	 * @param dvL is the leak induced voltage change rate [V/s].
	 * @param dvE is the excitatory current induced voltage change rate [V/s].
	 * @param dvI is the inhibitor current induced voltage change rate [V/s].
	 * @param dvTh is the threshold current induced voltage change rate [V/s].
	 */
	AuxiliaryState(Val dvL = 0.0, Val dvE = 0.0, Val dvI = 0.0, Val dvTh = 0.0)
	    : Vec4(vec4_t{dvL, dvE, dvI, dvTh})
	{
	}

	void dvL(Val dvL) { arr[0] = dvL; }

	void dvE(Val dvE) { arr[1] = dvE; }

	void dvI(Val dvI) { arr[2] = dvI; }

	void dvTh(Val dvTh) { arr[3] = dvTh; }

	Val &dvL() { return arr[0]; }

	Val &dvE() { return arr[1]; }

	Val &dvI() { return arr[2]; }

	Val &dvTh() { return arr[3]; }

	const Val &dvL() const { return arr[0]; }

	const Val &dvE() const { return arr[1]; }

	const Val &dvI() const { return arr[2]; }

	const Val &dvTh() const { return arr[3]; }
};

}

#endif /* _ADEXPSIM_STATE_HPP_ */

