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

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

#include <common/FastMath.hpp>

#include "Controller.hpp"
#include "Integrator.hpp"
#include "Parameters.hpp"
#include "Recorder.hpp"
#include "Spike.hpp"
#include "State.hpp"

namespace AdExpSim {
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
	 * Set this flag if a fast approximation of the exponential function should
	 * be used. This approximation is less accurate, however is significantly
	 * faster. An exponential function is only used if ITH is not disabled.
	 */
	static constexpr uint8_t FAST_EXP = (1 << 3);

	/**
	 * Downgrades the model to the simpler IF_COND_EXP model. This flag includes
	 * DISABLE_ITH and causes the spiking potential to be set to eTh instead of
	 * eSpike.
	 */
	static constexpr uint8_t IF_COND_EXP = (1 << 4);

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
		// Calculate dvTh, but only if iTh is not disabled, either be the
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
	                const WorkingParameters &p)
	{
		return State(
		    -(as.dvL() + as.dvE() + as.dvI() + as.dvTh() + s.dvW()),  // [V/s]
		    -s.lE() * p.lE(),                                         // [1/s^2]
		    -s.lI() * p.lI(),                                         // [1/s^2]
		    (Flags & IF_COND_EXP) ? 0.0 : -(s.dvW() - p.lA() * s.v()) *
		                                      p.lW()  // [V/s^2]
		    );
	}

public:
	template <uint8_t Flags = 0, typename Recorder = NullRecorder,
	          typename Integrator = RungeKuttaIntegrator,
	          typename Controller = DefaultController>
	static void simulate(const SpikeVec &spikes, Recorder &recorder,
	                     Controller &controller, Integrator &integrator,
	                     const WorkingParameters &p = WorkingParameters(),
	                     Time tDelta = Time(-1), Time tEnd = MAX_TIME,
	                     const State &s0 = State())
	{
		// Use the automatically calculated tDelta if no user-defined value is
		// given
		if (tDelta <= Time(0)) {
			tDelta = Time::sec(p.tDelta());
		}

		// Number of spikes and index of the next spike that should be processed
		const size_t nSpikes = spikes.size();
		size_t spikeIdx = 0;

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

				// Add the spike weight to either the excitatory or the
				// inhibitory channel
				const Val w = spikes[spikeIdx].w * p.wSpike();
				if (w > 0) {
					s.lE() += w;
				} else {
					s.lI() -= w;
				}

				// Record the new values
				recorder.inputSpike(t, s);
				recorder.record(t, s, aux<Flags>(s, p), true);

				// Advance the spike index and try again
				spikeIdx++;
				continue;
			}

			// Fetch the currently allowed maximum tDelta -- we'll limit this to
			// the time needed to reach the next spike (otherwise the ODE would
			// not be autonomous).
			const Time tDeltaMax = nextSpikeTime - t;

			// Perform the actual integration
			std::pair<State, Time> res =
			    integrator.integrate(std::min(tDelta, tDeltaMax), tDeltaMax, s,
			                         [&p](const State &s) {
				    return df<Flags>(s, aux<Flags>(s, p), p);
				});

			// Copy the result and advance the time by the performed timestep
			s = res.first;
			t += res.second;

			// Calculate the auxiliary state for the recorder
			AuxiliaryState as = aux<Flags>(s, p);

			// Reset the neuron if the spike potential is reached
			if (!(Flags & DISABLE_SPIKING)) {
				if (s.v() > ((Flags & IF_COND_EXP) ? p.eTh() : p.eSpike())) {
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
				}
			}

			// Record the value -- this is the regular position in which values
			// should be recorded
			recorder.record(t, s, as, false);

			// Ask the controller whether it is time to abort
			const ControllerResult cres = controller.control(t, s, as, p);
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

