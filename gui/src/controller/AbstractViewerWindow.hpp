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
 * @file AbstractViewerWindow.hpp
 *
 * Contains a base class for both the SimulationWindow and ExplorationWindow
 * classes -- the AbstractViewerWindow provides access to the parameters and
 * signals for parameter updates.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_ABSTRACT_VIEWER_WINDOW_HPP_
#define _ADEXPSIM_ABSTRACT_VIEWER_WINDOW_HPP_

#include <memory>
#include <set>

#include <QMainWindow>

namespace AdExpSim {

class Parameters;
class SpikeTrain;

/**
 * The ExplorationWindow class is a controller object, containing an exploration
 * model and the corresponding view widget. The ExplorationWindow is responsible
 * for exchanging events between these classes.
 */
class AbstractViewerWindow : public QMainWindow {
	Q_OBJECT

protected:
	/**
	 * Current parameters.
	 */
	std::shared_ptr<Parameters> params;

	/**
	 * Current spike train.
	 */
	std::shared_ptr<SpikeTrain> train;

signals:
	/**
	 * Emitted whenever the user updates the parameters in the view.
	 */
	void updateParameters(std::set<size_t> dims);

public:
	/**
	 * Constructor of the AbstractViewerWindow class.
	 *
	 * @param params is a shared instance containing the current neuron
	 * parameters.
	 * @param train is a shared instance containing spike train being used for
	 * the experimental runs.
	 * @param is the parent object.
	 */
	AbstractViewerWindow(std::shared_ptr<Parameters> params,
	                  std::shared_ptr<SpikeTrain> train,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the AbstractViewerWindow class.
	 */
	~AbstractViewerWindow();

	/**
	 * Should be called whenever the neuron parameters or the spike train
	 * have been updated.
	 *
	 * @param dims contains the indices of the dimensions that have been
	 * updated. If empty, this indicates that "everything" has changed.
	 */
	virtual void handleUpdateParameters(std::set<size_t> dims) = 0;
};
}

#endif /* _ADEXPSIM_ABSTRACT_VIEWER_WINDOW_HPP_ */

