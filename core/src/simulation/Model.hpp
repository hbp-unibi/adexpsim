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
 * @file Model.hpp
 *
 * Contains the implementation of the AdExp model. The implementation presented
 * here makes heavy use of templates in order to configure the model at compile
 * time. This minimizes branching and significantly increases the performance
 * of the code.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_MODEL_HPP_
#define _ADEXPSIM_MODEL_HPP_

#include <cmath>
#include <cstdint>

#include <common/FastMath.hpp>

#include "Controller.hpp"
#include "Integrator.hpp"
#include "Parameters.hpp"
#include "Recorder.hpp"
#include "Spike.hpp"
#include "State.hpp"

namespace AdExpSim {
/**
 * The ModelType enum defines the model that should be used in the simulation.
 */
enum class ModelType : int {
	/**
     * Represents the simple IF_COND_EXP model (integrate & fire conductance
     * based with exponential decay).
     */
	IF_COND_EXP = 0,

	/**
     * Represents the simple AD_IF_COND_EXP model (integrate & fire conductance
     * based with exponential decay and adaptation mechanism).
     */
	AD_IF_COND_EXP = 1
};

/**
 * The Model class contains the static function "simulate" which performs the
 * actual simulation of the AdExp model.
 */
class Model {
public:
	/**
	 * Set this flag if ITh should be completely disabled, allowing to downgrade
	 * the model from the Adaptive I&F model to the classical I&F model.
	 */
	static constexpr uint8_t DISABLE_ITH = (1 << 0);

	/**
	 * Set this flag if ITh should be clamped in such a way that the exponential
	 * runnaway effect cannot occur and thus no spikes will be issued.
	 */
	static constexpr uint8_t CLAMP_ITH = (1 << 1);

	/**
	 * Set this flag if the spiking mechanism of the neuron should be switched
	 * off. ITh will still flow, but the neuron will not be reset. Only use in
	 * conjunction with CLAMP_ITH or DISABLE_ITH.
	 */
	static constexpr uint8_t DISABLE_SPIKING = (1 << 2);

	/**
	 * Set this flag if the refractory period of the neuron should be switched
	 * off.
	 */
	static constexpr uint8_t DISABLE_REFRACTORY = (1 << 3);

	/**
	 * Set this flag if a fast approximation of the exponential function should
	 * be used. This approximation is less accurate, however is significantly
	 * faster. An exponential function is only used if ITH is not disabled.
	 */
	static constexpr uint8_t FAST_EXP = (1 << 4);

	/**
	 * Downgrades the model to the simpler IF_COND_EXP model. This flag includes
	 * DISABLE_ITH and causes the spiking potential to be set to eTh instead of
	 * eSpike.
	 */
	static constexpr uint8_t IF_COND_EXP = (1 << 5);

	/**
	 * Enables processing of "Special" input spikes as created using the
	 * SpecialSpike class.
	 */
	static constexpr uint8_t PROCESS_SPECIAL = (1 << 6);

private:
	/**
	 * Calculates the current auxiliary state. This function is the bottleneck
	 * of the simulation, with the "exp" for the threshold current taking more
	 * than half of the time (unless FAST_EXP is used).
	 *
	 * @tparam Flags is a bit field containing the simulation flags, which may
	 * be a combination of DISABLE_ITH, CLAMP_ITH and FAST_EXP.
	 * @param s is the state vector for which the auxiliary state should be
	 * calculated.
	 * @param p is a reference at the working parameter set p.
	 */
	template <uint8_t Flags>
	static AuxiliaryState aux(const State &s, const WorkingParameters &p)
	{
		// Calculate dvTh, but only if iTh is not disabled, either by the
		// DISABLE_ITH or the IF_COND_EXP flag
		Val dvTh = 0.0;
		if (!((Flags & DISABLE_ITH) || (Flags & IF_COND_EXP))) {
			// Calculate the exponent that should be used inside the formula for
			// iTh. Clamp the value to "maxIThExponent" to prevent uneccessary
			// overflows.
			const Val dvThExponent =
			    (Flags & CLAMP_ITH)
			        ? (std::min(p.eSpikeEffRed(), s.v()) - p.eTh()) *
			              p.invDeltaTh()
			        : std::min(p.maxIThExponent(),
			                   (s.v() - p.eTh()) * p.invDeltaTh());
			dvTh = -p.lL() * p.deltaTh() * (Flags & FAST_EXP
			                                    ? fast::exp(dvThExponent)
			                                    : exp(dvThExponent));
		}

		return AuxiliaryState(p.lL() * s.v(),             // dvL  [V/s]
		                      s.lE() * (s.v() - p.eE()),  // dvE  [V/s]
		                      s.lI() * (s.v() - p.eI()),  // dvI  [V/s]
		                      dvTh                        // dvTh [V/s]
		                      );
	}

