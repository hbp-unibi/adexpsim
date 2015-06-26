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

#include <utils/ParameterCollection.hpp>

#include "ParametersWidget.hpp"
#include "ParameterWidget.hpp"

namespace AdExpSim {

static constexpr Val MIN_HZ = 1;
static constexpr Val MAX_HZ = 1000;
static constexpr Val MIN_S = 0;
static constexpr Val MAX_S = 1e-7;
static constexpr Val MIN_A = 0;
static constexpr Val MAX_A = 1e-9;
static constexpr Val MIN_V = -0.5;
static constexpr Val MAX_V = 0.5;

ParametersWidget::ParametersWidget(std::shared_ptr<ParameterCollection> params,
                                   QWidget *parent)
    : QWidget(parent), params(params)
{
	// Create the update timer
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);
	connect(updateTimer, SIGNAL(timeout()), this,
	        SLOT(triggerUpdateParameters()));

	// Create the widgets for the parameters which are not in the working set
	Parameters &p = params->params;
	paramCM = new ParameterWidget(this, "cM", p.cM(), p.cM() * 0.1, p.cM() * 10,
	                              "F", "cM");
	paramEL = new ParameterWidget(this, "eL", p.eL(), MIN_V, MAX_V, "V", "eL");
	connect(paramCM, SIGNAL(update(Val, const QVariant &)), this,
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramEL, SIGNAL(update(Val, const QVariant &)), this,
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Create the layout and add the first two parameter widgets
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(paramCM);
	layout->addWidget(paramEL);

	// Create the parameter widgets for all parameters in the working set
	for (size_t i = 0; i < WorkingParameters::Size; i++) {
		std::string name = WorkingParameters::names[i];
		std::string unit = WorkingParameters::units[i];
		Val value = p[i];
		Val min = value * 0.1;
		Val max = value * 10;
		if (max < min) {
			std::swap(min, max);
		}
		if (WorkingParameters::linear[i]) {
			name = WorkingParameters::originalNames[i];
			unit = WorkingParameters::originalUnits[i];
		} else {
			value = WorkingParameters::fromParameter(value, i, p);
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
		workingParams[i]->setOptimizeEnabled(true);
		workingParams[i]->setOptimize(params->optimize[i]);
		workingParams[i]->setExploreEnabled(true);
		workingParams[i]->setExplore(params->explore[i]);

		connect(workingParams[i], SIGNAL(update(Val, const QVariant &)), this,
		        SLOT(handleParameterUpdate(Val, const QVariant &)));
		connect(workingParams[i], SIGNAL(updateExplore(bool, const QVariant &)),
		        this,
		        SLOT(handleParameterUpdateExplore(bool, const QVariant &)));
		connect(workingParams[i],
		        SIGNAL(updateOptimize(bool, const QVariant &)), this,
		        SLOT(handleParameterUpdateOptimize(bool, const QVariant &)));

		layout->addWidget(workingParams[i]);
	}

	// Update the parameters
	handleUpdateParameters(std::set<size_t>{});
}

void ParametersWidget::handleParameterUpdate(Val value, const QVariant &data)
{
	// Abort if signals are blocked for this widget
	if (signalsBlocked()) {
		return;
	}

	Parameters &p = params->params;
	if (data.type() == QVariant::String) {
		QString s = data.toString();
		updatedDims.clear();
		if (s == "cM") {
			p.cM() = value;
		} else if (s == "eL") {
			p.eL() = value;
		}
	} else if (data.type() == QVariant::UInt) {
		size_t i = data.toUInt();
		updatedDims.emplace(i);
		if (WorkingParameters::linear[i]) {
			p[i] = value;
		} else {
			p[i] = WorkingParameters::toParameter(value, i, p);
		}
	}

	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void ParametersWidget::handleParameterUpdateRange(Val min, Val max,
                                                  const QVariant &data)
{
}

void ParametersWidget::handleParameterUpdateOptimize(bool optimize,
                                                     const QVariant &data)
{
	if (data.type() == QVariant::UInt) {
		params->optimize[data.toUInt()] = optimize;
	}
}

void ParametersWidget::handleParameterUpdateExplore(bool explore,
                                                    const QVariant &data)
{
	if (data.type() == QVariant::UInt) {
		params->explore[data.toUInt()] = explore;
	}
}

void ParametersWidget::handleUpdateParameters(std::set<size_t> dims)
{
	blockSignals(true);
	Parameters &p = params->params;
	if (dims.empty()) {
		paramCM->setValue(p.cM());
		paramEL->setValue(p.eL());
		for (size_t d = 0; d < WorkingParameters::Size; d++) {
			workingParams[d]->setVisible(params->model ==
			                                 ModelType::AD_IF_COND_EXP ||
			                             WorkingParameters::inIfCondExp[d]);
			dims.emplace(d);
		}
	}
	for (size_t d : dims) {
		Val value = p[d];
		if (!WorkingParameters::linear[d]) {
			value = WorkingParameters::fromParameter(value, d, p);
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

