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
 * @file SurfacePlotIo.hpp
 *
 * Contains classes for generating a surface plot using Python matplotlib or
 * gnuplot.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SURFACE_PLOT_IO_HPP_
#define _ADEXPSIM_SURFACE_PLOT_IO_HPP_

#include <iostream>

#include <simulation/Parameters.hpp>
#include <exploration/Exploration.hpp>

namespace AdExpSim {

class SurfacePlotIo {
public:
	/**
	 * Stores a file with one X Y Z tuple per line, separated by a single space.
	 * If the "gnuplot" argument is set to true, a blank line is inserted after
	 * each "X" block.
	 */
	static void storeSurfacePlot(std::ostream &os,
	                             const Exploration &exploration, size_t dim,
	                             bool gnuplot = true);

	/**
	 * Runs GnuPlot and shows the given exploration.
	 */
	static void runGnuPlot(const Parameters &params,
	                       const Exploration &exploration, size_t dim);
};
}

#endif /* _ADEXPSIM_SURFACE_PLOT_IO_HPP_ */

