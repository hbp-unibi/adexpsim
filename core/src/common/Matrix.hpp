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

#include "Types.hpp"

namespace AdExpSim {

/**
 * Base class providing storage for a 2D memory region of an arbitrary type T.
 * Implements copy on write semantics.
 *
 * @tparam T is the type stored in the matrix.
 */
template <typename T>
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
		Buffer(size_t w, size_t h) : buf(new T[w * h]) {}

		/**
		 * Deletes the instance of the Buffer class and deallocated the memory.
		 */
		~Buffer() { delete[] buf; }
	};

	/**
	 * Shared pointer referencing the memory, used for the copy on write
	 * behaviour.
	 */
	std::shared_ptr<Buffer> buf;

	/**
	 * Width and height of the matrix.
	 */
	size_t w, h;

	/**
	 * Function used to check whether an access at x, y is correct. Disabled
	 * in release mode.
	 */
	void checkRange(size_t x, size_t y) const
	{
#ifndef NDEBUG
		if (x >= w || y >= h) {
			std::stringstream ss;
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
	MatrixBase(size_t w, size_t h)
	    : buf(std::make_shared<Buffer>(w, h)), w(w), h(h)
	{
	}

	/**
	 * Returns a reference at the element at position x and y.
	 */
	T &operator()(size_t x, size_t y)
	{
		checkRange(x, y);
		detatch();
		return *(buf->buf + x + y * w);
	}

	/**
	 * Returns a const reference at the element at position x and y.
	 */
	const T &operator()(size_t x, size_t y) const
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
	T *data()
	{
		detatch();
		return buf->buf;
	}

	/**
	 * Returns a const posize_ter at the raw data.
	 */
	const T *data() const { return buf->buf; }

	/**
	 * Dumps the matrix as CSV.
	 */
	friend std::ostream &operator<<(std::ostream &os, const MatrixBase<T> &m)
	{
		T const *d = m.data();
		for (size_t y = 0; y < m.h; y++) {
			for (size_t x = 0; x < m.w; x++) {
				os << (x == 0 ? "" : ",") << *d;
				d++;
			}
			os << std::endl;
		}
		return os;
	}


	/**
	 * Clones the internal memory buffer, making this instance independent from
	 * changes made to the memory by others.
	 */
	MatrixBase<T> &detatch()
	{
		// Only perform the copy operation if more than one instance refers to
		// the memory
		if (buf.use_count() > 1) {
			// Hold a reference to the old buffer on the stack
			std::shared_ptr<Buffer> oldBuf = buf;

			// Create a new Buffer object
			buf = std::make_shared<Buffer>(w, h);

			// Copy the memory from the old buffer ot the new buffer
			std::copy(oldBuf->buf, oldBuf->buf + w * h, buf->buf);
		}
		return *this;
	}

	/**
	 * Returns a new matrix with the exact same properties as this matrix but
	 * with detatched memory.
	 */
	MatrixBase<T> clone() const { return MatrixBase<T>(*this).detatch(); }
};

/**
 * Matrix class storing floating posize_t values.
 */
class Matrix : public MatrixBase<Val> {
	using MatrixBase<Val>::MatrixBase;
};
}

#endif /* _ADEXPSIM_MATRIX_HPP_ */
