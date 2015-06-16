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

#ifndef _ADEXPSIM_EXPLORATION_WIDGET_INVALID_OVERLAY_HPP_
#define _ADEXPSIM_EXPLORATION_WIDGET_INVALID_OVERLAY_HPP_

#include <QImage>
#include <qcustomplot.h>

#include <common/Types.hpp>
#include <common/Matrix.hpp>

namespace AdExpSim {
/**
 * The ExplorationWidgetInvalidOverlay class is used to draw the lines which
 * indicate the regions in an exploration with invalid parameters.
 */
class ExplorationWidgetInvalidOverlay : public QCPAbstractItem {
	Q_OBJECT

private:
	QPen mPen;
	Range rangeDimX, rangeDimY;
	MatrixBase<bool> mask;
	QImage buffer;

	void rebuildBuffer();

protected:
	void draw(QCPPainter *painter) override;

public:
	ExplorationWidgetInvalidOverlay(QCustomPlot *parentPlot);
	virtual ~ExplorationWidgetInvalidOverlay();

	// getters:
	QPen pen() const { return mPen; }

	// setters;
	void setPen(const QPen &pen) {mPen = pen;}

	double selectTest(const QPointF &pos, bool onlySelectable,
	                  QVariant *details = 0) const override;

	void setMask(Range rangeDimX, Range rangeDimY,
	             const MatrixBase<bool> &mask);
};
}

#endif /* _ADEXPSIM_EXPLORATION_WIDGET_INVALID_OVERLAY_HPP_ */
