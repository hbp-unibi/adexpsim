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

#include <qcustomplot.h>

#include "ExplorationWidgetGradients.hpp"

namespace AdExpSim {

// All colors from: http://colorbrewer2.com/

const QCPColorGradient &ExplorationWidgetGradients::blue()
{
	static QCPColorGradient blue;
	static bool initialized = false;
	if (!initialized) {
		blue.setColorStops({{0.0, QColor(255, 255, 255)},
		                    {0.125, QColor(255, 247, 251)},
		                    {0.25, QColor(236, 231, 242)},
		                    {0.375, QColor(208, 209, 230)},
		                    {0.5, QColor(166, 189, 219)},
		                    {0.625, QColor(116, 169, 207)},
		                    {0.75, QColor(54, 144, 192)},
		                    {0.875, QColor(5, 112, 176)},
		                    {0.90, QColor(3, 78, 123)},
		                    {1.0, QColor(1, 39, 61)}});
		initialized = true;
	}
	return blue;
}

const QCPColorGradient &ExplorationWidgetGradients::green()
{
	static QCPColorGradient green;
	static bool initialized = false;
	if (!initialized) {
		green.setColorStops({{0.0, QColor(255, 255, 255)},
		                     {0.125, QColor(229, 245, 224)},
		                     {0.25, QColor(229, 245, 224)},
		                     {0.375, QColor(199, 233, 192)},
		                     {0.5, QColor(161, 217, 155)},
		                     {0.625, QColor(116, 196, 118)},
		                     {0.75, QColor(65, 171, 93)},
		                     {0.875, QColor(35, 139, 69)},
		                     {0.90, QColor(0, 90, 50)},
		                     {1.0, QColor(0, 45, 25)}});
		initialized = true;
	}
	return green;
}

const QCPColorGradient &ExplorationWidgetGradients::orange()
{
	static QCPColorGradient orange;
	static bool initialized = false;
	if (!initialized) {
		orange.setColorStops({{0.0, QColor(255, 255, 255)},
		                      {0.125, QColor(255, 245, 235)},
		                      {0.25, QColor(254, 230, 206)},
		                      {0.375, QColor(253, 208, 162)},
		                      {0.5, QColor(253, 174, 107)},
		                      {0.625, QColor(253, 141, 60)},
		                      {0.75, QColor(241, 105, 19)},
		                      {0.875, QColor(217, 72, 1)},
		                      {0.90, QColor(140, 45, 4)},
		                      {1.0, QColor(70, 22, 2)}});
		initialized = true;
	}
	return orange;
}

const QCPColorGradient &ExplorationWidgetGradients::spectral()
{
	static QCPColorGradient orange;
	static bool initialized = false;
	if (!initialized) {
		orange.setColorStops({{0.0, QColor(213, 62, 79)},
		                      {0.125, QColor(244, 109, 67)},
		                      {0.25, QColor(253, 174, 97)},
		                      {0.375, QColor(254, 224, 139)},
		                      {0.5, QColor(255, 255, 191)},
		                      {0.625, QColor(230, 245, 152)},
		                      {0.75, QColor(171, 221, 164)},
		                      {0.875, QColor(102, 194, 165)},
		                      {1.0, QColor(50, 136, 189)}});
		initialized = true;
	}
	return orange;
}
}
