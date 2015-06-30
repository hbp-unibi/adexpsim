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

#include <cstdint>

#include <json/json.h>

#include "JsonIo.hpp"

namespace AdExpSim {

static constexpr Val PYNN_TIME = 1e3;
static constexpr Val PYNN_VOLT = 1e3;
static constexpr Val PYNN_CURR = 1e9;
static constexpr Val PYNN_COND = 1e6;
static constexpr Val PYNN_CAP = 1e9;

void JsonIo::storePyNNModel(std::ostream &os, const Parameters &params,
                            ModelType model)
{
#define ADEXP(CODE)                           \
	if (model == ModelType::AD_IF_COND_EXP) { \
		CODE;                                 \
	}

	Json::Value o;
	o["cm"] = params.cM() * PYNN_CAP;
	o["tau_m"] = params.tauM() * PYNN_TIME;
	o["tau_syn_E"] = params.tauE() * PYNN_TIME;
	o["tau_syn_I"] = params.tauI() * PYNN_TIME;
	ADEXP(o["tau_w"] = params.tauW() * PYNN_TIME);
	o["tau_refrac"] = 0.0 * PYNN_TIME;
	o["v_rest"] = params.eL() * PYNN_VOLT;
	o["v_thresh"] = params.eTh() * PYNN_VOLT;
	o["v_reset"] = params.eReset() * PYNN_VOLT;
	ADEXP(o["v_spike"] = params.eSpike() * PYNN_VOLT);
	o["e_rev_E"] = params.eE() * PYNN_VOLT;
	o["e_rev_I"] = params.eI() * PYNN_VOLT;
	ADEXP(o["a"] = params.a() * PYNN_CURR);
	ADEXP(o["b"] = params.b() * PYNN_CURR);
	ADEXP(o["delta_T"] = params.deltaTh() * PYNN_VOLT);
	o["i_offset"] = 0.0 * PYNN_CURR;
	o["syn_weight"] = params.w() * PYNN_COND;
	os << o << std::endl;;
#undef ADEXP
}

void JsonIo::storePyNNSetupNEST(std::ostream &os, const Parameters &params,
                                ModelType model)
{
	os << "def setup(data, sim):" << std::endl;
	os << "    return {" << std::endl;
	os << "        'timestep': data.timestep," << std::endl;
	os << "        'min_delay': data.timestep," << std::endl;
	os << "        'spike_precision': 'off_grid'," << std::endl;
	os << "        'model': '"
	   << (model == ModelType::IF_COND_EXP ? "IF_cond_exp"
	                                       : "EIF_cond_exp_isfa_ista") << "',"
	   << std::endl;
	os << "    }" << std::endl;
}

void JsonIo::storePyNNSetupESS(std::ostream &os, const Parameters &params,
                               ModelType model)
{
	os << "def setup(data, sim):" << std::endl;
	os << "    return {" << std::endl;
	os << "        'ignoreHWParameterRanges': False," << std::endl;
	os << "        'timestep': data.timestep," << std::endl;
	os << "        'min_delay': data.timestep," << std::endl;
	os << "        'hardware': sim.hardwareSetup[\"one-hicann\"]," << std::endl;
	os << "        'useSystemSim': True," << std::endl;
	os << "        'ignoreDatabase': True," << std::endl;
	os << "        'ignoreHWParameterRanges': False," << std::endl;
	os << "        'speedupFactor': 10000," << std::endl;
	os << "        'model': '"
	   << (model == ModelType::IF_COND_EXP ? "IF_cond_exp"
	                                       : "EIF_cond_exp_isfa_ista") << "',"
	   << std::endl;
	os << "    }" << std::endl;
}

/*void JsonIo::storeParameters(std::istream &os,
                             const ParameterCollection &params);

void JsonIo::loadParameters(std::istream &is, ParameterCollection &params);*/
}
