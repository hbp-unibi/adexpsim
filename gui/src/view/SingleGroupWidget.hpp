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
 * @file SingleGroupWidget.hpp
 *
 * Widget used to interactively set the parameters of a SingleGroupWidget
 * instance.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SINGLE_GROUP_WIDGET_HPP_
#define _ADEXPSIM_SINGLE_GROUP_WIDGET_HPP_

#include <memory>
#include <set>

#include <QWidget>

class QAction;
class QTimer;
class QToolBar;

namespace AdExpSim {

class ParameterCollection;
class ParameterWidget;

/**
 * The SingleGroupWidget allows to edit all properties of the
 * SingleGroupSpikeData class and informs its controller about changes in the
 * data by emitting the "updateParameters" singal.
 */
class SingleGroupWidget : public QWidget {
	Q_OBJECT
private:
	/**
	 * Reference at ParameterCollection instance shared throughout the
	 * application.
	 */
	std::shared_ptr<ParameterCollection> params;

	/* Actions */
	QAction *actCopyFromSpikeTrain;
	QAction *actCopyToSpikeTrain;
	QAction *actLockNM1;

	/* Widgets */
	QTimer *updateTimer;
	QToolBar *toolbar;
	ParameterWidget *paramN;
	ParameterWidget *paramNM1;
	ParameterWidget *paramNPatch;
	ParameterWidget *paramNOut;
	ParameterWidget *paramDeltaT;
	ParameterWidget *paramT;

private slots:
	void triggerUpdateParameters();
	void handleParameterUpdate(Val value, const QVariant &data);
	void copyFromSpikeTrain();
	void copyToSpikeTrain();

public slots:
	/**
	 * Refreshes the view to reflect the current parameters of the
	 * SingleGroupSpikeData instance.
	 */
	void refresh();

public:
	/**
	 * Costructor of the SingleGroupWidget class.
	 */
	SingleGroupWidget(std::shared_ptr<ParameterCollection> params,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the SingleGroupWidget class.
	 */
	~SingleGroupWidget();

signals:
	void updateParameters(std::set<size_t> dims);
};
}

#endif /* _ADEXPSIM_SINGLE_GROUP_WIDGET_HPP_ */

