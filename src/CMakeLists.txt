project(fss CXX C)

# Debug mode switch
SET(DEBUG_MODE 0 CACHE BOOL "Turn off to supress debug outputs")
  SET(DEBUG_FLAG "-g")
IF(DEBUG_MODE)
ENDIF(DEBUG_MODE)
IF(NOT DEBUG_MODE)
	ADD_DEFINITIONS(-DNDEBUG)
ENDIF(NOT DEBUG_MODE)

# Check for C++ compiler flags
INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-std=c++98 HAVE_STD)
IF(HAVE_STD)
	SET(STD_FLAG "-std=c++98")
ENDIF(HAVE_STD)
CHECK_CXX_COMPILER_FLAG("-pedantic -W -Wall" HAVE_PEDANTIC)
IF(HAVE_PEDANTIC)
	SET(PEDANTIC_FLAG "-pedantic -W -Wall")
ENDIF(HAVE_PEDANTIC)
CHECK_CXX_COMPILER_FLAG(-O3 HAVE_O3)

# Set C++ compiler flags
SET(CMAKE_CXX_FLAGS "${STD_FLAG} ${PEDANTIC_FLAG} ${DEBUG_FLAG}" CACHE STRING "C++ compiler flags" FORCE)

# Executable binary rrv-visualize
ADD_EXECUTABLE(fss SatSolverImpl.cpp SatProblem.cpp Scanner.cpp Formula.cpp fss.cpp)
TARGET_LINK_LIBRARIES(fss ga)

#TARGET_LINK_LIBRARIES(rrv-visualize rrv)
# make install
#INSTALL(TARGETS rrv-compute rrv-visualize DESTINATION bin)
