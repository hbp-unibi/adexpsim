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
 * @file SpikeTrainEnvironmentWidget.hpp
 *
 * Widget used to interactively create a SpikeTrainEnvironment instance.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_SPIKE_TRAIN_ENVIRONMENT_WIDGET_HPP_
#define _ADEXPSIM_SPIKE_TRAIN_ENVIRONMENT_WIDGET_HPP_

#include <memory>
#include <set>

#include <QWidget>

class QTimer;

namespace AdExpSim {

class ParameterCollection;
class ParameterWidget;

/**
 * The SpikeTrainEnvironmentWidget allows to edit all properties of the
 * SpikeTrainEnvironment structure which stores evaluation-independent
 * properties that define the way in which the input spike train is generated.
 */
class SpikeTrainEnvironmentWidget : public QWidget {
	Q_OBJECT
private:
	/**
	 * Reference at ParameterCollection instance shared throughout the
	 * application.
	 */
	std::shared_ptr<ParameterCollection> params;

	/* Widgets */
	QTimer *updateTimer;
	ParameterWidget *paramBundleSize;
	ParameterWidget *paramT;
	ParameterWidget *paramSigmaT;
	ParameterWidget *paramDeltaT;
	ParameterWidget *paramSigmaW;

private slots:
	void triggerUpdateParameters();
	void handleParameterUpdate(Val value, const QVariant &data);

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
	SpikeTrainEnvironmentWidget(std::shared_ptr<ParameterCollection> params,
	                            QWidget *parent = nullptr);

	/**
	 * Destructor of the SpikeTrainWidget class.
	 */
	~SpikeTrainEnvironmentWidget();

signals:
	void updateParameters(std::set<size_t> dims);
};
}

#endif /* _ADEXPSIM_SPIKE_TRAIN_ENVIRONMENT_WIDGET_HPP_ */

