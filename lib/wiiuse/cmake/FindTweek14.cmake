# - try to find Tweek 1.4 library
# Requires VPR 2.2 (thus FindVPR22.cmake)
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 3.0 - you probably want to use
# find_package(VRJuggler30) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler30.cmake for more information.
#
#  TWEEK14_LIBRARY_DIR, library search path
#  TWEEK14_INCLUDE_DIR, include search path
#  TWEEK14_LIBRARY, the library to link against
#  TWEEK14_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  TWEEK14_ROOT_DIR - A directory prefix to search
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
# Updated for VR Juggler 3.0 by:
# Brandon Newendorp <brandon@newendorp.com>


set(_HUMAN "Tweek 1.4")
set(_FP_PKG_NAME sonix)
set(_RELEASE_NAMES)
set(_DEBUG_NAMES)
foreach(VER 1_4 1_4_0 1_4_1 1_4_2)
	list(APPEND _RELEASE_NAMES ${_FP_PKG_NAME}-${VER})
	list(APPEND _DEBUG_NAMES ${_FP_PKG_NAME}_d-${VER})
endforeach()
set(_DIR tweek-1.4)
set(_HEADER tweek/tweek.h)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(TWEEK14_FIND_QUIETLY)
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

set(TWEEK14_ROOT_DIR
	"${TWEEK14_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for Tweek")
if(DEFINED VRJUGGLER30_ROOT_DIR)
	mark_as_advanced(TWEEK14_ROOT_DIR)
endif()
if(NOT TWEEK14_ROOT_DIR)
	set(TWEEK14_ROOT_DIR "${VRJUGGLER30_ROOT_DIR}")
endif()

set(_ROOT_DIR "${TWEEK14_ROOT_DIR}")

find_path(TWEEK14_INCLUDE_DIR
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

find_library(TWEEK14_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(TWEEK14_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(TWEEK14)

# Dependency
if(NOT VPR22_FOUND)
	find_package(VPR22 ${_FIND_FLAGS})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TWEEK14
	DEFAULT_MSG
	TWEEK14_LIBRARY
	TWEEK14_INCLUDE_DIR
	VPR22_FOUND
	VPR22_LIBRARIES
	VPR22_INCLUDE_DIR)

if(TWEEK14_FOUND)
	set(_DEPS ${VPR22_LIBRARIES})

	set(TWEEK14_INCLUDE_DIRS ${TWEEK14_INCLUDE_DIR})
	list(APPEND TWEEK14_INCLUDE_DIRS ${VPR22_INCLUDE_DIRS})

	clean_directory_list(TWEEK14_INCLUDE_DIRS)

	if(VRJUGGLER30_CREATE_IMPORTED_TARGETS)
		create_imported_target(TWEEK14 ${_DEPS})
	else()
		clean_library_list(TWEEK14_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(TWEEK14_ROOT_DIR)
endif()

mark_as_advanced(TWEEK14_LIBRARY_RELEASE
	TWEEK14_LIBRARY_DEBUG
	TWEEK14_INCLUDE_DIR)
