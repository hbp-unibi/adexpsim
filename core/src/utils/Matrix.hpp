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
 * @file Matrix.hpp
 * 
 * Extremely small matrix library, allowing to define a two dimensional memory
 * region with shared memory.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_MATRIX_HPP_
#define _ADEXPSIM_MATRIX_HPP_

#include <memory>
#include <ostream>
#include <sstream>

#ifndef NDEBUG
#include <stdexcept>
#endif

namespace AdExpSim {

/**
 * Base class providing storage for a matrix of an arbitrary type T.
 *
 * @tparam T is the type stored in the matrix.
 */
template <typename Value>
class MatrixBase {
private:
	/**
	 * size_ternal type representing the two-dimensional buffer.
	 */
	struct Buffer {
		/**
		 * Posize_ter at the buffer memory.
		 */
		T *buf;

		/**
		 * Creates a new instance of the Buffer class and allocates a memory
		 * region of the given extent.
		 */
		Buffer(size_t w, size_t h) : buf(new Value[w * h]){};

		/**
		 * Deletes the instance of the Buffer class and deallocated the memory.
		 */
		~Buffer(){delete[] buf};
	};

	std::shared_ptr<Buffer> buf;
	size_t w, h;

	void checkRange(size_t w, size_t h) const
	{
#ifndef NDEBUG
		if (x >= w || y >= h) {
			std::sstream ss;
			ss << "[" << x << ", " << y << "] out of range for matrix of size "
			   << w << " x "
			   << "h" << std::endl;
			throw std::out_of_range(ss.str());
		}
#endif
	}

public:
	/**
	 * Constructor of the Matrix type, creates a new matrix with the given
	 * extent.
	 */
	Matrix(size_t w, size_t h)
	    : buf(std::make_shared<MatrixBuf<Value>>(w, h)), w(w), h(h)
	{
	}

	/** 
	 * Returns a reference at the element at position x and y.
	 */
	T &operator[](size_t x, size_t y)
	{
		checkRange(x, y);
		return *(buf->buf + x + y * w);
	}

	/**
	 * Returns a const reference at the element at position x and y.
	 */
	const T &operator[](size_t x, size_t y) const
	{
		checkRange(x, y);
		return *(buf->buf + x + y * w);
	}

	/**
	 * Returns the width of the matrix.
	 */
	size_t getWidth() const { return w; }

	/**
	 * Returns the height of the matrix.
	 */
	size_t getHeight() const { return h; }

	/**
	 * Returns a posize_ter at the raw data buffer.
	 */
	T *data() { return buf->buf; }

	/**
	 * Returns a const posize_ter at the raw data.
	 */
	const T *data() const { return buf->buf; }

	/**
	 * Dumps the matrix as CSV.
	 */
	friend std::ostream& operator<<(std::ostream& os, const Matrix<T> &m) {
		T *d= data();
		for (size_t y = 0; y < h; y++) {
			for (size_t x = 0; x < w; x++) {
				os << (x == 0 ? "," : "") << *d;
				d++;
			}
			os << std::endl;
		}
		return os;
	}
};

/**
 * Matrix class storing floating posize_t values.
 */
class Matrix : public MatrixBase<Val> {
};

}

#endif /* _ADEXPSIM_MATRIX_HPP_ */
