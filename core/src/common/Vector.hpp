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
#include <cmath>

#include "Types.hpp"

namespace AdExpSim {

/**
 * Class containing a one dimensional, fixed size numerical vector. The alignas
 * specifier is important to allow the compiler to use SSE instructions.
 */
template <typename Impl, size_t N>
class alignas(16) Vector {
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
	static constexpr size_t Size = N;

	Vector(const Arr &arr) : arr(arr) {}

	Val sqrL2Norm() const {
		Val res = 0;
		for (size_t i = 0; i < N; i++) {
			res += arr[i] * arr[i];
		}
		return res * Val(1.0 / double(N));
	}

	Val L2Norm() const {
		return sqrtf(sqrL2Norm());
	}

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
 * The Vec4 class represents a vector with four Val entries. Defines a set of
 * operators that should use the SIMD extensions of the targeted processor
 * architecture.
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

#define NAMED_VECTOR_ELEMENT(NAME, IDX) \
	void NAME(Val x) { arr[IDX] = x; }  \
	Val &NAME() { return arr[IDX]; }    \
	Val NAME() const { return arr[IDX]; }
}

#endif /* _ADEXPSIM_VECTOR_HPP_ */

