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

#include <utils/FastMath.hpp>

#include "Parameters.hpp"
#include "Spike.hpp"
#include "State.hpp"

namespace AdExpSim {
/**
 * The NullRecorder class can be used to discard all incomming data for maximum
 * efficiency (in the case the data does not need to be recorded).
 */
class NullRecorder {
public:
	/**
	 * Actually called by the simulation to record the internal state, however
	 * this class just acts as a null sink for this data.
	 */
	void record(Time, const State &, const AuxiliaryState &, bool)
	{
		// Discard everything
	}

	/**
	 * Called whenever an input spike is consumed by the model
	 *
	 * @param t is the time at which the input spike has been consumed.
	 * @param s is the state after the input spike has been consumed.
	 */
	void inputSpike(Time, const State &)
	{
		// Discard everything
	}

	/**
	 * Called whenever an output spike is produced by the model.
	 *
	 * @param t is the time at which the output spike has been produced.
	 * @param s is the neuron state after the spike has been issued (the neuron
	 * has already been reset).
	 */
	void outputSpike(Time t, const State &)
	{
		// Discard everything
	}
};

/**
 * The EulerIntegrator class represents euler's method for integrating ODEs. Do
 * not use this. For debugging only.
 */
class EulerIntegrator {
public:
	/**
	 * Implements the second-order Runge-Kutta method (Midpoint method).
	 *
	 * @param tDelta is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	static std::pair<State, Time> integrate(Time tDelta, Time, const State &s,
	                                        Deriv df)
	{
		const Val h = tDelta.sec();
		return std::pair<State, Time>(s + h * df(s), tDelta);
	}
};

/**
 * The MidpointIntegrator class implements the second-order Runge-Kutta
 * method.
 */
class MidpointIntegrator {
public:
	/**
	 * Implements the second-order Runge-Kutta method (Midpoint method).
	 *
	 * @param tDelta is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	static std::pair<State, Time> integrate(Time tDelta, Time, const State &s,
	                                        Deriv df)
	{
		const Val h = tDelta.sec();
		const State k1 = h * df(s);
		const State k2 = h * df(s + 0.5f * k1);

		return std::pair<State, Time>(s + k2, tDelta);
	}
};

/**
 * The RungeKuttaIntegrator class implements the fourth-order Runge-Kutta
 * method.
 */
class RungeKuttaIntegrator {
public:
	/**
	 * Implements the fourth-order Runge-Kutta method.
	 *
	 * @param tDelta is the timestep width.
	 * @param s is the current state vector at the previous timestep.
	 * @param df is the function which calculates the derivative for a given
	 * state.
	 * @return the new state for the next timestep and the actually used
	 * timestep.
	 */
	template <typename Deriv>
	static std::pair<State, Time> integrate(Time tDelta, Time, const State &s,
	                                        Deriv df)
	{
		const Val h = tDelta.sec();
		const State k1 = h * df(s);
		const State k2 = h * df(s + 0.5f * k1);
		const State k3 = h * df(s + 0.5f * k2);
		const State k4 = h * df(s + k3);

		return std::pair<State, Time>(s + (k1 + 2.0f * (k2 + k3) + k4) / 6.0f,
		                              tDelta);
	}
};

enum class ControllerResult { CONTINUE, MAY_CONTINUE, ABORT };

/**
 * The NullController class runs the simulation until tEnd is reached, producing
 * no overhead.
 */
class NullController {
public:
	static ControllerResult control(Time, const State &, const AuxiliaryState &,
	                                const WorkingParameters &)
	{
		return ControllerResult::CONTINUE;
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
	static ControllerResult control(Time, const State &s,
	                                const AuxiliaryState &,
	                                const WorkingParameters &)
	{
		return (fabs(s.v()) > MIN_VOLTAGE || (s.lE() + s.lI()) > MIN_RATE)
		           ? ControllerResult::CONTINUE
		           : ControllerResult::MAY_CONTINUE;
	}
};

/**
 * The MaxValueController class is used to abort the model calculation once the
 * maximum voltage is reached and to record this maximum voltage.
 */
class MaxValueController {
public:
	/**
	 * Minimum excitatory plus inhibitory channel rate. This value is chosen
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
	Val vMax;

	/**
	 * Time point at which the maximum voltage was recorded.
	 */
	Time tVMax;

	/**
	 * Time point at which the effective spiking potential was reached.
	 */
	Time tSpike;

	/**
	 * Default constructor, resets this instance to its initial state.
	 */
	MaxValueController() { reset(); }

	/**
	 * Resets the controller to its initial state.
	 */
	void reset()
	{
		vMax = std::numeric_limits<Val>::min();
		tVMax = MAX_TIME;
		tSpike = MAX_TIME;
	}

	/**
	 * The control function is responsible for aborting the simulation. The
	 * default controller aborts the simulation once the neuron has settled,
	 * so there are no inhibitory or excitatory currents and the membrane
	 * potential is near its resting potential.
	 *
	 * @param s is the current neuron state.
	 * @return true if the neuron should continue, false otherwise.
	 */
	ControllerResult control(Time t, const State &s, const AuxiliaryState &aux,
	                         const WorkingParameters &p)
	{
		// Track the maximum voltage
		if (s.v() > vMax) {
			vMax = s.v();
			tVMax = t;
		}

		// Track the time at which the spike potential was reached
		if (s.v() > p.eSpikeEff() && t < tSpike) {
			tSpike = t;
		}

		// Calculate the total current (voltage change rate, without dvL)
		Val dvSum = aux.dvTh() + aux.dvE() + aux.dvI() + s.dvW();

		// Do not abort as long as lE is larger than the minimum rate and the
		// current is negative (charges the neuron)
		return (s.lE() > MIN_RATE ||
		        (dvSum < MAX_DV && dvSum + aux.dvL() < MAX_DV))
		           ? ControllerResult::CONTINUE
		           : ControllerResult::MAY_CONTINUE;
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
	 * Set this flag if the spiking mechanism of the neuron should be switched
	 * off. ITh will still flow, but the neuron will not be reset. Only use in
	 * conjunction with CLAMP_ITH or DISABLE_ITH.
	 */
	static constexpr uint8_t DISABLE_SPIKING = (1 << 2);

	/**
	 * Set this flag if a fast approximation of the exponential function should
	 * be used. This approximation is less accurate, however is significantly
	 * faster.
	 */
	static constexpr uint8_t FAST_EXP = (1 << 3);

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
			    (spikeIdx < nSpikes) ? spikes[spikeIdx].t : MAX_TIME;

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
			std::pair<State, Time> res = integrator.integrate(
			    std::min(tDelta, tDeltaMax), tDeltaMax, s,
			    [&p](const State &s) { return df(s, aux<Flags>(s, p), p); });

			// Copy the result and advance the time by the performed timestep
			s = res.first;
			t += res.second;

			// Calculate the auxiliary state for the recorder
			AuxiliaryState as = aux<Flags>(s, p);

			// Reset the neuron if the spike potential is reached
			if (!(Flags & DISABLE_SPIKING)) {
				if (s.v() > p.eSpike()) {
					// Record the spike event
					s.v() = p.eSpike();
					as = aux<Flags>(s, p);
					recorder.record(t, s, as, true);

					// Reset the voltage and increase the adaptation current
					s.v() = p.eReset();
					s.dvW() += p.lB();
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

