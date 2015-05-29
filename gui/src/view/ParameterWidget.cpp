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

#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ParameterWidget.hpp"

namespace AdExpSim {

static constexpr int SLIDER_MAX = 10000;

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
      data(data)
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
		lblUnit->setFixedWidth(50);
	}
	edtValue = new QLineEdit(cntValue);
	edtValue->setFixedWidth(100);
	edtMin = new QLineEdit(cntRange);
	edtMax = new QLineEdit(cntRange);
	sliderValue = new QSlider(Qt::Horizontal, cntValue);
	sliderValue->setMinimum(0);
	sliderValue->setMaximum(SLIDER_MAX);

	// Range toggle button
	btnRange = new QPushButton(cntRange);
	btnRange->setFlat(true);
	btnRange->setFixedWidth(24);
	btnRange->setFixedHeight(24);
	btnRange->setText("▸");

	// Connect all events
	connect(edtValue, SIGNAL(editingFinished()), this, SLOT(handleEdit()));
	connect(edtMin, SIGNAL(editingFinished()), this, SLOT(handleEdit()));
	connect(edtMax, SIGNAL(editingFinished()), this, SLOT(handleEdit()));
	connect(sliderValue, SIGNAL(valueChanged(int)), this, SLOT(handleSlide(int)));
	connect(btnRange, SIGNAL(clicked()), this, SLOT(toggleRange()));

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
	layMain->addWidget(cntValue);
	layMain->addWidget(cntRange);
	setLayout(layMain);

	// Write all values given in the constructor to the widgets
	refresh();
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

static void parseDouble(const QString &str, double &value, bool &valid)
{
	Val newValue = str.toDouble(&valid);
	if (valid) {
		value = newValue;
	}
}

static constexpr char EDIT_DEFAULT_STYLE[] = "";
static constexpr char EDIT_INVALID_STYLE[] =
    "QLineEdit {background-color: rgb(255, 100, 64);}";
static constexpr char EDIT_OUT_OF_RANGE_STYLE[] =
    "QLineEdit {background-color: rgb(255, 200, 100);}";

static void parseDoubleInLineEdit(QLineEdit *edit, double &value, bool &valid)
{
	parseDouble(edit->text(), value, valid);
	if (valid) {
		edit->setStyleSheet(EDIT_DEFAULT_STYLE);
	} else {
		edit->setStyleSheet(EDIT_INVALID_STYLE);
	}
}

void ParameterWidget::handleEdit()
{
	// Parse the numbers and update the lineedit style
	parseDoubleInLineEdit(edtValue, value, valueValid);
	parseDoubleInLineEdit(edtMin, min, minValid);
	parseDoubleInLineEdit(edtMax, max, maxValid);
	validate();
	refreshSlider();
	if (valueValid && edtValue->text() != oldValueStr) {
		emit update(value, data);
		oldValueStr = edtValue->text();
	}
}

void ParameterWidget::handleSlide(int x)
{
	value = double(x) * (max - min) / double(SLIDER_MAX) + min;
	valueValid = true;
	edtValue->setText(QString::number(value));
	edtValue->setStyleSheet(EDIT_DEFAULT_STYLE);
	validate();
	emit update(value, data);
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
	edtValue->setText(QString::number(value));
	oldValueStr = edtValue->text();
	valueValid = true;

	edtMin->setText(QString::number(min));
	minValid = true;

	edtMax->setText(QString::number(max));
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
		sliderValue->setValue((value - min) / (max - min) * SLIDER_MAX);
		sliderValue->blockSignals(false);
	}
}

}

