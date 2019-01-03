# - try to find the OpenAL ALUT library
#
# Users may optionally supply:
#  ALUT_ROOT_DIR - a prefix to start searching.
#
# Cache Variables: (probably not for direct use in your scripts)
#  ALUT_INCLUDE_DIR
#  ALUT_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  ALUT_FOUND
#  ALUT_INCLUDE_DIRS
#  ALUT_LIBRARIES
#  ALUT_WORKAROUND_INCLUDE_DIRS - add these to your include path with
#   include_directories(${ALUT_WORKAROUND_INCLUDE_DIRS} ${ALUT_INCLUDE_DIRS})
#   so you can always #include <AL/al.h> and #include <AL/alut.h> even on
#   Mac where the paths might differ.
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

set(ALUT_ROOT_DIR
	"${ALUT_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for ALUT library")

# Share search paths with OpenAL
if(NOT "$ENV{OPENALDIR}" STREQUAL "")
	if(NOT ALUT_ROOT_DIR)
		set(ALUT_ROOT_DIR "$ENV{OPENALDIR}")
	endif()
else()
	if(ALUT_ROOT_DIR)
		set(ENV{OPENALDIR} "${ALUT_ROOT_DIR}")
	endif()
endif()



###
# Configure ALUT
###
find_path(ALUT_INCLUDE_DIR
	NAMES
	alut.h
	HINTS
	"${ALUT_ROOT_DIR}"
	PATH_SUFFIXES
	AL
	alut
	OpenAL
	include
	include/alut
	include/freealut
	include/AL
	include/OpenAL
	PATHS
	/usr/local
	/opt/local
	/sw)
mark_as_advanced(ALUT_INCLUDE_DIR)

find_library(ALUT_LIBRARY
	NAMES
	alut
	HINTS
	"${ALUT_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	lib64
	PATHS
	/usr/local
	/opt/local
	/sw)
mark_as_advanced(ALUT_LIBRARY)

###
# Prereq: OpenAL
###

# On Mac OS X, the ALUT headers were in the OpenAL framework until 10.4.7
# If we found ALUT headers elsewhere, it's probably freealut which may
# define the same symbols as the library in the framework (?)
# so we might want to save/restore the CMake variable that controls searching
# in frameworks
find_package(OpenAL QUIET)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ALUT
	DEFAULT_MSG
	ALUT_LIBRARY
	ALUT_INCLUDE_DIR
	OPENAL_FOUND)

if(ALUT_FOUND)
	set(ALUT_INCLUDE_DIRS "${OPENAL_INCLUDE_DIR}" "${ALUT_INCLUDE_DIR}")
	set(ALUT_LIBRARIES "${OPENAL_LIBRARY}" ${ALUT_LIBRARY})
	if(APPLE)
		get_filename_component(_moddir ${CMAKE_CURRENT_LIST_FILE} PATH)
		if("${OPENAL_INCLUDE_DIR}" MATCHES "\\.framework$")
			# OpenAL is in a framework - need a workaround
			set(OPENAL_WORKAROUND_INCLUDE_DIR
				"${_moddir}/workarounds/mac-openal")
			list(APPEND
				ALUT_WORKAROUND_INCLUDE_DIRS
				"${OPENAL_WORKAROUND_INCLUDE_DIR}")
		endif()
		if("${ALUT_INCLUDE_DIR}" MATCHES "\\.framework$")
			# ALUT is in the OpenAL framework - need a workaround
			set(ALUT_WORKAROUND_INCLUDE_DIR
				"${_moddir}/workarounds/mac-alut-framework")
			list(APPEND
				ALUT_WORKAROUND_INCLUDE_DIRS
				"${ALUT_WORKAROUND_INCLUDE_DIR}")
		endif()
	endif()

	if("${ALUT_INCLUDE_DIR}" MATCHES "AL$")
		get_filename_component(_parent "${ALUT_INCLUDE_DIR}/.." ABSOLUTE)
		list(APPEND ALUT_INCLUDE_DIRS "${_parent}")
	endif()
	mark_as_advanced(ALUT_ROOT_DIR)
endif()
