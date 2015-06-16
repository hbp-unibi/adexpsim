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

#ifndef _ADEXPSIM_EXPLORATION_WIDGET_HPP_
#define _ADEXPSIM_EXPLORATION_WIDGET_HPP_

#include <memory>
#include <set>

#include <QWidget>

#include <common/Types.hpp>

class QAction;
class QComboBox;
class QLabel;
class QProgressBar;
class QCustomPlot;
class QCPAxis;
class QStatusBar;
class QToolBar;
class QVBoxLayout;

namespace AdExpSim {

class ParameterCollection;
class Exploration;
class ExplorationWidgetCrosshair;
class ExplorationWidgetInvalidOverlay;

/**
 * The ExplorationWidget shows the exploration result as colored image.
 */
class ExplorationWidget : public QWidget {
	Q_OBJECT

private:
	QAction *actShowHWLimits;
	QAction *actZoomFit;
	QAction *actZoomCenter;
	QAction *actLockXAxis;
	QAction *actLockYAxis;
	QComboBox *comboDimX;
	QComboBox *comboDimY;
	QComboBox *comboFunction;
	QVBoxLayout *layout;
	QProgressBar *progressBar;
	QLabel *statusLabel;
	QLabel *lblDimX;
	QLabel *lblDimY;
	QLabel *lblPBinary;
	QLabel *lblPFalsePositive;
	QLabel *lblPFalseNegative;
	QLabel *lblPSoft;
	QCustomPlot *pltExploration;
	ExplorationWidgetCrosshair *crosshair;
	ExplorationWidgetInvalidOverlay *overlay;
	ExplorationWidgetInvalidOverlay *overlayHW;
	std::shared_ptr<ParameterCollection> params;
	std::shared_ptr<Exploration> exploration;

	void dimensionChanged();

	QPointF workingParametersToPlot(Val x, Val y);
	QPointF parametersToPlot(Val x, Val y);
	QPointF plotToWorkingParameters(Val x, Val y);
	QPointF plotToParameters(Val x, Val y);

	QString axisName(size_t dim, bool unit = false);

private slots:
	void dimensionChanged(QCPAxis *axis, size_t dim);
	void dimensionXChanged();
	void dimensionYChanged();
	void updateInfo(QMouseEvent *event = nullptr);
	void updateCrosshair();
	void updateInvalidRegionsOverlay();
	void plotDoubleClick(QMouseEvent *event);
	void handleRestrictZoom();

public:
	/**
	 * Constructor of the ExplorationWidget class.
	 */
	ExplorationWidget(std::shared_ptr<ParameterCollection> params,
	                  std::shared_ptr<Exploration> exploration,
	                  QToolBar *toolbar,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the ExplorationWidget class.
	 */
	~ExplorationWidget();

	/**
	 * Returns the current X-dimension index.
	 */
	size_t getDimX();

	/**
	 * Returns the current Y-dimension index.
	 */
	size_t getDimY();

	/**
	 * Draws the current exploration as PDF.
	 */
	void saveToPdf(const QString &filename);

public slots:
	/**
	 * Slot that can be connected to the corresponding event of the
	 * IncrementalExploration class.
	 */
	void progress(float p, bool show);

	/**
	 * Redraws the results based on the current exploration instance.
	 */
	void refresh();

	/**
	 * Centers the view according to the current parameters.
	 */
	void centerView();

	/**
	 * Fits the view according to the current exploration.
	 */
	void fitView();

	/**
	 * Called whenever the axis range changed, emits the updateRange event.
	 */
	void rangeChanged();

signals:
	void updateRange(size_t dimX, size_t dimY, Val minX, Val maxX, Val minY,
	                 Val maxY);

	void updateParameters(std::set<size_t> dims);
};
}

#endif /* _ADEXPSIM_EXPLORATION_WIDGET_HPP_ */
