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
	os << "        'ignoreHWParameterRanges': False," << std::endl;
	os << "        'speedupFactor': 10000," << std::endl;
	os << "        'model': '"
	   << (model == ModelType::IF_COND_EXP ? "IF_cond_exp"
	                                       : "EIF_cond_exp_isfa_ista") << "',"
	   << std::endl;
	os << "    }" << std::endl;
}

template <typename Enum>
static const std::string &serializeEnum(const Enum &val,
                                        const std::vector<std::string> &names)
{
	return names[size_t(val)];
}

template <typename Enum>
static void deserializeEnum(const std::string &s,
                            const std::vector<std::string> &names, Enum &val)
{
	for (size_t i = 0; i < names.size(); i++) {
		if (s == names[i]) {
			val = Enum(i);
		}
	}
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
	res["equidistant"] = train.isEquidistant();
	res["T"] = train.getT().sec();
	res["sigmaT"] = train.getSigmaT();
	return res;
}

static SpikeTrain deserializeSpikeTrain(
    const Json::Value &value, const SpikeTrain &defaultValues = SpikeTrain(
                                  std::vector<SpikeTrain::Descriptor>{}))
{
	return SpikeTrain(
	    value.isMember("descrs")
	        ? deserializeSpikeTrainDescriptors(value["descrs"])
	        : defaultValues.getDescrs(),
	    value.get("n", int(defaultValues.getN())).asUInt(),
	    value.get("sorted", defaultValues.isSorted()).asBool(),
	    Time::sec(value.get("T", defaultValues.getT().sec()).asDouble()),
	    value.get("sigmaT", defaultValues.getSigmaT()).asDouble(),
	    value.get("equidistant", defaultValues.isEquidistant()).asBool());
}

static Json::Value serializeSingleGroup(
    const SingleGroupMultiOutSpikeData &singleGroup)
{
	Json::Value res;
	res["n"] = singleGroup.n;
	res["nM1"] = singleGroup.nM1;
	res["nOut"] = singleGroup.nOut;
	res["deltaT"] = singleGroup.deltaT.sec();
	res["T"] = singleGroup.T.sec();
	return res;
}

static SingleGroupMultiOutSpikeData deserializeSingleGroup(
    const Json::Value &value,
    const SingleGroupMultiOutSpikeData &defaultValues =
        SingleGroupMultiOutSpikeData())
{
	return SingleGroupMultiOutSpikeData(
	    value.get("n", defaultValues.n).asDouble(),
	    value.get("nM1", defaultValues.nM1).asDouble(),
	    value.get("nOut", defaultValues.nOut).asDouble(),
	    Time::sec(value.get("deltaT", defaultValues.deltaT.sec()).asDouble()),
	    Time::sec(value.get("T", defaultValues.T.sec()).asDouble()));
}

template <typename Arr>
static Json::Value serializeArray(const Arr &arr,
                                  const std::vector<std::string> &nameIds)
{
	Json::Value res;
	size_t i = 0;
	for (const auto &v : arr) {
		res[nameIds[i++]] = v;
	}
	return res;
}

template <typename Arr, typename Acc>
static Arr deserializeArray(const Arr &defaultValues, const Json::Value &value,
                            const std::vector<std::string> &nameIds, Acc acc)
{
	Arr res = defaultValues;
	for (size_t i = 0; i < nameIds.size(); i++) {
		const std::string &id = nameIds[i];
		if (value.isMember(id)) {
			res[i] = acc(value[id]);
		}
	}
	return res;
}

void JsonIo::storeParameters(std::ostream &os,
                             const ParameterCollection &params)
{
	Json::Value res;
	res["model"] = serializeEnum(params.model, ParameterCollection::modelNames);
	res["evaluation"] =
	    serializeEnum(params.evaluation, ParameterCollection::evaluationNames);
	res["spikeTrain"] = serializeSpikeTrain(params.train);
	res["singleGroup"] = serializeSingleGroup(params.singleGroup);
	res["parameters"] = serializeArray(params.params, Parameters::nameIds);
	res["min"] = serializeArray(params.min, WorkingParameters::nameIds);
	res["max"] = serializeArray(params.max, WorkingParameters::nameIds);
	res["optimize"] =
	    serializeArray(params.optimize, WorkingParameters::nameIds);
	res["explore"] = serializeArray(params.explore, WorkingParameters::nameIds);
	os << res << std::endl;
}

static void loadParametersFromValue(const Json::Value &value,
                                    ParameterCollection &params)
{
	const ParameterCollection DEFAULT_COLLECTION;

	Json::Value res;

	if (value.isMember("model")) {
		deserializeEnum(value["model"].asString(),
		                ParameterCollection::modelNames, params.model);
	}

	if (value.isMember("evaluation")) {
		deserializeEnum(value["evaluation"].asString(),
		                ParameterCollection::evaluationNames,
		                params.evaluation);
	}

	if (value.isMember("spikeTrain")) {
		params.train = deserializeSpikeTrain(value["spikeTrain"],
		                                     DEFAULT_COLLECTION.train);
	}

	if (value.isMember("singleGroup")) {
		params.singleGroup = deserializeSingleGroup(
		    value["singleGroup"], DEFAULT_COLLECTION.singleGroup);
	}

	if (value.isMember("parameters")) {
		params.params = deserializeArray(
		    DEFAULT_COLLECTION.params, value["parameters"], Parameters::nameIds,
		    [](const Json::Value &v) { return v.asDouble(); });
	}

	if (value.isMember("min")) {
		params.min = deserializeArray(
		    DEFAULT_COLLECTION.min, value["min"], WorkingParameters::nameIds,
		    [](const Json::Value &v) { return v.asDouble(); });
	}

	if (value.isMember("max")) {
		params.max = deserializeArray(
		    DEFAULT_COLLECTION.max, value["max"], WorkingParameters::nameIds,
		    [](const Json::Value &v) { return v.asDouble(); });
	}

	if (value.isMember("explore")) {
		params.explore =
		    deserializeArray(DEFAULT_COLLECTION.explore, value["explore"],
		                     WorkingParameters::nameIds,
		                     [](const Json::Value &v) { return v.asBool(); });
	}

	if (value.isMember("optimize")) {
		params.optimize =
		    deserializeArray(DEFAULT_COLLECTION.optimize, value["optimize"],
		                     WorkingParameters::nameIds,
		                     [](const Json::Value &v) { return v.asBool(); });
	}
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

