# Copyright (C) 2008 Kamil Dudka <xdudka00@stud.fit.vutbr.cz>
#
# This file is part of fss (Fast SAT Solver).
#
# fss is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# fss is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with fss.  If not, see <http://www.gnu.org/licenses/>.

project(fss CXX C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# Debug mode switch
SET(DEBUG_MODE 0 CACHE BOOL "Turn off to supress debug outputs")
  SET(DEBUG_FLAG "-g")
IF(DEBUG_MODE)
ENDIF(DEBUG_MODE)
IF(NOT DEBUG_MODE)
	ADD_DEFINITIONS(-DNDEBUG)
ENDIF(NOT DEBUG_MODE)

# Check GAlib availability
find_library(GALIB ga ${GALIB_DIR}/ga)
IF(NOT EXISTS ${GALIB})
  MESSAGE(FATAL_ERROR "Cannot find GAlib library")
ENDIF(NOT EXISTS ${GALIB})

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
SET(CMAKE_CXX_FLAGS "${STD_FLAG} ${PEDANTIC_FLAG} ${DEBUG_FLAG} -I${GALIB_DIR}" CACHE STRING "C++ compiler flags" FORCE)

# Executable binary rrv-visualize
ADD_EXECUTABLE(fss
  fss.cpp fssIO.cpp
  SatProblem.cpp Scanner.cpp Formula.cpp
  SatSolver.cpp SatSolverObserver.cpp
  BlindSatSolver.cpp GaSatSolver.cpp)
TARGET_LINK_LIBRARIES(fss ${GALIB})

ADD_EXECUTABLE(fss-satgen fss-satgen.cpp)

#TARGET_LINK_LIBRARIES(rrv-visualize rrv)
# make install
#INSTALL(TARGETS rrv-compute rrv-visualize DESTINATION bin)