	/**
	 * Function calculating the current derivative for the given state. Stores
	 * the auxiliary state (all currently flowing currents) in the as variable.
	 *
	 * @param s is the current state for which the state should be calculated.
	 * @param as is the variable which contains the current auxiliary state.
	 * @param p is a reference at the parameter vector.
	 * @return a new state variable containing the derivatives.
	 */
	template <uint8_t Flags>
	static State df(const State &s, const AuxiliaryState &as,
	                const WorkingParameters &p, bool inRefrac)
	{
		// Do not change membrane potential while in refractory period,
		// otherwise sum the currents.
		const Val dv =
		    ((Flags & DISABLE_REFRACTORY) || !inRefrac)
		        ? -(as.dvL() + as.dvE() + as.dvI() + as.dvTh() + s.dvW())
		        : 0;

		return State(dv,                // [V/s]
		             -s.lE() * p.lE(),  // [1/s^2]
		             -s.lI() * p.lI(),  // [1/s^2]
		             (Flags & IF_COND_EXP) ? 0.0 : -(s.dvW() - p.lA() * s.v()) *
		                                               p.lW()  // [V/s^2]
		             );
	}

	/**
	 * Method responsible for the generation of an output spike. Records the
	 * output spike, resets the membrane potential, increases the habituation
	 * current and starts the refractory period.
	 *
	 * @param t is the current time.
	 * @param s is the neuron state.
	 * @param tLastSpike is the time of the last spike.
	 * @param recorder is used to record the output spike event.
	 * @param p are the current neuron parameters.
	 */
	template <uint8_t Flags, typename Recorder>
	static void generateOutputSpike(Time t, State &s, Time &tLastSpike,
	                                Recorder &recorder,
	                                const WorkingParameters &p)
	{
		AuxiliaryState as;

		// Record the spike event
		s.v() = p.eSpike();
		as = aux<Flags>(s, p);
		recorder.record(t, s, as, true);

		// Reset the voltage and increase the adaptation current
		s.v() = p.eReset();
		if (!(Flags & IF_COND_EXP)) {
			s.dvW() += p.lB();
		}
		as = aux<Flags>(s, p);
		recorder.outputSpike(t, s);
		recorder.record(t, s, as, true);

		// Set tLastSpike in order to start the refractory period
		if (!(Flags & DISABLE_REFRACTORY)) {
			tLastSpike = t;
		}
	}

