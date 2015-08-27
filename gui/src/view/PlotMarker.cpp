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

#include "PlotMarker.hpp"

namespace AdExpSim {

PlotMarker::PlotMarker(QCustomPlot *parentPlot, Type type, int size)
    : QCPAbstractItem(parentPlot),
      mPen(Qt::black),
      mBrush(Qt::black),
      mSize(size),
      mType(type),
      center(createPosition(QLatin1String("center")))
{
	center->setType(QCPItemPosition::ptPlotCoords);
	center->setCoords(0, 0);
}

PlotMarker::~PlotMarker() {}

double PlotMarker::selectTest(const QPointF &pos, bool onlySelectable,
                              QVariant *details) const
{
	return -1;
}

void PlotMarker::draw(QCPPainter *painter)
{
	QPointF c = center->pixelPoint();
	int mSize2 = mSize / 2;
	switch (mType) {
		case Type::POINT:
			painter->setPen(mPen);
			painter->setBrush(mBrush);
			painter->drawEllipse(c.x() - mSize2, c.y() - mSize2, mSize, mSize);
			break;
		case Type::CROSSHAIR_WITH_OUTLINE:
			painter->setPen(QPen(Qt::white, mPen.widthF() + 2.0));
			painter->drawLine(QPointF(c.x() - mSize2 - 1, c.y()),
			                  QPointF(c.x() + mSize2 + 1, c.y()));
			painter->drawLine(QPointF(c.x(), c.y() - mSize2 - 1),
			                  QPointF(c.x(), c.y() + mSize2 + 1));
		// fallthrough
		case Type::CROSSHAIR:
			painter->setPen(mPen);
			painter->drawLine(QPointF(c.x() - mSize2, c.y()),
			                  QPointF(c.x() + mSize2, c.y()));
			painter->drawLine(QPointF(c.x(), c.y() - mSize2),
			                  QPointF(c.x(), c.y() + mSize2));
			break;
	}
}
}
