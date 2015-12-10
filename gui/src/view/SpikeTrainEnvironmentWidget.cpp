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
#include <QHBoxLayout>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#include <utils/ParameterCollection.hpp>

#include "ParameterWidget.hpp"
#include "SpikeTrainEnvironmentWidget.hpp"

namespace AdExpSim {

SpikeTrainEnvironmentWidget::SpikeTrainEnvironmentWidget(
    std::shared_ptr<ParameterCollection> params, QWidget *parent)
    : QWidget(parent), params(params)
{
	// Create the update timer
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);
	connect(updateTimer, SIGNAL(timeout()), this,
	        SLOT(triggerUpdateParameters()));

	// Create the main layout
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	// Create the other parameter widgets
	paramBurstSize =
	    new ParameterWidget(this, "burst", 1, 1, 30, "", "burst");
	paramBurstSize->setIntOnly(true);
	paramBurstSize->setMinMaxEnabled(false);
	paramT = new ParameterWidget(this, "T", 50.0, 1.0, 200.0, "ms", "T");
	paramT->setMinMaxEnabled(false);
	paramSigmaT =
	    new ParameterWidget(this, "σt", 0.0, 0.0, 20.0, "ms", "sigmaT");
	paramSigmaT->setMinMaxEnabled(false);
	paramSigmaTOffs =
	    new ParameterWidget(this, "σtOffs", 0.0, 0.0, 20.0, "ms", "sigmaTOffs");
	paramSigmaTOffs->setMinMaxEnabled(false);
	paramDeltaT =
	    new ParameterWidget(this, "Δt", 5.0, 0.0, 20.0, "ms", "deltaT");
	paramDeltaT->setMinMaxEnabled(false);
	paramSigmaW =
	    new ParameterWidget(this, "σw", 0.0, 0.0, 100.0, "%", "sigmaW");
	paramSigmaW->setMinMaxEnabled(false);

	connect(paramBurstSize, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramSigmaT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramSigmaTOffs, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramDeltaT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramSigmaW, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Add all widgets to the main layout
	layout->addWidget(paramBurstSize);
	layout->addWidget(paramT);
	layout->addWidget(paramSigmaT);
	layout->addWidget(paramSigmaTOffs);
	layout->addWidget(paramDeltaT);
	layout->addWidget(paramSigmaW);
	setLayout(layout);

	// Set the widgets to the correct values
	refresh();
}

SpikeTrainEnvironmentWidget::~SpikeTrainEnvironmentWidget()
{
	// Do nothing here, only needed for the shared_ptr
}

void SpikeTrainEnvironmentWidget::handleParameterUpdate(Val value,
                                                        const QVariant &data)
{
	if (signalsBlocked()) {
		return;
	}

	QString s = data.toString();
	if (s == "burst") {
		params->environment.burstSize = value;
	} else if (s == "T") {
		params->environment.T = Time::sec(value / 1000.0);
	} else if (s == "sigmaT") {
		params->environment.sigmaT = Time::sec(value / 1000.0);
	} else if (s == "sigmaTOffs") {
		params->environment.sigmaTOffs = Time::sec(value / 1000.0);
	} else if (s == "deltaT") {
		params->environment.deltaT = Time::sec(value / 1000.0);
	} else if (s == "sigmaW") {
		params->environment.sigmaW = value / 100.0;
	}
	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void SpikeTrainEnvironmentWidget::refresh()
{
	blockSignals(true);
	paramBurstSize->setValue(params->environment.burstSize);
	paramT->setValue(params->environment.T.sec() * 1000.0);
	paramSigmaT->setValue(params->environment.sigmaT.sec() * 1000.0);
	paramSigmaTOffs->setValue(params->environment.sigmaTOffs.sec() * 1000.0);
	paramDeltaT->setValue(params->environment.deltaT.sec() * 1000.0);
	paramSigmaW->setValue(params->environment.sigmaW * 100.0);
	blockSignals(false);
}

void SpikeTrainEnvironmentWidget::triggerUpdateParameters()
{
	// Update the environment of the spike train and rebuild it
	params->train.setEnvironment(params->environment);
	params->train.rebuild();

	// Emit the update event
	emit updateParameters(std::set<size_t>{});
}
}
