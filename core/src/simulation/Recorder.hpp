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

#include <algorithm>
#include <limits>
#include <ostream>

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
	Parameters const *params;

	/**
	 * Minimum time between two recorded events, if the events are not forced.
	 */
	Time interval;

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
	RecorderBase(const Parameters &params, Time interval = Time(0))
	    : params(&params), interval(interval)
	{
		reset();
	}

	/**
	 * Resets the vector recorder, allowing the same instance to record another
	 * simulation.
	 */
	void reset() { last = Time(TimeType(-(interval.t + 1))); }

	/**
	 * Called whenever an input spike is consumed by the model
	 *
	 * @param t is the time at which the input spike has been consumed.
	 * @param s is the state after the input spike has been consumed.
	 */
	void inputSpike(Time t, const State &s)
	{
		// The default record implementation does not care for spikes
	}

	/**
	 * Called whenever an output spike is produced by the model.
	 *
	 * @param t is the time at which the output spike has been produced.
	 * @param s is the neuron state after the spike has been issued (the neuron
	 * has already been reset).
	 */
	void outputSpike(Time t, const State &s)
	{
		// The default record implementation does not care for spikes
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
			State ss = s * params->cM;
			AuxiliaryState ass = as * params->cM;

			// Make sure the timestamps are monotonous.
			if (t <= last) {
				t = last + 1e-6_s;
			}

			// Call the actual record function with the correctly rescaled
			// values.
			static_cast<Impl *>(this)->doRecord(t, s.v() + params->eL, ss[1],
			                                    ss[2], ss[3], ass[0], ass[1],
			                                    ass[2], ass[3]);

			// Set the last timestamp to the new timestamp
			last = t;
		}
	}

	/**
	 * Returns a reference at the parameter descriptor.
	 */
	const Parameters &getParameters() const { return *params; }
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
 * VectorRecorderDataSample represents a single sample in the VectorRecorderData
 * instance and can be obtained by calling the "interpolate" function.
 */
struct VectorRecorderDataSample : public Vector<VectorRecorderDataSample, 8> {
	double ts;

	using Vector<VectorRecorderDataSample, 8>::Vector;

	VectorRecorderDataSample(double t, Val v, Val gE, Val gI, Val w, Val iL,
	                         Val iE, Val iI, Val iTh)
	    : Vector<VectorRecorderDataSample, 8>({v, gE, gI, w, iL, iE, iI, iTh}),
	      ts(t)
	{
	}

	NAMED_VECTOR_ELEMENT(v, 0);
	NAMED_VECTOR_ELEMENT(gE, 1);
	NAMED_VECTOR_ELEMENT(gI, 2);
	NAMED_VECTOR_ELEMENT(w, 3);
	NAMED_VECTOR_ELEMENT(iL, 4);
	NAMED_VECTOR_ELEMENT(iE, 5);
	NAMED_VECTOR_ELEMENT(iI, 6);
	NAMED_VECTOR_ELEMENT(iTh, 7);
};

/**
 * The VectorRecorderData is the class used to store the data recorded by the
 * VectorRecorder class.
 */
template <typename Vector>
class VectorRecorderData {
private:
	/**
	 * Adjusts the given min/max value pair to the value x.
	 *
	 * @param min is the variable to which the minimum value should be written.
	 * @param max is the variable to which the maximum value should be written.
	 * @param x is the value according to which min and max should be adapted.
	 */
	static void minMax(Val &min, Val &max, Val x)
	{
		if (x < min) {
			min = x;
		}
		if (x > max) {
			max = x;
		}
	}

public:
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

	/**
	 * Minimum recorded time.
	 */
	Val minTime;

	/**
	 * Maximum recorded time.
	 */
	Val maxTime;

	/**
	 * Minimum recorded voltage.
	 */
	Val minVoltage;

	/**
	 * Maximum recorded voltage.
	 */
	Val maxVoltage;

	/**
	 * Minimum recorded conductance.
	 */
	Val minConductance;

