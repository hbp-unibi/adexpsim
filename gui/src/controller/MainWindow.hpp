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
 * @file MainWindow.hpp
 *
 * Contains the declaration of the MainWindow class, which manages the GUI 
 * application.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_MAIN_WINDOW_HPP_
#define _ADEXPSIM_MAIN_WINDOW_HPP_

#include <memory>
#include <set>
#include <vector>

#include <QPointer>
#include <QMainWindow>

class QAction;
class QComboBox;
class QToolBar;
class QMenu;

namespace AdExpSim {

class ParameterCollection;
class ParametersWidget;
class SingleGroupWidget;
class SpikeTrainWidget;
class AbstractViewerWindow;

/**
 * The MainWindow class is the main controller object, distributing the 
 * experimental setup and results between all components of the application.
 */
class MainWindow: public QMainWindow {
	Q_OBJECT

private:
	/* Actions */
	QAction *actNewExplorationWnd;
	QAction *actNewSimulationWnd;
	QAction *actOpen;
	QAction *actSaveExploration;
	QAction *actSaveParameters;
	QAction *actExportPynnNest;
	QAction *actExportPynnESS;
	QAction *actExit;

	/* Experiment parameters */
	std::shared_ptr<ParameterCollection> params;

	/* Viewer windows */
	std::vector<QPointer<AbstractViewerWindow>> windows;

	/* Parameter widgets */
	SingleGroupWidget *singleGroupWidget;
	SpikeTrainWidget *spikeTrainWidget;
	ParametersWidget *parametersWidget;
	QToolBar *fileToolbar;
	QToolBar *simToolbar;
	QComboBox *modelComboBox;
	QComboBox *evaluationComboBox;

	void createActions();
	void createMenus();
	void createWidgets();

private slots:
	void newExploration();
	void newSimulation();
	void handleUpdateParameters(std::set<size_t> dims);
	void handleModelUpdate(int);
	void handleEvaluationUpdate(int);
	void handleOpen();
	void handleExportPynnNest();
	void handleExportPynnESS();
	void handleExportPynn(bool nest);

protected:
	void closeEvent(QCloseEvent *event) override;

public:
	MainWindow();
	~MainWindow();
};

}

#endif /* _ADEXPSIM_MAIN_WINDOW_HPP_ */
