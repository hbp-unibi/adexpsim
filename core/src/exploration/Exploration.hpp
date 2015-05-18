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
#include <utils/Matrix.hpp>
#include <utils/Types.hpp>

#include "Evaluation.hpp"

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
	 * Matrix containing the effective spike potentials for each parameters set.
	 */
	Matrix eSpikeEff;

	/**
	 * Matrix containing the maximum potential for Xi input spikes.
	 */
	Matrix eMaxXi;

	/**
	 * Matrix containing the maximum potential for Xi - 1 input spikes.
	 */
	Matrix eMaxXiM1;

	/**
	 * Matrix containing the time at which the spike potential is reached for
	 * Xi input spikes.
	 */
	Matrix tSpike;

	/**
	 * Matrix containing the time at which the membrane potential is reset to
	 * the original one for Xi input spikes.
	 */
	Matrix tReset;

	/**
	 * Constructor of the ExplorationMemory class for a certain resolution.
	 *
	 * @param resX is the resolution of the memory in X direction.
	 * @param resY is the resolution of the memory in Y direction.
	 */
	ExplorationMemory(size_t resX = 1024, size_t resY = 1024)
	    : resX(resX),
	      resY(resY),
	      eSpikeEff(resX, resY),
	      eMaxXi(resX, resY),
	      eMaxXiM1(resX, resY),
	      tSpike(resX, resY),
	      tReset(resX, resY)
	{
	}

	/**
	 * Returns an evaluation result structure for the matrix entry at the given
	 * coordinates.
	 */
	EvaluationResult operator()(size_t x, size_t y) const
	{
		return EvaluationResult(eSpikeEff(x, y), eMaxXi(x, y), eMaxXiM1(x, y),
		                        tSpike(x, y), tReset(x, y));
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

	/**
	 * Evaluation object.
	 */
	Evaluation evaluation;

public:
	/**
	 * Callback function used to allow another function to display some kind of
	 * progress indicator. The first parameter corresponds to the progress in a
	 * range between 0 and 1, the return value determines whether the operation
	 * should be aborted (return false), or continued (return true).
	 */
	using ProgressCallback = std::function<bool(Val)>;

	/**
	 * Creates a new Exploration instance and sets all its parameters.
	 *
	 * @param mem is a reference at the underlying ExplorationMemory instance.
	 * @param params is the base parameter set.
	 * @param xi is the number of input spikes for which the neuron should
	 * spike. Must be larger than one.
	 * @param T is the delay between two incomming input spikes.
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
	            WorkingParameters &params, Val Xi, Time T, size_t dimX,
	            Val minX, Val maxX, size_t dimY, Val minY, Val maxY);

	/**
	 * Runs the exploration process, returns true if the process has completed
	 * successfully, false if it was aborted (e.g. by the "progress" function
	 * returning false).
	 *
	 * @param progress specifies the current progress as a value between zero
	 * and one.
	 * @return true if the operation was sucessful, false otherwise.
	 */
	bool run(const ProgressCallback &progress = [](Val) { return true; });

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
