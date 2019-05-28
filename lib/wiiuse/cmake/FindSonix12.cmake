# - try to find Sonix 1.2 library
# Requires VPR 2.0 and GMTL (thus FindVPR20.cmake and FindGMTL.cmake)
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 2.2 - you probably want to use
# find_package(VRJuggler22) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler22.cmake for more information.
#
#  SONIX12_LIBRARY_DIR, library search path
#  SONIX12_INCLUDE_DIR, include search path
#  SONIX12_LIBRARY, the library to link against
#  SONIX12_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  SONIX12_ROOT_DIR - A directory prefix to search
#                     (a path that contains include/ as a subdirectory)
#
# This script will use Flagpoll, if found, to provide hints to the location
# of this library, but does not use the compiler flags returned by Flagpoll
# directly.
#
# The VJ_BASE_DIR environment variable is also searched (preferentially)
# when searching for this component, so most sane build environments should
# "just work."  Note that you need to manually re-run CMake if you change
# this environment variable, because it cannot auto-detect this change
# and trigger an automatic re-run.
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


set(_HUMAN "Sonix 1.2")
set(_RELEASE_NAMES sonix-1_2 libsonix-1_2)
set(_DEBUG_NAMES sonix_d-1_2 libsonix_d-1_2)
set(_DIR sonix-1.2)
set(_HEADER snx/sonix.h)
set(_FP_PKG_NAME sonix)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(Sonix12_FIND_QUIETLY)
	set(_FIND_FLAGS "QUIET")
else()
	set(_FIND_FLAGS "")
endif()

# Try flagpoll.
find_package(Flagpoll QUIET)

if(FLAGPOLL)
	flagpoll_get_include_dirs(${_FP_PKG_NAME} NO_DEPS)
	flagpoll_get_library_dirs(${_FP_PKG_NAME} NO_DEPS)
endif()

set(SONIX12_ROOT_DIR
	"${SONIX12_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for Sonix")
if(DEFINED VRJUGGLER22_ROOT_DIR)
	mark_as_advanced(SONIX12_ROOT_DIR)
endif()
if(NOT SONIX12_ROOT_DIR)
	set(SONIX12_ROOT_DIR "${VRJUGGLER22_ROOT_DIR}")
endif()

set(_ROOT_DIR "${SONIX12_ROOT_DIR}")

find_path(SONIX12_INCLUDE_DIR
	${_HEADER}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_INCLUDE_DIRS}
	PATH_SUFFIXES
	${_DIR}
	include/${_DIR}
	include/
	DOC
	"Path to ${_HUMAN} includes root")

find_library(SONIX12_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(SONIX12_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(SONIX12)

# Dependencies
if(NOT VPR20_FOUND)
	find_package(VPR20 ${_FIND_FLAGS})
endif()

if(NOT GMTL_FOUND)
	find_package(GMTL ${_FIND_FLAGS})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sonix12
	DEFAULT_MSG
	SONIX12_LIBRARY
	SONIX12_INCLUDE_DIR
	VPR20_FOUND
	VPR20_LIBRARIES
	VPR20_INCLUDE_DIR
	GMTL_FOUND
	GMTL_INCLUDE_DIR)

if(SONIX12_FOUND)
	set(_DEPS ${VPR20_LIBRARIES})

	set(SONIX12_INCLUDE_DIRS ${SONIX12_INCLUDE_DIR})
	list(APPEND
		SONIX12_INCLUDE_DIRS
		${VPR20_INCLUDE_DIRS}
		${GMTL_INCLUDE_DIRS})

	clean_directory_list(SONIX12_INCLUDE_DIRS)

	if(VRJUGGLER22_CREATE_IMPORTED_TARGETS)
		create_imported_target(SONIX12 ${_DEPS})
	else()
		clean_library_list(SONIX12_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(SONIX12_ROOT_DIR)
endif()

mark_as_advanced(SONIX12_LIBRARY_RELEASE
	SONIX12_LIBRARY_DEBUG
	SONIX12_INCLUDE_DIR)
