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
# This CMakeLists file builds the AdExpSimIo library used by both GUI and CLI
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.3)
PROJECT (AdExpSimCore)

# Add the src/ and lib/ directory as include directory
INCLUDE_DIRECTORIES(
	lib/
	src/
)

# jsoncpp JSON parser
ADD_LIBRARY(jsoncpp
	lib/jsoncpp
)

# AdExpSimIo library
ADD_LIBRARY(AdExpSimIo
	src/io/JsonIo
	src/io/SurfacePlotIo
)

# Link library
TARGET_LINK_LIBRARIES(AdExpSimIo
	jsoncpp
	rt
	pthread
	AdExpSimCore
)

