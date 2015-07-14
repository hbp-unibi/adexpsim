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
#include <exception>

#include <json/json.h>

#include "JsonIo.hpp"

namespace AdExpSim {

static constexpr Val PYNN_TIME = 1e3;
static constexpr Val PYNN_VOLT = 1e3;
static constexpr Val PYNN_CURR = 1e9;
static constexpr Val PYNN_COND = 1e6;
static constexpr Val PYNN_CAP = 1e9;

template <typename Trafo>
static void read(const Json::Value &value, const char *key, Parameters &params,
                 size_t idx, double scale, Trafo t)
{
	if (value.isMember(key)) {
		params[idx] = t(value[key].asDouble() / scale);
	}
}

static void read(const Json::Value &value, const char *key, Parameters &params,
                 size_t idx, double scale = 1.0)
{
	read(value, key, params, idx, scale, [](double v) -> double { return v; });
}

template <typename Enum>
static void readEnum(const std::string &s,
                     const std::vector<std::string> &names, Enum &val)
{
	for (size_t i = 0; i < names.size(); i++) {
		if (s == names[i]) {
			val = Enum(i);
		}
	}
}

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
	os << o << std::endl;
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
	os << "        'timestep': data.timestep," << std::endl;
	os << "        'min_delay': data.timestep," << std::endl;
	os << "        'hardware': sim.hardwareSetup[\"one-hicann\"]," << std::endl;
	os << "        'useSystemSim': True," << std::endl;
	os << "        'ignoreDatabase': True," << std::endl;
	os << "        'ignoreHWParameterRanges': True," << std::endl;
	os << "        'speedupFactor': 10000," << std::endl;
	os << "        'model': '"
	   << (model == ModelType::IF_COND_EXP ? "IF_cond_exp"
	                                       : "EIF_cond_exp_isfa_ista") << "',"
	   << std::endl;
	os << "    }" << std::endl;
}

static Json::Value serializeSpikeTrainDescriptors(
    const std::vector<SpikeTrain::Descriptor> &descrs)
{
	const int n = descrs.size();
	Json::Value res(Json::arrayValue);
	res.resize(n);
	for (int i = 0; i < n; i++) {
		Json::Value descr;
		descr["nE"] = descrs[i].nE;
		descr["nI"] = descrs[i].nI;
		descr["nOut"] = descrs[i].nOut;
		descr["sigmaT"] = descrs[i].sigmaT;
		descr["wE"] = descrs[i].wE;
		descr["wI"] = descrs[i].wI;
		descr["sigmaW"] = descrs[i].sigmaW;
		res[i] = descr;
	}
	return res;
}

static std::vector<SpikeTrain::Descriptor> deserializeSpikeTrainDescriptors(
    const Json::Value &value)
{
	const SpikeTrain::Descriptor DEFAULT_DESCR;

	std::vector<SpikeTrain::Descriptor> descrs;
	if (value.isArray()) {
		for (int i = 0; i < int(value.size()); i++) {
			descrs.emplace_back(
			    value[i].get("nE", int(DEFAULT_DESCR.nE)).asUInt(),
			    value[i].get("nI", int(DEFAULT_DESCR.nI)).asUInt(),
			    value[i].get("nOut", int(DEFAULT_DESCR.nOut)).asUInt(),
			    value[i].get("sigmaT", DEFAULT_DESCR.sigmaT).asDouble(),
			    value[i].get("wE", DEFAULT_DESCR.wE).asDouble(),
			    value[i].get("wI", DEFAULT_DESCR.wI).asDouble(),
			    value[i].get("sigmaW", DEFAULT_DESCR.sigmaW).asDouble());
		}
	}
	return descrs;
}

static Json::Value serializeSpikeTrain(const SpikeTrain &train)
{
	Json::Value res;
	res["descrs"] = serializeSpikeTrainDescriptors(train.getDescrs());
	res["n"] = int(train.getN());
	res["sorted"] = train.isSorted();
	res["T"] = train.getT().sec();
	res["sigmaT"] = train.getSigmaT();
	return res;
}

static SpikeTrain deserializeSpikeTrain(const Json::Value &value)
{
	std::vector<SpikeTrain::Descriptor> descrs =
	    deserializeSpikeTrainDescriptors(value["descrs"]);
	const SpikeTrain DEFAULT_TRAIN(descrs);

	return SpikeTrain(
	    descrs, value.get("n", int(DEFAULT_TRAIN.getN())).asUInt(),
	    value.get("sorted", DEFAULT_TRAIN.isSorted()).asBool(),
	    Time::sec(value.get("T", DEFAULT_TRAIN.getT().sec()).asDouble()),
	    value.get("sigmaT", DEFAULT_TRAIN.getSigmaT()).asDouble());
}

static Json::Value serializeSingleGroup(const SingleGroupSpikeData &singleGroup)
{
	Json::Value res;
	res["n"] = singleGroup.n;
	res["nM1"] = singleGroup.nM1;
	res["deltaT"] = singleGroup.deltaT.sec();
	res["T"] = singleGroup.T.sec();
	return res;
}

static Json::Value serializeParameters(const Parameters &params)
{
	Json::Value res;
	res["gL"] = params.gL();
	res["tauE"] = params.tauE();
	res["tauI"] = params.tauI();
	res["tauW"] = params.tauW();
	res["eE"] = params.eE();
	res["eI"] = params.eI();
	res["eTh"] = params.eTh();
	res["eSpike"] = params.eSpike();
	res["eReset"] = params.eReset();
	res["deltaTh"] = params.deltaTh();
	res["a"] = params.a();
	res["b"] = params.b();
	res["eL"] = params.eL();
	res["cM"] = params.cM();
	return res;
}

