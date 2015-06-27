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

#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <exploration/Evaluation.hpp>
#include <exploration/Simplex.hpp>
#include <common/Terminal.hpp>

#include <csignal>
#include <limits>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace AdExpSim;

std::string printVal(Val x)
{
	std::stringstream ss;
	ss << std::setw(6) << std::setprecision(4) << x;
	return ss.str();
}

std::string printCost(Terminal &term, Val cost)
{
	Val f1 = std::min<Val>(1.0, 0.2 * cost);
	Val f2 = 1.0 - std::min<Val>(1.0, 0.1 * cost);
	Val f3 = 1.0 - std::min<Val>(1.0, 0.5 * cost);
	std::stringstream ss;
	ss << term.rgb(f1 * 255, 255 * std::min(1.0, f3 + 0.25 * f2), 64 * f3,
	               false) << printVal(cost) << term.reset();
	return ss.str();
}

/**
 * SIGINT handler. Sets the global "cancel" flag to true when called once,
 * terminates the program if called twice. This allows to terminate the program,
 * even if it is not responsive (the cancel flag is not checked).
 */
static bool cancel = false;
void int_handler(int x)
{
	if (cancel) {
		exit(1);
	}
	cancel = true;
}

Val estimateW(WorkingParameters params, Val xi)
{
	return -log(1 - params.eSpikeEff() / params.eE()) * params.lE() / xi;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, int_handler);

	Terminal term(true);

	Evaluation evaluation(10, 1e-3);

	// Create the initial parameters
	WorkingParameters params;

	// Estimate a value for the weight w
	params.w() = estimateW(params, evaluation.getXi());

	std::cout << "Initial value for w: "
	          << printVal(1000.0 * 1000.0 * params.w() * DefaultParameters::cM)
	          << "µS" << std::endl;

	// Define the cost function f
	auto f = [&evaluation](const WorkingParameters &p) -> Val {
		auto res = evaluation.evaluate(p);

		// Penalize too large time constants (maximum value: 0.2s)
		const Val errMaxRate =
		    exp(100 * (1.0 / p.lL() - 0.2)) + exp(100 * (1.0 / p.lE() - 0.2));

		// Penalize too small time constants (minimum value: 1ms)
		const Val errMinRate =
		    exp(100 * (p.lL() - 1000)) + exp(100 * (p.lE() - 1000));

		// Penalize too large voltages (maximum value: 200mV)
		const Val errVolt =
		    exp(100 * (p.eE() - 0.2)) + exp(100 * (p.eTh() - 0.2));

		// Penalize the time that is being used -- calculate the difference
		// between the time of the last spike in the spike train and the time
		// of the first output spike
		const Val delay =
		    (std::get<1>(res) - evaluation.getLastSpikeTime()).sec();
		const Val timePenalty =
		    10 * std::max<Val>(0.0, std::min<Val>(0.1, delay));
		return std::get<0>(res) + timePenalty + errMaxRate + errMinRate + errVolt;
	};

	Simplex<WorkingParameters> simplex(
	    params, std::vector<size_t>{0, 1, 4, 6, 9, 12}, f);

	SimplexStepResult res;
	size_t it = 0;
	do {
		res = simplex.step(f);

		// Print the best and mean cost#
		if (it % 10 == 0) {
			std::cout << "It: " << std::setw(5) << it
			          << " C: " << printCost(term, res.bestValue) << " ("
			          << printCost(term, res.meanValue) << ") ";

			if (res.hasNewBest) {
				std::cout << "↗";
			} else if (res.reducing) {
				std::cout << "↘";
			} else {
				std::cout << "→";
			}

			// Print the current parameter set
			WorkingParameters x = simplex.getSimplex()[0].x;
			std::cout << " tauL: " << printVal(1000.0 / x.lL()) << "ms";
			std::cout << " tauE: " << printVal(1000.0 / x.lE()) << "ms";
			std::cout << " eE: " << printVal(x.eE() * 1000.0) << "mV";
			std::cout << " eTh: " << printVal(x.eTh() * 1000.0) << "mV";
			std::cout << " deltaTh: " << printVal(x.deltaTh() * 1000.0) << "mV";
			std::cout << " w: " << printVal(1000.0 * 1000.0 * x.w() *
			                                DefaultParameters::cM)
			          << "µS        ";
			std::cout << "\r";
		}
		it++;
	} while (!res.done && !cancel);

	auto evalRes = evaluation.evaluate(simplex.getSimplex()[0].x);
	bool ok = std::get<2>(evalRes);

	std::cout << std::endl;

	if (cancel) {
		std::cout << "Manually aborted optimization" << std::endl;
	}

	std::cout << "Final Raw Cost: " << std::get<0>(evalRes) << std::endl;
	std::cout << "Final Time    : " << std::get<1>(evalRes) * 1000 << "ms"
	          << std::endl;
	std::cout << "Final Delay   : "
	          << (std::get<1>(evalRes) -
	              evaluation.getLastSpikeTime().sec()) *
	                 1000 << "ms" << std::endl;
	std::cout << "Heaviside Beh.: "
	          << (ok ? term.rgb(64, 128, 32, true) + "   OK   "
	                 : term.rgb(128, 32, 32, true) + "  FAIL  ") << term.reset()
	          << std::endl;

	return 0;
}

