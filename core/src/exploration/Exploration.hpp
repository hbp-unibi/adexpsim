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
 * @file Exploration.hpp
 *
 * Implements the Exploration process, iterates over a two dimensional parameter
 * space.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_EXPLORATION_HPP_
#define _ADEXPSIM_EXPLORATION_HPP_

#include <functional>

#include <simulation/Parameters.hpp>
#include <common/Matrix.hpp>
#include <common/Types.hpp>

#include "EvaluationResult.hpp"

namespace AdExpSim {
/**
 * The ExplorationMemory structure provides the memory for an exploration run of
 * a certain resolution. It allows Exploration objects to access and modify this
 * memory. Copying an ExplorationMemory object is cheap due to copy on write
 * semantics of the underlying Matrix class.
 */
struct ExplorationMemory {
	/**
	 * Copy of the evaluation result descriptor.
	 */
	EvaluationResultDescriptor descriptor;

	/**
	 * Resolution in x-direction.
	 */
	size_t resX;

	/**
	 * Resolution in y-direction.
	 */
	size_t resY;

	/**
	 * Vector of matrices containing each result dimension.
	 */
	std::vector<Matrix> data;

	/**
	 * Vector of ranges containing the min/max values occuring in that
	 * dimension.
	 */
	std::vector<Range> extrema;

	/**
	 * Default constructor, creates an empty memory instance.
	 */
	ExplorationMemory() : resX(0), resY(0) {}

	/**
	 * Constructor of the ExplorationMemory class for a certain resolution.
	 *
	 * @param resX is the resolution of the memory in X direction.
	 * @param resY is the resolution of the memory in Y direction.
	 */
	ExplorationMemory(const EvaluationResultDescriptor &descriptor, size_t resX,
	                  size_t resY)
	    : descriptor(descriptor),
	      resX(resX),
	      resY(resY),
	      data(descriptor.size()),
	      extrema(descriptor.size())
	{
		for (size_t i = 0; i < descriptor.size(); i++) {
			data[i].resize(resX, resY);
			extrema[i] = Range::invalid();
		}
	}

	/**
	 * Returns an evaluation result structure for the matrix entry at the given
	 * coordinates.
	 */
	EvaluationResult operator()(size_t x, size_t y) const
	{
		EvaluationResult res(data.size());
		for (size_t i = 0; i < data.size(); i++) {
			res[i] = data[i](x, y);
		}
		return res;
	}

	/**
	 * Returns an the value of the evaluation at the given position.
	 */
	Val operator()(size_t x, size_t y, size_t dim) const
	{
		return data[dim](x, y);
	}

	/**
	 * Stores an EvaluationResult in the memory.
	 */
	void store(size_t x, size_t y, const EvaluationResult &res)
	{
		for (size_t i = 0; i < std::min(data.size(), res.size()); i++) {
			data[i](x, y) = res[i];
			extrema[i].expand(res[i]);
		}
	}

	/**
	 * Returns the data range for the given dimension. If an explicitly bounded
	 * range is specified in the EvaluationResultDescriptor this range is used,
	 * otherwise the actual data range is used.
	 */
	Range range(size_t i) const
	{
		return Range(descriptor.range(i).openMax() ? extrema[i].max
		                                           : descriptor.range(i).max,
		             descriptor.range(i).openMin() ? extrema[i].min
		                                           : descriptor.range(i).min);
	}

	/**
	 * Returns true if there is actually any data stored inside the memory.
	 */
	bool valid() const { return resX > 0 && resY > 0 && data.size() > 0; }
};

/**
 * The Exploration class is used to run a parameter space exploration. Note that
 * the Exploration class uses copy on write semantics, so copying an Exploration
 * class is actually quite cheap as the cloned instance will share the same
 * buffer until one of the buffers is modified.
 */
class Exploration {
private:
	/**
	 * ExplorationMemory instance on which the exploration is working.
	 */
	ExplorationMemory mMem;

	/**
	 * Explore working parameters or full parameters?
	 */
	bool mUseFullParams;

	/**
	 * Base parameter set.
	 */
	Parameters mFullParams;

	/**
	 * Base working parameters set.
	 */
	WorkingParameters mParams;

	/**
	 * Index of the parameter dimension that is varried along the x-axis.
	 */
	size_t mDimX;

	/**
	 * Index of the parameter dimension that is varried along the y-axis.
	 */
	size_t mDimY;

	/**
	 * Parameter range along the x-axis.
	 */
	DiscreteRange mRangeX;