template <typename Arr>
static Json::Value serializeArray(const Arr &arr)
{
	Json::Value res(Json::arrayValue);
	res.resize(arr.size());
	int i = 0;
	for (const auto &v : arr) {
		res[i++] = v;
	}
	return res;
}

void JsonIo::storeParameters(std::ostream &os,
                             const ParameterCollection &params)
{
	Json::Value res;
	res["model"] = ParameterCollection::modelNames[size_t(params.model)];
	res["evaluation"] =
	    ParameterCollection::evaluationNames[size_t(params.evaluation)];
	res["spikeTrain"] = serializeSpikeTrain(params.train);
	res["singleGroup"] = serializeSingleGroup(params.singleGroup);
	res["parameters"] = serializeParameters(params.params);
	res["min"] = serializeArray(params.min);
	res["max"] = serializeArray(params.max);
	res["optimize"] = serializeArray(params.optimize);
	res["explore"] = serializeArray(params.explore);
	os << res << std::endl;
}

static void loadParametersFromValue(const Json::Value &value,
                                    ParameterCollection &params)
{
	Json::Value res;

	// Read the model and evaluation enum
	if (value.isMember("model")) {
		readEnum(value["model"].asString(), ParameterCollection::modelNames,
		         params.model);
	}
	if (value.isMember("evaluation")) {
		readEnum(value["evaluation"].asString(),
		         ParameterCollection::evaluationNames, params.evaluation);
	}

	// Read the spike train setup
	if (value.isMember("spikeTrain")) {
		params.train = deserializeSpikeTrain(value["spikeTrain"]);
	}

	/*	res["singleGroup"] = serializeSingleGroup(params.singleGroup);
	    res["parameters"] = serializeParameters(params.params);
	    res["min"] = serializeArray(params.min);
	    res["max"] = serializeArray(params.max);
	    res["optimize"] = serializeArray(params.optimize);
	    res["explore"] = serializeArray(params.explore);*/
}

static void loadPyNNParametersFromValue(const Json::Value &value,
                                        Parameters &params)
{
	read(value, "cm", params, Parameters::idx_cM, PYNN_CAP);
	read(value, "tau_m", params, Parameters::idx_gL, PYNN_TIME,
	     [&params](double v) { return params.cM() / v; });
	read(value, "tau_syn_E", params, Parameters::idx_tauE, PYNN_TIME);
	read(value, "tau_syn_I", params, Parameters::idx_tauI, PYNN_TIME);
	read(value, "tau_w", params, Parameters::idx_tauW, PYNN_TIME);
	read(value, "v_rest", params, Parameters::idx_eL, PYNN_VOLT);
	read(value, "v_thresh", params, Parameters::idx_eTh, PYNN_VOLT);
	read(value, "v_reset", params, Parameters::idx_eReset, PYNN_VOLT);
	read(value, "v_spike", params, Parameters::idx_eSpike, PYNN_VOLT);
	read(value, "e_rev_E", params, Parameters::idx_eE, PYNN_VOLT);
	read(value, "e_rev_I", params, Parameters::idx_eI, PYNN_VOLT);
	read(value, "a", params, Parameters::idx_a, PYNN_CURR);
	read(value, "b", params, Parameters::idx_b, PYNN_CURR);
	read(value, "delta_T", params, Parameters::idx_deltaTh, PYNN_VOLT);
	read(value, "syn_weight", params, Parameters::idx_w, PYNN_COND);
}

bool JsonIo::loadParameters(std::istream &is, ParameterCollection &params)
{
	try {
		Json::Value o;
		is >> o;
		loadParametersFromValue(o, params);
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	return true;
}

bool JsonIo::loadPyNNParameters(std::istream &is, Parameters &params)
{
	try {
		Json::Value o;
		is >> o;
		loadPyNNParametersFromValue(o, params);
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	return true;
}

static int calcIsPyNNModelScore(const Json::Value &v)
{
	return (v.isMember("cm") + v.isMember("tau_m") + v.isMember("tau_syn_E") +
	        v.isMember("tau_syn_I") + v.isMember("tau_w") +
	        v.isMember("tau_refrac") + v.isMember("v_rest") +
	        v.isMember("v_thresh") + v.isMember("v_reset") +
	        v.isMember("v_spike") + v.isMember("e_rev_E") +
	        v.isMember("e_rev_I") + v.isMember("a") + v.isMember("b") +
	        v.isMember("delta_T") + v.isMember("i_offset") +
	        v.isMember("syn_weight")) *
	       100 / 17;
}

static int calcIsParametersScore(const Json::Value &v)
{
	return (v.isMember("model") + v.isMember("evaluation") +
	        v.isMember("spikeTrain") + v.isMember("singleGroup") +
	        v.isMember("parameters") + v.isMember("min") + v.isMember("max") +
	        v.isMember("optimize") + v.isMember("explore")) *
	       100 / 9;
}

bool JsonIo::loadGenericParameters(std::istream &is,
                                   ParameterCollection &params)
{
	try {
		// Parse the input stream into a JSON value
		Json::Value o;
		is >> o;

		// Decide whether to load PyNN parameters or internal parameters
		int sPyNN = calcIsPyNNModelScore(o);
		int sParams = calcIsParametersScore(o);
		if (sPyNN > 0 || sParams > 0) {
			if (sPyNN > sParams) {
				loadPyNNParametersFromValue(o, params.params);
			} else {
				params = ParameterCollection();  // Start with empty collection
				loadParametersFromValue(o, params);
			}
		}
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
	return true;
}
}

