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

#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <utils/Timer.hpp>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

using namespace AdExpSim;
using Recorder = VectorRecorder<std::vector<Val>, SIPrefixTransformation>;
using RecorderData = VectorRecorderData<std::vector<Val>>;

constexpr size_t SimulationFlags = Model::FAST_EXP;

/**
 * The BenchmarkResult class contains the results captured from a single
 * benchmark run.
 */
struct BenchmarkResult {
	std::string name;
	RecorderData data;
	double time;

	BenchmarkResult(const std::string &name) : name(name), time(0) {}

	/**
	 * Used internally to update an entry in the given maximum vector.
	 *
	 * @param max is the vector holding the maximum values that should be
	 * updated.
	 * @param k is the index within the maximum value that should be updated.
	 * @param v1 is the first vector from which the value should be read.
	 * @param i is the index inside v1.
	 * @param v2 is the second vector (the reference vector) from which a value
	 * should be read.
	 * @param j is the index inside v2. The function reads from this index and
	 * the preceding index.
	 */
	template <typename Vector>
	static Val minDist(const Vector &v1, size_t i, const Vector &v2, size_t j)
	{
		const size_t j1 = std::min<size_t>(v2.size() - 1, j);
		const size_t j2 = std::max<size_t>(0, j - 1);
		if (j1 < v2.size() || j2 < v2.size()) {
			Val d1 = std::numeric_limits<Val>::max();
			Val d2 = std::numeric_limits<Val>::max();
			if (j1 < v2.size()) {
				d1 = fabs(v1[i] - v2[j1]);
			}
			if (j2 < v2.size()) {
				d2 = fabs(v1[i] - v2[j2]);
			}
			return std::min(d1, d2);
		}
		return 0;
	}

	/**
	 * Used internally to update an entry in the given maximum vector.
	 *
	 * @param max is the vector holding the maximum values that should be
	 * updated.
	 * @param k is the index within the maximum value that should be updated.
	 * @param v1 is the first vector from which the value should be read.
	 * @param i is the index inside v1.
	 * @param v2 is the second vector (the reference vector) from which a value
	 * should be read.
	 * @param j is the index inside v2. The function reads from this index and
	 * the preceding index.
	 */
	template <typename Vector1, typename Vector2>
	static void updateMaximum(Vector1 &max, size_t k, const Vector2 &v1,
	                          size_t i, const Vector2 &v2, size_t j)
	{
		max[k] = std::max<Val>(max[k], minDist(v1, i, v2, j));
	}

	/**
	 * Used internally to update an entry in the given squared sum vector.
	 *
	 * @param max is the vector holding the maximum values that should be
	 * updated.
	 * @param k is the index within the maximum value that should be updated.
	 * @param v1 is the first vector from which the value should be read.
	 * @param i is the index inside v1.
	 * @param v2 is the second vector (the reference vector) from which a value
	 * should be read.
	 * @param j is the index inside v2. The function reads from this index and
	 * the preceding index.
	 */
	template <typename Vector1, typename Vector2>
	static void updateSqSum(Vector1 &sum, size_t k, const Vector2 &v1, size_t i,
	                        const Vector2 &v2, size_t j, Val h)
	{
		Val d = minDist(v1, i, v2, j);
		sum[k] += d * d * h;
	}

	struct Comparison {
		static State initMin()
		{
			constexpr Val MINV = std::numeric_limits<Val>::lowest();
			return State(MINV, MINV, MINV, MINV);
		}

		static State initMax()
		{
			constexpr Val MAXV = std::numeric_limits<Val>::max();
			return State(MAXV, MAXV, MAXV, MAXV);
		}

		static State initZero() { return State(0, 0, 0, 0); }

		BenchmarkResult &benchmark;
		State maxDelta;
		State maxDeltaNormalized;
		State rmseDelta;
		State rmseDeltaNormalized;
		State refMin;
		State refMax;

		Comparison(BenchmarkResult &benchmark)
		    : benchmark(benchmark),
		      maxDelta(initMin()),
		      maxDeltaNormalized(initMin()),
		      rmseDelta(initZero()),
		      rmseDeltaNormalized(initZero()),
		      refMin(initMax()),
		      refMax(initMin())
		{
		}

		std::string printPercentage(Val val)
		{
			std::stringstream ss;
			ss << std::setprecision(5) << std::setw(6)
			   << ceil(val * 10000.0) / 100.0 << "%";
			return ss.str();
		}

