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

#include <QMouseEvent>

#include <iostream>

#include "Colors.hpp"
#include "SpikeWidget.hpp"

namespace AdExpSim {

void SpikeWidget::paintEvent(QPaintEvent *event)
{
	// Fetch the widget geometry and calculate some regions
	const int h = height();
	const int w = width();
	const int inY0 = 0;
	const int inY1 = (h - 1) / 2 - 1;
	const int sep1 = (h - 1) / 2;
	const int outY0 = (h - 1) / 2 + 1;
	const int outY1 = (h - 1);
	const float fx = w / train.getMaxT().sec();

	// Create a QPainter instance
	QPainter painter(this);

	// Draw the groups
	for (auto const &group : groups) {
		// Calculate the x start-/end-coordinates
		const int x0 = group.start.sec() * fx;
		const int x1 = group.end.sec() * fx;

		// Draw the "ok/not ok" background
		const QColor c = (group.ok ? COLOR_LIGHT_GREEN : COLOR_LIGHT_RED);
		painter.fillRect(QRect(x0 + 1, inY0, x1, outY1), c);

		// Draw the group boundary
		painter.setPen(QPen(KS_GRAY, 1, Qt::DashLine));
		painter.drawLine(x0, inY0, x0, outY1);
	}

	// Draw the input spikes
	auto const &inputSpikes = train.getSpikes();
	for (auto const &spike : inputSpikes) {
		// Calculate the x-coordinate
		const int x = spike.t.sec() * fx;

		// Draw the "ok/not ok" background
		const QColor c = (spike.w > 0) ? COLOR_IE : COLOR_II;
		painter.setPen(QPen(c, 1));
		painter.drawLine(x, inY0, x, inY1);
	}

	// Draw the output spikes
	for (auto const &spike : spikes) {
		// Calculate the x-coordinate
		const int x = spike.t.sec() * fx;

		// Draw the "ok/not ok" background
		const QColor c = spike.ok ? KS_GREEN : KS_RED;
		painter.setPen(QPen(c, 1));
		painter.drawLine(x, outY0, x, outY1);
	}

	// Draw the lines separating input from output and labels
	painter.setPen(KS_GRAY);
	painter.drawLine(0, sep1, w - 1, sep1);

	// Highlight the rectangle that is currently being plotted by darkening the
	// surroundings
	const int x0 = rangeStart.sec() * fx;
	const int x1 = rangeEnd.sec() * fx;
	painter.fillRect(QRect(0, 0, x0, h), QColor(128, 128, 128, 128));
	painter.fillRect(QRect(x1, 0, w, h), QColor(128, 128, 128, 128));
}

float SpikeWidget::timeToPixel(Time t) const
{
	return t.sec() * width() / train.getMaxT().sec();
}

Time SpikeWidget::pixelToTime(float px) const
{
	return Time::sec(px * train.getMaxT().sec() / float(width()));
}

std::pair<int, int> SpikeWidget::pixelRange() const
{
	return std::pair<int, int>(timeToPixel(rangeStart), timeToPixel(rangeEnd));
}

bool SpikeWidget::inPixelRange(int x) const
{
	auto r = pixelRange();
	return x >= r.first && x <= r.second;
}

void SpikeWidget::updateRange(Time start, Time len)
{
	// Fetch the new range length (if len is smaller than zero, use current len)
	const Time maxT = train.getMaxT();
	Time newRangeLen = len < Time(0) ? (rangeEnd - rangeStart) : len;

	// Set the new start position, make sure it is positive
	Time newRangeStart = start;
	if (newRangeStart < Time(0)) {
		newRangeStart = Time(0);
	}

	// Make sure the end of the range is smaller than the max time
	Time newRangeEnd = newRangeStart + newRangeLen;
	if (newRangeEnd > maxT) {
		newRangeStart = maxT - newRangeLen;
		if (newRangeStart < Time(0)) {
			newRangeStart = Time(0);
			newRangeLen = maxT;
		}
		newRangeEnd = newRangeStart + newRangeLen;
	}

	// Emit update events in case anything changed
	if (rangeStart != newRangeStart || rangeEnd != newRangeEnd) {
		rangeStart = newRangeStart;
		rangeEnd = newRangeEnd;
		update();
		emit rangeChange();
	}
}

void SpikeWidget::mouseMoveEvent(QMouseEvent *event)
{
	int cx = event->pos().x();
	if (dragging) {
		setCursor(Qt::ClosedHandCursor);
		updateRange(dragStartRangeStart + pixelToTime(cx - dragStartX));
	} else {
		setCursor(inPixelRange(cx) ? Qt::OpenHandCursor : Qt::ArrowCursor);
	}
}

void SpikeWidget::mousePressEvent(QMouseEvent *event)
{
	int cx = event->pos().x();
	if (event->button() == Qt::LeftButton) {
		dragStartX = cx;
		dragStartRangeStart = rangeStart;
		if (inPixelRange(cx)) {
			dragging = true;
		}
	}
	mouseMoveEvent(event);
}

void SpikeWidget::mouseReleaseEvent(QMouseEvent *event)
{
	int cx = event->pos().x();
	if (event->button() == Qt::LeftButton && dragStartX == cx) {
		updateRange(pixelToTime(cx) - (rangeEnd - rangeStart) * 0.5);
	}
	dragging = false;
	mouseMoveEvent(event);
}

static constexpr Time MIN_LEN = 0.01_s;
static constexpr Time MAX_LEN = 1.0_s;

void SpikeWidget::wheelEvent(QWheelEvent *event)
{
	// Calculate the current range center
	const int cx = event->pos().x();
	const Time tCenter = pixelToTime(cx);

	// Scale the length
	const float f = std::max(0.0, 1.0 - event->angleDelta().y() / 360.0);
	const Time len =
	    std::max(MIN_LEN, std::min(MAX_LEN, (rangeEnd - rangeStart) * f));
	updateRange(tCenter - len * 0.5, len);

	// Reset dragStartX and dragStartRangeStart if dragging
	if (dragging) {
		dragStartX = cx;
		dragStartRangeStart = rangeStart;
	}

	event->accept();
}

SpikeWidget::SpikeWidget(QWidget *parent)
    : QWidget(parent),
      rangeStart(0),
      rangeEnd(0.1_s),
      dragging(false),
      dragStartX(0),
      dragStartRangeStart(0)

{
	setMouseTracking(true);
	setMinimumHeight(61);
	setMaximumHeight(61);
	setMinimumWidth(100);
}

void SpikeWidget::show(const SpikeTrain &train, const OutputSpikeVec &spikes,
                       const OutputGroupVec &groups)
{
	this->train = train;
	this->spikes = spikes;
	this->groups = groups;
	updateRange(rangeStart);
	update();
}
}

