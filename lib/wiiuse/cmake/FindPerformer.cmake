# - try to find the OpenGL Performer library
#
# Users may optionally supply:
#  PERFORMER_ROOT_DIR - a prefix to start searching.
#
# Cache Variables: (probably not for direct use in your scripts)
#  PERFORMER_INCLUDE_DIR
#  PERFORMER_LIBRARY
#  PERFORMER_PFUI_LIBRARY - doesn't get included in PERFORMER_LIBRARIES
#  PERFORMER_PFDU_UTIL_LIBRARY - doesn't get included in PERFORMER_LIBRARIES
#  PERFORMER_PFV_LIBRARY - doesn't get included in PERFORMER_LIBRARIES
#
# Non-cache variables you might use in your CMakeLists.txt:
#  PERFORMER_FOUND
#  PERFORMER_INCLUDE_DIRS
#  PERFORMER_LIBRARIES
#  PERFORMER_RUNTIME_LIBRARY_DIRS
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(PERFORMER_ROOT_DIR
	"${PERFORMER_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for OpenGL Performer library")


find_path(PERFORMER_INCLUDE_DIR
	NAMES
	pf.h
	HINTS
	"${PERFORMER_ROOT_DIR}"
	PATH_SUFFIXES
	include
	include/Performer
	Performer
	PATHS
	$ENV{PFROOT})
mark_as_advanced(PERFORMER_INCLUDE_DIR)

if(WIN32)
	set(_pf_libnameprefix lib)
	find_library(PERFORMER_PFDU_UTIL_LIBRARY
		NAMES
		libpfdu-util
		HINTS
		"${PERFORMER_ROOT_DIR}"
		PATH_SUFFIXES
		lib
		PATHS
		$ENV{PFROOT})
else()
	set(_pf_libnameprefix)
	find_library(PERFORMER_PFDU_LIBRARY
		NAMES
		pfdu
		HINTS
		"${PERFORMER_ROOT_DIR}"
		PATH_SUFFIXES
		lib
		PATHS
		$ENV{PFROOT})
	find_library(PERFORMER_PFUTIL_LIBRARY
		NAMES
		pfutil
		HINTS
		"${PERFORMER_ROOT_DIR}"
		PATH_SUFFIXES
		lib
		PATHS
		$ENV{PFROOT})
	if(PERFORMER_PFDU_LIBRARY AND PERFORMER_PFUTIL_LIBRARY)
		set(PERFORMER_PFDU_UTIL_LIBRARY
			${PERFORMER_PFDU_LIBRARY}
			${PERFORMER_PFUTIL_LIBRARY})
	endif()
endif()

find_library(PERFORMER_LIBRARY
	NAMES
	${_pf_libnameprefix}pf
	HINTS
	"${PERFORMER_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	PATHS
	$ENV{PFROOT})
find_library(PERFORMER_PFUI_LIBRARY
	NAMES
	${_pf_libnameprefix}pfui
	HINTS
	"${PERFORMER_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	PATHS
	$ENV{PFROOT})
find_library(PERFORMER_PFV_LIBRARY
	NAMES
	${_pf_libnameprefix}pfv
	HINTS
	"${PERFORMER_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	PATHS
	$ENV{PFROOT})

###
# Prereq: OpenGL
###

find_package(OpenGL QUIET)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Performer
	DEFAULT_MSG
	PERFORMER_LIBRARY
	PERFORMER_PFUI_LIBRARY
	PERFORMER_PFV_LIBRARY
	PERFORMER_PFDU_UTIL_LIBRARY
	PERFORMER_INCLUDE_DIR
	OPENGL_FOUND)

if(PERFORMER_FOUND)
	set(PERFORMER_INCLUDE_DIRS
		"${OPENGL_INCLUDE_DIR}"
		"${PERFORMER_INCLUDE_DIR}")
	if(PERFORMER_INCLUDE_DIR MATCHES ".*Performer.*")
		list(APPEND PERFORMER_INCLUDE_DIRS "${PERFORMER_INCLUDE_DIR}/..")
	endif()
	set(PERFORMER_LIBRARIES "${OPENGL_LIBRARY}" "${PERFORMER_LIBRARY}")
	mark_as_advanced(PERFORMER_ROOT_DIR)
endif()

mark_as_advanced(PERFORMER_LIBRARY
	PERFORMER_PFUI_LIBRARY
	PERFORMER_PFV_LIBRARY
	PERFORMER_PFDU_LIBRARY
	PERFORMER_PFUTIL_LIBRARY
	PERFORMER_PFDU_UTIL_LIBRARY
	PERFORMER_INCLUDE_DIR)
