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
#include <QHeaderView>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#include <simulation/Spike.hpp>

#include "ParameterWidget.hpp"
#include "SpikeTrainWidget.hpp"

namespace AdExpSim {

SpikeTrainWidget::SpikeTrainWidget(std::shared_ptr<SpikeTrain> train,
                                   QWidget *parent)
    : QWidget(parent), train(train)
{
	// Create the update timer
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);
	connect(updateTimer, SIGNAL(timeout()), this,
	        SLOT(triggerUpdateParameters()));

	// Create the main layout
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	// Create the toolbar and the corresponding actions
	actAddGroup = new QAction(QIcon::fromTheme("list-add"), "Add Group", this);
	actAddGroup->setToolTip("Add a new spike train group");
	actDeleteGroup =
	    new QAction(QIcon::fromTheme("list-remove"), "Delete Groups", this);
	actDeleteGroup->setToolTip("Delete the selected train groups");
	actRebuild = new QAction(QIcon::fromTheme("view-refresh"), "Rebuild", this);
	actRebuild->setToolTip("Builds a new random spike train");

	QToolBar *toolbar = new QToolBar();
	toolbar->addAction(actAddGroup);
	toolbar->addAction(actDeleteGroup);
	toolbar->addSeparator();
	toolbar->addAction(actRebuild);

	connect(actAddGroup, SIGNAL(triggered()), this, SLOT(handleAddGroup()));
	connect(actDeleteGroup, SIGNAL(triggered()), this,
	        SLOT(handleDeleteGroups()));
	connect(actRebuild, SIGNAL(triggered()), this,
	        SLOT(triggerUpdateParameters()));

	// Create the container and layout for the table and the add/delete
	// buttons
	tableWidget = new QTableWidget(this);
	tableWidget->setColumnCount(7);
	tableWidget->horizontalHeader()->setSectionResizeMode(
	    QHeaderView::ResizeToContents);
	tableWidget->verticalHeader()->setSectionResizeMode(
	    QHeaderView::ResizeToContents);
	tableWidget->setHorizontalHeaderLabels(
	    {"#E", "#I", "#Out", "σT [ms]", "wE [%]", "wI [%]", "σW [%]"});
	connect(tableWidget, SIGNAL(cellChanged(int, int)), this,
	        SLOT(handleCellChanged(int, int)));

	// Create the other parameter widgets
	paramSorted = new ParameterWidget(this, "sorted", 1, 0, 1, "", "sorted");
	paramSorted->setIntOnly(true);
	paramSorted->setMinMaxEnabled(false);
	paramN = new ParameterWidget(this, "N", 2, 1, 1000, "", "N");
	paramN->setIntOnly(true);
	paramN->setMinMaxEnabled(false);
	paramT = new ParameterWidget(this, "T", 33.0, 1.0, 200.0, "ms", "T");
	paramT->setMinMaxEnabled(false);
	paramSigmaT =
	    new ParameterWidget(this, "σT", 0.0, 0.0, 100.0, "ms", "sigmaT");
	paramSigmaT->setMinMaxEnabled(false);

