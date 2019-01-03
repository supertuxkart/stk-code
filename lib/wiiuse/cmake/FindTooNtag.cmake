# - try to find tag algorithm library (built on TooN)
#
# Users may optionally supply:
#  TAG_ROOT_DIR - a prefix to start searching for the toon headers.
#
# Cache Variables: (probably not for direct use in your scripts)
#  TAG_INCLUDE_DIR
#  TAG_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  TOONTAG_FOUND
#  TOONTAG_INCLUDE_DIRS
#  TOONTAG_LIBRARIES
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(TOONTAG_ROOT_DIR
	"${TOONTAG_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for tag")

###
# Dependencies
###
if(NOT TOON_ROOT_DIR)
	set(TOON_ROOT_DIR "${TOONTAG_ROOT_DIR}")
endif()
find_package(TooN QUIET)

find_package(TR1 QUIET)
include("${TR1_USE_FILE}")

###
# Configure tag
###
find_path(TOONTAG_INCLUDE_DIR
	NAMES
	tag/helpers.h
	HINTS
	"${TOONTAG_ROOT_DIR}"
	PATH_SUFFIXES
	tag
	include)
mark_as_advanced(TOONTAG_INCLUDE_DIR)

find_library(TOONTAG_LIBRARY
	NAMES
	toontag
	HINTS
	"${TOONTAG_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	lib64)
mark_as_advanced(TOONTAG_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TooNtag
	DEFAULT_MSG
	TOONTAG_LIBRARY
	TOONTAG_INCLUDE_DIR
	TOON_FOUND)

if(TOONTAG_FOUND)
	set(TOONTAG_INCLUDE_DIRS "${TOONTAG_INCLUDE_DIR}" ${TOON_INCLUDE_DIRS})
	set(TOONTAG_LIBRARIES "${TOONTAG_LIBRARY}" ${TOON_LIBRARIES})
	mark_as_advanced(TOONTAG_ROOT_DIR)
endif()



