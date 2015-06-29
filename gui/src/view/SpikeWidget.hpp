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
 * @file SpikeWidget.hpp
 *
 * Used to display an overview of the input and output spikes, allows scrolling
 * and zooming the visible range.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SPIKE_WIDGET_HPP_
#define _ADEXPSIM_SPIKE_WIDGET_HPP_

#include <vector>

#include <QPainter>
#include <QWidget>

#include <exploration/SpikeTrainEvaluation.hpp>

namespace AdExpSim {

/**
 * The SpikeWidget class visualizes the results of a SpikeTrainEvaluation.
 */
class SpikeWidget : public QWidget {
	Q_OBJECT

public:
	using OutputSpikeVec = std::vector<SpikeTrainEvaluation::OutputSpike>;
	using OutputGroupVec = std::vector<SpikeTrainEvaluation::OutputGroup>;

private:
	/**
	 * Copy of the current spike train instance.
	 */
	SpikeTrain train;

	/**
	 * Copy of the spikes recorded by the SpikeTrainEvaluation class.
	 */
	OutputSpikeVec spikes;

	/**
	 * Copy of the groups produced by the SpikeTrainEvaluation class.
	 */
	OutputGroupVec groups;

	/**
	 * Start of the current range.
	 */
	Time rangeStart;

	/**
	 * End of the current range.
	 */
	Time rangeEnd;

	/**
	 * Set to true if we're currently dragging the range.
	 */
	bool dragging;

	/**
	 * Contains the pixel position at which the dragging started.
	 */
	int dragStartX;
	Time dragStartRangeStart;

	/**
	 * Returns the pixel corresponding to the given time point.
	 */
	float timeToPixel(Time t) const;

	/**
	 * Returns the time corresponding to the given pixel.
	 */
	Time pixelToTime(float px) const;

	/**
	 * Returns the currently selected range in pixel coordinates.
	 */
	std::pair<int, int> pixelRange() const;

	/**
	 * Returns true if the given x coordinate is within the range.
	 */
	bool inPixelRange(int x) const;

	/**
	 * Updates the current range while keeping all boundaries.
	 */
	void updateRange(Time start, Time len = Time(-1));

protected:
	void paintEvent(QPaintEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

public:
	SpikeWidget(QWidget *parent);

	void show(const SpikeTrain &train, const OutputSpikeVec &spikes,
	          const OutputGroupVec &groups);

	Time getRangeStart() const { return rangeStart; }
	Time getRangeEnd() const { return rangeEnd; }

signals:
	void rangeChange();
};
}

#endif /* _ADEXPSIM_SPIKE_WIDGET_HPP_ */

