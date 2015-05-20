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

#include <QWidget>

class QComboBox;
class QLabel;
class QProgressBar;
class QCustomPlot;
class QCPAxis;
class QStatusBar;
class QToolBar;
class QVBoxLayout;

namespace AdExpSim {

class Parameters;
class Exploration;

/**
 * The ExplorationWidget shows the exploration result as colored image.
 */
class ExplorationWidget : public QWidget {
	Q_OBJECT

private:
	QToolBar *toolbar;
	QComboBox *comboDimX;
	QComboBox *comboDimY;
	QComboBox *comboFunction;
	QVBoxLayout *layout;
	QProgressBar *progressBar;
	QLabel *statusLabel;
	QLabel *lblDimX;
	QLabel *lblDimY;
	QLabel *lblPBinary;
	QLabel *lblPSoft;
	QCustomPlot *pltExploration;
	std::unique_ptr<Exploration> exploration;
	std::shared_ptr<Parameters> params;

	void dimensionChanged();

	size_t getDimX();
	size_t getDimY();

	QPointF workingParametersToPlot(Val x, Val y);
	QPointF parametersToPlot(Val x, Val y);
	QPointF plotToWorkingParameters(Val x, Val y);
	QPointF plotToParameters(Val x, Val y);

	QString axisName(size_t dim, bool unit = false);

private slots:
	void rangeChanged();
	void dimensionChanged(QCPAxis *axis, size_t dim);
	void dimensionXChanged();
	void dimensionYChanged();
	void updateInfo(QMouseEvent *event = nullptr);
	void update();
	void updateCrosshair();
	void plotDoubleClick(QMouseEvent *event);

public:
	/**
	 * Constructor of the ExplorationWidget class.
	 */
	ExplorationWidget(QWidget *parent,
	                  std::shared_ptr<Parameters> params);

	/**
	 * Destructor of the ExplorationWidget class.
	 */
	~ExplorationWidget();

	/**
	 * Displays the results from the given Exploration instance.
	 *
	 * @param exploration is the exploration instance containing the data that
	 * should be shown.
	 * @param fit if set to true, fits the view to the given data, otherwise
	 * leaves the view as it is.
	 */
	void show(const Exploration &exploration, bool fit = true);

public slots:
	/**
	 * Slot that can be connected to the corresponding event of the
	 * IncrementalExploration class.
	 */
	void progress(float p, bool show);

	/**
	 * Centers the view according to the current parameters.
	 */
	void centerView();

signals:
	void updateRange(size_t dimX, size_t dimY, Val minX, Val maxX, Val minY,
	                 Val maxY);

	void updateParameters();
};
}

#endif /* _ADEXPSIM_EXPLORATION_WIDGET_HPP_ */
