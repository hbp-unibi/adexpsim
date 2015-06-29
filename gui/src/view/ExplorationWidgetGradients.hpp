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

#ifndef _EXPLORATION_WIDGET_GRADIENTS_HPP_
#define _EXPLORATION_WIDGET_GRADIENTS_HPP_

#include <qcustomplot.h>

namespace AdExpSim {

struct ExplorationWidgetGradients {
	static const QCPColorGradient& blue();
	static const QCPColorGradient& green();
	static const QCPColorGradient& orange();
	static const QCPColorGradient& spectral();
};
}

#endif /* _EXPLORATION_WIDGET_GRADIENTS_HPP_ */
