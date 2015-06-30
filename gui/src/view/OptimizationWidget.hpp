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

#ifndef _ADEXPSIM_OPTIMIZATION_WIDGET_HPP_
#define _ADEXPSIM_OPTIMIZATION_WIDGET_HPP_

#include <memory>
#include <set>

#include <QWidget>

#include <model/OptimizationJob.hpp>

class QCheckBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTimer;

namespace AdExpSim {
/**
 * The OptimizationWidget class provides the interface for performing an
 * automated parameter optimization.
 */
class OptimizationWidget : public QWidget {
	Q_OBJECT

private:
	std::shared_ptr<ParameterCollection> params;

	OptimizationJob *job;
	std::vector<OptimizationResult> optimized;

	QTimer *updateTimer;
	QTableWidget *tableWidget;
	QCheckBox *chkOptimizeHw;
	QLabel *lblNIt;
	QLabel *lblNInput;
	QPushButton *btnOptimize;

private slots:
	void rebuildTable();
	void handleCellDoubleClicked(int row, int column);
	void handleOptimizeClicked();
	void handleProgress(bool done, size_t nIt, size_t nInput,
	                    std::vector<OptimizationResult> output);

public:
	/**
	 * Constructor of the OptimizationWidget class.
	 */
	OptimizationWidget(std::shared_ptr<ParameterCollection> params,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the OptimizationWidget class.
	 */
	~OptimizationWidget();

signals:
	void updateParameters(std::set<size_t> dims);
};
}

#endif /* _ADEXPSIM_OPTIMIZATION_WIDGET_HPP_ */
