# - try to find the argp library/component of glibc
#
# Users may optionally supply:
#  ARGP_ROOT_DIR - a prefix to start searching.
#
# Cache Variables: (probably not for direct use in your scripts)
#  ARGP_INCLUDE_DIR
#  ARGP_LIBRARY, only defined if linking to an extra library is required
#
# Non-cache variables you might use in your CMakeLists.txt:
#  ARGP_FOUND
#  ARGP_INCLUDE_DIRS
#  ARGP_LIBRARIES
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

set(ARGP_ROOT_DIR
	"${ARGP_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for ARGP library")

###
# Configure ARGP
###
set(_check ARGP_INCLUDE_DIR)

find_path(ARGP_INCLUDE_DIR
	NAMES
	argp.h
	HINTS
	"${ARGP_ROOT_DIR}"
	PATHS
	/usr/local
	/opt/local
	/sw)
mark_as_advanced(ARGP_INCLUDE_DIR)

include(CheckFunctionExists)
check_function_exists(argp_parse ARGP_BUILTIN)

if(NOT ARGP_BUILTIN)
	find_library(ARGP_LIBRARY
		NAMES
		argp
		HINTS
		"${ARGP_ROOT_DIR}"
		PATH_SUFFIXES
		lib
		lib64
		PATHS
		/usr/local
		/opt/local
		/sw)
	list(APPEND _check ARGP_LIBRARY)
	mark_as_advanced(ARGP_LIBRARY)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(argp
	DEFAULT_MSG
	${_check})

if(ARGP_FOUND)
	set(ARGP_INCLUDE_DIRS "${ARGP_INCLUDE_DIR}")
	set(ARGP_LIBRARIES "${ARGP_LIBRARY}")
	mark_as_advanced(ARGP_ROOT_DIR)
endif()