		void print()
		{
			std::cout << std::setw(30) << benchmark.name << "  ";

			std::cout << "t: " << std::setprecision(4) << std::setw(8)
			          << benchmark.time << "ms ";

			std::cout << "N: " << std::setprecision(4) << std::setw(8)
			          << benchmark.data.size() << " ";

			std::cout << "t/N: " << std::setprecision(4) << std::setw(8)
			          << (benchmark.time * 1000.0) / Val(benchmark.data.size())
			          << "us ";

			// Error in the v state
			std::cout << "| v: " << std::setprecision(4) << std::setw(11)
			          << rmseDelta[0] << "mV  ";
			std::cout << printPercentage(rmseDeltaNormalized[0]) << " ";

			// Error in the gE state
			std::cout << "| gE: " << std::setprecision(4) << std::setw(11)
			          << rmseDelta[1] << "uS  ";
			std::cout << printPercentage(rmseDeltaNormalized[1]) << " ";

			// Error in the gI state
			std::cout << "| gI: " << std::setprecision(4) << std::setw(11)
			          << rmseDelta[2] << "uS ";
			std::cout << printPercentage(rmseDeltaNormalized[2]) << " ";

			// Error in the w state
			std::cout << "| w: " << std::setprecision(4) << std::setw(11)
			          << rmseDelta[3] << "nA  ";
			std::cout << printPercentage(rmseDeltaNormalized[3]) << " ";

			// Calculate and print the average error
			Val avg = 0;
			Val avgMax = 0;
			for (size_t k = 0; k < 4; k++) {
				avg += rmseDeltaNormalized[k] * 0.25;
				avgMax += maxDeltaNormalized[k] * 0.25;
			}
			std::cout << "| µ: " << printPercentage(avg);
			std::cout << std::endl;
		}
	};

	/**
	 * Calculates the maximum delta compared to the given reference vector.
	 */
	Comparison compare(const RecorderData &ref)
	{
		Comparison res(*this);

		// Iterate over all entries in this data vector
		for (size_t i = 1; i < data.size(); i++) {
			// Fetch the current timestamp
			Val t = data.ts[i];
			Val tDelta = t - data.ts[i - 1];

			// Find the two indices in the reference data next to this timestamp
			size_t j = std::distance(
			    ref.ts.begin(),
			    std::lower_bound(ref.ts.begin(), ref.ts.end(), t));

			// Update the maximum value
			updateMaximum(res.maxDelta, 0, data.v, i, ref.v, j);
			updateMaximum(res.maxDelta, 1, data.gE, i, ref.gE, j);
			updateMaximum(res.maxDelta, 2, data.gI, i, ref.gI, j);
			updateMaximum(res.maxDelta, 3, data.w, i, ref.w, j);

			// Update the rmse value
			updateSqSum(res.rmseDelta, 0, data.v, i, ref.v, j, tDelta);
			updateSqSum(res.rmseDelta, 1, data.gE, i, ref.gE, j, tDelta);
			updateSqSum(res.rmseDelta, 2, data.gI, i, ref.gI, j, tDelta);
			updateSqSum(res.rmseDelta, 3, data.w, i, ref.w, j, tDelta);
		}

		// Calculate the rmse error from the squared sum
		for (size_t k = 0; k < 4; k++) {
			res.rmseDelta[k] = sqrt(res.rmseDelta[k] / ref.ts.back());
		}

		// Load the min/max reference values
		for (size_t i = 0; i < ref.size(); i++) {
			for (size_t k = 0; k < 4; k++) {
				res.refMin[k] = std::min(res.refMin[k], ref[i][k]);
				res.refMax[k] = std::max(res.refMax[k], ref[i][k]);
			}
		}

		// Calculate the normalized deltas
		for (size_t k = 0; k < 4; k++) {
			const Val norm = (res.refMax[k] - res.refMin[k]);
			if (norm != 0) {
				res.maxDeltaNormalized[k] = res.maxDelta[k] / norm;
				res.rmseDeltaNormalized[k] = res.rmseDelta[k] / norm;
			}
		}

		return res;
	}
};

template <typename Function>
BenchmarkResult runBenchmark(std::string name, const Parameters &params,
                             Function f)
{
	Timer t;
	BenchmarkResult res(name);
	Recorder recorder(params);
	DefaultController controller;

	f(controller, recorder);
	res.time = t.time();
	res.data = recorder.getData();

	return res;
}

