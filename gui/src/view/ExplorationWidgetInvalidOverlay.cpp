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

#include <QImage>
#include <QPainter>

#include "Colors.hpp"
#include "ExplorationWidgetInvalidOverlay.hpp"

namespace AdExpSim {

static constexpr int SPACING = 30;
static constexpr float LINE_WIDTH = 1;

static void drawHatchLine(QPainter &painter, int startX, int startY, int c)
{
	constexpr float ALPHA = 45.0f;
	constexpr float CALPHA = cos(ALPHA / 180.0f * M_PI);
	constexpr float SALPHA = sin(ALPHA / 180.0f * M_PI);
	painter.drawLine(startX, startY, startX + CALPHA * c, startY + SALPHA * c);
}

void ExplorationWidgetInvalidOverlay::rebuildBuffer()
{
	// Fetch the axis dimensions, abort if nothing has changed or no data is
	// available
	auto axisRect = mParentPlot->axisRect();
	const int w = axisRect->width();
	const int h = axisRect->height();
	if ((w == buffer.width() && h == buffer.height()) || mask.getWidth() == 0 ||
	    mask.getHeight() == 0) {
		return;
	}

	// Calculate the length of the diagonal
	const int c = hypot(w, h);

	// Create the buffer image, fill it with transparence
	buffer = QImage(w, h, QImage::Format_ARGB32_Premultiplied);
	buffer.fill(QColor(255, 255, 255, 0));

	// Create a QPainter instance accessing the buffer
	QPainter painter(&buffer);

	// Draw the line pattern
	painter.setPen(mPen);
	for (int x = 0; x < w; x += SPACING) {
		drawHatchLine(painter, x, 0, c);
	}
	for (int y = SPACING; y < h; y += SPACING) {
		drawHatchLine(painter, 0, y, c);
	}

	// Erase the regions where the image is explicitly valid
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	const int mw = mask.getWidth();
	const int mh = mask.getHeight();
	for (int x = 0; x < mw; x++) {
		int x0 = x * w / mw;
		int x1 = (x + 1) * w / mw + 1;
		for (int y = 0; y < mh; y++) {
			int y0 = y * h / mh;
			int y1 = (y + 1) * h / mh + 1;
			if (mask(x, mh - 1 - y)) {
				painter.fillRect(QRectF(x0, y0, x1 - x0, y1 - y0),
				                 QColor(255, 255, 255, 0));
			}
		}
	}
}

void ExplorationWidgetInvalidOverlay::draw(QCPPainter *painter)
{
	// Rebuild the image buffer -- only performs work if necessary. Abort if the
	// buffer is currently invalid (because no valid mask was given)
	rebuildBuffer();
	if (buffer.isNull()) {
		return;
	}

	// Transform the given coordinates to pixel coordinates and draw the image
	double x0 = mParentPlot->xAxis->coordToPixel(rangeDimX.min);
	double x1 = mParentPlot->xAxis->coordToPixel(rangeDimX.max);
	double y0 = mParentPlot->yAxis->coordToPixel(rangeDimY.min);
	double y1 = mParentPlot->yAxis->coordToPixel(rangeDimY.max);
	painter->drawImage(QRectF(x0, y1, x1 - x0, y0 - y1), buffer);
}

ExplorationWidgetInvalidOverlay::ExplorationWidgetInvalidOverlay(
    QCustomPlot *parentPlot)
    : QCPAbstractItem(parentPlot),
      mPen(KS_GRAY, LINE_WIDTH),
      rangeDimX(0, 0, 0),
      rangeDimY(0, 0, 0),
      mask(0, 0)
{
}

ExplorationWidgetInvalidOverlay::~ExplorationWidgetInvalidOverlay() {}

double ExplorationWidgetInvalidOverlay::selectTest(const QPointF &pos,
                                                   bool onlySelectable,
                                                   QVariant *details) const
{
	return -1.0;
}

void ExplorationWidgetInvalidOverlay::setMask(Range rangeDimX, Range rangeDimY,
                                              const MatrixBase<bool> &mask)
{
	this->rangeDimX = rangeDimX;
	this->rangeDimY = rangeDimY;
	this->mask = mask;
	this->buffer = QImage();
}
}
