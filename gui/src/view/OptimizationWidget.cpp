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

#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QStringList>
#include <QTimer>

#include <utils/ParameterCollection.hpp>

#include "OptimizationWidget.hpp"

namespace AdExpSim {
OptimizationWidget::OptimizationWidget(
    std::shared_ptr<ParameterCollection> params, QWidget *parent)
    : QWidget(parent), params(params)
{
	// Create the optimization job
	job = new OptimizationJob(params, this);
	connect(job, SIGNAL(progress(bool, size_t, size_t, float,
	                             std::vector<OptimizationResult>)),
	        this, SLOT(handleProgress(bool, size_t, size_t, float,
	                                  std::vector<OptimizationResult>)));

	// Create the update timer
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(rebuildTable()));

	// Build the table widget
	tableWidget = new QTableWidget(this);
	tableWidget->setColumnCount(WorkingParameters::Size + 1);
	tableWidget->horizontalHeader()->setSectionResizeMode(
	    QHeaderView::ResizeToContents);
	tableWidget->verticalHeader()->setSectionResizeMode(
	    QHeaderView::ResizeToContents);
	QStringList labels({"Eval"});
	for (size_t i = 0; i < WorkingParameters::Size; i++) {
		if (WorkingParameters::linear[i]) {
			labels.append(
			    QString::fromStdString(WorkingParameters::originalNames[i]));
		} else {
			labels.append(QString::fromStdString(WorkingParameters::names[i]));
		}
	}
	tableWidget->setHorizontalHeaderLabels(labels);
	connect(tableWidget, SIGNAL(cellDoubleClicked(int, int)), this,
	        SLOT(handleCellDoubleClicked(int, int)));

	// Create the other components
	chkOptimizeHw = new QCheckBox("Apply hardware constraints", this);
	chkOptimizeHw->setChecked(true);
	lblNIt = new QLabel("nIt:", this);
	lblNInput = new QLabel("nInput:", this);
	lblEval = new QLabel("eval:", this);

	btnOptimize = new QPushButton("Optimize", this);
	connect(btnOptimize, SIGNAL(clicked()), this,
	        SLOT(handleOptimizeClicked()));

	// Create the layout and add the widgets
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(tableWidget);
	layout->addWidget(chkOptimizeHw);
	layout->addWidget(lblNIt);
	layout->addWidget(lblNInput);
	layout->addWidget(lblEval);
	layout->addWidget(btnOptimize);
}

OptimizationWidget::~OptimizationWidget()
{
	// Required for the shared pointer
}

void OptimizationWidget::handleCellDoubleClicked(int row, int column)
{
	const ssize_t idx = optimized.size() - (row + 1);
	if (idx >= 0 && size_t(idx) < optimized.size()) {
		params->params = optimized[idx].params.toParameters(params->params);
		emit updateParameters(std::set<size_t>{});
	}
}

void OptimizationWidget::handleOptimizeClicked()
{
	if (!job->isActive()) {
		job->start(chkOptimizeHw->isChecked());
	} else {
		job->abort();
	}
	btnOptimize->setText("Wait...");
	btnOptimize->setEnabled(false);
}

void OptimizationWidget::handleProgress(bool done, size_t nIt, size_t nInput,
                                        float eval,
                                        std::vector<OptimizationResult> output)
{
	// Set the correct button text
	btnOptimize->setEnabled(true);
	if (done) {
		btnOptimize->setText("Optimize");
	} else {
		btnOptimize->setText("Cancel");
	}

	// Show the progress
	lblNIt->setText(QString("nIt: ") + QString::number(nIt));
	lblNInput->setText(QString("nInput: ") + QString::number(nInput));
	lblEval->setText(QString("eval: ") + QString::number(eval));

	// Copy the output to the optimized parameters
	optimized = output;
	if (done) {
		rebuildTable();
		updateTimer->stop();
	} else if (!updateTimer->isActive()) {
		updateTimer->start(1000);
	}
}

void OptimizationWidget::rebuildTable()
{
	tableWidget->setRowCount(optimized.size());
	for (size_t i = 0; i < optimized.size(); i++) {
		const size_t rowIdx = optimized.size() - (i + 1);
		tableWidget->setItem(
		    rowIdx, 0,
		    new QTableWidgetItem(QString::number(optimized[i].eval)));
		for (size_t j = 0; j < WorkingParameters::Size; j++) {
			tableWidget->setItem(
			    rowIdx, j + 1,
			    new QTableWidgetItem(QString::number(
			        optimized[i].params.workingToPlot(j, params->params))));
		}
	}
}
}
