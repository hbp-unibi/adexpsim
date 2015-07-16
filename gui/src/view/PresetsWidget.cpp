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

#include <QListView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QMessageBox>

#include <fstream>
#include <iostream>

#include <utils/ParameterCollection.hpp>
#include <io/JsonIo.hpp>

#include "PresetsWidget.hpp"

namespace AdExpSim {

PresetsWidget::PresetsWidget(std::shared_ptr<ParameterCollection> params,
                             QWidget *parent)
    : QWidget(parent), params(params)
{
	// Create the file system model and connect it to the list view
	QString path = "./presets";
	model = new QFileSystemModel(this);
	model->setNameFilters({"*.json"});
	model->setNameFilterDisables(false);
	listview = new QListView(this);
	listview->setModel(model);
	listview->setRootIndex(model->setRootPath(path));

	// Listen to the "doubleClicked" event
	connect(listview, SIGNAL(doubleClicked(const QModelIndex &)), this,
	        SLOT(handleListViewDoubleClick(const QModelIndex &)));

	// Create the main layout
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(listview);
	setLayout(layout);
}

PresetsWidget::~PresetsWidget()
{
	// Required for the shared_ptr
}

void PresetsWidget::handleListViewDoubleClick(const QModelIndex &index)
{
	QString filename = model->filePath(index);
	std::ifstream is(filename.toStdString());
	if (!is.good() || !JsonIo::loadGenericParameters(is, *params)) {
		QMessageBox::critical(
		    this, "Error while loading file",
		    "The specified parameter set could not be loaded.");
	} else {
		emit updateParameters(std::set<size_t>{});
	}
}
}

