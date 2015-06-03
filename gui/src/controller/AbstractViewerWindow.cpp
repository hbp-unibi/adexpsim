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

#include "AbstractViewerWindow.hpp"

#include <utils/ParameterCollection.hpp>

namespace AdExpSim {

AbstractViewerWindow::AbstractViewerWindow(
    std::shared_ptr<ParameterCollection> params, QWidget *parent)
    : QMainWindow(parent), params(params)
{
	setAttribute(Qt::WA_DeleteOnClose, true);
}

AbstractViewerWindow::~AbstractViewerWindow()
{
	// Only needed for the shared pointers
}
}

