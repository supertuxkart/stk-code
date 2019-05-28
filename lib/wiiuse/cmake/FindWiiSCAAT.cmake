# - try to find the Wii SCAAT library
#
# Users may optionally supply:
#  WIISCAAT_ROOT_DIR - a prefix to start searching for the headers.
#
# Cache Variables: (probably not for direct use in your scripts)
#  WIISCAAT_INCLUDE_DIR
#  WIISCAAT_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  WIISCAAT_FOUND
#  WIISCAAT_INCLUDE_DIRS
#  WIISCAAT_LIBRARIES
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

set(WIISCAAT_ROOT_DIR
	"${WIISCAAT_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for Wii SCAAT module")

###
# Prereq: tag
###
if(NOT TOONTAG_ROOT_DIR)
	set(TOONTAG_ROOT_DIR "${WIISCAAT_ROOT_DIR}")
endif()
find_package(TooNtag QUIET)

###
# Configure tag
###
find_path(WIISCAAT_INCLUDE_DIR
	NAMES
	HeadCollection.h
	HINTS
	"${WIISCAAT_ROOT_DIR}"
	PATH_SUFFIXES
	include)
mark_as_advanced(WIISCAAT_INCLUDE_DIR)

find_library(WIISCAAT_LIBRARY
	NAMES
	wiiscaattracker
	HINTS
	"${WIISCAAT_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	lib64)
mark_as_advanced(WIISCAAT_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WiiSCAAT
	DEFAULT_MSG
	WIISCAAT_LIBRARY
	WIISCAAT_INCLUDE_DIR
	TOONTAG_FOUND)

if(WIISCAAT_FOUND)
	set(WIISCAAT_INCLUDE_DIRS
		"${WIISCAAT_INCLUDE_DIR}"
		${TOONTAG_INCLUDE_DIRS})
	set(WIISCAAT_LIBRARIES "${WIISCAAT_LIBRARY}" ${TOONTAG_LIBRARIES})
	mark_as_advanced(WIISCAAT_ROOT_DIR)
endif()
