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
 * @file ExplorationWindow.hpp
 *
 * Contains the ExplorationWindow class, which is used to control a single
 * parameter space exploration in two dimensions.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_EXPLORATION_WINDOW_HPP_
#define _ADEXPSIM_EXPLORATION_WINDOW_HPP_

#include <exploration/Exploration.hpp>

#include <memory>
#include <set>

#include <QMainWindow>

namespace AdExpSim {

class Parameters;
class SpikeTrain;
class ExplorationWidget;
class IncrementalExploration;

/**
 * The ExplorationWindow class is a controller object, containing an exploration
 * model and the corresponding view widget. The ExplorationWindow is responsible
 * for exchanging events between these classes.
 */
class ExplorationWindow : public QMainWindow {
	Q_OBJECT

private:
	/**
	 * Set to true if the view of the exploration widget should be fitted the
	 * next time data is received from the incremental exploration.
	 */
	bool fitView;

	/* Actions */

	/* Experiment parameters */
	std::shared_ptr<Parameters> params;
	std::shared_ptr<SpikeTrain> train;

	/* Widgets and model */
	std::shared_ptr<Exploration> exploration;
	IncrementalExploration *incrementalExploration;
	ExplorationWidget *explorationWidget;

	void createModel();
	void createWidgets();

private slots:
	/**
	 * Called whenever new Exploration data is available from the
	 * IncrementalExploration.
	 *
	 * @param data is the available exploration data.
	 */
	void handleExplorationData(Exploration data);

	/**
	 * Called whenever the exploration widget has updated itself.
	 */
	void handleInternalUpdateParameters(std::set<size_t> dims);

public slots:
	/**
	 * Should be called whenever the neuron parameters or the spike train
	 * have been updated.
	 *
	 * @param dims contains the indices of the dimensions that have been
	 * updated. If empty, this indicates that "everything" has changed.
	 */
	void handleUpdateParameters(std::set<size_t> dims);

signals:
	/**
	 * Emitted whenever the user updates the parameters in the exploration view.
	 */
	void updateParameters(std::set<size_t> dims);

public:
	/**
	 * Constructor of the ExplorationWindow class.
	 *
	 * @param params is a shared instance containing the current neuron
	 * parameters.
	 * @param train is a shared instance containing spike train being used for
	 * the experimental runs.
	 * @param is the parent object.
	 */
	ExplorationWindow(std::shared_ptr<Parameters> params,
	                  std::shared_ptr<SpikeTrain> train,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the ExplorationWindow class.
	 */
	~ExplorationWindow();
};
}

#endif /* _ADEXPSIM_EXPLORATION_WINDOW_HPP_ */
