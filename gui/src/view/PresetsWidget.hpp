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
 * @file PresetWidget.hpp
 *
 * Widget used to load preset configurations from the filesystem.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_PRESET_WIDGET_HPP_
#define _ADEXPSIM_PRESET_WIDGET_HPP_

#include <memory>
#include <set>

#include <QWidget>

class QFileSystemModel;
class QListView;
class QModelIndex;

namespace AdExpSim {

class ParameterCollection;

/**
 * The PresetWidget class allows to load preset configuration from the
 * filesystem with a single click.
 */
class PresetsWidget : public QWidget {
	Q_OBJECT
private:
	/**
	 * Reference at ParameterCollection instance shared throughout the
	 * application.
	 */
	std::shared_ptr<ParameterCollection> params;

	/* Widgets */
	QListView *listview;
	QFileSystemModel *model;

private slots:
	void handleListViewDoubleClick(const QModelIndex &index);

public:
	/**
	 * Costructor of the PresetWidget class.
	 */
	PresetsWidget(std::shared_ptr<ParameterCollection> params,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the PresetWidget class.
	 */
	~PresetsWidget();

signals:
	void updateParameters(std::set<size_t> dims);
};
}

#endif /* _ADEXPSIM_PRESET_WIDGET_HPP_ */

