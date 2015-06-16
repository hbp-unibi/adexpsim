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

#include "AbstractViewerWindow.hpp"

class QAction;
class QToolbar;
class QComboBox;

namespace AdExpSim {

class ExplorationWidget;
class IncrementalExploration;

/**
 * The ExplorationWindow class is a controller object, containing an exploration
 * model and the corresponding view widget. The ExplorationWindow is responsible
 * for exchanging events between these classes.
 */
class ExplorationWindow : public AbstractViewerWindow {
	Q_OBJECT

private:
	/**
	 * Set to true if the view of the exploration widget should be fitted the
	 * next time data is received from the incremental exploration.
	 */
	bool fitView;

	/**
	 * Set to true if the view is locked and there has been an update.
	 */
	bool hadUpdate;

	/* Actions */
	QAction *actLockView;
	QAction *actSavePDF;
	QAction *actSaveExploration;

	/* Widgets and model */
	std::shared_ptr<Exploration> exploration;
	IncrementalExploration *incrementalExploration;
	QToolBar *toolbar;
	QComboBox *resolutionComboBox;
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

	/**
	 * Called whenever the exploration widget has changed its range.
	 */
	void handleUpdateRange(size_t dimX, size_t dimY, Val minX, Val maxX,
	                               Val minY, Val maxY);

	/**
	 * Called whenever the "lock view" action is triggered.
	 */
	void handleLockView(bool checked);

	/**
	 * Called whenever the incrementalExploration reports some progress.
	 */
	void handleProgress(float p, bool show);

	/**
	 * Called whenever the resolution combobox is updated-
	 */
	void handleUpdateResolution(int index);

	/**
	 * Called when the "save as pdf" button is pressed.
	 */
	void handleSavePdf();

public slots:
	void lock();
	void unlock();

public:
	/**
	 * Constructor of the ExplorationWindow class.
	 */
	ExplorationWindow(std::shared_ptr<ParameterCollection> params,
	                  QWidget *parent = nullptr);

	/**
	 * Destructor of the ExplorationWindow class.
	 */
	~ExplorationWindow();

	/**
	 * Should be called whenever the neuron parameters or the spike train
	 * have been updated.
	 *
	 * @param dims contains the indices of the dimensions that have been
	 * updated. If empty, this indicates that "everything" has changed.
	 */
	void handleUpdateParameters(std::set<size_t> dims) override;

	/**
	 * Returns true if this window is currently locked.
	 */
	bool isLocked();
};
}

#endif /* _ADEXPSIM_EXPLORATION_WINDOW_HPP_ */