	/**
	 * Method responsible for handling special spikes.
	 *
	 * @param t is the current time.
	 * @param s is the neuron state.
	 * @param tLastSpike is the time of the last spike.
	 * @param recorder is used to record the output spike event.
	 * @param p are the current neuron parameters.
	 */
	template <uint8_t Flags, typename Recorder>
	static bool handleSpecialSpikes(const Spike &spike, Time t, State &s,
	                                Time &tLastSpike, Recorder &recorder,
	                                const WorkingParameters &p)
	{
		if (SpecialSpike::isSpecial(spike)) {
			switch (SpecialSpike::kind(spike)) {
				case SpecialSpike::Kind::FORCE_OUTPUT_SPIKE: {
					generateOutputSpike<Flags>(t, s, tLastSpike, recorder, p);
					break;
				}
				case SpecialSpike::Kind::SET_VOLTAGE: {
					Val f = SpecialSpike::payload(spike) /
					        Val(std::numeric_limits<uint16_t>::max());
					s.v() = p.eReset() + (p.eSpike() - p.eReset()) * f;
					break;
				}
			}
			return true;
		}
		return false;
	}

public:
	/**
	 * Performs a single neuron simulation. Allows to customize the simulation
	 * by disabling certain parts of the model using the template parameter and
	 * customizing the differential equation integrator, the data recorder and
	 * the controller.
	 *
	 * @param spikes is a vector containing the input spikes. Spikes have to be
	 * sorted by input time, with the earliest spikes first.
	 * @param recorder is an object to which the current simulation state and
	 * output spikes are passed. Use an instance of the NullRecorder class
	 * to disable recording.
	 * @param controller is the object which determines when the simulation
	 * will end.
	 * @param integrator is the object responsible for integrating the
	 * differential equation. The best integrator to use is the
	 * DormandPrincIntegrator (which has an adaptive stepsize) or the
	 * RungeKuttaIntegrator with small timestep if a fixed timestep is required.
	 * @param p contains the neuron model parameters. The WorkingParameters
	 * contains the rescaled original parameter required for an efficient
	 * implementation.
	 * @param tDelta is the timestep that should be used. If set to a value
	 * smaller or equal to zero, the timestep is chosen automatically. If an
	 * adaptive stepsize controller is used tDelta represents the initial
	 * stepsize the controller tries to use.
	 * @param tEnd is the time at which the simulation will end independent of
	 * the current state of the controller. Set to MAX_TIME to let the
	 * controller decide when to end the simulation.
	 * @param s0 is the initial state of the neuron. It is recommended to set
	 * the initial membrane potential to the resting potential.
	 * @param tLastSpike is the time at which the last spike was issued by the
	 * neuron. This variable is used by the refractory mechanism. Values smaller
	 * than zero correspond to "there has been no last spike". This parameter is
	 * important if a neuron simulation is restarted from a certain point in
	 * time.
	 */
	template <uint8_t Flags = 0, typename Recorder = NullRecorder,
	          typename Integrator = RungeKuttaIntegrator,
	          typename Controller = DefaultController>
	static void simulate(const SpikeVec &spikes, Recorder &recorder,
	                     Controller &controller, Integrator &integrator,
	                     const WorkingParameters &p = WorkingParameters(),
	                     Time tDelta = Time(-1), Time tEnd = MAX_TIME,
	                     const State &s0 = State(), Time tLastSpike = Time(-1))
	{
		// Use the automatically calculated tDelta if no user-defined value is
		// given
		if (tDelta <= Time(0)) {
			tDelta = Time::sec(p.tDelta());
		}

		// Number of spikes and index of the next spike that should be processed
		const size_t nSpikes = spikes.size();
		size_t spikeIdx = 0;

		// Convert the refractory period from the parameters into the internal
		// time measure. Initialize tLastSpike with -tRefrac if no valid value
		// for tLastSpike has been given by the user in order make sure that
		// t - tLastSpike > tRefrac evaluates to false.
		const Time tRefrac = Time::sec(p.tauRef());
		if (tLastSpike < Time(0)) {
			tLastSpike = -tRefrac;
		}

		// Start with state s0
		State s = s0;

		// Iterate over all time slices
		Time t;
		while (t < tEnd) {
			// Fetch the next spike time
			Time nextSpikeTime =
			    (spikeIdx < nSpikes) ? spikes[spikeIdx].t : tEnd;

			// Handle incomming spikes
			if (nextSpikeTime <= t) {
				// Record the old values
				recorder.record(t, s, aux<Flags>(s, p), true);

				// Fetch the spike from the list
				const Spike &spike = spikes[spikeIdx++];

				// Handle special spikes (if processing of special input spikes
				// is enabled)
				if ((Flags & PROCESS_SPECIAL) &&
				    handleSpecialSpikes<Flags>(spike, t, s, tLastSpike,
				                               recorder, p)) {
					continue;
				}

				// Add the spike weight to either the excitatory or the
				// inhibitory channel
				const Val w = spike.w * p.w();
				if (w > 0) {
					s.lE() += w;
				} else {
					s.lI() -= w;
				}

				// Record the new values
				recorder.inputSpike(t, s);
				recorder.record(t, s, aux<Flags>(s, p), true);
				continue;
			}

			// Fetch the currently allowed maximum tDelta -- we'll limit this to
			// the time needed to reach the next spike (otherwise the ODE would
			// not be autonomous) and the time until the refractory periof has
			// passed.
			const bool inRefrac =
			    (!(Flags & DISABLE_REFRACTORY)) && t - tLastSpike < tRefrac;
			Time tDeltaMax = nextSpikeTime - t;
			if (!(Flags & DISABLE_REFRACTORY) && inRefrac) {
				const Time tRefLeft = tLastSpike + tRefrac - t;
				if (tRefLeft < tDeltaMax) {
					tDeltaMax = tRefLeft;
				}
			}

			// Perform the actual integration
			std::pair<State, Time> res =
			    integrator.integrate(std::min(tDelta, tDeltaMax), tDeltaMax, s,
			                         [&p, inRefrac](const State &s) {
				    return df<Flags>(s, aux<Flags>(s, p), p, inRefrac);
				});

			// Copy the result and advance the time by the performed
			// timestep
			s = res.first;
			t += res.second;

			// Calculate the auxiliary state for the recorder
			AuxiliaryState as = aux<Flags>(s, p);

			// Reset the neuron if the spike potential is reached
			if (!(Flags & DISABLE_SPIKING) &&
			    s.v() > ((Flags & IF_COND_EXP) ? p.eTh() : p.eSpike())) {
				generateOutputSpike<Flags>(t, s, tLastSpike, recorder, p);
			}

			// Record the value -- this is the regular position in which values
			// should be recorded
			recorder.record(t, s, as, false);

			// Ask the controller whether it is time to abort
			const ControllerResult cres =
			    controller.control(t, s, as, p, inRefrac);
			if (cres == ControllerResult::ABORT ||
			    (cres == ControllerResult::MAY_CONTINUE &&
			     spikeIdx >= nSpikes)) {
				break;
			}
		}
	}
};
}

#endif /* _ADEXPSIM_MODEL_HPP_ */

