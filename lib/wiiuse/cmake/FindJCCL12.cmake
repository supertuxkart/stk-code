# - try to find JCCL 1.2 library
# Requires VPR 2.0 (thus FindVPR20.cmake)
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 2.2 - you probably want to use
# find_package(VRJuggler22) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler22.cmake for more information.
#
#  JCCL12_LIBRARY_DIR, library search path
#  JCCL12_INCLUDE_DIR, include search path
#  JCCL12_LIBRARY, the library to link against
#  JCCL12_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  JCCL12_ROOT_DIR - A directory prefix to search
#                    (a path that contains include/ as a subdirectory)
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


set(_HUMAN "JCCL 1.2")
set(_RELEASE_NAMES jccl-1_2 libjccl-1_2)
set(_DEBUG_NAMES jccl_d-1_2 libjccl_d-1_2)
set(_DIR jccl-1.2)
set(_HEADER jccl/jcclConfig.h)
set(_FP_PKG_NAME jccl)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(JCCL12_FIND_QUIETLY)
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

set(JCCL12_ROOT_DIR
	"${JCCL12_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for JCCL")
if(DEFINED VRJUGGLER22_ROOT_DIR)
	mark_as_advanced(JCCL12_ROOT_DIR)
endif()
if(NOT JCCL12_ROOT_DIR)
	set(JCCL12_ROOT_DIR "${VRJUGGLER22_ROOT_DIR}")
endif()

set(_ROOT_DIR ${JCCL12_ROOT_DIR})

find_path(JCCL12_INCLUDE_DIR
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

find_library(JCCL12_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(JCCL12_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(JCCL12)

# Dependency
if(NOT VPR20_FOUND)
	find_package(VPR20 ${_FIND_FLAGS})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JCCL12
	DEFAULT_MSG
	JCCL12_LIBRARY
	JCCL12_INCLUDE_DIR
	VPR20_FOUND
	VPR20_LIBRARIES
	VPR20_INCLUDE_DIR)

if(JCCL12_FOUND)
	set(_DEPS ${VPR20_LIBRARIES})

	set(JCCL12_INCLUDE_DIRS ${JCCL12_INCLUDE_DIR})
	list(APPEND JCCL12_INCLUDE_DIRS ${VPR20_INCLUDE_DIRS})
	clean_directory_list(JCCL12_INCLUDE_DIRS)

	if(VRJUGGLER22_CREATE_IMPORTED_TARGETS)
		create_imported_target(JCCL12 ${_DEPS})
	else()
		clean_library_list(JCCL12_LIBRARIES)
	endif()

	mark_as_advanced(JCCL12_ROOT_DIR)
endif()

mark_as_advanced(JCCL12_LIBRARY_RELEASE
	JCCL12_LIBRARY_DEBUG
	JCCL12_INCLUDE_DIR)
