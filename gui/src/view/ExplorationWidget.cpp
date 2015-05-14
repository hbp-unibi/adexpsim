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

#include <QComboBox>
#include <QProgressBar>
#include <QToolBar>
#include <QVBoxLayout>

#include <iostream>

#include <exploration/Exploration.hpp>

#include "ExplorationWidget.hpp"

namespace AdExpSim {

constexpr double CostScale = 0.1;

static void fillDimensionCombobox(QComboBox *box)
{
	box->addItem("λL");
	box->addItem("λE");
	box->addItem("eE");
	box->addItem("eTh");
	box->addItem("ΔTh");
	box->addItem("w");

	box->setItemData(0, "Membrane Leak Frequency", Qt::ToolTipRole);
	box->setItemData(1, "Excitatory Decay Frequency", Qt::ToolTipRole);
	box->setItemData(2, "Excitatory Reversal Potential", Qt::ToolTipRole);
	box->setItemData(3, "Spike Threshold Potential", Qt::ToolTipRole);
	box->setItemData(4, "Spike Slope", Qt::ToolTipRole);
	box->setItemData(5, "Synapse Weight", Qt::ToolTipRole);
}

ExplorationWidget::ExplorationWidget(QWidget *parent)
{
	// Create the layout widget
	layout = new QVBoxLayout(this);

	// Create the toolbar widget and its children
	toolbar = new QToolBar(this);
	comboDimX = new QComboBox(toolbar);
	comboDimY = new QComboBox(toolbar);
	comboFunction = new QComboBox(toolbar);
	comboFunction->addItem("Cost 𝓒");
	comboFunction->addItem("Max. potential (ξ)");
	comboFunction->addItem("Max. potential (ξ - 1)");
	comboFunction->addItem("Spike Time (ξ)");
	comboFunction->addItem("Reset Time (ξ)");
	comboFunction->addItem("Reset Time (ξ - 1)");
	progressBar = new QProgressBar(toolbar);
	progressBar->setMinimum(0);
	progressBar->setMaximum(1000);
	fillDimensionCombobox(comboDimX);
	fillDimensionCombobox(comboDimY);
	toolbar->addWidget(new QLabel("X:"));
	toolbar->addWidget(comboDimX);
	toolbar->addWidget(new QLabel("Y:"));
	toolbar->addWidget(comboDimY);
	toolbar->addWidget(new QLabel("Function:"));
	toolbar->addWidget(comboFunction);
	toolbar->addWidget(progressBar);

	// Add the plot widget
	pltExploration = new QCustomPlot(this);
	pltExploration->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	pltExploration->axisRect()->setupFullAxesBox(true);

	// Connect the axis change events to allow updating the view once the axes
	// change
	connect(pltExploration->xAxis, SIGNAL(rangeChanged(const QCPRange &)), this,
	        SLOT(axisChange(const QCPRange &)));
	connect(pltExploration->yAxis, SIGNAL(rangeChanged(const QCPRange &)), this,
	        SLOT(axisChange(const QCPRange &)));

	// Add the stops to the gradients
	gradCost.setColorInterpolation(QCPColorGradient::ColorInterpolation::ciRGB);
	gradCost.setColorStops({{0.0, QColor(255, 255, 255)},
	                        {0.125, QColor(255, 247, 251)},
	                        {0.25, QColor(236, 231, 242)},
	                        {0.375, QColor(208, 209, 230)},
	                        {0.5, QColor(166, 189, 219)},
	                        {0.625, QColor(116, 169, 207)},
	                        {0.75, QColor(54, 144, 192)},
	                        {0.875, QColor(5, 112, 176)},
	                        {1.0, QColor(3, 78, 123)}});

	// Combine the widgets in the layout
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(toolbar);
	layout->addWidget(pltExploration);

	// Set the layout instance as the layout of this widget
	setLayout(layout);
}

void ExplorationWidget::axisChange(const QCPRange &newRange)
{
	// Fetch the range
	const Val minX = pltExploration->xAxis->range().lower;
	const Val maxX = pltExploration->xAxis->range().upper;
	const Val minY = pltExploration->yAxis->range().lower;
	const Val maxY = pltExploration->yAxis->range().upper;

	// Emit the update event
	//emit updateRange(0, 1, minX, maxX, minY, maxY);
}

void ExplorationWidget::progress(float p, bool show)
{
	if (show) {
		progressBar->show();
		progressBar->setValue(p * 1000.0);
	} else {
		progressBar->hide();
	}
}

void ExplorationWidget::show(const Exploration &exploration, bool fit)
{
	// Clear the graph
	pltExploration->clearPlottables();

	// Fetch the X and Y range
	const Range &rX = exploration.getRangeX();
	const Range &rY = exploration.getRangeY();

	// Create a "plottable" from the
	QCPColorMap *map =
	    new QCPColorMap(pltExploration->xAxis, pltExploration->yAxis);
	pltExploration->addPlottable(map);
	map->data()->setSize(rX.steps, rY.steps);
	map->data()->setRange(QCPRange(rX.min, rX.max), QCPRange(rY.min, rY.max));

	// Fill the plot data
	const ExplorationMemory &mem = exploration.getMemory();
	for (size_t x = 0; x < rX.steps; x++) {
		for (size_t y = 0; y < rY.steps; y++) {
			const double cost = mem(x, y).cost() * CostScale;
			map->data()->setCell(x, y, cost);
		}
	}

	// Set the gradient and plot it
	map->setGradient(gradCost);
	map->setDataRange(QCPRange(0, 1));
	if (fit) {
		pltExploration->rescaleAxes();
	}
	pltExploration->replot();
}
}

