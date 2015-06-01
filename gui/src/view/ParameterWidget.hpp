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
 * @file ParameterWidget.hpp
 *
 * Widget used to adjust a parameter with minimum/maximum value.
 *
 * @author Andreas Stöckel
 */

#include <QString>
#include <QVariant>
#include <QWidget>

class QLabel;
class QPushButton;
class QLineEdit;
class QSlider;
class QWidget;

#include <utils/Types.hpp>

namespace AdExpSim {

class ParameterWidget : public QWidget {
	Q_OBJECT
private:
	QWidget *cntValue;
	QWidget *cntRange;
	QLineEdit *edtValue;
	QLineEdit *edtMin;
	QLineEdit *edtMax;
	QLabel *lblName;
	QLabel *lblUnit;
	QPushButton *btnRange;
	QSlider *sliderValue;

	QString name;
	double value, min, max;
	bool valueValid, minValid, maxValid;
	QString oldValueStr;
	QVariant data;
	bool intOnly;

	void validate();
	void refresh();
	void refreshSlider();

	QString toStr(Val v) const;

private slots:
	void toggleRange();
	void handleEdit();
	void handleSlide(int value);

public:
	ParameterWidget(QWidget *parent, QString name = "", Val value = 50.0,
	                Val min = 0.0, Val max = 100.0, QString unit = "",
	                QVariant data = QVariant());

	Val getValue() const { return value; }
	Val getMin() const { return min; }
	Val getMax() const { return max; }
	void setValue(Val value);
	void setMin(Val min);
	void setMax(Val max);
	void setIntOnly(bool intOnly);
	void setMinMaxEnabled(bool enabled);

signals:
	void update(Val value, const QVariant &data);
};
}

