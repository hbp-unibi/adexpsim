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
#include <common/Timer.hpp>

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

using namespace AdExpSim;
using Recorder = VectorRecorder<std::vector<double>, SIPrefixTrafo>;
using RecorderData = VectorRecorderData<std::vector<double>>;

/**
 * Class used to print the benchmark result as table.
 */
class Tablefmt {
public:
	struct RowData {
		std::string integratorName;
		std::string integratorParam;
		double t, v, vp, gE, gEp, gI, gIp, w, wp;
		size_t N;

		RowData(const std::string &integratorName,
		        const std::string &integratorParam, double t, size_t N,
		        double v, double vp, double gE, double gEp, double gI,
		        double gIp, double w, double wp)
		    : integratorName(integratorName),
		      integratorParam(integratorParam),
		      t(t),
		      v(v),
		      vp(vp),
		      gE(gE),
		      gEp(gEp),
		      gI(gI),
		      gIp(gIp),
		      w(w),
		      wp(wp),
		      N(N)
		{
		}

		double avgp() const { return (vp + gEp + gIp + wp) / 4.0; }
	};

private:
	std::vector<RowData> rows;

	std::string printValue(double val)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(3) << std::setw(10)
		   << round(val * 1000.0) / 1000.0;
		return ss.str();
	}

	std::string printPercentage(double val)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << std::setw(8)
		   << round(val * 100.0 * 100.0) / 100.0 << "%";
		return ss.str();
	}

	std::string printLPercentage(double val)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2)
		   << round(val * 100.0 * 100.0) / 100.0;
		return ss.str();
	}

	std::string &replace(std::string &haystack, const std::string &needle,
	                     const std::string &replacement)
	{
		size_t pos;
		while ((pos = haystack.find(needle)) != std::string::npos) {
			haystack.erase(pos, needle.size());
			haystack.insert(pos, replacement);
		}
		return haystack;
	}