	connect(paramSorted, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramN, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramSigmaT, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Add all widgets to the main layout
	mainLayout->addWidget(toolbar);
	mainLayout->addWidget(tableWidget);
	mainLayout->addWidget(paramSorted);
	mainLayout->addWidget(paramN);
	mainLayout->addWidget(paramT);
	mainLayout->addWidget(paramSigmaT);
	setLayout(mainLayout);

	// Set the widgets to the correct values
	refresh();
}

SpikeTrainWidget::~SpikeTrainWidget()
{
	// Do nothing here, only needed for the shared_ptr
}

void SpikeTrainWidget::handleCellChanged(int row, int column)
{
	if (signalsBlocked()) {
		return;
	}

	// Copy the current descriptors
	std::vector<SpikeTrain::Descriptor> descrs = train->getDescrs();

	// Make sure the coordinates are in range
	if (row < 0 || size_t(row) >= descrs.size() || column < 0 || column >= 7) {
		return;
	}

	// Update the changed descriptor
	double value = tableWidget->currentItem()->data(Qt::DisplayRole).toDouble();
	switch (column) {
		case 0:
			descrs[row].nE = std::max(0.0, value);
			break;
		case 1:
			descrs[row].nI = std::max(0.0, value);
			break;
		case 2:
			descrs[row].nOut = std::max(0.0, value);
			break;
		case 3:
			descrs[row].sigmaT = std::max(0.0, value / 1000.0);
			break;
		case 4:
			descrs[row].wE = value / 100.0;
			break;
		case 5:
			descrs[row].wI = value / 100.0;
			break;
		case 6:
			descrs[row].sigmaW = std::max(0.0, value / 100.0);
			break;
	}

	// Make sure there is at least one spike
	if (descrs[row].nE + descrs[row].nI == 0) {
		descrs[row].nE = 1;
		descrs[row].wE = 0;
	}

	// Copy the updated descriptors back to the spike train instance
	train->setDescrs(descrs);
	refresh();

	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void SpikeTrainWidget::handleParameterUpdate(Val value, const QVariant &data)
{
	if (signalsBlocked()) {
		return;
	}

	QString s = data.toString();
	if (s == "sorted") {
		train->setSorted(value != 0.0);
	} else if (s == "N") {
		train->setN(size_t(value));
	} else if (s == "T") {
		train->setT(Time::sec(value / 1000.0));
	} else if (s == "sigmaT") {
		train->setSigmaT(value / 1000.0);
	}

	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void SpikeTrainWidget::refresh()
{
	blockSignals(true);

	// Update the descriptor table
	auto descrs = train->getDescrs();
	tableWidget->setRowCount(descrs.size());
	for (size_t i = 0; i < descrs.size(); i++) {
		QTableWidgetItem *itmNE = new QTableWidgetItem();
		itmNE->setData(Qt::DisplayRole, QVariant(descrs[i].nE));

		QTableWidgetItem *itmNI = new QTableWidgetItem();
		itmNI->setData(Qt::DisplayRole, QVariant(descrs[i].nI));

		QTableWidgetItem *itmNOut = new QTableWidgetItem();
		itmNOut->setData(Qt::DisplayRole, QVariant(descrs[i].nOut));

		QTableWidgetItem *itmSigmaT = new QTableWidgetItem();
		itmSigmaT->setData(Qt::DisplayRole,
		                   QVariant(double(descrs[i].sigmaT * 1000.0)));

		QTableWidgetItem *itmWE = new QTableWidgetItem();
		itmWE->setData(Qt::DisplayRole, QVariant(double(descrs[i].wE) * 100.0));

		QTableWidgetItem *itmWI = new QTableWidgetItem();
		itmWI->setData(Qt::DisplayRole, QVariant(double(descrs[i].wI) * 100.0));

		QTableWidgetItem *itmSigmaW = new QTableWidgetItem();
		itmSigmaW->setData(Qt::DisplayRole,
		                   QVariant(double(descrs[i].sigmaW * 100.0)));

		tableWidget->setItem(i, 0, itmNE);
		tableWidget->setItem(i, 1, itmNI);
		tableWidget->setItem(i, 2, itmNOut);
		tableWidget->setItem(i, 3, itmSigmaT);
		tableWidget->setItem(i, 4, itmWE);
		tableWidget->setItem(i, 5, itmWI);
		tableWidget->setItem(i, 6, itmSigmaW);
	}

	// Update the parameter sliders
	paramSorted->setValue(train->isSorted());
	paramN->setValue(train->getN());
	paramT->setValue(train->getT().sec() * 1000.0);
	paramSigmaT->setValue(train->getSigmaT() * 1000.0);

	blockSignals(false);
}

void SpikeTrainWidget::handleAddGroup()
{
	// Copy the current descriptors
	std::vector<SpikeTrain::Descriptor> descrs = train->getDescrs();

	// Add a new descriptor
	descrs.emplace_back(1.0, 0.0, 1e-3);

	// Set the new descriptors
	train->setDescrs(descrs);

	// Refresh the view
	refresh();

	// Trigger a parameter update
	triggerUpdateParameters();
}

void SpikeTrainWidget::handleDeleteGroups()
{
	// Copy the current descriptors
	std::vector<SpikeTrain::Descriptor> descrs = train->getDescrs();

	// Fetch all currently selected rows
	std::set<int> selectedRows;
	for (QTableWidgetItem *item : tableWidget->selectedItems()) {
		selectedRows.emplace(item->row());
	}

	// Copy all but the selected rows to a new descriptor list
	std::vector<SpikeTrain::Descriptor> newDescrs;
	for (size_t i = 0; i < descrs.size(); i++) {
		if (selectedRows.count(i) == 0) {
			newDescrs.emplace_back(descrs[i]);
		}
	}

	// Set the new descriptors
	train->setDescrs(newDescrs);

	// Refresh the view
	refresh();

	// Trigger a parameter update
	triggerUpdateParameters();
}

void SpikeTrainWidget::triggerUpdateParameters()
{
	// Rebuild the spike train
	train->rebuild(true);

	// Emit the update event
	emit updateParameters(std::set<size_t>{});
}
}
