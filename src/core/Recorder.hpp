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
			// Set the last timestamp to the new timestamp
			last = t;

			// Scale state and auxiliary state by the membrane capacitance
			State ss = s * params.cM;
			AuxiliaryState ass = as * params.cM;

			// Call the actual record function with the correctly rescaled
			// values.
			static_cast<Impl *>(this)->doRecord(t, s.v() + params.eL, ss[1],
			                                    ss[2], ss[3], ass[0], ass[1],
			                                    ass[2], ass[3]);
		}
	}

	/**
	 * Returns a reference at the parameter descriptor.
	 */
	const Parameters &getParameters() { return params; }
};

/**
 * The NullRecorder class can be used to discard all incomming data for maximum
 * efficiency.
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
private:
	friend RecorderBase<CsvRecorder<recordAux>>;

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
				os << sep << "iL" << sep << "iE" << sep << "iI" << sep
				   << "iTh";
			}
			os << std::endl;
		}
	}
};
}

#endif /* _ADEXPSIM_RECORDER_HPP_ */

