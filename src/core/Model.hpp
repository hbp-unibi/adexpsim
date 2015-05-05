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

#include <utils/FastMath.hpp>

#include "Parameters.hpp"
#include "Spike.hpp"
#include "State.hpp"

namespace AdExpSim {
/**
 * The EulerIntegrator class represents euler's method for integrating ODEs. Do
 * not use this. For debugging only.
 */
class EulerIntegrator {
public:
	/**
	 * Implements the standard Euler integrator.
	 *
	 * @param h is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep.
	 */
	template <typename Func>
	static State integrate(Val h, const State &s, Func df)
	{
		return s + h * df(s);
	}
};

/**
 * The RungeKuttaIntegrator class implements the fourth-order Runge-Kutta
 * method. Allows about one magnitude smaller step size than Euler's method.
 */
class RungeKuttaIntegrator {
public:
	/**
	 * Implements the fourth-order Runge-Kutta method.
	 *
	 * @param h is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep.
	 */
	template <typename Func>
	static State integrate(Val h, const State &s, Func df)
	{
		State k1 = df(s);
		State k2 = df(s + h * 0.5f * k1);
		State k3 = df(s + h * 0.5f * k2);
		State k4 = df(s + h * k3);

		return s + h * 0.16666666667f * (k1 + 2.0f * (k2 + k3) + k4);
	}
};

/**
 * The DefaultController class is used to abort the model calculation once the
 * model has settled: The leak current is set to zero and the channel
 * conductance is small.
 */
class DefaultController {
public:
	/**
	 * Minimum neuron voltage at which the simulation can be aborted.
	 */
	static constexpr Val MIN_VOLTAGE = 1e-4;

	/**
	 * Minim excitatory plus inhibitory channel rate.
	 */
	static constexpr Val MIN_RATE = 1e-3;

	/**
	 * The control function is responsible for aborting the simulation. The
	 * default controller aborts the simulation once the neuron has settled,
	 * so there are no inhibitory or excitatory currents and the membrane
	 * potential is near its resting potential.
	 *
	 * @param s is the current neuron state.
	 * @return true if the neuron should continue, false otherwise.
	 */
	static bool control(Time, const State &s, const AuxiliaryState &,
	                    const WorkingParameters &)
	{
		return fabs(s.v()) > MIN_VOLTAGE || (s.lE() + s.lI()) > MIN_RATE;
	}
};

/**
 * The MaxValueController class is used to abort the model calculation once the
 * maximum voltage is reached and to record this maximum voltage.
 */
class MaxValueController {
public:
	/**
	 * Minim excitatory plus inhibitory channel rate. This value is chosen
	 * rather high as we want to abort as early as possible and only if the
	 * high excitatory current could still increase the membrane potential.
	 */
	static constexpr Val MIN_RATE = 10;

	/**
	 * Minim excitatory plus inhibitory channel rate. This value is chosen
	 * rather high as we want to abort as early as possible and only if the
	 * high excitatory current could still increase the membrane potential.
	 */
	static constexpr Val MAX_DV = -1e-4;

	/**
	 * Maximum voltage
	 */
	Val vMax = std::numeric_limits<Val>::min();

	/**
	 * Time point at which the maximum voltage was recorded.
	 */
	Time vMaxTime = MAX_TIME;

	/**
	 * Time point at which the effective spiking potential was reached.
	 */
	Time tSpike = MAX_TIME;

