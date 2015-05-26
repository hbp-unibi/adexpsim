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

#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/Spike.hpp>
#include <model/NeuronSimulation.hpp>

#include "Colors.hpp"
#include "NeuronSimulationWidget.hpp"

namespace AdExpSim {

NeuronSimulationWidget::NeuronSimulationWidget(QWidget *parent)
    : QWidget(parent)
{
	// Create the layout widget
	layout = new QVBoxLayout(this);

	// Create the plot widgets and layers for each modality
	pltVolt = new QCustomPlot(this);
	pltVolt->addLayer("v");
	pltVolt->addLayer("spikes");

	pltCond = new QCustomPlot(this);
	pltCond->addLayer("gE");
	pltCond->addLayer("gI");
	pltCond->addLayer("spikes");

	pltCurr = new QCustomPlot(this);
	pltCurr->addLayer("w");
	pltCurr->addLayer("iL");
	pltCurr->addLayer("iE");
	pltCurr->addLayer("iI");
	pltCurr->addLayer("iTh");
	pltCurr->addLayer("iSum");
	pltCurr->addLayer("spikes");

	// Combine the widgets in the layout
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(pltVolt);
	layout->addWidget(pltCond);
	layout->addWidget(pltCurr);

	// Set the layout instance as the layout of this widget
	setLayout(layout);
}

static void addSpikes(QCustomPlot *plot, const SpikeVec &spikes)
{
	plot->setCurrentLayer("spikes");
	for (const auto &spike : spikes) {
		QCPItemStraightLine *line = new QCPItemStraightLine(plot);
		plot->addItem(line);

		Val t = SIPrefixTransformation::transformTime(spike.t.sec());
		line->positions()[0]->setType(QCPItemPosition::ptPlotCoords);
		line->positions()[0]->setCoords(t, 0);
		line->positions()[1]->setType(QCPItemPosition::ptPlotCoords);
		line->positions()[1]->setCoords(t, 1);
		line->setPen(QPen(KS_GRAY, 1, Qt::DashLine));
	}
}

void NeuronSimulationWidget::show(
    const std::vector<const NeuronSimulation *> &sims)
{
	// Clear the graphs
	pltVolt->clearGraphs();
	pltCond->clearGraphs();
	pltCurr->clearGraphs();
	pltVolt->clearItems();
	pltCond->clearItems();
	pltCurr->clearItems();

	// Min/max values
	Val minTime = std::numeric_limits<Val>::max();
	Val maxTime = std::numeric_limits<Val>::lowest();
	Val minVoltage = std::numeric_limits<Val>::max();
	Val maxVoltage = std::numeric_limits<Val>::lowest();
	Val minConductance = std::numeric_limits<Val>::max();
	Val maxConductance = std::numeric_limits<Val>::lowest();
	Val minCurrent = std::numeric_limits<Val>::max();
	Val maxCurrent = std::numeric_limits<Val>::lowest();

	// Iterate over all simulations
	for (ssize_t i = sims.size() - 1; i >= 0; i--) {
		const int lf = (i == 0) ? 100 : 150;  // Lightness factor (/100)
		const float lw = (i == 0) ? 1.5 : 1;  // Line width

		// Fetch the simulation and the data
		const NeuronSimulation &sim = *sims[i];
		const VectorRecorderData<QVector<double>> &data = sim.getData();

		// Adapt the min/max values
		minTime = std::min(minTime, data.minTime);
		maxTime = std::max(maxTime, data.maxTime);
		minVoltage = std::min(minVoltage, data.minVoltage);
		maxVoltage = std::max(maxVoltage, data.maxVoltage);
		minConductance = std::min(minConductance, data.minConductance);
		maxConductance = std::max(maxConductance, data.maxConductance);
		minCurrent = std::min(minCurrent, data.minCurrentSmooth);
		maxCurrent = std::max(maxCurrent, data.maxCurrentSmooth);

		// Create the voltage graph
		pltVolt->setCurrentLayer("v");
		pltVolt->addGraph();
		pltVolt->graph()->setData(data.ts, data.v);
		pltVolt->graph()->setPen(QPen(COLOR_V.lighter(lf), lw));

		addSpikes(pltVolt, sim.getSpikes());

		// Create the conductance graph
		pltCond->setCurrentLayer("gE");
		pltCond->addGraph();
		pltCond->graph()->setData(data.ts, data.gE);
		pltCond->graph()->setPen(QPen(COLOR_GE.lighter(lf), lw));

		pltCond->setCurrentLayer("gI");
		pltCond->addGraph();
		pltCond->graph()->setData(data.ts, data.gI);
		pltCond->graph()->setPen(QPen(COLOR_GI.lighter(lf), lw));

		addSpikes(pltCond, sim.getSpikes());

		// Create the current graph
		pltCurr->setCurrentLayer("w");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.w);
		pltCurr->graph()->setPen(QPen(COLOR_W.lighter(lf), lw));

		pltCurr->setCurrentLayer("iL");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iL);
		pltCurr->graph()->setPen(QPen(COLOR_IL.lighter(lf), lw));

		pltCurr->setCurrentLayer("iE");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iE);
		pltCurr->graph()->setPen(QPen(COLOR_IE.lighter(lf), lw));

		pltCurr->setCurrentLayer("iI");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iI);
		pltCurr->graph()->setPen(QPen(COLOR_II.lighter(lf), lw));

		pltCurr->setCurrentLayer("iTh");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iTh);
		pltCurr->graph()->setPen(QPen(COLOR_ITH.lighter(lf), lw));

		pltCurr->setCurrentLayer("iSum");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iSum);
		pltCurr->graph()->setPen(
		    QPen(COLOR_ISUM.lighter(lf), lw, Qt::DashLine));

		addSpikes(pltCurr, sim.getSpikes());
	}

	// Set titles and ranges
	pltVolt->xAxis->setLabel("Time [ms]");
	pltVolt->yAxis->setLabel("Membrane Potential [mV]");
	pltVolt->xAxis->setRange(minTime, maxTime);
	pltVolt->yAxis->setRange(minVoltage, maxVoltage);

	pltCond->xAxis->setLabel("Time t [ms]");
	pltCond->yAxis->setLabel("Conductance [µS]");
	pltCond->xAxis->setRange(minTime, maxTime);
	pltCond->yAxis->setRange(minConductance, maxConductance);

	pltCurr->xAxis->setLabel("Time t [ms]");
	pltCurr->yAxis->setLabel("Current [nA]");
	pltCurr->xAxis->setRange(minTime, maxTime);
	pltCurr->yAxis->setRange(minCurrent, maxCurrent);

	// Replot everything
	pltVolt->replot();
	pltCond->replot();
	pltCurr->replot();
}
}

