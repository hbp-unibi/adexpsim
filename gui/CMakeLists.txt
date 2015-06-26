#  AdExpSim -- Simulator for the AdExp model
#  Copyright (C) 2015  Andreas Stöckel
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#
# This CMakeLists file builds the GUI application which requires Qt5 and a
# recent CMake version.
#

# CMakeLists for the AdExpSimGui tool
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.11)
PROJECT (AdExpSimGui)

# Enable C++11 and all warnings, enable vector optimizations for this processor
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
	-Wall -pedantic-errors -std=c++11 -march=native -ffast-math")

# Find includes in corresponding build directories
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
SET(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
FIND_PACKAGE(Qt5 COMPONENTS Widgets PrintSupport)

# Add the library and source directories to the search path
INCLUDE_DIRECTORIES(lib/)
INCLUDE_DIRECTORIES(src/)

# Compile the QCustomPlot library
ADD_LIBRARY(qcustomplot
	lib/qcustomplot
)

TARGET_LINK_LIBRARIES(qcustomplot
	Qt5::Widgets
	Qt5::PrintSupport
)

# Tell CMake to create the helloworld executable
ADD_EXECUTABLE(AdExpSimGui
	src/controller/AbstractViewerWindow
	src/controller/MainWindow
	src/controller/ExplorationWindow
	src/controller/SimulationWindow
	src/model/NeuronSimulation
	src/model/IncrementalExploration
	src/view/Colors
	src/view/ExplorationWidget
	src/view/ExplorationWidgetInvalidOverlay
	src/view/ExplorationWidgetGradients
	src/view/ExplorationWidgetCrosshair
	src/view/NeuronSimulationWidget
	src/view/ParameterWidget
	src/view/ParametersWidget
	src/view/SingleGroupWidget
	src/view/SpikeTrainWidget
	src/view/SpikeWidget
	src/main
)

# Use the Widgets module from Qt 5.
TARGET_LINK_LIBRARIES(AdExpSimGui
	AdExpSimCore
	AdExpSimIo
	qcustomplot
	Qt5::Widgets
)

# Copy the data directory to the bin directory
FILE(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data/)
