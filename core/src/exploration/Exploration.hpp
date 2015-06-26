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
#include <memory>

#include <simulation/Parameters.hpp>
#include <common/Matrix.hpp>
#include <common/Types.hpp>

#include "EvaluationResult.hpp"

namespace AdExpSim {

/**
 * The ExplorationMemory structure provides the memory for an exploration run of
 * a certain resolution. It allows Exploration objects to access and modify this
 * memory.
 */
struct ExplorationMemory {
	/**
	 * Resolution in x-direction.
	 */
	size_t resX;

	/**
	 * Resolution in y-direction.
	 */
	size_t resY;

	/**
	 * Matrix containing the probability of the number of expected spikes being
	 * fulfilled for the given spike train.
	 */
	Matrix pBinary;

	/**
	 * Matrix containing the probability of false positive spikes.
	 */
	Matrix pFalsePositive;

	/**
	 * Matrix containing the probability of false negative spikes.
	 */
	Matrix pFalseNegative;

	/**
	 * Matrix containing hte probability of the number of expected spikes begin
	 * fulfilled under noise.
	 */
	Matrix pSoft;

	/**
	 * Constructor of the ExplorationMemory class for a certain resolution.
	 *
	 * @param resX is the resolution of the memory in X direction.
	 * @param resY is the resolution of the memory in Y direction.
	 */
	ExplorationMemory(size_t resX = 1024, size_t resY = 1024)
	    : resX(resX),
	      resY(resY),
	      pBinary(resX, resY),
	      pFalsePositive(resX, resY),
	      pFalseNegative(resX, resY),
	      pSoft(resX, resY)
	{
	}

	/**
	 * Returns an evaluation result structure for the matrix entry at the given
	 * coordinates.
	 */
	EvaluationResult operator()(size_t x, size_t y) const
	{
		return EvaluationResult(pBinary(x, y), pFalsePositive(x, y),
		                        pFalseNegative(x, y), pSoft(x, y));
	}

	/**
	 * Returns a value for the corresponding evaluation result dimension.
	 */
	Val operator()(size_t x, size_t y, EvaluationResultDimension dim) const
	{
		switch (dim) {
			case EvaluationResultDimension::BINARY:
				return pBinary(x, y);
			case EvaluationResultDimension::FALSE_POSITIVE:
				return pFalsePositive(x, y);
			case EvaluationResultDimension::FALSE_NEGATIVE:
				return pFalseNegative(x, y);
			case EvaluationResultDimension::SOFT:
				return pSoft(x, y);
		}
		return 0.0;
	}
};

/**
 * The Exploration class is used to run a parameter space exploration. It
 * automatically
 */
class Exploration {
private:
	/**
	 * ExplorationMemory instance on which the exploration is working.
	 */
	std::shared_ptr<ExplorationMemory> mem;

	/**
	 * Base working parameter set.
	 */
	WorkingParameters params;

	/**
	 * Parameter range along the x-axis.
	 */
	Range rangeX;

	/**
	 * Parameter range along the y-axis.
	 */
	Range rangeY;

	/**
	 * Index of the parameter dimension that is varried along the x-axis.
	 */
	size_t dimX;

	/**
	 * Index of the parameter dimension that is varried along the y-axis.
	 */
	size_t dimY;

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
	Exploration() : mem(nullptr), dimX(0), dimY(1) {}

	/**
	 * Creates a new Exploration instance and sets all its parameters.
	 *
	 * @param mem is a reference at the underlying ExplorationMemory instance.
	 * @param params is the base parameter set.
	 * @param dimX is the index of the parameter vector entry which is varried
	 * in x-direction.
	 * @param minX is the minimum parameter value in x-direction.
	 * @param maxX is the maximum parameter value in x-direction.
	 * @param dimY is the index of the parameter vector entry which is varried
	 * in y-direction.
	 * @param minY is the minimum parameter value in y-direction.
	 * @param maxY is the maximum parameter value in y-direction.
	 */
	Exploration(std::shared_ptr<ExplorationMemory> mem,
	            const WorkingParameters &params, size_t dimX, Val minX,
	            Val maxX, size_t dimY, Val minY, Val maxY);

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
	bool valid() const { return mem != nullptr; }

	/**
	 * Returns a reference at the exploration memory.
	 */
	const ExplorationMemory &getMemory() const { return *mem; }

	/**
	 * Returns a reference at the base working parameters.
	 */
	const WorkingParameters &getParams() const { return params; }

	/**
	 * Returns a reference at the x-range.
	 */
	const Range &getRangeX() const { return rangeX; }

	/**
	 * Returns a reference at the y-range.
	 */
	const Range &getRangeY() const { return rangeY; }

	/**
	 * Returns a new Exploration instance with cloned, not shared memory.
	 */
	Exploration clone() const;
};

}

#endif /* _ADEXPSIM_EXPLORATION_HPP_ */
