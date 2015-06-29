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
# This CMakeLists file builds various command line programs ontop of the
# AdExpSimCore library
#

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (AdExSimCore)

# Enable C++11 and all warnings, enable vector optimizations for this processor
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
	-Wall -pedantic-errors -std=c++11 -march=native -ffast-math")

# AdExpSim executable
ADD_EXECUTABLE(AdExpSim
	src/AdExpSim
)

TARGET_LINK_LIBRARIES(AdExpSim
	AdExpSimCore
)

# AdExploration executable
ADD_EXECUTABLE(AdExploration
	src/AdExploration
)

TARGET_LINK_LIBRARIES(AdExploration
	AdExpSimCore
)

# AdExpIntegratorBenchmark executable
ADD_EXECUTABLE(AdExpIntegratorBenchmark
	src/AdExpIntegratorBenchmark
)

TARGET_LINK_LIBRARIES(AdExpIntegratorBenchmark
	AdExpSimCore
)

# AdExpOptimization executable
ADD_EXECUTABLE(AdExpOptimization
	src/AdExpOptimization
)

TARGET_LINK_LIBRARIES(AdExpOptimization
	AdExpSimCore
	AdExpSimIo
)