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

// Tiny program used to Debug a problem with the DormandPrinceIntegrator

#include <iostream>

#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>
#include <simulation/SpikeTrain.hpp>

using namespace AdExpSim;

namespace {
class Controller {
public:
	/**
	 * Minimum neuron voltage at which the simulation can be aborted.
	 */
	static constexpr Val MIN_VOLTAGE = 1e-4;

	/**
	 * Minimum excitatory plus inhibitory channel rate.
	 */
	static constexpr Val MIN_RATE = 1e-3;

	/**
	 * Minimum total current. A non-zero voltage is ignored if the current is
	 * near zero, as the AdExp model may go into an additional equilibrium
	 * stage.
	 */
	static constexpr Val MIN_DV = 1e-3;

	/**
	 * The control function is responsible for aborting the simulation. The
	 * default controller aborts the simulation once the neuron has settled,
	 * so there are no inhibitory or excitatory currents and the membrane
	 * potential is near its resting potential.
	 *
	 * @param s is the current neuron state.
	 * @return true if the neuron should continue, false otherwise.
	 */
	static ControllerResult control(Time t, const State &s,
	                                const AuxiliaryState &as,
	                                const WorkingParameters &, bool inRefrac)
	{
		std::cout << t << ";" << s << "; " << as << "; " << inRefrac
		          << std::endl;
		std::cout << (fabs(s.v()) > MIN_VOLTAGE) << ", "
		          << (fabs(as.dvL() + as.dvE() + as.dvI() + as.dvTh()) > MIN_DV)
		          << ", " << ((s.lE() + s.lI()) > MIN_RATE) << ", " << inRefrac
		          << std::endl;
		return ((fabs(s.v()) > MIN_VOLTAGE &&
		         fabs(as.dvL() + as.dvE() + as.dvI() + as.dvTh()) > MIN_DV) ||
		        (s.lE() + s.lI()) > MIN_RATE || inRefrac)
		           ? ControllerResult::CONTINUE
		           : ControllerResult::MAY_CONTINUE;
	}
};
}

int main(int argc, char *argv[])
{
	// Raw data as extracted from a GDB memory dump, convert to parameters
	/*	const uint32_t raw[14] = {0x4064d180, 0x43a663c3, 0x43480000,
	   0x40de38e4,
	                              0x0,        0x3d8f5c29, 0x0, 0x3c83126e,
	                              0x3db851ec, 0xbc23d708, 0x3b03126f,
	   0x40800000,
	                              0x3da4dd30, 0x41f00000};*/
	const uint32_t raw[14] = {0x44f238b0, 0x438e5809, 0x43480000, 0x40de38e4,
	                          0x0,        0x3d4ccccd, 0xbd4ccccd, 0x3cc91700,
	                          0x3d4ccccd, 0xbcf5c28e, 0x3a03126f, 0x3e800000,
	                          0x3e5c28f5, 0xc30cb86a};
	WorkingParameters p(
	    *reinterpret_cast<const std::array<float, 14> *>(
	        &raw));  // Blah strict-aliasing blah blah blah... Just do it!
	p.update();

	DormandPrinceIntegrator integrator(0.1e-3);
	Controller controller;
	NullRecorder recorder;
//	SpikeVec train = buildInputSpikes(3, 1e-3_s);
	SpikeVec train = buildInputSpikes(3, 1e-3_s);

	Model::simulate<Model::IF_COND_EXP>(train, recorder, controller, integrator,
	                                    p);

	return 0;
}
