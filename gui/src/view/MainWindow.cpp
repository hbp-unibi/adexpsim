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

#include <limits>

#include <QVector>
#include <QVBoxLayout>

#include <qcustomplot.h>

#include <core/Model.hpp>
#include <core/Recorder.hpp>
#include <core/Spike.hpp>

#include "MainWindow.hpp"

namespace AdExpSim {

namespace {
/**
 * The Transformation class is used to rescale the recorded data to more
 * convenient ranges used for displaying.
 */
struct Transformation {
	static constexpr Val TIME_SCALE = 1000.0;
	static constexpr Val VOLTAGE_SCALE = 1000.0;
	static constexpr Val CONDUCTANCE_SCALE = 1000.0 * 1000.0;
	static constexpr Val CURRENT_SCALE = 1000.0 * 1000.0 * 1000.0;

	Val minTime = std::numeric_limits<Val>::max();
	Val maxTime = std::numeric_limits<Val>::lowest();
	Val minVoltage = std::numeric_limits<Val>::max();
	Val maxVoltage = std::numeric_limits<Val>::lowest();
	Val minConductance = std::numeric_limits<Val>::max();
	Val maxConductance = std::numeric_limits<Val>::lowest();
	Val minCurrent = std::numeric_limits<Val>::max();
	Val maxCurrent = std::numeric_limits<Val>::lowest();

	/**
	 * Adjusts the min/max value while automatically rejecting outliers.
	 */
	static void smoothMinMax(Val &min, Val &max, Val x, Val slope = 50)
	{
		if (x < min &&
		    (x > min - slope || min == std::numeric_limits<Val>::max())) {
			min = x;
		}
		if (x > max &&
		    (x < max + slope || max == std::numeric_limits<Val>::lowest())) {
			max = x;
		}
	}

	Val transformTs(Val ts)
	{
		ts *= TIME_SCALE;
		smoothMinMax(minTime, maxTime, ts);
		return ts;
	}

	Val transformV(Val v)
	{
		v *= VOLTAGE_SCALE;
		smoothMinMax(minVoltage, maxVoltage, v, 100);
		return v;
	}

	Val transformGE(Val gE)
	{
		gE *= CONDUCTANCE_SCALE;
		smoothMinMax(minConductance, maxConductance, gE);
		return gE;
	}

	Val transformGI(Val gI)
	{
		gI *= CONDUCTANCE_SCALE;
		smoothMinMax(minConductance, maxConductance, gI);
		return gI;
	}

	Val transformW(Val w)
	{
		w *= CURRENT_SCALE;
		smoothMinMax(minCurrent, maxCurrent, w);
		return w;
	}

	Val transformIL(Val iL)
	{
		iL *= CURRENT_SCALE;
		smoothMinMax(minCurrent, maxCurrent, iL);
		return iL;
	}

	Val transformIE(Val iE)
	{
		iE *= CURRENT_SCALE;
		smoothMinMax(minCurrent, maxCurrent, iE);
		return iE;
	}

	Val transformII(Val iI)
	{
		iI *= CURRENT_SCALE;
		smoothMinMax(minCurrent, maxCurrent, iI);
		return iI;
	}

