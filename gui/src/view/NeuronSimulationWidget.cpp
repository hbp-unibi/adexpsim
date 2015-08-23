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
#include "SpikeWidget.hpp"
#include "PlotMarker.hpp"

namespace AdExpSim {

NeuronSimulationWidget::NeuronSimulationWidget(QWidget *parent)
    : QWidget(parent)
{
	// Create the layout widget
	layout = new QVBoxLayout(this);

	// Create the spike widget and the update delay timer
	spikeWidget = new SpikeWidget(this);
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);

	// Create the plot widgets and layers for each modality
	pltVolt = new QCustomPlot(this);
	pltVolt->addLayer("v");
	pltVolt->addLayer("spikes");
	pltVolt->addLayer("limits");
	pltVolt->addLayer("maxima");

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
	layout->addWidget(spikeWidget);
	layout->addWidget(pltVolt);
	layout->addWidget(pltCond);
	layout->addWidget(pltCurr);

	// Set the layout instance as the layout of this widget
	setLayout(layout);

	// Connect the spikeWidget rangeChange event to the update event
	connect(spikeWidget, SIGNAL(rangeChange()), this, SLOT(rangeChange()));
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePlot()));
}

static void addSpikes(QCustomPlot *plot, const SpikeVec &spikes, Val minT,
                      Val maxT)
{
	plot->setCurrentLayer("spikes");
	for (const auto &spike : spikes) {
		Val t = SIPrefixTrafo::transformTime(spike.t.sec());
		if (t >= minT && t <= maxT) {
			QCPItemStraightLine *line = new QCPItemStraightLine(plot);
			plot->addItem(line);

			line->positions()[0]->setType(QCPItemPosition::ptPlotCoords);
			line->positions()[0]->setCoords(t, 0);
			line->positions()[1]->setType(QCPItemPosition::ptPlotCoords);
			line->positions()[1]->setCoords(t, 1);
			line->setPen(QPen(KS_GRAY, 1, Qt::DashLine));
		}
	}
}

static void addHorzLine(QCustomPlot *plot, Val y)
{
	QCPItemStraightLine *line = new QCPItemStraightLine(plot);
	plot->addItem(line);
	line->positions()[0]->setType(QCPItemPosition::ptPlotCoords);
	line->positions()[0]->setCoords(0, y);
	line->positions()[1]->setType(QCPItemPosition::ptPlotCoords);
	line->positions()[1]->setCoords(1, y);
	line->setPen(QPen(Qt::black, 1, Qt::DotLine));
}

static void addMaxima(QCustomPlot *plot, const Parameters &p,
                      const NeuronSimulation::MaximaData &maxima, Val minT,
                      Val maxT)
{
	plot->setCurrentLayer("maxima");
	for (const auto &max : maxima) {
		Val t = SIPrefixTrafo::transformTime(max.t.sec());
		if (t >= minT && t <= maxT) {
			Val v = SIPrefixTrafo::transformVoltage(max.s.v() + p.eL());
			PlotMarker *marker = new PlotMarker(plot, 5);
			plot->addItem(marker);
			marker->setCoords(t, v);
		}
	}
}

void NeuronSimulationWidget::rangeChange() { updateTimer->start(100); }

