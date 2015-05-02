# CMakeLists for the AdExpSim tool
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (AdExpSim)

# Enable C++11 and all warnings, enable vector optimizations for this processor
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic-errors -std=c++11 -march=native -ffast-math")

# AdExpSim library
ADD_LIBRARY(AdExpSimCore
	src/Model
	src/Parameters
	src/Spike
	src/State
	src/Types
)

# AdExpSim executable
ADD_EXECUTABLE(AdExpSim
	src/AdExpSim
)

TARGET_LINK_LIBRARIES(AdExpSim
	AdExpSimCore
)

INSTALL_TARGETS(/bin AdExpSim)
