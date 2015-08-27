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

#ifndef _PLOT_MARKER_HPP_
#define _PLOT_MARKER_HPP_

#include <qcustomplot.h>

namespace AdExpSim {
class PlotMarker : public QCPAbstractItem {
	Q_OBJECT
public:
	enum class Type { POINT, CROSSHAIR, CROSSHAIR_WITH_OUTLINE };

protected:
	QPen mPen;
	QBrush mBrush;
	int mSize;
	Type mType;

	void draw(QCPPainter *painter) override;

public:
	PlotMarker(QCustomPlot *parentPlot, Type type = Type::POINT, int size = 5);
	virtual ~PlotMarker();

	// getters:
	QPen pen() const { return mPen; }
	QBrush brush() const { return mBrush; }
	int size() const { return mSize; }

	// setters;
	void setPen(const QPen &pen) { mPen = pen; }
	void setBrush(const QBrush &brush) { mBrush = brush; }
	void setSize(int size) { mSize = size; }
	void setType(Type type) { mType = type; }

	QCPItemPosition *const center;

	double selectTest(const QPointF &pos, bool onlySelectable,
	                  QVariant *details = 0) const override;

	void setCoords(double x, double y) { center->setCoords(x, y); };
	void setCoords(const QPointF &pos) { center->setCoords(pos.x(), pos.y()); }
};
}

#endif /* _PLOT_MARKER_HPP_ */