	/**
	 * Maximum recorded conductance.
	 */
	Val maxConductance;

	/**
	 * Minimum recorded current with rejected outliers.
	 */
	Val minCurrentSmooth;

	/**
	 * Maximum recorded current with rejected outliers.
	 */
	Val maxCurrentSmooth;

	/**
	 * Default constructor, resets the data instance to its initial state.
	 */
	VectorRecorderData() { reset(); }

	/**
	 * Returns the number of elements stored in the VectorRecorderData.
	 */
	size_t size() const { return ts.size(); }

	/**
	 * Returns true if the VectorRecorderData instance is empty.
	 */
	bool empty() const { return ts.empty(); }

	/**
	 * Returns a subset of the recorded data.
	 *
	 * @param t1 is the timestamp of the first sample in the resulting slice.
	 * @param t2 is the timestamp of the last sample in the resulting slice.
	 */
	VectorRecorderData slice(double t1, double t2) const
	{
		VectorRecorderData res;

		// If no data is available, abort
		if (empty()) {
			return res;
		}

		// Append the first interpolated timestamp
		res.append(interpolate(t1));

		// Append all other timestamps
		const auto it1 = std::upper_bound(ts.begin(), ts.end(), t1);
		const auto it2 = std::lower_bound(ts.begin(), ts.end(), t2);
		size_t i1 = std::distance(ts.begin(), it1);
		size_t i2 = std::distance(ts.begin(), it2);
		for (size_t i = i1; i < i2; i++) {
			res.append((*this)[i]);
		}

		// Append the last interpolated timestamp
		res.append(interpolate(t2));

		return res;
	}

	/**
	 * Returns a VectorRecorderDataSample compund instance for the given index.
	 */
	VectorRecorderDataSample operator[](size_t i) const
	{
		return VectorRecorderDataSample(ts[i], v[i], gE[i], gI[i], w[i], iL[i],
		                                iE[i], iI[i], iTh[i]);
	}

	/**
	 * Returns a linearly interpolated VectorRecorderDataSample for the given
	 * timestamp. Must not be called if this VectorRecorderData instance is
	 * empty.
	 */
	VectorRecorderDataSample interpolate(double t) const
	{
		// If there is only one entry, we cannot interpolate
		if (size() == 1) {
			return (*this)[0];
		}

		// Fetch the next larger and smaller timestamp (it2 and it1)
		auto it2 = std::upper_bound(ts.begin(), ts.end(), t);
		auto it1 = it2--;
		if (it2 == ts.begin()) {
			it2++;
			it1 = ts.begin();
		}
		size_t i1 = std::distance(ts.begin(), it1);
		size_t i2 = std::distance(ts.begin(), it2);

		// Calculate the interpolation factor f and perform linear interpolation
		const Val f = (t - *it1) / (*it2 - *it1);
		VectorRecorderDataSample res = (*this)[i1] * (1.0f - f) + (*this)[i2] * f;
		res.ts = t;
		return res;
	}

	/**
	 * Appends a single data value and updates the minimum/maximum values.
	 */
	void append(double t, Val v, Val gE, Val gI, Val w, Val iL, Val iE, Val iI,
	            Val iTh)
	{
		// Calculate compund values
		Val iSum = w + iL + iE + iI + iTh;

		// Adjust the minimum/maximum values
		minMax(minTime, maxTime, t);
		minMax(minVoltage, maxVoltage, v);
		minMax(minConductance, maxConductance, gE);
		minMax(minConductance, maxConductance, gI);

		// Adjust the smooth minimum/maximum values (does not contain iTh)
		minMax(minCurrentSmooth, maxCurrentSmooth, w);
		minMax(minCurrentSmooth, maxCurrentSmooth, iL);
		minMax(minCurrentSmooth, maxCurrentSmooth, iE);
		minMax(minCurrentSmooth, maxCurrentSmooth, iI);

		// Store the data in the vector
		this->ts.push_back(t);
		this->v.push_back(v);
		this->gE.push_back(gE);
		this->gI.push_back(gI);
		this->w.push_back(w);
		this->iL.push_back(iL);
		this->iE.push_back(iE);
		this->iI.push_back(iI);
		this->iTh.push_back(iTh);
		this->iSum.push_back(iSum);
	}

