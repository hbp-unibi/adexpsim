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
 * @file Vector.hpp
 *
 * Contains the Vector and the Vec4 class.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_VECTOR_HPP_
#define _ADEXPSIM_VECTOR_HPP_

#include <array>

#include "Types.hpp"

namespace AdExpSim {

/**
 * Class containing a one dimensional, fixed size numerical vector.
 */
template <typename Impl, size_t N>
class Vector {
protected:
	/**
	 * Internal array containing the data.
	 */
	std::array<Val, N> arr;

public:
	/**
	 * Shortcut referencing the type of this class.
	 */
	using T = Vector<Impl, N>;
	using Arr = std::array<Val, N>;

	Vector(const Arr &arr) : arr(arr) {}

	template <typename Func>
	friend Impl map(const T &v1, const T &v2, Func f)
	{
		Arr arr;
		for (size_t i = 0; i < N; i++) {
			arr[i] = f(v1.arr[i], v2.arr[i]);
		}
		return Impl(arr);
	}

	template <typename Func>
	friend Impl map(const T &v, Func f)
	{
		Arr arr;
		for (size_t i = 0; i < N; i++) {
			arr[i] = f(v.arr[i]);
		}
		return Impl(arr);
	}

	template <typename Func>
	friend void map2(T &v1, const T &v2, Func f)
	{
		for (size_t i = 0; i < N; i++) {
			f(v1.arr[i], v2.arr[i]);
		}
	}

	Val &operator[](size_t idx) { return arr[idx]; }

	Val operator[](size_t idx) const { return arr[idx]; }

	friend void operator+=(T &v1, const T &v2)
	{
		map2(v1, v2, [](Val &a, Val b) { return a += b; });
	}

	friend void operator-=(T &v1, const T &v2)
	{
		map2(v1, v2, [](Val &a, Val b) { return a -= b; });
	}

	friend void operator*=(T &v1, const T &v2)
	{
		map2(v1, v2, [](Val &a, Val b) { return a *= b; });
	}

	friend void operator/=(T &v1, const T &v2)
	{
		map2(v1, v2, [](Val &a, Val b) { return a /= b; });
	}

	friend Impl operator+(const T &v1, const T &v2)
	{
		return map(v1, v2, [](Val a, Val b) { return a + b; });
	}

	friend Impl operator-(const T &v1, const T &v2)
	{
		return map(v1, v2, [](Val a, Val b) { return a - b; });
	}

	friend Impl operator*(const T &v1, const T &v2)
	{
		return map(v1, v2, [](Val a, Val b) { return a * b; });
	}

	friend Impl operator/(const T &v1, const T &v2)
	{
		return map(v1, v2, [](Val a, Val b) { return a / b; });
	}

	friend Impl operator*(Val s, const T &v)
	{
		return map(v, [s](Val a) { return s * a; });
	}

	friend Impl operator*(const T &v, Val s)
	{
		return map(v, [s](Val a) { return a * s; });
	}

	friend Impl operator/(const T &v, Val s)
	{
		return map(v, [s](Val a) { return a / s; });
	}
};

/**
 * Remove this define if you are using a compiler that does not allow the
 * definition of a 4-element vector type as specified below.
 */
#define USE_GCC_VEC4

#ifdef USE_GCC_VEC4

/**
 * Vector data type which is used as a special optimization of the Vec4 class.
 */
typedef Val vec4_t __attribute__((vector_size(sizeof(Val) * 4)));

/**
 * The Vec4 class represents a vector with four Val entries. Defines a set of
 * operators that should use the SIMD extensions of the targeted processor
 * architecture.
 */
template <typename Impl>
class Vec4 {
protected:
	/**
	 * Type containing the actual data. Use the vec4_t type which if possible
	 * uses the processor vector extensions.
	 */
	vec4_t arr;

public:
	using T = Vec4<Impl>;

	Vec4(Val v0, Val v1, Val v2, Val v3) : arr(vec4_t{v0, v1, v2, v3}) {}

	Vec4(vec4_t arr) : arr(arr) {}

	Val &operator[](size_t idx) { return arr[idx]; }

	Val operator[](size_t idx) const { return arr[idx]; }

	friend void operator+=(T &v1, const T &v2) { v1.arr += v2.arr; }

	friend void operator-=(T &v1, const T &v2) { v1.arr -= v2.arr; }

	friend void operator*=(T &v1, const T &v2) { v1.arr *= v2.arr; }

	friend void operator/=(T &v1, const T &v2) { v1.arr /= v2.arr; }

	friend Impl operator+(const T &v1, const T &v2)
	{
		return Impl(v1.arr + v2.arr);
	}

	friend Impl operator-(const T &v1, const T &v2)
	{
		return Impl(v1.arr - v2.arr);
	}

	friend Impl operator*(const T &v1, const T &v2)
	{
		return Impl(v1.arr * v2.arr);
	}

	friend Impl operator/(const T &v1, const T &v2)
	{
		return Vec4(v1.arr / v2.arr);
	}

	friend Impl operator*(Val s, const T &v) { return Impl(s * v.arr); }

	friend Impl operator*(const T &v, Val s) { return Impl(v.arr * s); }

	friend Impl operator/(const T &v, Val s) { return Impl(v.arr / s); }
};

#else

/**
 * Stub implementation of an unoptimized Vec4.
 */
template <typename Impl>
class Vec4 : public Vector<Impl, 4> {
public:
	using Vector<Impl, 4>::Vector;

	Vec4(Val v0, Val v1, Val v2, Val v3)
	    : Vector<Impl, 4>({v0, v1, v2, v3})
	{
	}
};

#endif

#define NAMED_VECTOR_ELEMENT(NAME, IDX) \
	void NAME(Val x) { arr[IDX] = x; }  \
	Val &NAME() { return arr[IDX]; }    \
	Val NAME() const { return arr[IDX]; }
}

#endif /* _ADEXPSIM_VECTOR_HPP_ */

