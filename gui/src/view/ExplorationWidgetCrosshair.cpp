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

#include "ExplorationWidgetCrosshair.hpp"

namespace AdExpSim {

ExplorationWidgetCrosshair::ExplorationWidgetCrosshair(QCustomPlot *parentPlot)
    : QCPAbstractItem(parentPlot),
      center(createPosition(QLatin1String("center")))
{
	center->setCoords(0, 0);
	setPen(QPen(Qt::black, 2));
}

ExplorationWidgetCrosshair::~ExplorationWidgetCrosshair() {}

void ExplorationWidgetCrosshair::setPen(const QPen &pen) { mPen = pen; }

double ExplorationWidgetCrosshair::selectTest(const QPointF &pos,
                                              bool onlySelectable,
                                              QVariant *details) const
{
	return -1;
}

void ExplorationWidgetCrosshair::draw(QCPPainter *painter)
{
	QPointF c = center->pixelPoint();
	painter->setPen(mPen);
	painter->drawLine(QPointF(c.x() - 5, c.y()), QPointF(c.x() + 5, c.y()));
	painter->drawLine(QPointF(c.x(), c.y() - 5), QPointF(c.x(), c.y() + 5));
}
}
