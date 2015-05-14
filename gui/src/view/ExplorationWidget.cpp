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
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include <exploration/Exploration.hpp>

#include "ExplorationWidget.hpp"

namespace AdExpSim {

constexpr double COST_SCALE = 0.1;

static void fillDimensionCombobox(QComboBox *box)
{
	box->addItem("λL", 0);
	box->addItem("λE", 1);
	box->addItem("eE", 4);
	box->addItem("eTh", 6);
	box->addItem("ΔTh", 9);
	box->addItem("w", 12);

	box->setItemData(0, "Membrane Leak Frequency", Qt::ToolTipRole);
	box->setItemData(1, "Excitatory Decay Frequency", Qt::ToolTipRole);
	box->setItemData(2, "Excitatory Reversal Potential", Qt::ToolTipRole);
	box->setItemData(3, "Spike Threshold Potential", Qt::ToolTipRole);
	box->setItemData(4, "Spike Slope", Qt::ToolTipRole);
	box->setItemData(5, "Synapse Weight", Qt::ToolTipRole);
}

ExplorationWidget::ExplorationWidget(QWidget *parent)
    : currentExploration(nullptr)
{
	// Create the layout widget
	layout = new QVBoxLayout(this);

	// Create the toolbar widget and its children
	toolbar = new QToolBar(this);
	comboDimX = new QComboBox(toolbar);
	comboDimY = new QComboBox(toolbar);
	comboFunction = new QComboBox(toolbar);
	comboFunction->addItem("Cost C", "cost");
	comboFunction->addItem("Max. potential (ξ)", "eMaxXi");
	comboFunction->addItem("Max. potential (ξ - 1)", "eMaxXiM1");
	comboFunction->addItem("Spike Time (ξ)", "tSpike");
	comboFunction->addItem("Reset Time (ξ)", "tResetXi");
	comboFunction->addItem("Reset Time (ξ - 1)", "tResetXiM1");

	fillDimensionCombobox(comboDimX);
	comboDimX->setCurrentIndex(0);

	fillDimensionCombobox(comboDimY);
	comboDimY->setCurrentIndex(5);

	connect(comboFunction, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(update()));
	connect(comboDimX, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(rangeChanged()));
	connect(comboDimY, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(rangeChanged()));

	toolbar->addWidget(new QLabel("X:"));
	toolbar->addWidget(comboDimX);
	toolbar->addWidget(new QLabel("Y:"));
	toolbar->addWidget(comboDimY);
	toolbar->addWidget(new QLabel("Function:"));
	toolbar->addWidget(comboFunction);

	// Add the plot widget
	pltExploration = new QCustomPlot(this);
	pltExploration->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	pltExploration->axisRect()->setupFullAxesBox(true);
	pltExploration->moveLayer(pltExploration->layer("grid"),
	                          pltExploration->layer("main"));

	// Connect the axis change events to allow updating the view once the axes
	// change
	connect(pltExploration->xAxis, SIGNAL(rangeChanged(const QCPRange &)), this,
	        SLOT(rangeChanged()));
	connect(pltExploration->yAxis, SIGNAL(rangeChanged(const QCPRange &)), this,
	        SLOT(rangeChanged()));

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

	// Create the status bar
	QWidget *statusWidget = new QWidget(this);
	statusWidget->setMaximumHeight(35);
	statusLabel = new QLabel(statusWidget);
	statusLabel->setMinimumWidth(50);
	progressBar = new QProgressBar(statusWidget);
	progressBar->setMinimum(0);
	progressBar->setMaximum(1000);
	progressBar->setTextVisible(false);
	progressBar->setMaximumHeight(10);

	QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
	statusLayout->addWidget(statusLabel);
	statusLayout->addWidget(progressBar);

	// Combine the widgets in the layout
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(toolbar);
	layout->addWidget(pltExploration);
	layout->addWidget(statusWidget);

	// Set the layout instance as the layout of this widget
	setLayout(layout);
}

ExplorationWidget::~ExplorationWidget()
{
	// Only needed for unique_ptr
}

void ExplorationWidget::rangeChanged()
{
	// Fetch the range
	const Val minX = pltExploration->xAxis->range().lower;
	const Val maxX = pltExploration->xAxis->range().upper;
	const Val minY = pltExploration->yAxis->range().lower;
	const Val maxY = pltExploration->yAxis->range().upper;

	// Fetch the current dimensions
	const size_t dimX = comboDimX->itemData(comboDimX->currentIndex()).toInt();
	const size_t dimY = comboDimY->itemData(comboDimY->currentIndex()).toInt();

	// Emit the update event
	emit updateRange(dimX, dimY, minX, maxX, minY, maxY);
}

void ExplorationWidget::progress(float p, bool show)
{
	if (show) {
		std::stringstream ss;
		ss << std::setprecision(5) << std::setw(4) << ceil(p * 1000) / 10
		   << "%";
		statusLabel->setText(QString::fromStdString(ss.str()));
		progressBar->setValue(p * 1000);
		progressBar->show();
	} else {
		if (p == 0.0) {
			statusLabel->setText("Wait...");
		} else {
			statusLabel->setText("Ready.");
		}
		progressBar->hide();
	}
}

void ExplorationWidget::update()
{
	// Clear the graph
	pltExploration->clearPlottables();

	// Fetch the X and Y range
	const Range &rX = currentExploration->getRangeX();
	const Range &rY = currentExploration->getRangeY();

	// Create a "plottable" for the data
	QCPColorMap *map =
	    new QCPColorMap(pltExploration->xAxis, pltExploration->yAxis);
	pltExploration->addPlottable(map);
	map->data()->setSize(rX.steps, rY.steps);
	map->data()->setRange(QCPRange(rX.min, rX.max), QCPRange(rY.min, rY.max));

	// Fill the plot data
	const QString funStr =
	    comboFunction->itemData(comboFunction->currentIndex()).toString();
	Val (*fun)(const EvaluationResult &) =
	    [](const EvaluationResult &res) -> Val { return log(res.cost()); };
	if (funStr == "eMaxXi") {
		fun = [](const EvaluationResult &res) -> Val { return res.eMaxXi; };
	} else if (funStr == "eMaxXiM1") {
		fun = [](const EvaluationResult &res) -> Val { return res.eMaxXiM1; };
	} else if (funStr == "tSpike") {
		fun = [](const EvaluationResult &res) -> Val { return res.tSpike; };
	}

	Val minF = std::numeric_limits<Val>::max();
	Val maxF = std::numeric_limits<Val>::lowest();
	const ExplorationMemory &mem = currentExploration->getMemory();
	for (size_t x = 0; x < rX.steps; x++) {
		for (size_t y = 0; y < rY.steps; y++) {
			const EvaluationResult res = mem(x, y);
			const Val f = fun(res);
			if (res.valid()) {
				minF = std::min(minF, f);
				maxF = std::max(maxF, f);
			}
			map->data()->setCell(x, y, f);
		}
	}

	// Set the gradient and plot it
	map->setGradient(gradCost);
	map->setDataRange(QCPRange(minF, maxF));

	// Replot the graph
	pltExploration->replot();
}

void ExplorationWidget::show(const Exploration &exploration, bool fit)
{
	// Clone the given exploration instance
	currentExploration =
	    std::unique_ptr<Exploration>(new Exploration(exploration.clone()));
	update();

	// Rescale the axes according to the exploration
	if (fit) {
		pltExploration->rescaleAxes();
		pltExploration->replot();
	}
}
}

