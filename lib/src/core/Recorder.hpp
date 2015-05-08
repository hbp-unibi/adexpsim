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
 * @file Recorder.hpp
 *
 * Contains various Recorders which can be used to record the data returned from
 * the neuron simulator.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_RECORDER_HPP_
#define _ADEXPSIM_RECORDER_HPP_

#include <ostream>
#include <limits>

#include <utils/Types.hpp>

#include "Parameters.hpp"
#include "State.hpp"

namespace AdExpSim {

/**
 * Recorder is a common base class for all Recorder implementations. Uses the
 * Curiously Recurring Template Pattern (CRTP) to allow non-virtual
 * polymorphism.
 */
template <typename Impl>
class RecorderBase {
private:
	/**
	 * Reference at the parameters that should be used to rescale the internal,
	 * state values to more interpretable values with the correct units.
	 */
	const Parameters &params;

	/**
	 * Minimum time between two recorded events, if the events are not forced.
	 */
	const Time interval;

	/**
	 * Time of the last event.
	 */
	Time last;

public:
	/**
	 * Creates a new instance of the RecorderBase class.
	 *
	 * @param interval is the minimum time between two recorded events (default:
	 * record every event).
	 */
	RecorderBase(const Parameters &params, Time interval = 0)
	    : params(params), interval(interval), last(-(interval + Time(1)))
	{
	}

	/**
	 * Called by the simulation to record the current internal state.
	 *
	 * @param t is the current timestamp.
	 * @param s is the current state.
	 * @param as is the current auxiliary state.
	 * @param force is set to true if the given data is of special importance
	 * and should be recorded in any case.
	 */
	void record(Time t, const State &s, const AuxiliaryState &as, bool force)
	{
		// Only call the actual record method if the time between two record
		// calls is larger than the initially specified interval.
		if (t - last > interval || force) {
			// Scale state and auxiliary state by the membrane capacitance
			State ss = s * params.cM;
			AuxiliaryState ass = as * params.cM;

			// Make sure the timestamps are monotonous.
			if (t <= last) {
				t = Time(last.t + TimeType(1));
			}

			// Call the actual record function with the correctly rescaled
			// values.
			static_cast<Impl *>(this)->doRecord(t, s.v() + params.eL, ss[1],
			                                    ss[2], ss[3], ass[0], ass[1],
			                                    ass[2], ass[3]);

			// Set the last timestamp to the new timestamp
			last = t;
		}
	}

	/**
	 * Returns a reference at the parameter descriptor.
	 */
	const Parameters &getParameters() { return params; }
};

/**
 * The NullRecorder class can be used to discard all incomming data for maximum
 * efficiency (in the case the data does not need to be recorded).
 */
class NullRecorder {
public:
	/**
	 * Actually called by the simulation to record the internal state, however
	 * this class just acts as a null sink for this data.
	 */
	void record(Time, const State &, const AuxiliaryState &, bool)
	{
		// Discard everything
	}
};

/**
 * The CsvRecorder class records the incomming data to an output stream as
 * simple delimiter separated values.
 *
 * @tparam recordAux should be set to true if auxiliary values ought to be
 * recorded.
 */
template <bool recordAux = true>
class CsvRecorder : public RecorderBase<CsvRecorder<recordAux>> {
	friend RecorderBase<CsvRecorder<recordAux>>;

private:
	/**
	 * Target output stream.
	 */
	std::ostream &os;

	/**
	 * Used separator.
	 */
	std::string sep;

	/**
	 * Actual record function, gets the correctly rescaled state variables and
	 * prints them to the given output stream.
	 */
	void doRecord(Time ts, Val v, Val gE, Val gI, Val w, Val iL, Val iE, Val iI,
	              Val iTh)
	{
		os << ts << sep << v << sep << gE << sep << gI << sep << w;
		if (recordAux) {
			os << sep << iL << sep << iE << sep << iI << sep << iTh;
		}
		os << std::endl;
	}

public:
	CsvRecorder(const Parameters &params, Time interval, std::ostream &os,
	            std::string sep = ",", bool header = true)
	    : RecorderBase<CsvRecorder<recordAux>>(params, interval),
	      os(os),
	      sep(sep)
	{
		if (header) {
			os << "v" << sep << "gE" << sep << "gI" << sep << "w";
			if (recordAux) {
				os << sep << "iL" << sep << "iE" << sep << "iI" << sep << "iTh";
			}
			os << std::endl;
		}
	}
};

/**
 * The VectorRecorderData is the class used to store the data recorded by the
 * VectorRecorder class.
 */
template <typename Vector>
struct VectorRecorderData {
	/**
	 * Vector used to store the incomming time stamps.
	 */
	Vector ts;

