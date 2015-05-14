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

#include <qcustomplot.h>

class QComboBox;
class QStatusBar;
class QToolBar;
class QVBoxLayout;

namespace AdExpSim {

/**
 * The ExplorationWidget shows the exploration result as colored image.
 */
class ExplorationWidget: public QWidget {
Q_OBJECT

private:
	QToolBar *toolbar;
	QComboBox *comboDimX;
	QComboBox *comboDimY;
	QComboBox *comboFunction;
	QVBoxLayout *layout;
	QProgressBar *progressBar;
	QLabel *statusLabel;
	QCustomPlot *pltExploration;
	QCPColorGradient gradCost;

	std::unique_ptr<Exploration> currentExploration;

private slots:
	void axisChange(const QCPRange &newRange);

public:
	/**
	 * Constructor of the ExplorationWidget class.
	 */
	ExplorationWidget(QWidget *parent);

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

signals:
	void updateRange(size_t dimX, size_t dimY, Val minX, Val maxX, Val minY, Val maxY);
};


}

#endif /* _ADEXPSIM_EXPLORATION_WIDGET_HPP_ */
