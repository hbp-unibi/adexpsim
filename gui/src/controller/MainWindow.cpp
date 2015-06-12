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

#include <iostream>

#include <QAction>
#include <QComboBox>
#include <QMenu>
#include <QMenuBar>
#include <QToolBox>
#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <utils/ParameterCollection.hpp>
#include <view/ParametersWidget.hpp>
#include <view/SingleGroupWidget.hpp>
#include <view/SpikeTrainWidget.hpp>

#include "MainWindow.hpp"
#include "ExplorationWindow.hpp"
#include "SimulationWindow.hpp"

namespace AdExpSim {
MainWindow::MainWindow() : params(std::make_shared<ParameterCollection>())
{
	// Create all actions and menus
	createActions();
	createMenus();
	createWidgets();

	// Resize the window
	resize(400, 600);

	// Open a new exploration and simulation window
	newExploration();
	newSimulation();

	// Set the window title and the icon
	setWindowIcon(QIcon("data/icon_main.svg"));
	setWindowTitle("Control Panel ‒ AdExpSim");
}

MainWindow::~MainWindow() {}

void MainWindow::createActions()
{
	actNewExplorationWnd = new QAction(QIcon::fromTheme("window-new"),
	                                   tr("New Exploration Window..."), this);
	connect(actNewExplorationWnd, SIGNAL(triggered()), this,
	        SLOT(newExploration()));

	actNewSimulationWnd = new QAction(QIcon::fromTheme("document-new"),
	                                  tr("New Simulation Window..."), this);
	connect(actNewSimulationWnd, SIGNAL(triggered()), this,
	        SLOT(newSimulation()));

	actOpenExploration = new QAction(QIcon::fromTheme("document-open"),
	                                 tr("Open Exploration..."), this);

	actSaveExploration = new QAction(QIcon::fromTheme("document-save"),
	                                 tr("Save Current Exploration..."), this);

	actExit = new QAction(tr("Exit"), this);
	connect(actExit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMenus()
{
	QMenu *fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(actNewExplorationWnd);
	fileMenu->addAction(actNewSimulationWnd);
	fileMenu->addSeparator();
	fileMenu->addAction(actOpenExploration);
	fileMenu->addAction(actSaveExploration);
	fileMenu->addSeparator();
	fileMenu->addAction(actExit);

	menuBar()->addMenu(fileMenu);
}

void MainWindow::createWidgets()
{
	// Create the toolbar
	toolbar = new QToolBar(this);

	// Create the model widget
	modelComboBox = new QComboBox(this);
	for (size_t i = 0; i < ParameterCollection::modelNames.size(); i++) {
		modelComboBox->addItem(
		    QString::fromStdString(ParameterCollection::modelNames[i]),
		    QVariant(int(i)));
	}
	connect(modelComboBox, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(handleModelUpdate(int)));

	// Create the evaluation widget
	evaluationComboBox = new QComboBox(this);
	for (size_t i = 0; i < ParameterCollection::evaluationNames.size(); i++) {
		evaluationComboBox->addItem(
		    QString::fromStdString(ParameterCollection::evaluationNames[i]),
		    QVariant(int(i)));
	}
	connect(evaluationComboBox, SIGNAL(currentIndexChanged(int)), this,
	        SLOT(handleEvaluationUpdate(int)));

	// Fill the toolbar
	toolbar->addAction(actNewExplorationWnd);
	toolbar->addAction(actNewSimulationWnd);
	toolbar->addSeparator();
	toolbar->addWidget(new QLabel("Model: "));
	toolbar->addWidget(modelComboBox);
	toolbar->addWidget(new QLabel(" Eval: "));
	toolbar->addWidget(evaluationComboBox);

	// Create the tool box
	QToolBox *tools = new QToolBox(this);

	// Create the spike train panel and add it to the tool box
	spikeTrainWidget = new SpikeTrainWidget(params, this);
	connect(spikeTrainWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(spikeTrainWidget, "Spike Train");

	// Create the spike train panel and add it to the tool box
	singleGroupWidget = new SingleGroupWidget(params, this);
	connect(singleGroupWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(singleGroupWidget, "Single Group Parameters");

	// Create the parameters panel and add it to the tool box
	parametersWidget = new ParametersWidget(params, this);
	connect(parametersWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(parametersWidget, "Parameters");

	// Set the tool box as central widget
	setCentralWidget(tools);
	addToolBar(Qt::TopToolBarArea, toolbar);

	// Refresh the view
	handleUpdateParameters(std::set<size_t>{});
}

void MainWindow::newExploration()
{
	ExplorationWindow *wnd = new ExplorationWindow(params);
	connect(wnd, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	windows.push_back(wnd);
	wnd->show();
}

void MainWindow::newSimulation()
{
	SimulationWindow *wnd = new SimulationWindow(params);
	connect(wnd, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	windows.push_back(wnd);
	wnd->show();
}

void MainWindow::handleUpdateParameters(std::set<size_t> dims)
{
	// Set the model and the evaluation method
	modelComboBox->setCurrentIndex(int(params->model));
	evaluationComboBox->setCurrentIndex(int(params->evaluation));

	// Forward the event to the SpikeTrain and ParametersWidget instance
	spikeTrainWidget->refresh();
	parametersWidget->handleUpdateParameters(dims);

	// Forward the event to all exploration and simulation windows
	for (auto window : windows) {
		if (window != nullptr) {
			window->handleUpdateParameters(dims);
		}
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	for (auto window : windows) {
		if (window != nullptr) {
			window->close();
		}
	}
}

void MainWindow::handleModelUpdate(int idx)
{
	params->model = ModelType(idx);
	handleUpdateParameters(std::set<size_t>{});
}

void MainWindow::handleEvaluationUpdate(int idx)
{
	params->evaluation = EvaluationType(idx);
	handleUpdateParameters(std::set<size_t>{});
}

}

