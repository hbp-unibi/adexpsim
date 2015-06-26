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
# This CMakeLists file builds the AdExpSimCore library, the CLI applications
# and the GUI application
#

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (AdExpSim)

# Place all binaries and libraries in the same folder
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# Include the core and io source directory as include directory
INCLUDE_DIRECTORIES(core/src)
INCLUDE_DIRECTORIES(io/src)

# Add the subprojects
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(cli)
ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(io)
