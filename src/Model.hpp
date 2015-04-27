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

#ifndef _ADEXPSIM_MODEL_HPP_
#define _ADEXPSIM_MODEL_HPP_

#include <cmath>

#include "Parameters.hpp"
#include "Spike.hpp"
#include "State.hpp"

namespace AdExpSim {

class Model {
public:
	template <typename Recorder>
	static void simulate(Time tEnd, const SpikeVec &spikes, Recorder &recorder,
	                     const State &s0, const Parameters &p = Parameters(),
	                     Time tDelta = 1e-6, int recordInterval = 10)
	{
		const Val tD = tDelta.toSeconds();
		size_t si = 0;
		int ri = 1;
		State s = s0, sN;
		AuxiliaryState as;
		for (Time t = 0.0; t < tEnd; t += tDelta) {
			// Copy the old state to the new state
			sN = s;

			// Decrease the record interval counter
			ri--;

			// Calculate the auxiliary state
			as.iL = p.gL * (s.v - p.eL);
			as.iE = s.gE * (s.v - p.eE);
			as.iI = s.gI * (s.v - p.eI);
			as.iTh = -p.gL * p.deltaTh * exp((s.v - p.eTh) / p.deltaTh);

			// Handle incomming spikes
			if (si < spikes.size() && spikes[si].t <= t) {
				// Record the old values
				recorder.record(t, s, as);

				// Add the spike weight to either the excitatory or the
				// inhibitory channel
				const Val w = spikes[si].w;
				if (w > 0) {
					s.gE += w;
				} else {
					s.gI -= w;
				}

				// Record the new values
				recorder.record(t, s, as);

				// Go to the next spike, reset the record interval counter
				si++;
				ri = recordInterval;
			}

			// Calculate the next voltage value
			sN.v = s.v - ((as.iL + as.iE + as.iI + as.iTh) / p.cM) * tD;
			if (sN.v > p.eSpike) {
				// Record the spike event
				sN.v = p.eSpike;
				recorder.record(t, sN, as);

				// Reset the voltage and increase the adaptation current
				sN.v = p.eReset;
				sN.w = sN.w + p.b;
				recorder.record(t, sN, as);

				// Reset the record counter -- we've already recorded something
				ri = recordInterval;
			}

			// Record the data if the record interval is reached
			if (ri == 0) {
				recorder.record(t, sN, as);
				ri = recordInterval;
			}

			// Calculate all other state values
			sN.gE = s.gE - (s.gE / p.tauE) * tD;
			sN.gI = s.gI - (s.gI / p.tauI) * tD;
			sN.w = s.w - ((s.w - p.a * (s.v - p.eL)) / p.tauW) * tD;

			// Assign the new state to the current state
			s = sN;
		}
	}
};
}

#endif /* _ADEXPSIM_MODEL_HPP_ */
