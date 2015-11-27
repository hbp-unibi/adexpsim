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
 * @file Types.hpp
 *
 * Contains types used throughout the simulator such as the "Val" type used
 * for storing floating point values and the "Time" type used for representing
 * times within the simulation.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_TYPES_HPP_
#define _ADEXPSIM_TYPES_HPP_

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace AdExpSim {

/**
 * Val is the value type used for storing floating point values. This allows
 * switching to double for higher precision (if needed).
 */
using Val = float;

/**
 * Integer type used internally by the Time type to represent times.
 */
using TimeType = int64_t;

/**
 * Factor for converting a floating point time in seconds to a time value.
 */
constexpr double SEC_TO_TIME = double(1L << 48);

/**
 * Factor for converting a time value to seconds.
 */
constexpr double TIME_TO_SEC = 1.0 / SEC_TO_TIME;

/**
 * Maximum internal time value.
 */
constexpr TimeType MAX_INT_TIME = std::numeric_limits<TimeType>::max();

/**
 * Minimum internal time value.
 */
constexpr TimeType MIN_INT_TIME = std::numeric_limits<TimeType>::min();

/**
 * Time is the type used for storing time values. Times are stored as a 32 bit
 * integer in microsecond resolution (with fixed divisor 2^10). This is done to
 * avoid artifacts by shifting precision over time.
 */
struct Time {
private:
	/**
	 * Static version of the "fromSeconds" method.
	 *
	 * @param ft is the floating point value for which the corresponding
	 * integer value should be returned.
	 * @return the internal integer value used to represent the time.
	 */
	static constexpr TimeType secondsToTimeType(Val ft)
	{
		return (int64_t)(ft * SEC_TO_TIME) > MAX_INT_TIME
		           ? MAX_INT_TIME
		           : ((int64_t)(ft * SEC_TO_TIME) < MIN_INT_TIME
		                  ? MIN_INT_TIME
		                  : ft * SEC_TO_TIME);
	}

public:
	/**
	 * Internal Time value.
	 */
	TimeType t;

	/**
	 * Default constructor, initializes the time value to zero.
	 */
	Time() : t(0) {}

	/**
	 * Constructor, initializes the tiem value with the given internal integer
	 * timestamp.
	 *
	 * @param t is the internal integer time the Time instance should be
	 * initialized to.
	 */
	explicit constexpr Time(TimeType t) : t(t) {}

	/**
	 * Creates a new instance of the Time structure from a float.
	 *
	 * @param t is a time in seconds.
	 */
	static constexpr Time sec(double t) { return Time(secondsToTimeType(t)); }

	/**
	 * Creates a new instance of the Time structure from a float.
	 *
	 * @param t is a time in milliseconds.
	 */
	static constexpr Time msec(double t)
	{
		return Time(secondsToTimeType(t / 1000.0));
	}

	/**
	 * Converts the internal integer Time to a floating point time in seconds.
	 *
	 * @return the current time value in seconds.
	 */
	double sec() const { return double(t) * TIME_TO_SEC; }

	/* Operators */

	friend Time operator+(const Time &t1) { return Time(t1.t); }

	friend Time operator-(const Time &t1) { return Time(-t1.t); }

	friend Time operator+(const Time &t1, const Time &t2)
	{
		return Time(t1.t + t2.t);
	}

	friend Time operator-(const Time &t1, const Time &t2)
	{
		return Time(t1.t - t2.t);
	}

	friend Time operator/(const Time &t1, const Time &t2)
	{
		return Time(t1.t / t2.t);
	}

	friend Time operator*(const Time &t, Val s)
	{
		return Time(TimeType(t.t * s));
	}

	friend Time operator*(Val s, const Time &t)
	{
		return Time(TimeType(s * t.t));
	}

	friend Time operator*(const Time &t1, const Time &t2)
	{
		return Time(t1.t * t2.t);
	}

	friend Time operator%(const Time &t1, const Time &t2)
	{
		return Time(t1.t % t2.t);
	}

	friend void operator+=(Time &t1, const Time &t2) { t1.t += t2.t; }

	friend void operator-=(Time &t1, const Time &t2) { t1.t -= t2.t; }

	friend bool operator==(const Time &t1, const Time &t2)
	{
		return t1.t == t2.t;
	}

	friend bool operator!=(const Time &t1, const Time &t2)
	{
		return t1.t != t2.t;
	}

	friend bool operator<(const Time &t1, const Time &t2)
	{
		return t1.t < t2.t;
	}

	friend bool operator<=(const Time &t1, const Time &t2)
	{
		return t1.t <= t2.t;
	}

	friend bool operator>(const Time &t1, const Time &t2)
	{
		return t1.t > t2.t;
	}

	friend bool operator>=(const Time &t1, const Time &t2)
	{
		return t1.t >= t2.t;
	}

	friend std::ostream &operator<<(std::ostream &os, const Time &t)
	{
		return os << t.sec();
	}
};

static constexpr Time operator"" _s(long double t) { return Time::sec(t); }

static constexpr Time operator"" _s(unsigned long long int t)
{
	return Time::sec(t);
}

static constexpr Time operator"" _ms(long double t) { return Time::msec(t); }

static constexpr Time operator"" _ms(unsigned long long int t)
{
	return Time::msec(t);
}

/**
 * Maximum representable time.
 */
constexpr Time MAX_TIME = Time(MAX_INT_TIME);

/**
 * Maximum representable time.
 */
constexpr Time MIN_TIME = Time(MIN_INT_TIME);

/**
 * Maximum representable time in seconds.
 */
constexpr double MAX_TIME_SEC = MAX_INT_TIME / SEC_TO_TIME;

/**
 * Maximum representable time in seconds.
 */
constexpr double MIN_TIME_SEC = MIN_INT_TIME / SEC_TO_TIME;

/**
 * Minimum time difference representable by the Time type.
 */
constexpr double MIN_TIME_DELTA = TIME_TO_SEC;

/**
 * Vector of Time values.
 */
using TimeVec = std::vector<Time>;

/**
 * Range type, represents an interval from a minimum to a maximum value.
 */
struct Range {
	/**
	 * Minimum value of the range (inclusive).
	 */
	Val min;

	/**
	 * Maximum value of the range (inclusive).
	 */
	Val max;

	/**
	 * Default constructor, initializes the range to [0, 0].
	 */
	Range() : min(0), max(0) {}

	/**
	 * Creates a range with the given minimum/maximum values.
	 */
	Range(Val min, Val max) : min(min), max(max) {}

	static Range upperBound(Val max)
	{
		return Range(std::numeric_limits<Val>::lowest(), max);
	};

	static Range lowerBound(Val min)
	{
		return Range(min, std::numeric_limits<Val>::max());
	};

	static Range unbounded()
	{
		return Range(std::numeric_limits<Val>::lowest(),
		             std::numeric_limits<Val>::max());
	}

	static Range invalid()
	{
		return Range(std::numeric_limits<Val>::max(),
		             std::numeric_limits<Val>::lowest());
	}

	void expand(Val v)
	{
		max = std::max(v, max);
		min = std::min(v, min);
	}

	bool contains(Val v) const { return (v >= min) && (v <= max); }

	Val clamp(Val v) const { return v > max ? max : (v < min ? min : v); };

	bool valid() const { return min <= max; }

	bool openMin() const { return min <= std::numeric_limits<Val>::lowest(); }

	bool openMax() const { return max >= std::numeric_limits<Val>::max(); }

	bool open() const { return openMin() || openMax(); }

	bool boundedMin() const { return !openMin(); }

	bool boundedMax() const { return !openMax(); }

	bool bounded() const { return boundedMin() && boundedMax(); }
};

/**
 * Same as the Range class but with a discrete number of steps in between.
 * Provides means to iterate over all values in the range.
 */
struct DiscreteRange : public Range {
	class Iterator {
	private:
		size_t i;
		Val offs;
		Val scale;

	public:
		Iterator(size_t i, Val offs, Val scale) : i(i), offs(offs), scale(scale)
		{
		}
		Iterator &operator++()
		{
			i++;
			return *this;
		}
		Iterator operator++(int) { return Iterator(i + 1, offs, scale); }
		Iterator &operator--()
		{
			i--;
			return *this;
		}
		Iterator operator--(int) { return Iterator(i - 1, offs, scale); }
		Val operator*() { return Val(i) * scale + offs; }
		bool operator==(const Iterator &rhs) { return i == rhs.i; }
		bool operator!=(const Iterator &rhs) { return i != rhs.i; }
	};

	size_t steps;

	DiscreteRange() : Range(), steps(1) {}

	DiscreteRange(Val min, Val max, size_t steps = 1)
	    : Range(min, max), steps(steps)
	{
	}

	Val value(size_t i) const { return getOffs() + getScale() * i; }

	Val index(Val x) const { return (x - getOffs()) / getScale(); }

	Val getOffs() const { return min; }

	Val getScale() const { return (max - min) / Val(steps); }

	Iterator begin() { return Iterator(0, getOffs(), getScale()); }

	Iterator end() { return Iterator(steps, 0.0, 0.0); }
};
}

#endif /* _ADEXPSIM_TYPES_HPP_ */

