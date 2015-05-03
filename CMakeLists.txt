# CMakeLists for the AdExpSim tool
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (AdExpSim)

# Enable C++11 and all warnings, enable vector optimizations for this processor
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic-errors -std=c++11 -march=native -ffast-math")

INCLUDE_DIRECTORIES(
	src/
)

# AdExpSimUtils library
ADD_LIBRARY(AdExpSimUtils
	src/utils/Timer
	src/utils/Types
	src/utils/Vector
)

TARGET_LINK_LIBRARIES(AdExpSimUtils
	rt
)

# AdExpSimCore library
ADD_LIBRARY(AdExpSimCore
	src/core/Model
	src/core/Parameters
	src/core/Recorder
	src/core/Spike
	src/core/State
)

# AdExpSimExploration library
ADD_LIBRARY(AdExpSimExploration
	src/exploration/Evaluation
)

TARGET_LINK_LIBRARIES(AdExpSimExploration
	AdExpSimCore
)

# AdExpSim executable
ADD_EXECUTABLE(AdExpSim
	src/AdExpSim
)

TARGET_LINK_LIBRARIES(AdExpSim
	AdExpSimCore
	AdExpSimUtils
)

# AdExploration executable
ADD_EXECUTABLE(AdExploration
	src/AdExploration
)

TARGET_LINK_LIBRARIES(AdExploration
	AdExpSimExploration
)

INSTALL_TARGETS(/bin AdExpSim)
