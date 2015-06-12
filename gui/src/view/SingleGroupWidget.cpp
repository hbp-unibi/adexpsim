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
#include <QVBoxLayout>

#include <utils/ParameterCollection.hpp>

#include "ParameterWidget.hpp"
#include "SingleGroupWidget.hpp"

namespace AdExpSim {

SingleGroupWidget::SingleGroupWidget(
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
	paramN = new ParameterWidget(this, "n", 3, 1, 20, "", "n");
	paramN->setIntOnly(true);
	paramN->setMinMaxEnabled(false);
	paramDeltaT =
	    new ParameterWidget(this, "Δt", 1.0, 0.0, 20.0, "ms", "deltaT");
	paramDeltaT->setMinMaxEnabled(false);
	paramT = new ParameterWidget(this, "T", 33.0, 1.0, 200.0, "ms", "T");
	paramT->setMinMaxEnabled(false);

	connect(paramN, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramDeltaT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Add all widgets to the main layout
	layout->addWidget(paramN);
	layout->addWidget(paramDeltaT);
	layout->addWidget(paramT);
	setLayout(layout);

	// Set the widgets to the correct values
	refresh();
}

SingleGroupWidget::~SingleGroupWidget()
{
	// Do nothing here, only needed for the shared_ptr
}

void SingleGroupWidget::handleParameterUpdate(Val value, const QVariant &data)
{
	if (signalsBlocked()) {
		return;
	}

	QString s = data.toString();
	if (s == "n") {
		params->singleGroup.n = value;
	} else if (s == "deltaT") {
		params->singleGroup.deltaT = Time::sec(value / 1000.0);
	} else if (s == "T") {
		params->singleGroup.T = Time::sec(value / 1000.0);
	}

	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void SingleGroupWidget::refresh()
{
	blockSignals(true);
	paramN->setValue(params->singleGroup.n);
	paramDeltaT->setValue(params->singleGroup.deltaT.sec() * 1000.0);
	paramT->setValue(params->singleGroup.T.sec() * 1000.0);
	blockSignals(false);
}

void SingleGroupWidget::triggerUpdateParameters()
{
	emit updateParameters(std::set<size_t>{});
}
}
