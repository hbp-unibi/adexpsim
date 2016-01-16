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

CMAKE_MINIMUM_REQUIRED (VERSION 3.3)
PROJECT (AdExpSim)

# Place all binaries and libraries in the same folder
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# Enable all warnings on MSVC and GCC/Clang/Intel
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR
		("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR
		("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel"))

	# Enable warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra  -Wstrict-aliasing=2\
			-pedantic")

	# Enable C++14 and all warnings, enable vector optimizations for this
	# processor, make sure fast math is always enabled
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -march=native\
			-ffast-math -fstrict-aliasing")
endif()

# Set a default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Include the core and io source directory as include directory
INCLUDE_DIRECTORIES(core/src)
INCLUDE_DIRECTORIES(io/src)

# Add the subprojects
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(cli)
ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(io)
