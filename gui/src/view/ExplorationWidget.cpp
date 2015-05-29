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

#include <QAction>
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
#include <simulation/Parameters.hpp>
#include <utils/Matrix.hpp>

#include "ExplorationWidget.hpp"
#include "ExplorationWidgetCrosshair.hpp"
#include "ExplorationWidgetGradients.hpp"
#include "ExplorationWidgetInvalidOverlay.hpp"

namespace AdExpSim {

constexpr double COST_SCALE = 0.1;

static void fillDimensionCombobox(QComboBox *box)
{
	for (int i = 0; i < 13; i++) {
		if (WorkingParameters::linear[i]) {
			box->addItem(
			    QString::fromStdString(WorkingParameters::originalNames[i]), i);
			box->setItemData(i, QString::fromStdString(
			                        WorkingParameters::originalDescriptions[i]),
			                 Qt::ToolTipRole);
		} else {
			box->addItem(QString::fromStdString(WorkingParameters::names[i]),
			             i);
			box->setItemData(
			    i, QString::fromStdString(WorkingParameters::descriptions[i]),
			    Qt::ToolTipRole);
		}
	}
}

ExplorationWidget::ExplorationWidget(QWidget *parent,
                                     std::shared_ptr<Parameters> params)
    : exploration(nullptr), params(params)
{
	// Create the layout widget
	layout = new QVBoxLayout(this);

	// Create the toolbar widget and its children
	toolbar = new QToolBar(this);
	comboDimX = new QComboBox(toolbar);
	comboDimY = new QComboBox(toolbar);
	comboFunction = new QComboBox(toolbar);
	comboFunction->addItem("Soft Success Probability", "pSoft");
	comboFunction->addItem("Binary False Positive Probability",
	                       "pFalsePositive");
	comboFunction->addItem("Binary False Negative Probability",
	                       "pFalseNegative");
	comboFunction->addItem("Binary Success Probability", "pBinary");

	fillDimensionCombobox(comboDimX);
	comboDimX->setCurrentIndex(0);

	fillDimensionCombobox(comboDimY);
	comboDimY->setCurrentIndex(1);

	connect(comboFunction, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(update()));
	connect(comboDimX, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(dimensionXChanged()));
	connect(comboDimY, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(dimensionYChanged()));

	QAction *centerViewAct = new QAction(QIcon::fromTheme("zoom-fit-best"),
	                                     tr("&Center View"), this);
	connect(centerViewAct, SIGNAL(triggered()), this, SLOT(centerView()));

	toolbar->addAction(centerViewAct);
	toolbar->addSeparator();
	toolbar->addWidget(new QLabel("X: "));
	toolbar->addWidget(comboDimX);
	toolbar->addWidget(new QLabel(" Y: "));
	toolbar->addWidget(comboDimY);
	toolbar->addSeparator();
	toolbar->addWidget(new QLabel("Function: "));
	toolbar->addWidget(comboFunction);

	// Add the plot widget
	pltExploration = new QCustomPlot(this);
	pltExploration->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	pltExploration->axisRect()->setupFullAxesBox(true);
	pltExploration->moveLayer(pltExploration->layer("grid"),
	                          pltExploration->layer("main"));

	// Add the crosshair and the "invalid overlay"
	overlay = new ExplorationWidgetInvalidOverlay(pltExploration);
	pltExploration->addItem(overlay);
	pltExploration->addLayer("overlay");
	overlay->setLayer("overlay");

	crosshair = new ExplorationWidgetCrosshair(pltExploration);
	pltExploration->addItem(crosshair);
	pltExploration->addLayer("crosshair");
	crosshair->setLayer("crosshair");

	// Connect the axis change events to allow updating the view once the axes
	// change
	connect(pltExploration->xAxis, SIGNAL(rangeChanged(const QCPRange &)), this,
	        SLOT(rangeChanged()));
	connect(pltExploration->yAxis, SIGNAL(rangeChanged(const QCPRange &)), this,
	        SLOT(rangeChanged()));
	connect(pltExploration, SIGNAL(mouseMove(QMouseEvent *)), this,
	        SLOT(updateInfo(QMouseEvent *)));
	connect(pltExploration, SIGNAL(mouseDoubleClick(QMouseEvent *)), this,
	        SLOT(plotDoubleClick(QMouseEvent *)));

	// Create the status bar
	QWidget *statusWidget = new QWidget(this);
	statusWidget->setMaximumHeight(35);

	lblDimX = new QLabel(statusWidget);
	lblDimX->setMinimumWidth(100);
	lblDimY = new QLabel(statusWidget);
	lblDimY->setMinimumWidth(100);
	lblPBinary = new QLabel(statusWidget);
	lblPBinary->setMinimumWidth(75);
	lblPFalsePositive = new QLabel(statusWidget);
	lblPFalsePositive->setMinimumWidth(75);
	lblPFalseNegative = new QLabel(statusWidget);
	lblPFalseNegative->setMinimumWidth(75);
	lblPSoft = new QLabel(statusWidget);
	lblPSoft->setMinimumWidth(75);

	statusLabel = new QLabel(statusWidget);
	statusLabel->setMinimumWidth(50);
	progressBar = new QProgressBar(statusWidget);
	progressBar->setMinimum(0);
	progressBar->setMaximum(1000);
	progressBar->setTextVisible(false);
	progressBar->setMaximumHeight(10);
	progressBar->setMaximumWidth(200);

	QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
	statusLayout->addWidget(lblDimX);
	statusLayout->addWidget(lblDimY);
	statusLayout->addWidget(lblPBinary);
	statusLayout->addWidget(lblPSoft);
	statusLayout->addWidget(lblPFalsePositive);
	statusLayout->addWidget(lblPFalseNegative);
	statusLayout->addWidget(progressBar);
	statusLayout->addWidget(statusLabel);

	// Combine the widgets in the layout
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(toolbar);
	layout->addWidget(pltExploration);
	layout->addWidget(statusWidget);

	// Set the layout instance as the layout of this widget
	setLayout(layout);

	// Update the status bar information
	updateInfo();
}

ExplorationWidget::~ExplorationWidget()
{
	// Only needed for unique_ptr
}

size_t ExplorationWidget::getDimX()
{
	return comboDimX->itemData(comboDimX->currentIndex()).toInt();
}
size_t ExplorationWidget::getDimY()
{
	return comboDimY->itemData(comboDimY->currentIndex()).toInt();
}

/*
 * Coordinate transformation functions
 */

QPointF ExplorationWidget::workingParametersToPlot(Val x, Val y)
{
	size_t dimX = getDimX();
	if (WorkingParameters::linear[dimX]) {
		x = WorkingParameters::toParameter(x, dimX, *params);
	}

	size_t dimY = getDimY();
	if (WorkingParameters::linear[dimY]) {
		y = WorkingParameters::toParameter(y, dimY, *params);
	}

	return QPointF(x, y);
}

QPointF ExplorationWidget::parametersToPlot(Val x, Val y)
{
	size_t dimX = getDimX();
	if (!WorkingParameters::linear[dimX]) {
		x = WorkingParameters::fromParameter(x, dimX, *params);
	}

	size_t dimY = getDimY();
	if (!WorkingParameters::linear[dimY]) {
		y = WorkingParameters::fromParameter(y, dimY, *params);
	}

	return QPointF(x, y);
}

QPointF ExplorationWidget::plotToWorkingParameters(Val x, Val y)
{
	size_t dimX = getDimX();
	if (WorkingParameters::linear[dimX]) {
		x = WorkingParameters::fromParameter(x, dimX, *params);
	}

	size_t dimY = getDimY();
	if (WorkingParameters::linear[dimY]) {
		y = WorkingParameters::fromParameter(y, dimY, *params);
	}

	return QPointF(x, y);
}

QPointF ExplorationWidget::plotToParameters(Val x, Val y)
{
	size_t dimX = getDimX();
	if (!WorkingParameters::linear[dimX]) {
		x = WorkingParameters::toParameter(x, dimX, *params);
	}

	size_t dimY = getDimY();
	if (!WorkingParameters::linear[dimY]) {
		y = WorkingParameters::toParameter(y, dimY, *params);
	}

	return QPointF(x, y);
}

QString ExplorationWidget::axisName(size_t dim, bool unit)
{
	if (WorkingParameters::linear[dim]) {
		return QString::fromStdString(
		    WorkingParameters::originalNames[dim] +
		    (unit ? " [" + WorkingParameters::originalUnits[dim] + "]" : ""));
	}
	return QString::fromStdString(
	    WorkingParameters::names[dim] +
	    (unit ? " [" + WorkingParameters::units[dim] + "]" : ""));
}

/*
 * Signal handlers
 */

void ExplorationWidget::rangeChanged()
{
	QPointF min = plotToWorkingParameters(pltExploration->xAxis->range().lower,
	                                      pltExploration->yAxis->range().lower);
	QPointF max = plotToWorkingParameters(pltExploration->xAxis->range().upper,
	                                      pltExploration->yAxis->range().upper);
	emit updateRange(getDimX(), getDimY(), min.x(), max.x(), min.y(), max.y());
}

void ExplorationWidget::dimensionChanged(QCPAxis *axis, size_t dim)
{
	// Center the axis around the value
	Val v = WorkingParameters::fetchParameter(dim, *params);
	if (!WorkingParameters::linear[dim]) {
		v = WorkingParameters::fromParameter(v, dim, *params);
	}
	if (v == 0) {
		axis->setRange(QCPRange(-0.1, 0.1));
	} else {
		axis->setRange(QCPRange(v * 0.5, v * 1.5));
	}

	// Invalidate the exploration instance
	exploration = nullptr;

	// Notify about the range change and replot
	rangeChanged();
	update();
}

void ExplorationWidget::dimensionXChanged()
{
	dimensionChanged(pltExploration->xAxis,
	                 comboDimX->itemData(comboDimX->currentIndex()).toInt());
}

void ExplorationWidget::dimensionYChanged()
{
	dimensionChanged(pltExploration->yAxis,
	                 comboDimY->itemData(comboDimY->currentIndex()).toInt());
}

void ExplorationWidget::updateInfo(QMouseEvent *event)
{
	// If event is set to nullptr, just update the label captions
	lblPBinary->setText("");
	lblPSoft->setText("");
	lblPFalsePositive->setText("");
	lblPFalseNegative->setText("");
	if (!event) {
		lblDimX->setText(axisName(getDimX()));
		lblDimY->setText(axisName(getDimY()));
	} else {
		// Fetch the raw x/y position
		Val x = pltExploration->xAxis->pixelToCoord(event->localPos().x());
		Val y = pltExploration->yAxis->pixelToCoord(event->localPos().y());

		// Print it
		lblDimX->setText(axisName(getDimX()) + ": " + QString::number(x));
		lblDimY->setText(axisName(getDimY()) + ": " + QString::number(y));

		// Convert it to working parameters and check whether a corresponding
		// value can be found in the current exploration
		QPointF p = plotToWorkingParameters(x, y);
		if (exploration != nullptr) {
			Range rX = exploration->getRangeX();
			Range rY = exploration->getRangeY();
			int iX = floor(rX.index(p.x()));
			int iY = floor(rY.index(p.y()));
			if (iX >= 0 && iX < (int)rX.steps && iY >= 0 &&
			    iY < (int)rY.steps) {
				SpikeTrainEvaluationResult res =
				    exploration->getMemory()(iX, iY);
				lblPBinary->setText("pBin: " +
				                    QString::number(res.pBinary, 'f', 3));
				lblPFalsePositive->setText(
				    "pFPos: " + QString::number(res.pFalsePositive, 'f', 3));
				lblPFalseNegative->setText(
				    "pFNeg: " + QString::number(res.pFalseNegative, 'f', 3));
				lblPSoft->setText("pSoft: " +
				                  QString::number(res.pSoft, 'f', 3));
			}
		}
	}
}

void ExplorationWidget::plotDoubleClick(QMouseEvent *event)
{
	// Transform the coordinates to working parameters
	Val x = pltExploration->xAxis->pixelToCoord(event->localPos().x());
	Val y = pltExploration->yAxis->pixelToCoord(event->localPos().y());
	QPointF p = plotToWorkingParameters(x, y);

	// Create working parameters from the current parameters
	WorkingParameters wp(*params);

	// Update the corrsponding dimensions and check whether the working
	// parameters are still valid -- if yes, update the parameters and emit the
	// updateParameters event
	wp[getDimX()] = p.x();
	wp[getDimY()] = p.y();
	if (wp.valid()) {
		// Update the parameters and emit the corrsponding event
		QPointF wpp = plotToParameters(x, y);
		WorkingParameters::fetchParameter(getDimX(), *params) = wpp.x();
		WorkingParameters::fetchParameter(getDimY(), *params) = wpp.y();
		emit updateParameters();

		// Move the parameter crosshair to a new position and replot
		updateCrosshair();
		pltExploration->replot();
	}
}

void ExplorationWidget::centerView()
{
	QPointF p =
	    parametersToPlot(WorkingParameters::fetchParameter(getDimX(), *params),
	                     WorkingParameters::fetchParameter(getDimY(), *params));
	pltExploration->xAxis->setRange(QCPRange(p.x() * 0.5, p.x() * 1.5));
	pltExploration->yAxis->setRange(QCPRange(p.y() * 0.5, p.y() * 1.5));
	pltExploration->replot();
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

/*
 * Main draw function
 */

template <typename Fun>
static void fillColorMap(QCPColorMap *map, size_t nx, size_t ny, Fun f)
{
	for (size_t x = 0; x < nx; x++) {
		for (size_t y = 0; y < ny; y++) {
			map->data()->setCell(x, y, f(x, y));
		}
	}
}

void ExplorationWidget::updateCrosshair()
{
	crosshair->positions()[0]->setType(QCPItemPosition::ptPlotCoords);
	crosshair->positions()[0]->setCoords(parametersToPlot(
	    WorkingParameters::fetchParameter(getDimX(), *params),
	    WorkingParameters::fetchParameter(getDimY(), *params)));
}

void ExplorationWidget::updateInvalidRegionsOverlay()
{
	constexpr size_t RES = 256;

	if (exploration != nullptr) {
		// Calculate the validity mask
		MatrixBase<bool> mask(RES, RES);
		const Range &rX = exploration->getRangeX();
		const Range &rY = exploration->getRangeY();
		const Range rEX(rX.min, rX.max, RES);
		const Range rEY(rY.min, rY.max, RES);
		const size_t dimX = getDimX();
		const size_t dimY = getDimY();
		WorkingParameters wp(*params);
		for (size_t x = 0; x < RES; x++) {
			for (size_t y = 0; y < RES; y++) {
				wp[dimX] = rEX.value(x);
				wp[dimY] = rEY.value(y);
				mask(x, y) = wp.valid();
			}
		}

		// Update the overlay
		QPointF min = workingParametersToPlot(rX.min, rY.min);
		QPointF max = workingParametersToPlot(rX.max, rY.max);
		overlay->setMask(Range(min.x(), max.x(), RES),
		                 Range(min.y(), max.y(), RES), mask);
	} else {
		overlay->setMask(Range(0, 0, 0), Range(0, 0, 0),
		                 MatrixBase<bool>(0, 0));
	}
}

void ExplorationWidget::update()
{
	// Clear the graph
	pltExploration->setCurrentLayer("main");
	pltExploration->clearPlottables();

	// Update the x- and y- axis labels
	pltExploration->xAxis->setLabel(axisName(getDimX(), true));
	pltExploration->yAxis->setLabel(axisName(getDimY(), true));

	// Plot the exploration data
	if (exploration != nullptr) {
		// Fetch the X and Y range
		Range rX = exploration->getRangeX();
		Range rY = exploration->getRangeY();

		// Transform the range
		QPointF min = workingParametersToPlot(rX.min, rY.min);
		QPointF max = workingParametersToPlot(rX.max, rY.max);

		// Create a "plottable" for the data
		QCPColorMap *map =
		    new QCPColorMap(pltExploration->xAxis, pltExploration->yAxis);
		pltExploration->addPlottable(map);
		map->data()->setSize(rX.steps, rY.steps);
		map->data()->setRange(QCPRange(min.x(), max.x()),
		                      QCPRange(min.y(), max.y()));

		// Fill the plot data
		const QString funStr =
		    comboFunction->itemData(comboFunction->currentIndex()).toString();
		const ExplorationMemory &mem = exploration->getMemory();
		if (funStr == "pSoft") {
			fillColorMap(map, rX.steps, rY.steps, [&mem](size_t x, size_t y) {
				return mem.pSoft(x, y);
			});
			map->setGradient(ExplorationWidgetGradients::blue());
		} else if (funStr == "pBinary") {
			fillColorMap(map, rX.steps, rY.steps, [&mem](size_t x, size_t y) {
				return mem.pBinary(x, y);
			});
			map->setGradient(ExplorationWidgetGradients::orange());
		} else if (funStr == "pFalsePositive") {
			fillColorMap(map, rX.steps, rY.steps, [&mem](size_t x, size_t y) {
				return mem.pFalsePositive(x, y);
			});
			map->setGradient(ExplorationWidgetGradients::green());
		} else if (funStr == "pFalseNegative") {
			fillColorMap(map, rX.steps, rY.steps, [&mem](size_t x, size_t y) {
				return mem.pFalseNegative(x, y);
			});
			map->setGradient(ExplorationWidgetGradients::green());
		}
		map->setDataRange(QCPRange(0, 1));
	}

	updateInvalidRegionsOverlay();
	updateCrosshair();

	// Replot the graph
	pltExploration->replot();
}

void ExplorationWidget::show(const Exploration &exploration, bool fit)
{
	// Clone the given exploration instance
	this->exploration =
	    std::unique_ptr<Exploration>(new Exploration(exploration.clone()));
	update();

	// Rescale the axes according to the exploration
	if (fit) {
		pltExploration->rescaleAxes();
		pltExploration->replot();
	}
}
}

