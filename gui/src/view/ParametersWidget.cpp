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

#include <algorithm>
#include <iostream>

#include <QTimer>
#include <QVBoxLayout>
#include <QFrame>

#include <simulation/Parameters.hpp>

#include "ParametersWidget.hpp"
#include "ParameterWidget.hpp"

namespace AdExpSim {

static QFrame *createSeparator(QWidget *parent)
{
	QFrame *line = new QFrame(parent);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	return line;
}

static constexpr Val MIN_HZ = 1;
static constexpr Val MAX_HZ = 1000;
static constexpr Val MIN_S = 0;
static constexpr Val MAX_S = 1e-7;
static constexpr Val MIN_A = 0;
static constexpr Val MAX_A = 1e-9;
static constexpr Val MIN_V = -0.5;
static constexpr Val MAX_V = 0.5;

ParametersWidget::ParametersWidget(std::shared_ptr<Parameters> params,
                                   QWidget *parent)
    : QWidget(parent), params(params)
{
	// Create the update timer
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);
	connect(updateTimer, SIGNAL(timeout()), this,
	        SLOT(triggerUpdateParameters()));

	// Create the widgets for the parameters which are not in the working set
	paramCM = new ParameterWidget(this, "cM", params->cM, params->cM * 0.1,
	                              params->cM * 10, "F", "cM");
	paramEL =
	    new ParameterWidget(this, "eL", params->eL, MIN_V, MAX_V, "V", "eL");
	connect(paramCM, SIGNAL(update(Val, const QVariant &)), this,
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramEL, SIGNAL(update(Val, const QVariant &)), this,
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Create the layout and add the first two parameter widgets
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(paramCM);
	layout->addWidget(createSeparator(this));
	layout->addWidget(paramEL);
	layout->addWidget(createSeparator(this));

	// Create the parameter widgets for all parameters in the working set
	for (size_t i = 0; i < 13; i++) {
		std::string name = WorkingParameters::names[i];
		std::string unit = WorkingParameters::units[i];
		Val value = WorkingParameters::fetchParameter(i, *params);
		Val min = value * 0.1;
		Val max = value * 10;
		if (max < min) {
			std::swap(min, max);
		}
		if (WorkingParameters::linear[i]) {
			name = WorkingParameters::originalNames[i];
			unit = WorkingParameters::originalUnits[i];
		} else {
			value = WorkingParameters::fromParameter(value, i, *params);
		}

		if (unit == "Hz") {
			min = MIN_HZ;
			max = MAX_HZ;
		} else if (unit == "S") {
			min = MIN_S;
			max = MAX_S;
		} else if (unit == "A") {
			min = MIN_A;
			max = MAX_A;
		} else if (unit == "V") {
			min = MIN_V;
			max = MAX_V;
		}

		workingParams[i] =
		    new ParameterWidget(this, QString::fromStdString(name), value, min,
		                        max, QString::fromStdString(unit), uint(i));
		connect(workingParams[i], SIGNAL(update(Val, const QVariant &)), this,
		        SLOT(handleParameterUpdate(Val, const QVariant &)));

		layout->addWidget(workingParams[i]);
		layout->addWidget(createSeparator(this));
	}
}

void ParametersWidget::handleParameterUpdate(Val value, const QVariant &data)
{
	// Abort if signals are blocked for this widget
	if (signalsBlocked()) {
		return;
	}

	if (data.type() == QVariant::String) {
		QString p = data.toString();
		updatedDims.clear();
		if (p == "cM") {
			params->cM = value;
		} else if (p == "eL") {
			params->eL = value;
		}
	} else if (data.type() == QVariant::UInt) {
		size_t i = data.toUInt();
		updatedDims.emplace(i);
		if (WorkingParameters::linear[i]) {
			WorkingParameters::fetchParameter(i, *params) = value;
		} else {
			WorkingParameters::fetchParameter(i, *params) =
			    WorkingParameters::toParameter(value, i, *params);
		}
	}

	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void ParametersWidget::handleUpdateParameters(std::set<size_t> dims)
{
	blockSignals(true);
	if (dims.empty()) {
		paramCM->setValue(params->cM);
		paramEL->setValue(params->eL);
		for (size_t i = 0; i < 13; i++) {
			dims.emplace(i);
		}
	}
	for (size_t d : dims) {
		Val value = WorkingParameters::fetchParameter(d, *params);
		if (!WorkingParameters::linear[d]) {
			value = WorkingParameters::fromParameter(value, d, *params);
		}
		workingParams[d]->setValue(value);
	}
	blockSignals(false);
}

void ParametersWidget::triggerUpdateParameters()
{
	emit updateParameters(updatedDims);
	updatedDims.clear();
}
}