	/**
	 * Appends a single data value and updates the minimum/maximum values.
	 */
	void append(const VectorRecorderDataSample &s)
	{
		append(s.ts, s.v(), s.gE(), s.gI(), s.w(), s.iL(), s.iE(), s.iI(),
		       s.iTh());
	}

	/**
	 * Resets the data instance.
	 */
	void reset()
	{
		// Reset the vectors.
		ts.clear();
		v.clear();
		gE.clear();
		gI.clear();
		w.clear();
		iL.clear();
		iE.clear();
		iI.clear();
		iTh.clear();
		iSum.clear();

		// Reset the min/max values
		minTime = std::numeric_limits<Val>::max();
		maxTime = std::numeric_limits<Val>::lowest();
		minVoltage = std::numeric_limits<Val>::max();
		maxVoltage = std::numeric_limits<Val>::lowest();
		minConductance = std::numeric_limits<Val>::max();
		maxConductance = std::numeric_limits<Val>::lowest();
		minCurrentSmooth = std::numeric_limits<Val>::max();
		maxCurrentSmooth = std::numeric_limits<Val>::lowest();
	}
};

/**
 * The DefaultRecorderTransformation class is used by the vector recorder to
 * provide a default transformation that does not change the data.
 */
struct DefaultRecorderTransformation {
	static double transformTime(double t) { return t; }
	static Val transformVoltage(Val v) { return v; }
	static Val transformConductance(Val g) { return g; }
	static Val transformCurrent(Val i) { return i; }
};

/**
 * Scales the recorded values to more convenient SI-prefixes. Volt is scaled
 * to millivolt, siemens is scaled to millisiemens and ampere is scaled to
 * nanoampere, seconds turn to milliseconds.
 */
struct SIPrefixTransformation {
	static constexpr Val TIME_SCALE = 1000.0;
	static constexpr Val VOLTAGE_SCALE = 1000.0;
	static constexpr Val CONDUCTANCE_SCALE = 1000.0 * 1000.0;
	static constexpr Val CURRENT_SCALE = 1000.0 * 1000.0 * 1000.0;

	static double transformTime(double t) { return t * TIME_SCALE; }
	static Val transformVoltage(Val v) { return v * VOLTAGE_SCALE; }
	static Val transformConductance(Val g) { return g * CONDUCTANCE_SCALE; }
	static Val transformCurrent(Val i) { return i * CURRENT_SCALE; }
};

/**
 * The VectorRecorder class records the simulation to memory and uses the
 * specified vector type for this operation. Additionally, it tracks the minimum
 * and maximum values for each recorded modality.
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
		// Transform the values
		double t = trafo.transformTime(ts.sec());
		v = trafo.transformVoltage(v);
		gE = trafo.transformConductance(gE);
		gI = trafo.transformConductance(gI);
		w = trafo.transformCurrent(w);
		iL = trafo.transformCurrent(iL);
		iE = trafo.transformCurrent(iE);
		iI = trafo.transformCurrent(iI);
		iTh = trafo.transformCurrent(iTh);

		// Append them to the data store
		data.append(t, v, gE, gI, w, iL, iE, iI, iTh);
	}

public:
	/**
	 * Creates a new instance of the RecorderBase class.
	 *
	 * @param interval is the minimum time between two recorded events (default:
	 * record every event).
	 */
	VectorRecorder(const Parameters &params, Time interval = Time(0))
	    : Base(params, interval)
	{
	}

	/**
	 * Resets the vector recorder, allowing the same instance to record another
	 * simulation.
	 */
	void reset()
	{
		Base::reset();
		data.reset();
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

