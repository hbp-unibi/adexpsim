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

/**
 * @file SpikeTrainWidget.hpp
 *
 * Widget used to interactively create a SpikeTrain instance.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SPIKE_TRAIN_WIDGET_HPP_
#define _ADEXPSIM_SPIKE_TRAIN_WIDGET_HPP_

#include <memory>
#include <set>

#include <QWidget>

class QAction;
class QLineEdit;
class QTableWidget;
class QTimer;

namespace AdExpSim {

class ParameterCollection;
class ParameterWidget;

/**
 * The SpikeTrainWidget allows to edit all properties of the SpikeTrain class
 * and informs its controller about changes in the SpikeTrain instance by
 * emitting the "updateParameters" signal.
 */
class SpikeTrainWidget : public QWidget {
	Q_OBJECT
private:
	/**
	 * Reference at ParameterCollection instance shared throughout the
	 * application.
	 */
	std::shared_ptr<ParameterCollection> params;

	/* Actions */
	QAction *actAddGroup;
	QAction *actDeleteGroup;
	QAction *actRebuild;

	/* Widgets */
	QTimer *updateTimer;
	QTableWidget *tableWidget;
	ParameterWidget *paramSorted;
	ParameterWidget *paramN;
	ParameterWidget *paramT;
	ParameterWidget *paramSigmaT;

private slots:
	void triggerUpdateParameters();
	void handleCellChanged(int row, int column);
	void handleParameterUpdate(Val value, const QVariant &data);
	void handleAddGroup();
	void handleDeleteGroups();

public slots:
	/**
	 * Refreshes the view to reflect the current parameters of the SpikeTrain
	 * instance.
	 */
	void refresh();

public:
	/**
	 * Costructor of the SpikeTrainWidget class.
	 */
	SpikeTrainWidget(std::shared_ptr<ParameterCollection> params,
	                 QWidget *parent = nullptr);

	/**
	 * Destructor of the SpikeTrainWidget class.
	 */
	~SpikeTrainWidget();

signals:
	void updateParameters(std::set<size_t> dims);
};
}

#endif /* _ADEXPSIM_SPIKE_TRAIN_WIDGET_HPP_ */