public:
	void addRow(const RowData &data) { rows.emplace_back(data); }

	void printRaw(std::ostream &os)
	{
		std::string lastIntegratorName;
		bool first = true;
		for (const auto &row : rows) {
			// Print a separator as soon as the integrator changes
			if (lastIntegratorName != row.integratorName && !first) {
				os << "--" << std::endl;
			}
			lastIntegratorName = row.integratorName;

			// Prepare the caption, remove some LaTeX
			std::string caption =
			    row.integratorName + " (" + row.integratorParam + ")";
			replace(caption, "\\SI", "");
			replace(caption, "{", "");
			replace(caption, "}", "");
			replace(caption, "\\milli", "m");
			replace(caption, "\\micro", "u");
			replace(caption, "\\nano", "n");
			replace(caption, "\\second", "s");
			replace(caption, "\\nothing", "");

			// Print all columns
			os << std::setw(30) << caption << "  ";
			os << "t: " << printValue(row.t) << "ms ";
			os << "N: " << std::setw(8) << row.N << " ";
			os << "t/N: " << printValue(row.t * 1000.0 / double(row.N))
			   << "us ";
			os << "| v: " << printValue(row.v) << "mV  ";
			os << printPercentage(row.vp) << " ";
			os << "| gE: " << printValue(row.gE * 1000.0) << "nS  ";
			os << printPercentage(row.gEp) << " ";
			os << "| gI: " << printValue(row.gI * 1000.0) << "nS ";
			os << printPercentage(row.gIp) << " ";
			os << "| w: " << printValue(row.w) << "nA  ";
			os << printPercentage(row.wp) << " ";
			os << "| µ: " << printPercentage(row.avgp());
			os << std::endl;

			first = false;
		}
	}

	void printLaTeX(std::ostream &os)
	{
		os << "\\begin{tabular}{p{1.5cm} l r r r rr rr rr rr r}" << std::endl;
		os << "\t\\toprule" << std::endl;

		// Top header
		os << "\t\\multicolumn{2}{c}{\\multirow{2}{*}{\\textit{Integrator}}}";
		os << " &\\multicolumn{3}{c}{\\textit{Time and samples}}";
		os << " &\\multicolumn{9}{c}{\\textit{Error (RMSE)}} \\\\" << std::endl;

		// Rule
		std::cout << "\t\\cmidrule(r){3-5}"
		             "\\cmidrule(l){6-14}" << std::endl;

		// Sub-header
		os << "\t &" << std::endl;
		os << " & \\multicolumn{1}{c}{$t \\, [\\si{\\milli\\second}]$}";
		os << " & \\multicolumn{1}{c}{$N$}";
		os << " & \\multicolumn{1}{c}{$\\frac{t}{N} \\, "
		      "[\\si{\\micro\\second}]$}";
		os << " & \\multicolumn{2}{c}{$v \\, [\\si{\\milli\\volt}]$ (\\%)}";
		os << " & \\multicolumn{2}{c}{$\\Ge \\, [\\si{\\nano\\siemens}]$ "
		      "(\\%)}";
		os << " & \\multicolumn{2}{c}{$\\Gi \\, [\\si{\\nano\\siemens}]$ "
		      "(\\%)}";
		os << " & \\multicolumn{2}{c}{$w \\, [\\si{\\nano\\ampere}]$ (\\%)}";
		os << " & Avg. \\% \\\\" << std::endl;

		// Print all other rows
		for (size_t i = 0; i < rows.size(); i++) {
			// Detect the number of rows with the same integrator
			size_t nRows = 0;
			size_t j;
			for (j = i; j < rows.size(); j++) {
				if (rows[i].integratorName == rows[j].integratorName) {
					nRows++;
				} else {
					break;
				}
			}

			// Print the column group
			bool first = true;
			for (; i < j; i++) {
				// Print the columns
				const RowData &row = rows[i];
				if (first) {
					os << std::endl;
					std::cout << "\t\\cmidrule(r){1-2}\\cmidrule(r){3-5}"
					             "\\cmidrule(r){6-7}\\cmidrule(r){8-9}"
					             "\\cmidrule(r){10-11}\\cmidrule(r){12-13}"
					             "\\cmidrule(l){14-14}" << std::endl;
					os << std::endl;
					os << "\t\\multirow{" << nRows
					   << "}{*}{\\parbox{1.5cm}{\\raggedleft "
					   << row.integratorName << "}}" << std::endl;
				}
				os << "\t";
				os << "\t& " << row.integratorParam;
				os << "\t& " << printValue(row.t);
				os << "\t& " << std::setw(8) << row.N;
				os << "\t& " << printValue(row.t * 1000.0 / double(row.N));
				os << "\t& " << printValue(row.v);
				os << "\t& (" << printLPercentage(row.vp) << ")";
				os << "\t& " << printValue(row.gE * 1000.0);
				os << "\t& (" << printLPercentage(row.gEp) << ")";
				os << "\t& " << printValue(row.gI * 1000.0);
				os << "\t& (" << printLPercentage(row.gIp) << ")";
				os << "\t& " << printValue(row.w);
				os << "\t& (" << printLPercentage(row.wp) << ")";
				os << "\t& " << printLPercentage(row.avgp());
				os << "\\\\" << std::endl;
				first = false;
			}
			i--;
		}
		os << "\t\\bottomrule" << std::endl;
		os << "\\end{tabular}" << std::endl;
	}
};

/**
 * The BenchmarkResult class contains the results captured from a single
 * benchmark run.
 */
struct BenchmarkResult {
	std::string integratorName;
	std::string integratorParam;
	RecorderData data;
	double time;

