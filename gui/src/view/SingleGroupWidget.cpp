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

	// Create the toolbar and the corresponding actions
	actCopyFromSpikeTrain =
	    new QAction(QIcon::fromTheme("go-down"), "From SpikeTrain", this);
	actCopyFromSpikeTrain->setToolTip(
	    "Copy parameters from Spike Train settings");
	actCopyToSpikeTrain =
	    new QAction(QIcon::fromTheme("go-up"), "To SpikeTrain", this);
	actCopyToSpikeTrain->setToolTip("Copy parameters to Spike Train settings");
	actLockNM1 = new QAction(QIcon("data/lock.png"), "Lock n", this);
	actLockNM1->setToolTip("Locks the n and n-1 sliders");
	actLockNM1->setCheckable(true);
	actLockNM1->setChecked(true);

	toolbar = new QToolBar();
	toolbar->addAction(actCopyFromSpikeTrain);
	toolbar->addAction(actCopyToSpikeTrain);
	toolbar->addSeparator();
	toolbar->addAction(actLockNM1);

	connect(actCopyFromSpikeTrain, SIGNAL(triggered()), this,
	        SLOT(copyFromSpikeTrain()));
	connect(actCopyToSpikeTrain, SIGNAL(triggered()), this,
	        SLOT(copyToSpikeTrain()));

	// Create the other parameter widgets
	paramN = new ParameterWidget(this, "n", 3, 1, 10, "", "n");
	paramN->setIntOnly(true);
	paramN->setMinMaxEnabled(false);
	paramNM1 = new ParameterWidget(this, "n-1", 2, 0, 9, "", "nM1");
	paramNM1->setIntOnly(true);
	paramNM1->setMinMaxEnabled(false);
	paramNPatch = new ParameterWidget(this, "nPatch", 1, 1, 20, "", "nPatch");
	paramNPatch->setIntOnly(true);
	paramNPatch->setMinMaxEnabled(false);
	paramNOut = new ParameterWidget(this, "nOut", 1, 0, 100, "", "nOut");
	paramNOut->setIntOnly(true);
	paramNOut->setMinMaxEnabled(false);
	paramDeltaT =
	    new ParameterWidget(this, "Δt", 1.0, 0.0, 20.0, "ms", "deltaT");
	paramDeltaT->setMinMaxEnabled(false);
	paramT = new ParameterWidget(this, "T", 33.0, 1.0, 200.0, "ms", "T");
	paramT->setMinMaxEnabled(false);

	connect(paramN, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramNM1, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramNPatch, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramNOut, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramDeltaT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Add all widgets to the main layout
	layout->addWidget(toolbar);
	layout->addWidget(paramN);
	layout->addWidget(paramNM1);
	layout->addWidget(paramNPatch);
	layout->addWidget(paramNOut);
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
		if (actLockNM1->isChecked()) {
			params->singleGroup.nM1 = value - 1;
			paramNM1->setValue(value - 1);
		}
	} else if (s == "nM1") {
		params->singleGroup.nM1 = value;
		if (actLockNM1->isChecked()) {
			params->singleGroup.n = value + 1;
			paramN->setValue(value + 1);
		}
	} else if (s == "nPatch") {
		params->singleGroup.nPatch = value;
	} else if (s == "nOut") {
		params->singleGroup.nOut = value;
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
	paramNM1->setValue(params->singleGroup.nM1);
	paramNPatch->setValue(params->singleGroup.nPatch);
	paramNOut->setValue(params->singleGroup.nOut);
	paramDeltaT->setValue(params->singleGroup.deltaT.sec() * 1000.0);
	paramT->setValue(params->singleGroup.T.sec() * 1000.0);
	blockSignals(false);
}

void SingleGroupWidget::triggerUpdateParameters()
{
	emit updateParameters(std::set<size_t>{});
}

void SingleGroupWidget::copyFromSpikeTrain()
{
	params->singleGroup = params->train.toSingleGroupSpikeData();
	refresh();
	emit updateParameters(std::set<size_t>{});
}

void SingleGroupWidget::copyToSpikeTrain()
{
	params->train.fromSingleGroupSpikeData(params->singleGroup);
	emit updateParameters(std::set<size_t>{});
}
}
