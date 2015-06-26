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

#include <common/Matrix.hpp>
#include <exploration/EvaluationResult.hpp>
#include <exploration/Exploration.hpp>
#include <utils/ParameterCollection.hpp>
#include <simulation/HardwareParameters.hpp>

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

ExplorationWidget::ExplorationWidget(
    std::shared_ptr<ParameterCollection> params,
    std::shared_ptr<Exploration> exploration, QToolBar *toolbar,
    QWidget *parent)
    : params(params), exploration(exploration)
{
	// Create the layout widget
	layout = new QVBoxLayout(this);

	// Create the toolbar widget and its children
	comboDimX = new QComboBox(toolbar);
	comboDimY = new QComboBox(toolbar);
	comboFunction = new QComboBox(toolbar);
	comboFunction->addItem("Soft Success", 0);
	comboFunction->addItem("Binary Success", 1);
	comboFunction->addItem("False Positive", 2);
	comboFunction->addItem("False Negative", 3);

	fillDimensionCombobox(comboDimX);
	comboDimX->setCurrentIndex(0);

	fillDimensionCombobox(comboDimY);
	comboDimY->setCurrentIndex(1);

	connect(comboFunction, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(refresh()));
	connect(comboDimX, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(dimensionXChanged()));
	connect(comboDimY, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(dimensionYChanged()));

	// Create the actions
	actShowHWLimits = new QAction(QIcon("data/icon_hw.png"), "HW Limits", this);
	actShowHWLimits->setToolTip("Show the hardware limits");
	actShowHWLimits->setCheckable(true);
	actShowHWLimits->setChecked(false);
	actZoomFit =
	    new QAction(QIcon::fromTheme("zoom-original"), "Fit View", this);
	actZoomFit->setToolTip(
	    "Fits the view to the range of the current exploration");
	actZoomCenter =
	    new QAction(QIcon::fromTheme("zoom-fit-best"), "Center View", this);
	actZoomCenter->setToolTip(
	    "Centers the view according to the current parameters");
	actLockXAxis =
	    new QAction(QIcon::fromTheme("object-flip-horizontal"), "Zoom X", this);
	actLockXAxis->setCheckable(true);
	actLockXAxis->setChecked(true);
	actLockXAxis->setToolTip("Allow zoom in X direction");
	actLockYAxis =
	    new QAction(QIcon::fromTheme("object-flip-vertical"), "Zoom Y", this);
	actLockYAxis->setCheckable(true);
	actLockYAxis->setChecked(true);
	actLockYAxis->setToolTip("Allow zoom in Y direction");

	// Connect the action events to the corresponding slots
	connect(actShowHWLimits, SIGNAL(triggered()), this, SLOT(refresh()));
	connect(actZoomFit, SIGNAL(triggered()), this, SLOT(fitView()));
	connect(actZoomCenter, SIGNAL(triggered()), this, SLOT(centerView()));
	connect(actLockXAxis, SIGNAL(triggered()), this,
	        SLOT(handleRestrictZoom()));
	connect(actLockYAxis, SIGNAL(triggered()), this,
	        SLOT(handleRestrictZoom()));

	toolbar->addAction(actShowHWLimits);
	toolbar->addSeparator();
	toolbar->addAction(actZoomFit);
	toolbar->addAction(actZoomCenter);
	toolbar->addAction(actLockXAxis);
	toolbar->addAction(actLockYAxis);
	toolbar->addSeparator();
	toolbar->addWidget(new QLabel("X: "));
	toolbar->addWidget(comboDimX);
	toolbar->addWidget(new QLabel(" Y: "));
	toolbar->addWidget(comboDimY);
	toolbar->addSeparator();
	toolbar->addWidget(comboFunction);

	// Add the plot widget
	pltExploration = new QCustomPlot(this);
	pltExploration->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
	pltExploration->axisRect()->setupFullAxesBox(true);
	pltExploration->moveLayer(pltExploration->layer("grid"),
	                          pltExploration->layer("main"));

	// Add the crosshair and the "invalid overlay"
	overlay = new ExplorationWidgetInvalidOverlay(pltExploration);
	overlayHW = new ExplorationWidgetInvalidOverlay(pltExploration);
	overlayHW->setPen(QPen(QColor(200, 75, 25), 1));
	pltExploration->addItem(overlay);
	pltExploration->addItem(overlayHW);

	pltExploration->addLayer("overlay");
	overlay->setLayer("overlay");
	overlayHW->setLayer("overlay");

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

EvaluationResultDimension ExplorationWidget::getDimZ()
{
	return EvaluationResultDimension(
	    comboFunction->itemData(comboFunction->currentIndex()).toInt());
}

void ExplorationWidget::saveToPdf(const QString &filename)
{
	overlayHW->setPen(QPen(QColor(200, 75, 25), 2));
	pltExploration->savePdf(filename);
	overlayHW->setPen(QPen(QColor(200, 75, 25), 1));
}

/*
 * Coordinate transformation functions
 */

QPointF ExplorationWidget::workingParametersToPlot(Val x, Val y)
{
	return QPointF(
	    WorkingParameters::workingToPlot(x, getDimX(), params->params),
	    WorkingParameters::workingToPlot(y, getDimY(), params->params));
}

QPointF ExplorationWidget::parametersToPlot(Val x, Val y)
{
	return QPointF(
	    WorkingParameters::parameterToPlot(x, getDimX(), params->params),
	    WorkingParameters::parameterToPlot(y, getDimY(), params->params));
}

QPointF ExplorationWidget::plotToWorkingParameters(Val x, Val y)
{
	return QPointF(
	    WorkingParameters::plotToWorking(x, getDimX(), params->params),
	    WorkingParameters::plotToWorking(y, getDimY(), params->params));
}

QPointF ExplorationWidget::plotToParameters(Val x, Val y)
{
	return QPointF(
	    WorkingParameters::plotToParameter(x, getDimX(), params->params),
	    WorkingParameters::plotToParameter(y, getDimY(), params->params));
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
	Val v = params->params[dim];
	if (!WorkingParameters::linear[dim]) {
		v = WorkingParameters::fromParameter(v, dim, params->params);
	}
	if (v == 0) {
		axis->setRange(QCPRange(-0.1, 0.1));
	} else {
		axis->setRange(QCPRange(v * 0.5, v * 1.5));
	}

	// Invalidate the exploration instance
	*exploration = Exploration();

	// Notify about the range change and replot
	rangeChanged();
	refresh();
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
		if (exploration->valid()) {
			Range rX = exploration->getRangeX();
			Range rY = exploration->getRangeY();
			int iX = floor(rX.index(p.x()));
			int iY = floor(rY.index(p.y()));
			if (iX >= 0 && iX < (int)rX.steps && iY >= 0 &&
			    iY < (int)rY.steps) {
				EvaluationResult res = exploration->getMemory()(iX, iY);
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
	WorkingParameters wp(params->params);

	// Update the corrsponding dimensions and check whether the working
	// parameters are still valid -- if yes, update the parameters and emit the
	// updateParameters event
	wp[getDimX()] = p.x();
	wp[getDimY()] = p.y();
	if (wp.valid()) {
		// Update the parameters and emit the corrsponding event
		QPointF wpp = plotToParameters(x, y);
		params->params[getDimX()] = wpp.x();
		params->params[getDimY()] = wpp.y();
		emit updateParameters({getDimX(), getDimY()});

		// Move the parameter crosshair to a new position and replot
		updateCrosshair();
		pltExploration->replot();
	}
}

void ExplorationWidget::handleRestrictZoom()
{
	pltExploration->axisRect()->setRangeZoomFactor(
	    actLockXAxis->isChecked() ? 0.85 : 0.0,
	    actLockYAxis->isChecked() ? 0.85 : 0.0);
}

void ExplorationWidget::centerView()
{
	QPointF p =
	    parametersToPlot(params->params[getDimX()], params->params[getDimY()]);
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
	crosshair->positions()[0]->setCoords(
	    parametersToPlot(params->params[getDimX()], params->params[getDimY()]));
}

void ExplorationWidget::updateInvalidRegionsOverlay()
{
	constexpr size_t RES = 256;

	if (exploration->valid()) {
		// Calculate the validity mask
		const bool showHWOverlay = actShowHWLimits->isChecked();
		const size_t HW_RES = showHWOverlay ? RES : 0;
		MatrixBase<bool> mask(RES, RES);
		MatrixBase<bool> maskHW(HW_RES, HW_RES);
		const Range &rX = exploration->getRangeX();
		const Range &rY = exploration->getRangeY();
		const Range rEX(rX.min, rX.max, RES);
		const Range rEY(rY.min, rY.max, RES);
		const size_t dimX = getDimX();
		const size_t dimY = getDimY();
		WorkingParameters wp(params->params);
		for (size_t x = 0; x < RES; x++) {
			for (size_t y = 0; y < RES; y++) {
				wp[dimX] = rEX.value(x);
				wp[dimY] = rEY.value(y);
				mask(x, y) = wp.valid();
				if (showHWOverlay) {
					maskHW(x, y) =
					    BrainScaleSParameters::inst
					        .map(wp, params->model == ModelType::IF_COND_EXP,
					             true)
					        .size() > 0;
				}
			}
		}

		// Update the overlay
		QPointF min = workingParametersToPlot(rX.min, rY.min);
		QPointF max = workingParametersToPlot(rX.max, rY.max);
		overlay->setMask(Range(min.x(), max.x(), RES),
		                 Range(min.y(), max.y(), RES), mask);
		overlayHW->setMask(Range(min.x(), max.x(), HW_RES),
		                   Range(min.y(), max.y(), HW_RES), maskHW);
	} else {
		overlay->setMask(Range(0, 0, 0), Range(0, 0, 0),
		                 MatrixBase<bool>(0, 0));
		overlayHW->setMask(Range(0, 0, 0), Range(0, 0, 0),
		                   MatrixBase<bool>(0, 0));
	}
}

void ExplorationWidget::refresh()
{
	// Clear the graph
	pltExploration->setCurrentLayer("main");
	pltExploration->clearPlottables();

	// Update the x- and y- axis labels
	pltExploration->xAxis->setLabel(axisName(getDimX(), true));
	pltExploration->yAxis->setLabel(axisName(getDimY(), true));

	// Plot the exploration data
	if (exploration->valid()) {
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
		const ExplorationMemory &mem = exploration->getMemory();
		switch (getDimZ()) {
			case EvaluationResultDimension::SOFT:
				fillColorMap(
				    map, rX.steps, rY.steps,
				    [&mem](size_t x, size_t y) { return mem.pSoft(x, y); });
				map->setGradient(ExplorationWidgetGradients::blue());
				break;
			case EvaluationResultDimension::BINARY:
				fillColorMap(
				    map, rX.steps, rY.steps,
				    [&mem](size_t x, size_t y) { return mem.pBinary(x, y); });
				map->setGradient(ExplorationWidgetGradients::orange());
				break;
			case EvaluationResultDimension::FALSE_POSITIVE:
				fillColorMap(map, rX.steps, rY.steps,
				             [&mem](size_t x, size_t y) {
					return mem.pFalsePositive(x, y);
				});
				map->setGradient(ExplorationWidgetGradients::green());
				break;
			case EvaluationResultDimension::FALSE_NEGATIVE:
				fillColorMap(map, rX.steps, rY.steps,
				             [&mem](size_t x, size_t y) {
					return mem.pFalseNegative(x, y);
				});
				map->setGradient(ExplorationWidgetGradients::green());
				break;
		}
		map->setDataRange(QCPRange(0, 1));
	}

	updateInvalidRegionsOverlay();
	updateCrosshair();

	// Replot the graph
	pltExploration->replot();
}

void ExplorationWidget::fitView()
{
	pltExploration->rescaleAxes();
	pltExploration->replot();
}
}

