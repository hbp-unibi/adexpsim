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
#include <fstream>

#include <QAction>
#include <QComboBox>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBox>
#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>

#include <io/JsonIo.hpp>
#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <utils/ParameterCollection.hpp>
#include <view/OptimizationWidget.hpp>
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
	actReset = new QAction(QIcon::fromTheme("document-new"),
	                       tr("Reset parameters"), this);
	connect(actReset, SIGNAL(triggered()), this, SLOT(reset()));

	actNewExplorationWnd = new QAction(QIcon::fromTheme("window-new"),
	                                   tr("New exploration window..."), this);
	connect(actNewExplorationWnd, SIGNAL(triggered()), this,
	        SLOT(newExploration()));

	actNewSimulationWnd = new QAction(QIcon::fromTheme("document-new"),
	                                  tr("New simulation window..."), this);
	connect(actNewSimulationWnd, SIGNAL(triggered()), this,
	        SLOT(newSimulation()));

	actOpen =
	    new QAction(QIcon::fromTheme("document-open"), tr("Open..."), this);
	connect(actOpen, SIGNAL(triggered()), this, SLOT(handleOpen()));

	actSaveParameters = new QAction(QIcon::fromTheme("document-properties"),
	                                tr("Save parameters..."), this);
	connect(actSaveParameters, SIGNAL(triggered()), this,
	        SLOT(handleSaveParameters()));

	actSaveExploration = new QAction(QIcon::fromTheme("document-save-as"),
	                                 tr("Save exploration..."), this);

	actExportPyNNNest = new QAction(QIcon("data/PyNN_logo.png"),
	                                tr("Export to PyNN for NEST"), this);
	connect(actExportPyNNNest, SIGNAL(triggered()), this,
	        SLOT(handleExportPyNNNest()));

	actExportPyNNESS = new QAction(QIcon("data/icon_hw.png"),
	                               tr("Export to PyNN for ESS"), this);
	connect(actExportPyNNESS, SIGNAL(triggered()), this,
	        SLOT(handleExportPyNNESS()));

	actExit = new QAction(tr("Exit"), this);
	connect(actExit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMenus()
{
	QMenu *fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(actNewExplorationWnd);
	fileMenu->addAction(actNewSimulationWnd);
	fileMenu->addSeparator();
	fileMenu->addAction(actReset);
	fileMenu->addSeparator();
	fileMenu->addAction(actOpen);
	fileMenu->addSeparator();
	fileMenu->addAction(actSaveParameters);
	fileMenu->addAction(actSaveExploration);
	fileMenu->addSeparator();
	fileMenu->addAction(actExit);

	QMenu *exportMenu = new QMenu(tr("&Export"), this);
	exportMenu->addAction(actExportPyNNNest);
	exportMenu->addAction(actExportPyNNESS);

	menuBar()->addMenu(fileMenu);
	menuBar()->addMenu(exportMenu);
}

void MainWindow::createWidgets()
{
	// Create the toolbars
	fileToolbar = new QToolBar(this);
	simToolbar = new QToolBar(this);

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

	// Build the export toolbutton
	fileToolbar->addAction(actOpen);
	fileToolbar->addSeparator();
	fileToolbar->addAction(actSaveParameters);
	fileToolbar->addAction(actSaveExploration);
	fileToolbar->addSeparator();
	fileToolbar->addAction(actExportPyNNNest);
	fileToolbar->addAction(actExportPyNNESS);

	// Fill the toolbar
	simToolbar->addAction(actNewExplorationWnd);
	simToolbar->addAction(actNewSimulationWnd);
	simToolbar->addSeparator();
	simToolbar->addWidget(new QLabel("Model: "));
	simToolbar->addWidget(modelComboBox);
	simToolbar->addWidget(new QLabel(" Eval: "));
	simToolbar->addWidget(evaluationComboBox);

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

	// Create the optimization widget and add it to the tool box
	optimizationWidget = new OptimizationWidget(params, this);
	connect(optimizationWidget, SIGNAL(updateParameters(std::set<size_t>)),
	        this, SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(optimizationWidget, "Optimization");

	// Create the parameters panel and add it to the tool box
	parametersWidget = new ParametersWidget(params, this);
	connect(parametersWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(parametersWidget, "Parameters");

	// Set the tool box as central widget
	setCentralWidget(tools);
	addToolBar(Qt::TopToolBarArea, fileToolbar);
	addToolBarBreak(Qt::TopToolBarArea);
	addToolBar(Qt::TopToolBarArea, simToolbar);

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

void MainWindow::reset()
{
	*params = ParameterCollection();
	handleUpdateParameters(std::set<size_t>{});
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

void MainWindow::handleOpen()
{
	QString fileName = QFileDialog::getOpenFileName(
	    this, QString("Load parameters"), QString(), "JSON Files (*.json)");
	if (!fileName.isEmpty()) {
		std::ifstream jsonStream(fileName.toStdString());
		if (JsonIo::loadGenericParameters(jsonStream, *params)) {
			handleUpdateParameters(std::set<size_t>{});
		} else {
			QMessageBox::critical(this, "Error while loading file",
			                      "The given file could not be opened - either "
			                      "due to syntax errors or an incompatible "
			                      "file format.");
		}
	}
}

static QString replaceFileExt(const QString &s, const QString &ext)
{
	int i = s.lastIndexOf(".");
	if (i == -1) {
		i = s.size();
	}
	return s.left(i) + "." + ext;
}

void MainWindow::handleSaveParameters()
{
	QString fileName = QFileDialog::getSaveFileName(
	    this, QString("Save parameters"), QString(), "JSON Files (*.json)");
	if (!fileName.isEmpty()) {
		std::ofstream jsonStream(
		    replaceFileExt(fileName, "json").toStdString());
		JsonIo::storeParameters(jsonStream, *params);
	}
}

void MainWindow::handleExportPyNN(bool nest)
{
	QString fileName = QFileDialog::getSaveFileName(
	    this, QString("Export ") + (nest ? "NEST" : "ESS") + " PyNN parameters",
	    QString(), "JSON Files (*.json)");
	if (!fileName.isEmpty()) {
		std::ofstream jsonStream(
		    replaceFileExt(fileName, "json").toStdString());
		std::ofstream pyStream(replaceFileExt(fileName, "py").toStdString());

		JsonIo::storePyNNModel(jsonStream, params->params, params->model);
		if (nest) {
			JsonIo::storePyNNSetupNEST(pyStream, params->params, params->model);
		} else {
			JsonIo::storePyNNSetupESS(pyStream, params->params, params->model);
		}
	}
}

void MainWindow::handleExportPyNNESS() { handleExportPyNN(false); }

void MainWindow::handleExportPyNNNest() { handleExportPyNN(true); }
}