	BenchmarkResult(const std::string &integratorName,
	                const std::string &integratorParam)
	    : integratorName(integratorName),
	      integratorParam(integratorParam),
	      time(0)
	{
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
	                        const Vector2 &v2, size_t j, double h)
	{
		double d = minDist(v1, i, v2, j);
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

		void print(Tablefmt &f)
		{
			f.addRow({benchmark.integratorName, benchmark.integratorParam,
			          benchmark.time, benchmark.data.size(), rmseDelta[0],
			          rmseDeltaNormalized[0], rmseDelta[1],
			          rmseDeltaNormalized[1], rmseDelta[2],
			          rmseDeltaNormalized[2], rmseDelta[3],
			          rmseDeltaNormalized[3]});
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
			double t = data.ts[i];
			double tDelta = t - data.ts[i - 1];

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
			const double norm = (res.refMax[k] - res.refMin[k]);
			if (norm != 0) {
				res.maxDeltaNormalized[k] = res.maxDelta[k] / norm;
				res.rmseDeltaNormalized[k] = res.rmseDelta[k] / norm;
			}
		}

		return res;
	}
};

template <typename Function>
BenchmarkResult runBenchmark(const std::string &integratorName,
                             const std::string &integratorParam,
                             const Parameters &params, Function f)
{
	Timer t;
	BenchmarkResult res(integratorName, integratorParam);
	Recorder recorder(params);
	DefaultController controller;

	f(controller, recorder);
	res.time = t.time();
	res.data = recorder.getData();

	return res;
}

template <typename Integrator, size_t Flags>
void benchmarkSimple(const std::string &integratorName,
                     const std::string &integratorParam, double tDelta,
                     const Parameters &params, const SpikeTrain &train,
                     const BenchmarkResult &reference, Tablefmt &fmt)
{
	runBenchmark(integratorName, integratorParam, params,
	             [&](DefaultController &controller, Recorder &recorder) {
		             Integrator integrator;
		             Model::simulate<Flags>(train.getSpikes(), recorder,
		                                    controller, integrator, params,
		                                    Time::sec(tDelta), train.getMaxT());
		         })
	    .compare(reference.data)
	    .print(fmt);
}

template <typename Integrator, size_t Flags>
void benchmarkAdaptive(const std::string &integratorName,
                       const std::string &integratorParam, Val eTar,
                       const Parameters &params, const SpikeTrain &train,
                       const BenchmarkResult &reference, Tablefmt &fmt)
{
	runBenchmark(integratorName, integratorParam, params,
	             [&](DefaultController &controller, Recorder &recorder) {
		             Integrator integrator(eTar);
		             Model::simulate<Flags>(train.getSpikes(), recorder,
		                                    controller, integrator, params,
		                                    Time::sec(1e-6), train.getMaxT());
		         })
	    .compare(reference.data)
	    .print(fmt);
}

