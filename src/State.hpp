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

#include "Parameters.hpp"
#include "Types.hpp"

namespace AdExpSim {

struct State {
	Val v;   // Current membrane potential [V]
	Val gE;  // Conductance of the exitatory channel [S]
	Val gI;  // Conductance of the inhibitory channle [S]
	Val w;   // Adaptation current [A]

	/**
	 * Default constructor
	 */
	State() : State(0.0) {}

	/**
	 * Constructor of the State variable with an initial membrane potential.
	 */
	State(Val v) : v(v), gE(0.0), gI(0.0), w(0.0) {}
};

struct AuxiliaryState {
	Val iL;   // Leak current [A]
	Val iE;   // Excitatory channel current [A]
	Val iI;   // Inhibitory channel current [A]
	Val iTh;  // Above-threshold current [A]

	AuxiliaryState() : iL(0.0), iE(0.0), iI(0.0), iTh(0.0) {}
};

class StateRecorder {
private:
	TimeVec t;
	ValVec v;
	ValVec gE;
	ValVec gI;
	ValVec w;

public:
	const TimeVec &getT() { return t; }
	const ValVec &getV() { return v; }
	const ValVec &getGE() { return gE; }
	const ValVec &getGI() { return gI; }
	const ValVec &getW() { return w; }

	void record(Time ts, const State &state, const AuxiliaryState &auxState)
	{
		t.push_back(ts);
		v.push_back(state.v);
		gE.push_back(state.gE);
		gI.push_back(state.gI);
		w.push_back(state.w);
	}
};

class AuxiliaryStateRecorder : public StateRecorder {
private:
	ValVec iL;
	ValVec iE;
	ValVec iI;
	ValVec iTh;

public:
	const ValVec &getIL() { return iL; }
	const ValVec &getIE() { return iE; }
	const ValVec &getII() { return iI; }
	const ValVec &getITh() { return iTh; }

	void record(Time ts, const State &state, const AuxiliaryState &auxState)
	{
		StateRecorder::record(ts, state, auxState);
		iL.push_back(auxState.iL);
		iE.push_back(auxState.iE);
		iI.push_back(auxState.iI);
		iTh.push_back(auxState.iTh);
	}
};

class StreamRecorder {
private:
	std::ostream &os;
	std::string sep;
	bool recordAux;

public:
	StreamRecorder(std::ostream &os, const std::string &sep = ",",
	               bool recordAux = true)
	    : os(os), sep(sep), recordAux(recordAux)
	{
	}

	void record(Time ts, const State &state, const AuxiliaryState &auxState)
	{
		// Record the standard parameters
		os << ts << sep << state.v << sep << state.gE << sep << state.gI << sep
		   << state.w;

		// Record the auxiliary parameters
		if (recordAux) {
			os << sep << auxState.iL << sep << auxState.iE << sep << auxState.iI
			   << sep << auxState.iTh;
		}

		// End the output line
		os << std::endl;
	}
};

class BinaryStreamRecorder {
private:
	std::ostream &os;
	bool recordAux;

public:
	BinaryStreamRecorder(std::ostream &os, bool recordAux = true) : os(os)
	{
		// Write some easy to interpret JSON header
		os << "{";

		// Number of columns
		os << "\"nCols\":" << (recordAux ? 9 : 5) << ",";

		// Number of bytes per column entry (to distiguish floats from doubles)
		os << "\"nBytes\":" << sizeof(Val) << ",";

		// Name of the columns
		os << "\"cols\": [\"t\",\"v\",\"gE\",\"gI\",\"w\""
		   << (recordAux ? ",\"iL\",\"iE\",\"iI\",\"iTh\"" : "") << "]";

		// Terminate the header with a linebreak
		os << "}" << std::endl;
	}

	void record(Time ts, const State &state, const AuxiliaryState &auxState)
	{
		// Move the time in seconds onto the stack
		Val t = ts.toSeconds();

		// Write the timestamp and the data
		os.write((char *)&t, sizeof(Val));
		os.write((char *)&state.v, sizeof(Val));
		os.write((char *)&state.gE, sizeof(Val));
		os.write((char *)&state.gI, sizeof(Val));
		os.write((char *)&state.w, sizeof(Val));

		// Record the auxiliary data
		if (recordAux) {
			os.write((char *)&state.iL, sizeof(Val));
			os.write((char *)&state.iE, sizeof(Val));
			os.write((char *)&state.iI, sizeof(Val));
			os.write((char *)&state.iTh, sizeof(Val));
		}
	}
};
}

#endif /* _ADEXPSIM_STATE_HPP_ */

