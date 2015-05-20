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
 * Contains the declaration of the MainWindow class, which contains the
 * complete GUI application.
 */

#ifndef _ADEXPSIM_MAIN_WINDOW_HPP_
#define _ADEXPSIM_MAIN_WINDOW_HPP_

#include <memory>

#include <QMainWindow>

class QDockWidget;

namespace AdExpSim {

class NeuronSimulationWidget;
class IncrementalExploration;
class Exploration;
class ExplorationWidget;
class Parameters;

class MainWindow: public QMainWindow {
	Q_OBJECT

private:
	bool fitExploration;
	std::shared_ptr<Parameters> parameters;

	IncrementalExploration *exploration;
	QDockWidget *explorationDockWidget;
	ExplorationWidget *explorationWidget;

	QDockWidget *simulationDockWidget;
	NeuronSimulationWidget *simulationWidget;

private slots:
	void data(const Exploration &exploration);

public:
	MainWindow();
	~MainWindow();
};

}

#endif /* _ADEXPSIM_MAIN_WINDOW_HPP_ */
