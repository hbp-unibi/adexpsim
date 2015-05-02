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

#ifndef _ADEXPSIM_MODEL_HPP_
#define _ADEXPSIM_MODEL_HPP_

#include <array>
#include <cmath>
#include <cstdint>

#include "Parameters.hpp"
#include "Spike.hpp"
#include "State.hpp"

#define USE_FAST_EXP 1

namespace AdExpSim {

static Val maxV = std::numeric_limits<Val>::min();

/**
 * The EulerIntegrator class represents euler's method for integrating ODEs. Do
 * not use this. For debugging only.
 */
class EulerIntegrator {
public:
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

class Model {
private:
#if USE_FAST_EXP
	// Source: https://code.google.com/p/fastapprox/
	// See also: http://www.schraudolph.org/pubs/Schraudolph99.pdf
	static float fastpow2(float p)
	{
		float offset = (p < 0) ? 1.0f : 0.0f;
		float clipp = (p < -126) ? -126.0f : p;
		int w = clipp;
		float z = clipp - w + offset;
		union {
			uint32_t i;
			float f;
		} v = {static_cast<uint32_t>(
		      (1 << 23) * (clipp + 121.2740575f +
		                   27.7280233f / (4.84252568f - z) - 1.49012907f * z))};

		return v.f;
	}

	static float fastexp(float p) { return fastpow2(1.442695040f * p); }
#endif

	/**
	 * Calculates the current auxiliary state. Note that the "exp" for the
	 * threshold current takes more than half of the time.
	 */
	static AuxiliaryState aux(const State &s, const WorkingParameters &p)
	{
		// Calculate the exponent that should be used inside the formula for
		// iTh. Clamp the value to "maxIThExponent" to prevent uneccessary
		// overflows.
		const Val iThExponent =
		    std::min(p.maxIThExponent, (s.v() - p.eTh) * p.invDeltaTh);

		return AuxiliaryState(p.lL * s.v(),             // dvL
		                      s.lE() * (s.v() - p.eE),  // dvE
		                      s.lI() * (s.v() - p.eI),  // dvI
#if USE_FAST_EXP
		                      -p.lL * p.deltaTh * fastexp(iThExponent)  // dvTh
#else
		                      -p.lL * p.deltaTh * exp(iThExponent)  // dvTh
#endif
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
		    -s.lE() * p.lE,                                           // [1/s^2]
		    -s.lI() * p.lI,                                           // [1/s^2]
		    -(s.dvW() - p.lA * s.v()) * p.lW                          // [V/s^2]
		    );
	}

public:
	template <typename Recorder, typename Integrator = RungeKuttaIntegrator>
	static void simulate(Time tEnd, const SpikeVec &spikes, Recorder &recorder,
	                     const State &s0,
	                     const WorkingParameters &p = WorkingParameters(),
	                     Time tDelta = 10e-6)
	{
		const Val h = tDelta.toSeconds();
		size_t spikeIdx = 0;
		State s = s0;
		for (Time t = 0.0; t < tEnd; t += tDelta) {
			// Handle incomming spikes
			while (spikeIdx < spikes.size() && spikes[spikeIdx].t <= t) {
				// Record the old values
				recorder.record(t, s, aux(s, p), true);

				// Add the spike weight to either the excitatory or the
				// inhibitory channel
				const Val w = spikes[spikeIdx].w;
				if (w > 0) {
					s.lE() += w;
				} else {
					s.lI() -= w;
				}

				// Record the new values
				recorder.record(t, s, aux(s, p), true);

				// Go to the next spike
				spikeIdx++;
			}

			// Perform the actual integration
			s = Integrator::integrate(
			    h, s, [&p](const State &s) { return df(s, aux(s, p), p); });

			// Calculate the auxiliary state for the recorder
			AuxiliaryState as = aux(s, p);

			// Reset the neuron if the spike potential is reached
			if (s.v() > p.eSpike) {
				// Record the spike event
				s.v() = p.eSpike;
				as = aux(s, p);
				recorder.record(t, s, as, true);

				// Track the maximum voltage
				maxV = std::max(s.v(), maxV);

				// Reset the voltage and increase the adaptation current
				s.v() = p.eReset;
				s.dvW() += p.lB;
				as = aux(s, p);
				recorder.record(t, s, as, true);
			}

			// Record the value -- this is the regular position in which values
			// should be recorded
			recorder.record(t, s, as, false);

			// Track the maximum voltage
			maxV = std::max(s.v(), maxV);
		}
	}
};
}

#endif /* _ADEXPSIM_MODEL_HPP_ */

