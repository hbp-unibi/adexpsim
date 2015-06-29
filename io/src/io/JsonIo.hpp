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

/**
 * @file JsonIo.hpp
 *
 * Contains the JsonIo class for storing and loading parameters in the JSON
 * format. Furthermore it allows to export the parameters in a format that can
 * be read by the PyNN BiNAM simulator, both when using NEST or the ESS as a
 * backend.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_JSON_IO_HPP_
#define _ADEXPSIM_JSON_IO_HPP_

#include <utils/ParameterCollection.hpp>

#include <iostream>

namespace AdExpSim {
/**
 * The JsonIo class contains functions for loading and storing parameter sets.
 */
class JsonIo {
public:
	/**
	 * Stores the PyNN neuron model parameters. Depending on the given model
	 * only stores the parameters relevant for IfCondExp or the complete AdExp
	 * model.
	 */
	static void storePyNNModel(std::ostream &os, const Parameters &params,
	                                ModelType model);

	/**
	 * Generates a setup for PyNN using the NEST software simulator.
	 */
	static void storePyNNSetupNEST(std::ostream &os, const Parameters &params,
	                               ModelType model);

	/**
	 * Generates a setup for PyNN using the ESS hardware emulator.
	 */
	static void storePyNNSetupESS(std::ostream &os, const Parameters &params,
	                              ModelType model);

	/**
	 * Stores the complete parameter collection as a JSON file.
	 */
//	static void storeParameters(std::istream &os,
//	                                const ParameterCollection &params);

	/**
	 * Loads the complete parameter collection back from a JSON file.
	 */
//	static bool loadParameters(std::istream &is, ParameterCollection &params);
};
}

#endif /* _ADEXPSIM_JSON_IO_HPP_ */