	/**
	 * Parameter range along the y-axis.
	 */
	DiscreteRange mRangeY;

public:
	/**
	 * Callback function used to allow another function to display some kind of
	 * progress indicator. The first parameter corresponds to the progress in a
	 * range between 0 and 1, the return value determines whether the operation
	 * should be aborted (return false), or continued (return true).
	 */
	using ProgressCallback = std::function<bool(Val)>;

	/**
	 * Default constructor. Resulting exploration is invalid.
	 */
	Exploration() : mDimX(0), mDimY(1) {}

	/**
	 * Creates a new Exploration instance and sets all its parameters.
	 *
	 * @param params is the base parameter set.
	 * @param dimX is the index of the parameter vector entry which is varried
	 * in x-direction.
	 * @param dimY is the index of the parameter vector entry which is varried
	 * @param rangeX is the range descriptor for the X-direction.
	 * @param rangeY is the range descriptor for the Y-direction.
	 */
	Exploration(const WorkingParameters &params, size_t dimX, size_t dimY,
	            DiscreteRange rangeX, DiscreteRange rangeY)
	    : mUseFullParams(false),
	      mFullParams(params.toParameters(DefaultParameters::cM,
	                                      DefaultParameters::eL)),
	      mParams(params),
	      mDimX(dimX),
	      mDimY(dimY),
	      mRangeX(rangeX),
	      mRangeY(rangeY){};

	/**
	 * Constructor which allows to construct an exploration instance which
	 * explores the full parameter space instead of the limited, DoF reduced
	 * working parameter space.
	 *
	 * @param useFullParams if true, the full parameter set is used, if false
	 * the given parameters are converted to the DoF reduced parameter set
	 * first.
	 * @param params is the base parameter set.
	 * @param dimX is the index of the parameter vector entry which is varried
	 * in x-direction.
	 * @param dimY is the index of the parameter vector entry which is varried
	 * @param rangeX is the range descriptor for the X-direction.
	 * @param rangeY is the range descriptor for the Y-direction.
	 */
	Exploration(bool useFullParams, const Parameters &params, size_t dimX,
	            size_t dimY, DiscreteRange rangeX, DiscreteRange rangeY)
	    : mUseFullParams(useFullParams),
	      mFullParams(params),
	      mParams(params),
	      mDimX(dimX),
	      mDimY(dimY),
	      mRangeX(rangeX),
	      mRangeY(rangeY){};

	/**
	 * Runs the exploration process, returns true if the process has completed
	 * successfully, false if it was aborted (e.g. by the "progress" function
	 * returning false).
	 *
	 * @param evaluation is a reference at a class with an "evaluate" method
	 * that calculates the actual cost function values.
	 * @param progress specifies the current progress as a value between zero
	 * and one.
	 * @return true if the operation was sucessful, false otherwise.
	 */
	template <typename Evaluation>
	bool run(const Evaluation &evaluation,
	         const ProgressCallback &progress = [](Val) { return true; });

	/**
	 * Flag indicating whether the exploration is valid or not.
	 */
	bool valid() const { return mMem.valid(); }

	/**
	 * Returns a reference at the exploration memory.
	 */
	const ExplorationMemory &mem() const { return mMem; }

	/**
	 * Returns a reference at the evaluation result descriptor.
	 */
	const EvaluationResultDescriptor &descriptor() const
	{
		return mMem.descriptor;
	}

	/**
	 * Returns a reference at the base working parameters.
	 */
	const WorkingParameters &params() const { return mParams; }

	/**
	 * Returns a reference at the base working parameters.
	 */
	const Parameters &fullParams() const { return mFullParams; }

	/**
	 * Flag indicating whether the full parameter set should be used.
	 */
	bool useFullParams() const { return mUseFullParams; }

	/**
	 * Returns the resolution in x-direction.
	 */
	size_t resX() const { return mRangeX.steps; }

	/**
	 * Returns the resolution in y-direction.
	 */
	size_t resY() const { return mRangeY.steps; }

	/**
	 * Returns a reference at the x-range.
	 */
	const DiscreteRange &rangeX() const { return mRangeX; }

	/**
	 * Returns a reference at the y-range.
	 */
	const DiscreteRange &rangeY() const { return mRangeY; }

	/**
	 * Returns the x-dimension.
	 */
	size_t dimX() const { return mDimX; }

	/**
	 * Returns the y-dimension.
	 */
	size_t dimY() const { return mDimY; }
};
}

#endif /* _ADEXPSIM_EXPLORATION_HPP_ */