	/**
	 * The control function is responsible for aborting the simulation. The
	 * default controller aborts the simulation once the neuron has settled,
	 * so there are no inhibitory or excitatory currents and the membrane
	 * potential is near its resting potential.
	 *
	 * @param s is the current neuron state.
	 * @return true if the neuron should continue, false otherwise.
	 */
	bool control(Time t, const State &s, const AuxiliaryState &aux,
	             const WorkingParameters &p)
	{
		// Track the maximum voltage
		if (s.v() > vMax) {
			vMax = s.v();
			vMaxTime = t;
		}

		// Track the time at which the spike potential was reached
		if (s.v() > p.eSpikeEff() && t < tSpike) {
			tSpike = t;
		}

		// Do not abort as long as lE is larger than the minimum rate and the
		// current is negative (charges the neuron)
		return s.lE() > MIN_RATE ||
		       (aux.dvL() + aux.dvTh() + aux.dvE() + aux.dvI()) < MAX_DV;
	}
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
	 * Set this flag if a fast approximation of the exponential function should
	 * be used. This approximation is less accurate, however is signigicantly
	 * faster.
	 */
	static constexpr uint8_t FAST_EXP = (1 << 2);

private:
	/**
	 * Calculates the current auxiliary state. This function is the bottleneck
	 * of the simulation, with the "exp" for the threshold current taking more
	 * than half of the time.
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
		// Calculate the exponent that should be used inside the formula for
		// iTh. Clamp the value to "maxIThExponent" to prevent uneccessary
		// overflows.
		const Val dvThExponent =
		    (Flags & CLAMP_ITH)
		        ? (std::min(p.eSpikeEffRed(), s.v()) - p.eTh()) * p.invDeltaTh()
		        : std::min(p.maxIThExponent(),
		                   (s.v() - p.eTh()) * p.invDeltaTh());

		// Calculate dvTh depending on the flags
		const Val dvTh = (Flags & DISABLE_ITH)
		                     ? 0.0f
		                     : -p.lL() * p.deltaTh() *
		                           (Flags & FAST_EXP ? fast::exp(dvThExponent)
		                                             : exp(dvThExponent));

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
	static State df(const State &s, const AuxiliaryState &as,
	                const WorkingParameters &p)
	{
		return State(
		    -(as.dvL() + as.dvE() + as.dvI() + as.dvTh() + s.dvW()),  // [V/s]
		    -s.lE() * p.lE(),                                         // [1/s^2]
		    -s.lI() * p.lI(),                                         // [1/s^2]
		    -(s.dvW() - p.lA() * s.v()) * p.lW()                      // [V/s^2]
		    );
	}

public:
	template <uint8_t Flags = 0, typename Integrator = RungeKuttaIntegrator,
	          typename Recorder, typename Controller>
	static void simulate(const SpikeVec &spikes, Recorder &recorder,
	                     Controller &controller,
	                     const WorkingParameters &p = WorkingParameters(),
	                     Time tDelta = -1, Time tEnd = MAX_TIME,
	                     const State &s0 = State())
	{
		// Use the automatically calculated tDelta if no user-defined value is
		// given
		if (tDelta <= 0.0) {
			tDelta = p.tDelta();
		}

		// Fetch the timestep h in sconds
		const Val h = tDelta.toSeconds();

		// Number of spikes and index of the next spike that should be processed
		const size_t nSpikes = spikes.size();
		size_t spikeIdx = 0;

		// Start with state s0
		State s = s0;

		// Iterate over all time slices
		for (Time t = 0.0; t < tEnd; t += tDelta) {
			// Handle incomming spikes
			while (spikeIdx < nSpikes && spikes[spikeIdx].t <= t) {
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
				recorder.record(t, s, aux<Flags>(s, p), true);

				// Go to the next spike
				spikeIdx++;
			}

			// Perform the actual integration
			s = Integrator::integrate(h, s, [&p](const State &s) {
				return df(s, aux<Flags>(s, p), p);
			});

			// Calculate the auxiliary state for the recorder
			AuxiliaryState as = aux<Flags>(s, p);

			// Reset the neuron if the spike potential is reached
			if (s.v() > p.eSpike()) {
				// Record the spike event
				s.v() = p.eSpike();
				as = aux<Flags>(s, p);
				recorder.record(t, s, as, true);

				// Reset the voltage and increase the adaptation current
				s.v() = p.eReset();
				s.dvW() += p.lB();
				as = aux<Flags>(s, p);
				recorder.record(t, s, as, true);
			}

			// Record the value -- this is the regular position in which values
			// should be recorded
			recorder.record(t, s, as, false);

			// Ask the controller whether it is time to abort
			if (!controller.control(t, s, as, p) && spikeIdx >= nSpikes) {
				break;
			}
		}
	}
};
}

#endif /* _ADEXPSIM_MODEL_HPP_ */

