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
 *  GNU General Public License for more details.d.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file Optimization.hpp
 *
 * Contains a threaded implementation of the Parameter optimization
 * implementation. Uses a simplex from multiple starting points. Can respect
 * the hardware parameter boundaries.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_OPTIMIZATION_HPP_
#define _ADEXPSIM_OPTIMIZATION_HPP_

#include <atomic>
#include <functional>
#include <vector>

#include <exploration/EvaluationResult.hpp>
#include <simulation/Model.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/HardwareParameters.hpp>

namespace AdExpSim {

class Pool;

/**
 * Contains a single result returned by the optimizer.
 */
struct OptimizationResult {
	/**
	 * The actual parameter vector returned by the optimizer.
	 */
	WorkingParameters params;

	/**
	 * Evaluation result for the requested dimension
	 */
	Val eval;

	/**
	 * Constructor, initializes all members with the given parameters.
	 */
	OptimizationResult(const WorkingParameters &params, Val eval)
	    : params(params), eval(eval)
	{
	}

	/**
	 * Comperator used for sorting.
	 */
	bool operator<(const OptimizationResult &o) { return eval < o.eval; }
};

/**
 * The Optimization class performs a threaded optimization.
 */
class Optimization {
private:
	/**
	 * Specifies whether the AdExp or IfCondExp model should be used.
	 */
	ModelType model;

	/**
	 * Evaluation result dimension that should be optimized.
	 */
	EvaluationResultDimension evalDim;

	/**
	 * Parameter dimensions that should be optimized.
	 */
	std::vector<size_t> dims;

	/**
	 * Pointer at the hardware parameters which are used to limit the parameter
	 * range or nullptr if no hardware restriction should be assumed.
	 */
	HardwareParameters const *hw;

	/**
	 * Filters the to-be-optimized dimensions based on the chosen neuron model.
	 */
	static std::vector<size_t> filterDims(ModelType model,
	                                      const std::vector<size_t> &dims);

	/**
	 * Function containing the actual optimization thread.
	 *
	 * @param eval is a reference at the object performing the actual evaluation
	 * @param optimization is a const reference at the optimization instance.
	 * @param pool is the class holding the input and output parameters.
	 */
	template <typename Evaluation>
	static void optimizationThread(const Optimization &optimization,
	                               const Evaluation &eval, Pool &pool,
	                               std::atomic<bool> &abort,
	                               std::atomic<size_t> &nIdle,
	                               std::atomic<size_t> &nIt);

public:
	/**
	 * Default constructor. Contains an invalid optimization, calls to optimize
	 * will return directly.
	 */
	Optimization();

	/**
	 * Constructor of the Optimization class. Initializes an optimization with
	 * hardware restrictions.
	 *
	 * @param model specifies whether the AdExp or IfCondExp model should be
	 * used.
	 * @param evalDim is the result dimension in the evaluation that should be
	 * optimized.
	 * @param dims as an array containing the working parameter dimensions that
	 * should be optimized.
	 * @param hw is a reference at a HardwareParameters object specifying the
	 * hardware restrictions.
	 */
	Optimization(ModelType model, EvaluationResultDimension evalDim,
	             const std::vector<size_t> &dims, const HardwareParameters &hw);

	/**
	 * Constructor of the Optimization class. This optimization instance will
	 * not respect hardware restrictions.
	 *
	 * @param model specifies whether the AdExp or IfCondExp model should be
	 * used.
	 * @param evalDim is the result dimension in the evaluation that should be
	 * optimized.
	 * @param dims as an array containing the working parameter dimensions that
	 * should be optimized.
	 */
	Optimization(ModelType model, EvaluationResultDimension evalDim,
	             const std::vector<size_t> &dims);

	/**
	 * Callback function which gets called periodically to inform the calling
	 * thread that the optimization is still running. Contains a reference at
	 * the current optimization results. The return value determines whether the
	 * operation should be aborted (return false), or continued (return true).
	 */
	using ProgressCallback = std::function<
	    bool(size_t, size_t, const std::vector<OptimizationResult> &)>;

	/**
	 * Optimizes the given parameters for the selected model and evaluation
	 * type. Informs the calling thread about the progress via the
	 * ProgressCallback callback.
	 */
	template <typename Evaluation>
	std::vector<OptimizationResult> optimize(
	    const std::vector<WorkingParameters> &params, const Evaluation &eval,
	    ProgressCallback callback) const;

	/**
	 * Returns the to-be-optimized parameters. If "clampDiscrete" is set to true
	 * the in-hardware discrete parameters are not added to the result.
	 */
	std::vector<size_t> getDims(bool clampDiscrete) const;
};
}

#endif /* _ADEXPSIM_OPTIMIZATION_HPP_ */