	Val transformITh(Val iTh)
	{
		iTh *= CURRENT_SCALE;
		smoothMinMax(minCurrent, maxCurrent, iTh, 10);
		return iTh;
	}
};
}

MainWindow::MainWindow()
{
	const QColor ORANGE(245, 159, 0);
	const QColor BLUE(0, 98, 154);
	const QColor GREEN(0, 117, 86);
	const QColor LIGHT_GREEN(182, 200, 40);
	const QColor RED(121, 16, 17);
	const QColor PRUNE(127, 13, 93);
	const QColor GRAY(191, 191, 191);

	// Run the simulation
	SpikeVec spikes = buildInputSpikes(3, 2e-3, 0, 0.04e-6);
	Parameters params;
	VectorRecorder<QVector<double>, Transformation> recorder(params, 0.1e-3);
	MaxValueController controller;
	Model::simulate<Model::FAST_EXP>(spikes, recorder, controller, params,
	                                 0.01e-3);

	// Add three plot widgets
	QVBoxLayout *layout = new QVBoxLayout();

	QCustomPlot *pltVolt = new QCustomPlot();
	QCustomPlot *pltCond = new QCustomPlot();
	QCustomPlot *pltCurr = new QCustomPlot();

	layout->setSpacing(0);
	layout->addWidget(pltVolt);
	layout->addWidget(pltCond);
	layout->addWidget(pltCurr);

	// Create the voltage graph
	pltVolt->addGraph();
	pltVolt->graph(0)->setData(recorder.getData().ts, recorder.getData().v);
	pltVolt->graph(0)->setPen(QPen(Qt::black, 1.5));
	pltVolt->xAxis->setLabel("Time [ms]");
	pltVolt->yAxis->setLabel("Membrane Potential [mV]");
	pltVolt->xAxis->setRange(recorder.getTrafo().minTime,
	                         recorder.getTrafo().maxTime);
	pltVolt->yAxis->setRange(recorder.getTrafo().minVoltage,
	                         recorder.getTrafo().maxVoltage);

	// Create the conductance graph
	pltCond->addGraph();
	pltCond->graph(0)->setData(recorder.getData().ts, recorder.getData().gE);
	pltCond->graph(0)->setPen(QPen(BLUE, 1.5));
	pltCond->addGraph();
	pltCond->graph(1)->setData(recorder.getData().ts, recorder.getData().gI);
	pltCond->graph(1)->setPen(QPen(ORANGE, 1.5));
	pltCond->xAxis->setLabel("Time t [ms]");
	pltCond->yAxis->setLabel("Conductance [µS]");
	pltCond->xAxis->setRange(recorder.getTrafo().minTime,
	                         recorder.getTrafo().maxTime);
	pltCond->yAxis->setRange(recorder.getTrafo().minConductance,
	                         recorder.getTrafo().maxConductance);

	// Create the current graph
	pltCurr->addGraph();
	pltCurr->graph(0)->setData(recorder.getData().ts, recorder.getData().w);
	pltCurr->graph(0)->setPen(QPen(RED, 1.5));
	pltCurr->addGraph();
	pltCurr->graph(1)->setData(recorder.getData().ts, recorder.getData().iL);
	pltCurr->graph(1)->setPen(QPen(LIGHT_GREEN, 1.5));
	pltCurr->addGraph();
	pltCurr->graph(2)->setData(recorder.getData().ts, recorder.getData().iE);
	pltCurr->graph(2)->setPen(QPen(BLUE, 1.5));
	pltCurr->addGraph();
	pltCurr->graph(3)->setData(recorder.getData().ts, recorder.getData().iI);
	pltCurr->graph(3)->setPen(QPen(ORANGE, 1.5));
	pltCurr->addGraph();
	pltCurr->graph(4)->setData(recorder.getData().ts, recorder.getData().iTh);
	pltCurr->graph(4)->setPen(QPen(PRUNE, 1.5));
	pltCurr->addGraph();
	pltCurr->graph(5)->setData(recorder.getData().ts, recorder.getData().iSum);
	pltCurr->graph(5)->setPen(QPen(Qt::black, 1.5, Qt::DashLine));
	pltCurr->xAxis->setLabel("Time t [ms]");
	pltCurr->yAxis->setLabel("Current [nA]");
	pltCurr->xAxis->setRange(recorder.getTrafo().minTime,
	                         recorder.getTrafo().maxTime);
	pltCurr->yAxis->setRange(recorder.getTrafo().minCurrent,
	                         recorder.getTrafo().maxCurrent);

	pltVolt->replot();
	pltCond->replot();
	pltCurr->replot();

	// Add the widget containing the VBox-Layout
	QWidget *widget = new QWidget();
	widget->setLayout(layout);
	setCentralWidget(widget);
}
}