template <size_t Flags = 0>
void benchmark()
{
	// Use a new Tablefmt object
	Tablefmt fmt;

	// Use the default parameters
	Parameters p;

	// Create a vector containing all input spikes
	SpikeTrain train({{4, 0, 1, 1e-3, 1.0, -1.0, 0.0},
	                  {4, 2, 1, 1e-3, 1.0, -1.0, 0.0},
	                  {3, 0, 0, 1e-3, 1.0, -1.0, 0.0}},
	                 100, false, 0.1_s, 0.01);

	// Generate the reference data
	std::cout << "Generating reference data..." << std::endl;
	BenchmarkResult ref =
	    runBenchmark("Runge-Kutta", "t=\\SI{1}{\\micro\\second}", p,
	                 [&](DefaultController &controller, Recorder &recorder) {
		    RungeKuttaIntegrator integrator;
		    Model::simulate<Flags & ~Model::FAST_EXP>(
		        train.getSpikes(), recorder, controller, integrator, p, 1e-7_s,
		        train.getMaxT());
		});
	std::cout << "Done." << std::endl;

	// Print the reference data as stub
	benchmarkSimple<EulerIntegrator, Flags>(
	    "Euler", "t=\\SI{1}{\\micro\\second}", 1e-6, p, train, ref, fmt);
	benchmarkSimple<EulerIntegrator, Flags>(
	    "Euler", "t=\\SI{10}{\\micro\\second}", 10e-6, p, train, ref, fmt);
	benchmarkSimple<EulerIntegrator, Flags>(
	    "Euler", "t=\\SI{100}{\\micro\\second}", 100e-6, p, train, ref, fmt);
	benchmarkSimple<EulerIntegrator, Flags>(
	    "Euler", "t=\\SI{1}{\\milli\\second}", 1e-3, p, train, ref, fmt);

	benchmarkSimple<MidpointIntegrator, Flags>(
	    "Midpoint", "t=\\SI{1}{\\micro\\second}", 1e-6, p, train, ref, fmt);
	benchmarkSimple<MidpointIntegrator, Flags>(
	    "Midpoint", "t=\\SI{10}{\\micro\\second}", 10e-6, p, train, ref, fmt);
	benchmarkSimple<MidpointIntegrator, Flags>(
	    "Midpoint", "t=\\SI{100}{\\micro\\second}", 100e-6, p, train, ref, fmt);
	benchmarkSimple<MidpointIntegrator, Flags>(
	    "Midpoint", "t=\\SI{1}{\\milli\\second}", 1e-3, p, train, ref, fmt);

	benchmarkSimple<RungeKuttaIntegrator, Flags>(
	    "Runge-Kutta", "t=\\SI{1}{\\micro\\second}", 1e-6, p, train, ref, fmt);
	benchmarkSimple<RungeKuttaIntegrator, Flags>("Runge-Kutta",
	                                             "t=\\SI{10}{\\micro\\second}",
	                                             10e-6, p, train, ref, fmt);
	benchmarkSimple<RungeKuttaIntegrator, Flags>("Runge-Kutta",
	                                             "t=\\SI{100}{\\micro\\second}",
	                                             100e-6, p, train, ref, fmt);
	benchmarkSimple<RungeKuttaIntegrator, Flags>(
	    "Runge-Kutta", "t=\\SI{1}{\\milli\\second}", 1e-3, p, train, ref, fmt);

	benchmarkAdaptive<DormandPrinceIntegrator, Flags>(
	    "Dormand-Prince", "e=\\SI{1}{\\micro\\nothing}", 1e-6, p, train, ref,
	    fmt);
	benchmarkAdaptive<DormandPrinceIntegrator, Flags>(
	    "Dormand-Prince", "e=\\SI{10}{\\micro\\nothing}", 10e-6, p, train, ref,
	    fmt);
	benchmarkAdaptive<DormandPrinceIntegrator, Flags>(
	    "Dormand-Prince", "e=\\SI{100}{\\micro\\nothing}", 100e-6, p, train,
	    ref, fmt);
	benchmarkAdaptive<DormandPrinceIntegrator, Flags>(
	    "Dormand-Prince", "e=\\SI{1}{\\milli\\nothing}", 1e-3, p, train, ref,
	    fmt);
	benchmarkAdaptive<DormandPrinceIntegrator, Flags>(
	    "Dormand-Prince", "e=\\SI{10}{\\milli\\nothing}", 10e-3, p, train, ref,
	    fmt);
	benchmarkAdaptive<DormandPrinceIntegrator, Flags>(
	    "Dormand-Prince", "e=\\SI{100}{\\milli\\nothing}", 100e-3, p, train,
	    ref, fmt);

	fmt.printLaTeX(std::cout);
}

int main()
{
	// Run the AdExp model
	std::cout << std::endl;
	std::cout << "BENCHMARK 1: AdExp Model" << std::endl;
	std::cout << "========================" << std::endl;
	std::cout << std::endl;

	benchmark<>();

	// Run the AdExp model
	std::cout << std::endl;
	std::cout << "BENCHMARK 2: AdExp Model, fast exp" << std::endl;
	std::cout << "==================================" << std::endl;
	std::cout << std::endl;

	benchmark<Model::FAST_EXP>();

	// Run the AdExp model
	std::cout << std::endl;
	std::cout << "BENCHMARK 3: IfCondExp Model" << std::endl;
	std::cout << "============================" << std::endl;
	std::cout << std::endl;

	benchmark<Model::IF_COND_EXP>();

	return 0;
}