template <typename Integrator>
void benchmarkSimple(const std::string &name, double tDelta,
                     const Parameters &params, const SpikeTrain &train,
                     const BenchmarkResult &reference)
{
	runBenchmark(name, params,
	             [&](DefaultController &controller, Recorder &recorder) {
		             Integrator integrator;
		             Model::simulate<SimulationFlags>(
		                 train.getSpikes(), recorder, controller, integrator,
		                 params, Time::sec(tDelta), train.getMaxT());
		         })
	    .compare(reference.data)
	    .print();
}

template <typename Integrator>
void benchmarkAdaptive(const std::string &name, Val eTar,
                       const Parameters &params, const SpikeTrain &train,
                       const BenchmarkResult &reference)
{
	runBenchmark(
	    name, params, [&](DefaultController &controller, Recorder &recorder) {
		    Integrator integrator(eTar);
		    Model::simulate<SimulationFlags>(train.getSpikes(), recorder,
		                                     controller, integrator, params,
		                                     Time::sec(1e-6), train.getMaxT());
		})
	    .compare(reference.data)
	    .print();
}

int main()
{
	// Use the default parameters
	Parameters p;

	// Create a vector containing all input spikes
	SpikeTrain train({
	                  {4, 0, 1, 1e-3, 1.0, -1.0, 0.0},
	                  {4, 1, 1, 1e-3, 1.0, -1.0, 0.0},
	                  {3, 0, 0, 1e-3, 1.0, -1.0, 0.0},
	                 },
	                 100, false, 0.1_s, 0.01);

	// Generate the reference data
	std::cout << "Generating reference data..." << std::endl;
	BenchmarkResult ref =
	    runBenchmark("Runge-Kutta (t=1uS)", p,
	                 [&](DefaultController &controller, Recorder &recorder) {
		    RungeKuttaIntegrator integrator;
		    Model::simulate<SimulationFlags>(train.getSpikes(), recorder,
		                                     controller, integrator, p, 1e-6_s,
		                                     train.getMaxT());
		});
	ref.compare(ref.data).print();
	std::cout << "Done." << std::endl;
	std::cout << "--" << std::endl;

	// Print the reference data as stub
	benchmarkSimple<EulerIntegrator>("Euler (t=1us)", 1e-6, p, train, ref);
	benchmarkSimple<EulerIntegrator>("Euler (t=10us)", 10e-6, p, train, ref);
	benchmarkSimple<EulerIntegrator>("Euler (t=100us)", 100e-6, p, train, ref);
	benchmarkSimple<EulerIntegrator>("Euler (t=1ms)", 1e-3, p, train, ref);

	std::cout << "--" << std::endl;

	benchmarkSimple<MidpointIntegrator>("Midpoint (t=1us)", 1e-6, p, train,
	                                    ref);
	benchmarkSimple<MidpointIntegrator>("Midpoint (t=10us)", 10e-6, p, train,
	                                    ref);
	benchmarkSimple<MidpointIntegrator>("Midpoint (t=100us)", 100e-6, p, train,
	                                    ref);
	benchmarkSimple<MidpointIntegrator>("Midpoint (t=1ms)", 1e-3, p, train,
	                                    ref);

	std::cout << "--" << std::endl;

	benchmarkSimple<RungeKuttaIntegrator>("Runge-Kutta (t=1us)", 1e-6, p, train,
	                                      ref);
	benchmarkSimple<RungeKuttaIntegrator>("Runge-Kutta (t=10us)", 10e-6, p,
	                                      train, ref);
	benchmarkSimple<RungeKuttaIntegrator>("Runge-Kutta (t=100us)", 100e-6, p,
	                                      train, ref);
	benchmarkSimple<RungeKuttaIntegrator>("Runge-Kutta (t=1ms)", 1e-3, p, train,
	                                      ref);

	std::cout << "--" << std::endl;

	benchmarkAdaptive<DormandPrinceIntegrator>("Dormand-Prince 5 (e=1u)", 1e-6,
	                                           p, train, ref);
	benchmarkAdaptive<DormandPrinceIntegrator>("Dormand-Prince 5 (e=10u)",
	                                           10e-6, p, train, ref);
	benchmarkAdaptive<DormandPrinceIntegrator>("Dormand-Prince 5 (e=100u)",
	                                           100e-6, p, train, ref);
	benchmarkAdaptive<DormandPrinceIntegrator>("Dormand-Prince 5 (e=1m)", 1e-3,
	                                           p, train, ref);
	benchmarkAdaptive<DormandPrinceIntegrator>("Dormand-Prince 5 (e=10m)",
	                                           10e-3, p, train, ref);
	benchmarkAdaptive<DormandPrinceIntegrator>("Dormand-Prince 5 (e=100m)",
	                                           100e-3, p, train, ref);

	return 0;
}