	/**
	 * Vector used to store the membrane voltage over time.
	 */
	Vector v;

	/**
	 * Vector used to store the excitatory channel conductance over time.
	 */
	Vector gE;

	/**
	 * Vector used to store the inhibitory channel conductance over time.
	 */
	Vector gI;

	/**
	 * Vector used to store the adaptive current over time.
	 */
	Vector w;

	/**
	 * Vector used to store the leak channel current over time.
	 */
	Vector iL;

	/**
	 * Vector used to store the excitatory channel current over time.
	 */
	Vector iE;

	/**
	 * Vector used to store the inhibitory channel current over time.
	 */
	Vector iI;

	/**
	 * Vector used to store the threshold channel current over time.
	 */
	Vector iTh;

	/**
	 * Vector used to store sum current.
	 */
	Vector iSum;
};

/**
 * The DefaultRecorderTransformation class is used by the vector recorder to
 * provide a default transformation that does not change the data.
 */
struct DefaultRecorderTransformation {
	Val transformTs(Val ts) const { return ts; }
	Val transformV(Val v) const { return v; }
	Val transformGE(Val gE) const { return gE; }
	Val transformGI(Val gI) const { return gI; }
	Val transformW(Val w) const { return w; }
	Val transformIL(Val iL) const { return iL; }
	Val transformIE(Val iE) const { return iE; }
	Val transformII(Val iI) const { return iI; }
	Val transformITh(Val iTh) const { return iTh; }
};

/**
 * The VectorRecorder class records the simulation data to the given
 */
template <typename Vector,
          typename Transformation = DefaultRecorderTransformation>
class VectorRecorder
    : public RecorderBase<VectorRecorder<Vector, Transformation>> {
public:
	using Base = RecorderBase<VectorRecorder<Vector, Transformation>>;
	friend Base;

private:
	/**
	 * Transformation instance used by this class.
	 */
	Transformation trafo;

	/**
	 * Data container used for storing the incomming data.
	 */
	VectorRecorderData<Vector> data;

	/**
	 * Actual record function, gets the correctly rescaled state variables and
	 * prints them to the given output stream.
	 */
	void doRecord(Time ts, Val v, Val gE, Val gI, Val w, Val iL, Val iE, Val iI,
	              Val iTh)
	{
		data.ts.push_back(trafo.transformTs(ts.toSeconds()));
		data.v.push_back(trafo.transformV(v));
		data.gE.push_back(trafo.transformGE(gE));
		data.gI.push_back(trafo.transformGI(gI));
		data.w.push_back(trafo.transformW(w));
		data.iL.push_back(trafo.transformIL(iL));
		data.iE.push_back(trafo.transformIE(iE));
		data.iI.push_back(trafo.transformII(iI));
		data.iTh.push_back(trafo.transformITh(iTh));
		data.iSum.push_back(trafo.transformW(w) + trafo.transformIL(iL) +
		                    trafo.transformIE(iE) + trafo.transformII(iI) +
		                    trafo.transformITh(iTh));
	}

public:
	/**
	 * Creates a new instance of the RecorderBase class.
	 *
	 * @param interval is the minimum time between two recorded events (default:
	 * record every event).
	 */
	VectorRecorder(const Parameters &params, Time interval = 0)
	    : Base(params, interval)
	{
	}

	/**
	 * Returns a reference at the recorded data.
	 */
	const VectorRecorderData<Vector> &getData() const { return data; }

	/**
	 * Returns a reference at the transformation instance.
	 */
	const Transformation &getTrafo() const { return trafo; }
};
}

#endif /* _ADEXPSIM_RECORDER_HPP_ */

