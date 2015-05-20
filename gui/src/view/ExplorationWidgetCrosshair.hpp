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

#ifndef _EXPLORATION_WIDGET_CROSSHAIR_HPP_
#define _EXPLORATION_WIDGET_CROSSHAIR_HPP_

#include <qcustomplot.h>

namespace AdExpSim {
class ExplorationWidgetCrosshair : public QCPAbstractItem {
	Q_OBJECT
public:
	ExplorationWidgetCrosshair(QCustomPlot *parentPlot);
	virtual ~ExplorationWidgetCrosshair();

	// getters:
	QPen pen() const { return mPen; }

	// setters;
	void setPen(const QPen &pen);

	QCPItemPosition *const center;

	double selectTest(const QPointF &pos, bool onlySelectable,
	                  QVariant *details = 0) const override;

protected:
	QPen mPen;
	void draw(QCPPainter *painter) override;
};
}

#endif /* _EXPLORATION_WIDGET_CROSSHAIR_HPP_ */
