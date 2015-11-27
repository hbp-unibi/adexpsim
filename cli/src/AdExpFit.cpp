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
#include <simulation/SpikeTrain.hpp>
#include <exploration/SimplexPool.hpp>

#include <csignal>
#include <cmath>
#include <iostream>
#include <fstream>
#include <limits>
#include <string>

using namespace AdExpSim;

struct ReferenceData {
	float t, v;

	ReferenceData(float t = 0.0, float v = 0.0) : t(t), v(v) {}
};

std::vector<std::string> split(const std::string &text, char sep)
{
	std::vector<std::string> tokens;
	size_t start = 0, end = 0;
	while ((end = text.find(sep, start)) != std::string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
	return tokens;
}

bool cancel = false;
void int_handler(int x)
{
	if (cancel) {
		exit(1);
	}
	cancel = true;
}

class Simulation {
private:
	VectorRecorder<std::vector<float>> recorder;
	NullController controller;
	RungeKuttaIntegrator integrator;
//	DormandPrinceIntegrator integrator;
	WorkingParameters wParams;
	SpikeVec spikes;
	bool valid_;

public:
	Simulation(const Parameters &params, Time max_t)
	    : recorder(params, 0.1_ms),
	      wParams(params),
	      spikes({Spike(1002_ms, 1.0)}),
	      valid_(wParams.valid())
	{
		if (valid()) {
			Model::simulate<Model::IF_COND_EXP | Model::DISABLE_SPIKING>(
			    spikes, recorder, controller, integrator, wParams, Time(-1),
			    2_s);
		}
	}

	const VectorRecorderData<std::vector<float>> &data() const
	{
		return recorder.getData();
	}

	bool valid() const { return valid_; }
};

int main(int argc, char *argv[])
{
	signal(SIGINT, int_handler);

	if (argc != 3) {
		std::cout << "Tries to fit a model parameter to a previously recorded "
		          << "spike train." << std::endl;
		std::cout << "Usage: " << argv[0]
		          << " <REFERENCE_DATA> <FITTED_DATA_OUT>" << std::endl;
		return 1;
	}

	// Read the CSV data
	std::cout << "Reading CSV file..." << std::endl;
	float min_t = std::numeric_limits<float>::max();
	float max_t = std::numeric_limits<float>::lowest();
	std::vector<ReferenceData> ref;
	std::ifstream fin(argv[1]);
	while (fin.good()) {
		std::string line;
		std::getline(fin, line);
		auto parts = split(line, ',');
		if (parts.size() == 2) {
			ReferenceData data(std::stof(parts[0]) / 1000.0,
			                   std::stof(parts[1]) / 1000.0);
			ref.push_back(data);
			min_t = std::min(min_t, data.t);
			max_t = std::max(max_t, data.t);
		}
	}

	// Initialize the parameters
	Parameters params;
	params.cM() = 0.2e-9;
	params.eE() = 0.0e-3;
	params.eReset() = -75e-3;
	params.eL() = -70e-3;
	params.eTh() = -55e-3;
	params.w() = 16.0e-9;
	params.tauE() = 5e-3;
	params.gL() = params.cM() / 5.0e-3;

	auto f = [&ref, max_t](const Parameters &params) -> float {
		Simulation sim(params, Time::sec(max_t));
		if (!sim.valid()) {
			return std::numeric_limits<Val>::max();
		}
		double err = 0;
		for (auto &r : ref) {
			auto val = sim.data().interpolate(r.t);
			float dv = r.v - val.v();
			err += dv * dv;
		}
		return sqrt(err / ref.size());
	};

	// Create the simplex algorithm instance, fetch the to-be-optimized
	// dimensions
	std::vector<size_t> dims = {Parameters::idx_tauE,
	                            /*Parameters::idx_gL, Parameters::idx_eL,*/
	                            Parameters::idx_w};
	SimplexPool<Parameters> simplex(params, dims);
	auto res = simplex.run(f, [](size_t nIt, size_t sample, Val err) -> bool {
		std::cout << "nIt: " << nIt << ", sample: " << sample
		          << ", err: " << err << "             \r";
		return !cancel;
	});

	std::cout << std::endl;
	std::cout << "Done." << std::endl;
	std::cout << std::endl;
	std::cout << "Initial error: " << res.costInit << std::endl;
	std::cout << "Final error: " << res.costBest << std::endl;

	Parameters best = res.best;
	for (size_t i = 0; i < Parameters::Size; i++) {
		std::cout << Parameters::names[i] << ": " << best[i] << " ("
		          << params[i] << ")" << std::endl;
	}

	// Output the fitted curve
	Simulation sim(best, Time::sec(max_t));
	std::ofstream fout(argv[2]);
	for (size_t i = 0; i < sim.data().size(); i++) {
		fout << sim.data().ts[i] * 1000.0 << "," << sim.data().v[i] * 1000.0
		     << std::endl;
	}

	return 0;
}

