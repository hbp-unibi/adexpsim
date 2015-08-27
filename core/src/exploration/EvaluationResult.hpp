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
 * @file EvaluationResult.hpp
 *
 * Contains the EvaluationResult structure which is shared between all
 * implemented evaluation methods.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_EVALUATION_RESULT_HPP_
#define _ADEXPSIM_EVALUATION_RESULT_HPP_

#include <string>
#include <vector>

#include <common/Types.hpp>

namespace AdExpSim {
/**
 * The EvaluationType enum defines the evaluation method which is going to be
 * used in the simulation.
 */
enum class EvaluationType : int {
	/**
     * Uses a spike train template for the evaluation -- determines how well the
     * actual simulation matches a predefined spike train.
     */
	SPIKE_TRAIN = 0,

	/**
     * Only uses a single spike group for the evaluation and checks it for
     * fulfilling the binary threshold and reset condition.
     */
	SINGLE_GROUP = 1,

	/**
     * Same as SINGLE_GROUP, but allows multiple output spikes. Internally uses
     * the far more sophisticated FractionalSpikeCount measure.
     */
	SINGLE_GROUP_MULTI_OUT = 2
};

/**
 * Available evaluation result dimensions.
 */
enum class EvaluationResultDimension {
	SOFT = 0,
	BINARY = 1,
	FALSE_POSITIVE = 2,
	FALSE_NEGATIVE = 3
};

/**
 * The EvaluationResult class stores a single result from an evaluation method.
 * It simply consists of an vector of elements of an particular fixed size and
 * provides methods for setting and reading these values. Note that all values
 * need to be interpreted in the scope of an EvaluationResultDescriptor.
 */
struct EvaluationResult {
	/**
	 * Actual vector of values.
	 */
	std::vector<Val> values;

	/**
	 * Creates a new EvaluationResult instance with a given size.
	 *
	 * @param size is the number of dimensions in the result vector.
	 */
	EvaluationResult(size_t size = 0) : values(size, Val(0.0)) {}

	/**
	 * Creates a new EvaluationResult instance with a given size.
	 *
	 * @param size is the number of dimensions in the result vector.
	 */
	EvaluationResult(std::initializer_list<Val> init) : values(init) {}

	/**
	 * Returns a const reference at the i-th entry.
	 */
	const Val &operator[](size_t i) const { return values[i]; }

	/**
	 * Returns a reference at the i-th entry.
	 */
	Val &operator[](size_t i) { return values[i]; }

	/**
	 * Returns the number of dimensions in the result vector.
	 */
	size_t size() const { return values.size(); }
};

/**
 * The EvaluationResultDescriptor describes the result vector returned by a
 * certain evaluation methods. It contains the canonical size of the result, the
 * names of the dimensions (both human readable names and ids) and their units.
 */
class EvaluationResultDescriptor {
private:
	/**
	 * Evaluation type descriptor.
	 */
	EvaluationType mType;

	/**
	 * Number of dimensions.
	 */
	size_t mSize;

	/**
	 * Dimension which should be optimized when performing any optimization.
	 * This is also the dimension which will be displayed first in the
	 * exploration view of the GUI.
	 */
	size_t mOptimizationDim;

	/**
	 * Human-readable names of the dimensions.
	 */
	std::vector<std::string> mNames;

	/**
	 * Ids of the dimensions to be used internally.
	 */
	std::vector<std::string> mIds;

	/**
	 * Units of the individual dimensions.
	 */
	std::vector<std::string> mUnits;

	/**
	 * Contains the default values of each each result dimension.
	 */
	EvaluationResult mDefault;

	/**
	 * Value range of the unit. If a range is open in any direction (the range
	 * boundaries are set to the numeric limits of Val or +-Inf), anyone using
	 * the data is supposed to rescale the dimension according to the actually
	 * occuring values.
	 */
	std::vector<Range> mRanges;

public:
	/**
	 * Default constructor of the EvaluationResultDescriptor class. Creates an
	 * empty result descriptor.
	 */
	EvaluationResultDescriptor(
	    EvaluationType type = EvaluationType::SPIKE_TRAIN)
	    : mType(type), mSize(0), mOptimizationDim(0)
	{
	}

	/**
	 * Adds a new entry to the result descriptor, returning a reference to this
	 * instance of the result descriptor. This allows to construct an
	 * EvaluationResultDescriptor as a chain of calls to the "add()" method.
	 */
	EvaluationResultDescriptor &add(const std::string &name,
	                                const std::string &id,
	                                const std::string &unit,
	                                const Val defaultValue = 0.0,
	                                const Range &range = Range::unbounded(),
	                                bool optimize = false)
	{
		mNames.push_back(name);
		mIds.push_back(id);
		mUnits.push_back(unit);
		mDefault.values.push_back(defaultValue);
		mRanges.push_back(range);
		if (optimize) {
			mOptimizationDim = mSize;
		}
		mSize++;
		return *this;
	}

	EvaluationType type() const { return mType; }

	size_t size() const { return mSize; }
	size_t optimizationDim() const { return mOptimizationDim; }

	const std::vector<std::string> &names() const { return mNames; }
	const std::vector<std::string> &ids() const { return mIds; }
	const std::vector<std::string> &units() const { return mUnits; }
	const std::vector<Range> &ranges() const { return mRanges; }
	const EvaluationResult &defaultResult() const { return mDefault; }

	const std::string &name(size_t i) const { return mNames[i]; }
	const std::string &id(size_t i) const { return mIds[i]; }
	const std::string &unit(size_t i) const { return mUnits[i]; }
	const Range &range(size_t i) const { return mRanges[i]; }

	bool bounded(size_t i) const { return mRanges[i].bounded(); }
	bool open(size_t i) const { return mRanges[i].open(); }
};
}

#endif /* _ADEXPSIM_EVALUATION_RESULT_HPP_ */