void NeuronSimulationWidget::updatePlot()
{
	const float LINE_W = 1.5;  // Line width

	// Clear the graphs
	pltVolt->clearGraphs();
	pltCond->clearGraphs();
	pltCurr->clearGraphs();
	pltVolt->clearItems();
	pltCond->clearItems();
	pltCurr->clearItems();

	// Fetch the current data slice
	double minT =
	    SIPrefixTrafo::transformTime(spikeWidget->getRangeStart().sec());
	double maxT =
	    SIPrefixTrafo::transformTime(spikeWidget->getRangeEnd().sec());
	const VectorRecorderData<QVector<double>> &oData = sim.getValues();
	const VectorRecorderData<QVector<double>> &data = oData.slice(minT, maxT);

	// Abort if the simulation is not valid
	if (sim.valid()) {
		// Create the voltage graph
		pltVolt->setCurrentLayer("v");
		pltVolt->addGraph();
		pltVolt->graph()->setData(data.ts, data.v);
		pltVolt->graph()->setPen(QPen(COLOR_V, LINE_W));

		pltVolt->setCurrentLayer("limits");
		const ParameterCollection &p = sim.getParameters();
		WorkingParameters wp(p.params);
		addHorzLine(pltVolt, SIPrefixTrafo::transformVoltage(p.params.eE()));
		addHorzLine(pltVolt, SIPrefixTrafo::transformVoltage(p.params.eI()));
		if (p.model == ModelType::AD_IF_COND_EXP) {
			addHorzLine(pltVolt, SIPrefixTrafo::transformVoltage(
			                         wp.eSpikeEff() + p.params.eL()));
		}
		addHorzLine(pltVolt, SIPrefixTrafo::transformVoltage(p.params.eTh()));
		addHorzLine(pltVolt, SIPrefixTrafo::transformVoltage(p.params.eL()));
		addHorzLine(pltVolt,
		            SIPrefixTrafo::transformVoltage(p.params.eReset()));

		addSpikes(pltVolt, sim.getInputSpikes(), minT, maxT);
		addMaxima(pltVolt, p.params, sim.getMaxima(), minT, maxT);

		// Create the conductance graph
		pltCond->setCurrentLayer("gE");
		pltCond->addGraph();
		pltCond->graph()->setData(data.ts, data.gE);
		pltCond->graph()->setPen(QPen(COLOR_GE, LINE_W));

		pltCond->setCurrentLayer("gI");
		pltCond->addGraph();
		pltCond->graph()->setData(data.ts, data.gI);
		pltCond->graph()->setPen(QPen(COLOR_GI, LINE_W));

		addSpikes(pltCond, sim.getInputSpikes(), minT, maxT);

		// Create the current graph
		pltCurr->setCurrentLayer("w");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.w);
		pltCurr->graph()->setPen(QPen(COLOR_W, LINE_W));

		pltCurr->setCurrentLayer("iL");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iL);
		pltCurr->graph()->setPen(QPen(COLOR_IL, LINE_W));

		pltCurr->setCurrentLayer("iE");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iE);
		pltCurr->graph()->setPen(QPen(COLOR_IE, LINE_W));

		pltCurr->setCurrentLayer("iI");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iI);
		pltCurr->graph()->setPen(QPen(COLOR_II, LINE_W));

		pltCurr->setCurrentLayer("iTh");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iTh);
		pltCurr->graph()->setPen(QPen(COLOR_ITH, LINE_W));

		pltCurr->setCurrentLayer("iSum");
		pltCurr->addGraph();
		pltCurr->graph()->setData(data.ts, data.iSum);
		pltCurr->graph()->setPen(QPen(COLOR_ISUM, LINE_W, Qt::DashLine));

		addSpikes(pltCurr, sim.getInputSpikes(), minT, maxT);
	}

	// Set titles and ranges
	pltVolt->xAxis->setLabel("Time [ms]");
	pltVolt->yAxis->setLabel("Membrane Potential [mV]");
	pltVolt->xAxis->setRange(minT, maxT);
	pltVolt->yAxis->setRange(oData.minVoltage, oData.maxVoltage);

	pltCond->xAxis->setLabel("Time t [ms]");
	pltCond->yAxis->setLabel("Conductance [µS]");
	pltCond->xAxis->setRange(minT, maxT);
	pltCond->yAxis->setRange(oData.minConductance, oData.maxConductance);

	pltCurr->xAxis->setLabel("Time t [ms]");
	pltCurr->yAxis->setLabel("Current [nA]");
	pltCurr->xAxis->setRange(minT, maxT);
	pltCurr->yAxis->setRange(oData.minCurrentSmooth, oData.maxCurrentSmooth);

	// Replot everything
	pltVolt->replot();
	pltCond->replot();
	pltCurr->replot();
}

void NeuronSimulationWidget::show(const NeuronSimulation &sim)
{
	// Copy the simulation
	this->sim = sim;

	// Update the SpikeWidget (this recalculates the range)
	spikeWidget->show(sim.getTrain(), sim.getOutputSpikes(),
	                  sim.getOutputGroups());

	// Force an update of the plots
	updatePlot();
}
}

