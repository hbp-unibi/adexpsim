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

#include "ParameterCollection.hpp"

namespace AdExpSim {

const std::vector<std::string> ParameterCollection::modelNames = {
    "IfCondExp", "AdIfCondExp"};

const std::vector<std::string> ParameterCollection::evaluationNames = {
    "Train", "SgSo", "SgMo"};

ParameterCollection::ParameterCollection()
    : model(ModelType::IF_COND_EXP),
      evaluation(EvaluationType::SINGLE_GROUP),
      singleGroup(),
      min({MIN_HZ, MIN_HZ, MIN_HZ, MIN_HZ, MIN_SEC, MIN_V, MIN_V, MIN_V, MIN_V,
           MIN_V, MIN_V, MIN_HZ,
           WorkingParameters::fromParameter(MIN_A, 11, DefaultParameters::cM,
                                            DefaultParameters::eL),
           WorkingParameters::fromParameter(MIN_S, 12, DefaultParameters::cM,
                                            DefaultParameters::eL)}),
      max({MAX_HZ, MAX_HZ, MAX_HZ, MAX_HZ, MAX_SEC, MAX_V, MAX_V, MAX_V, MAX_V,
           MAX_V, MAX_V, MAX_HZ,
           WorkingParameters::fromParameter(MAX_A, 11, DefaultParameters::cM,
                                            DefaultParameters::eL),
           WorkingParameters::fromParameter(MAX_S, 12, DefaultParameters::cM,
                                            DefaultParameters::eL)}),
      optimize({true, true, false, true, true, true, false, true, false, true,
                true, true, true,
                true})  // Do not optimize the inhibitory channels and eSpike
{
	// Initialize the optimize/explore flags
	explore.fill(false);

	// Initialize the spike train
	train.fromSingleGroupSpikeData(singleGroup);
}

template <typename T>
static std::vector<size_t> activeElements(const T &list)
{
	std::vector<size_t> res;
	for (size_t i = 0; i < list.size(); i++) {
		if (list[i]) {
			res.push_back(i);
		}
	}
	return res;
}

std::vector<size_t> ParameterCollection::optimizationDims() const
{
	return activeElements(optimize);
}

std::vector<size_t> ParameterCollection::explorationDims() const
{
	return activeElements(explore);
}
}

