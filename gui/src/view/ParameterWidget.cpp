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

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QWidget>
#include <QVBoxLayout>

#include "ParameterWidget.hpp"

namespace AdExpSim {

static constexpr int SLIDER_MAX = 10000;

static QFrame *createSeparator(QWidget *parent)
{
	QFrame *line = new QFrame(parent);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	return line;
}

ParameterWidget::ParameterWidget(QWidget *parent, QString name, Val value,
                                 Val min, Val max, QString unit, QVariant data)
    : QWidget(parent),
      name(name),
      value(value),
      min(min),
      max(max),
      valueValid(true),
      minValid(true),
      maxValid(true),
      data(data),
      intOnly(false)
{
	// Create the two container widgets and their layouts
	cntValue = new QWidget(this);
	cntRange = new QWidget(this);
	QHBoxLayout *layValue = new QHBoxLayout(cntValue);
	QHBoxLayout *layRange = new QHBoxLayout(cntRange);
	cntValue->setLayout(layValue);
	cntRange->setLayout(layRange);
	cntRange->hide();

	// Create all components
	lblName = new QLabel(cntValue);
	lblName->setText(name);
	lblName->setFixedWidth(50);
	lblName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	lblUnit = new QLabel(cntValue);
	if (!unit.isEmpty()) {
		lblUnit->setText("[" + unit + "]");
	}
	lblUnit->setFixedWidth(50);
	edtValue = new QLineEdit(cntValue);
	edtValue->setFixedWidth(100);
	edtMin = new QLineEdit(cntRange);
	edtMax = new QLineEdit(cntRange);
	sliderValue = new QSlider(Qt::Horizontal, cntValue);
	sliderValue->setMinimumWidth(100);
	sliderValue->setMinimum(0);
	sliderValue->setMaximum(SLIDER_MAX);

	// Range toggle button
	btnRange = new QPushButton(cntRange);
	btnRange->setFlat(true);
	btnRange->setFixedWidth(24);
	btnRange->setFixedHeight(24);
	btnRange->setText("▸");

	// "Optimize" and "Explore" buttons (default: invisible)
	btnOptimize = new QToolButton(cntRange);
	btnOptimize->setIcon(QIcon::fromTheme("edit-find"));
	btnOptimize->setCheckable(true);
	btnOptimize->setToolTip(
	    "Optimize this parameter");
	btnOptimize->hide();

	btnExplore = new QToolButton(cntRange);
	btnExplore->setIcon(QIcon::fromTheme("camera-photo"));
	btnExplore->setCheckable(true);
	btnExplore->setToolTip(
	    "Mark this parameter for exploration (in CLI)");
	btnExplore->hide();

	// Connect all events
	connect(edtValue, SIGNAL(editingFinished()), this, SLOT(handleEdit()));
	connect(edtMin, SIGNAL(editingFinished()), this, SLOT(handleEdit()));
	connect(edtMax, SIGNAL(editingFinished()), this, SLOT(handleEdit()));
	connect(sliderValue, SIGNAL(valueChanged(int)), this,
	        SLOT(handleSlide(int)));
	connect(btnRange, SIGNAL(clicked()), this, SLOT(toggleRange()));
	connect(btnOptimize, SIGNAL(toggled(bool)), this,
	        SLOT(handleOptimizeToggled(bool)));
	connect(btnExplore, SIGNAL(toggled(bool)), this,
	        SLOT(handleExploreToggled(bool)));

	// Combine all widgets in the layout
	QVBoxLayout *layMain = new QVBoxLayout(this);
	layMain->setSpacing(0);
	layMain->setMargin(0);
	layValue->addWidget(btnRange);
	layValue->addWidget(lblName);
	layValue->addSpacing(10);
	layValue->addWidget(sliderValue);
	layValue->addSpacing(10);
	layValue->addWidget(edtValue);
	layValue->addWidget(lblUnit);
	layRange->addWidget(new QLabel("Min:"));
	layRange->addSpacing(10);
	layRange->addWidget(edtMin);
	layRange->addWidget(new QLabel("Max:"));
	layRange->addSpacing(10);
	layRange->addWidget(edtMax);
	layRange->addSpacing(10);
	layRange->addWidget(btnOptimize);
	layRange->addWidget(btnExplore);
	layMain->addWidget(cntValue);
	layMain->addWidget(cntRange);
	layMain->addWidget(createSeparator(this));
	setLayout(layMain);

	// Write all values given in the constructor to the widgets
	refresh();
}

QString ParameterWidget::toStr(Val v) const
{
	if (intOnly) {
		return QString::number(long(round(v)));
	} else {
		return QString::number(v);
	}
}

void ParameterWidget::toggleRange()
{
	if (cntRange->isVisible()) {
		cntRange->hide();
		btnRange->setText("▸");
	} else {
		cntRange->show();
		btnRange->setText("▾");
	}
	updateGeometry();
}

void ParameterWidget::setValue(Val value)
{
	blockSignals(true);
	this->value = value;
	refresh();
	blockSignals(false);
}

void ParameterWidget::setMin(Val min)
{
	blockSignals(true);
	this->min = min;
	refresh();
	blockSignals(false);
}

void ParameterWidget::setMax(Val max)
{
	blockSignals(true);
	this->max = max;
	refresh();
	blockSignals(false);
}

void ParameterWidget::setOptimize(bool optimize)
{
	blockSignals(true);
	btnOptimize->setChecked(optimize);
	blockSignals(false);
}

void ParameterWidget::setExplore(bool explore)
{
	blockSignals(true);
	btnExplore->setChecked(explore);
	blockSignals(false);
}

void ParameterWidget::setIntOnly(bool intOnly)
{
	blockSignals(true);
	this->intOnly = intOnly;
	if (intOnly) {
		sliderValue->setMinimum(min);
		sliderValue->setMaximum(max);
	} else {
		sliderValue->setMinimum(0);
		sliderValue->setMaximum(SLIDER_MAX);
	}
	refresh();
	blockSignals(false);
}

void ParameterWidget::setMinMaxEnabled(bool enabled)
{
	blockSignals(true);
	cntRange->hide();
	btnRange->setEnabled(enabled);
	if (!enabled) {
		btnRange->setText("");
	} else {
		btnRange->setText("▸");
	}
	blockSignals(false);
}

void ParameterWidget::setOptimizeEnabled(bool enabled)
{
	btnOptimize->setVisible(enabled);
}

void ParameterWidget::setExploreEnabled(bool enabled)
{
	btnExplore->setVisible(enabled);
}

static bool parseDouble(const QString &str, Val &value, bool &valid)
{
	Val newValue = str.toDouble(&valid);
	bool changed = valid && (newValue != value);
	if (valid) {
		value = newValue;
	}
	return changed;
}

static bool parseLong(const QString &str, Val &value, bool &valid)
{
	Val newValue = str.toLong(&valid);
	bool changed = valid && (newValue != value);
	if (valid) {
		value = newValue;
	}
	return changed;
}

static constexpr char EDIT_DEFAULT_STYLE[] = "";
static constexpr char EDIT_INVALID_STYLE[] =
    "QLineEdit {background-color: rgb(255, 100, 64);}";
static constexpr char EDIT_OUT_OF_RANGE_STYLE[] =
    "QLineEdit {background-color: rgb(255, 200, 100);}";

static bool parseDoubleInLineEdit(QLineEdit *edit, Val &value, bool &valid)
{
	bool changed = parseDouble(edit->text(), value, valid);
	if (valid) {
		edit->setStyleSheet(EDIT_DEFAULT_STYLE);
	} else {
		edit->setStyleSheet(EDIT_INVALID_STYLE);
	}
	return changed;
}

static bool parseLongInLineEdit(QLineEdit *edit, Val &value, bool &valid)
{
	bool changed = parseLong(edit->text(), value, valid);
	if (valid) {
		edit->setStyleSheet(EDIT_DEFAULT_STYLE);
	} else {
		edit->setStyleSheet(EDIT_INVALID_STYLE);
	}
	return changed;
}

void ParameterWidget::handleEdit()
{
	// Parse the numbers and update the lineedit style
	bool changedValue, changedMin, changedMax;
	if (intOnly) {
		changedValue = parseLongInLineEdit(edtValue, value, valueValid);
		changedMin = parseLongInLineEdit(edtMin, min, minValid);
		changedMax = parseLongInLineEdit(edtMax, max, maxValid);
	} else {
		changedValue = parseDoubleInLineEdit(edtValue, value, valueValid);
		changedMin = parseDoubleInLineEdit(edtMin, min, minValid);
		changedMax = parseDoubleInLineEdit(edtMax, max, maxValid);
	}
	validate();
	refreshSlider();
	if (changedValue) {
		emit update(value, data);
	}
	if (changedMin || changedMax) {
		emit updateRange(min, max, data);
	}
}

void ParameterWidget::handleSlide(int x)
{
	if (intOnly) {
		value = x;
	} else {
		value = double(x) * (max - min) / double(SLIDER_MAX) + min;
	}
	valueValid = true;
	edtValue->setText(toStr(value));
	edtValue->setStyleSheet(EDIT_DEFAULT_STYLE);
	validate();
	emit update(value, data);
}

void ParameterWidget::handleOptimizeToggled(bool checked)
{
	emit updateOptimize(checked, data);
}

void ParameterWidget::handleExploreToggled(bool checked)
{
	emit updateExplore(checked, data);
}

void ParameterWidget::validate()
{
	if (valueValid) {
		if ((value < min) || (value > max)) {
			edtValue->setStyleSheet(EDIT_OUT_OF_RANGE_STYLE);
		} else {
			edtValue->setStyleSheet(EDIT_DEFAULT_STYLE);
		}
	}

	if (minValid) {
		if (min > max) {
			edtMin->setStyleSheet(EDIT_OUT_OF_RANGE_STYLE);
		} else {
			edtMin->setStyleSheet(EDIT_DEFAULT_STYLE);
		}
	}

	if (maxValid) {
		if (max < min) {
			edtMax->setStyleSheet(EDIT_OUT_OF_RANGE_STYLE);
		} else {
			edtMin->setStyleSheet(EDIT_DEFAULT_STYLE);
		}
	}
}

void ParameterWidget::refresh()
{
	// Update the edit fields
	edtValue->setText(toStr(value));
	valueValid = true;

	edtMin->setText(toStr(min));
	minValid = true;

	edtMax->setText(toStr(max));
	maxValid = true;

	// Inform about the validity of the values
	validate();
	refreshSlider();
}

void ParameterWidget::refreshSlider()
{
	// Update the slider, prevent this update triggering the "handleSlide"
	// function
	if (valueValid) {
		sliderValue->blockSignals(true);
		if (intOnly) {
			sliderValue->setValue(value);
		} else {
			sliderValue->setValue((value - min) / (max - min) * SLIDER_MAX);
		}
		sliderValue->blockSignals(false);
	}
}
}

