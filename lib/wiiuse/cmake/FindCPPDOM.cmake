# - try to find CPPDOM library
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
#  CPPDOM_LIBRARY_DIR, library search path
#  CPPDOM_INCLUDE_DIR, include search path
#  CPPDOM_LIBRARY, the library to link against
#  CPPDOM_CXX_FLAGS
#  CPPDOM_FOUND, If false, do not try to use this library.
#
# Useful configuration variables you might want to add to your cache:
#  CPPDOM_ROOT_DIR - A directory prefix to search
#                    (a path that contains include/ as a subdirectory)
#  CPPDOM_ADDITIONAL_VERSIONS - Additional versions (outside of 0.7.8 to 1.2.0)
#                               to use when constructing search names and paths
#
# This script will use Flagpoll, if found, to provide hints to the location
# of this library, but does not use the compiler flags returned by Flagpoll
# directly.
#
# VR Juggler requires this package, so this Find script takes that into
# account when determining where to search for the desired files.
# The VJ_BASE_DIR environment variable is searched (preferentially)
# when searching for this package, so most sane VR Juggler build environments
# should "just work."  Note that you need to manually re-run CMake if you
# change this environment variable, because it cannot auto-detect this change
# and trigger an automatic re-run.
#
# Original Author:
# 2009-2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(_HUMAN "cppdom")
set(_HEADER cppdom/cppdom.h)
set(_FP_PKG_NAME cppdom)

set(CPPDOM_VERSIONS
	${CPPDOM_ADDITIONAL_VERSIONS}
	1.3.0
	1.2.0
	1.1.2
	1.1.1
	1.1.0
	1.0.3
	1.0.2
	1.0.1
	1.0.0
	0.7.10
	0.7.9
	0.7.8)
set(CPPDOM_DIRS)
set(CPPDOM_RELEASE_LIB_NAMES)
set(CPPDOM_DEBUG_LIB_NAMES)
foreach(_version ${CPPDOM_VERSIONS})
	string(REGEX REPLACE "[-\\.]" "_" _versionclean ${_version})
	list(APPEND CPPDOM_DIRS cppdom-${_version})
	list(APPEND CPPDOM_HEADER_DIRS include/cppdom-${_version})
	list(APPEND CPPDOM_RELEASE_LIB_NAMES cppdom-${_versionclean})
	list(APPEND CPPDOM_DEBUG_LIB_NAMES cppdom_d-${_versionclean})
endforeach()

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)
include(FindPackageHandleStandardArgs)

# Handle the case where a recent cppdom is supplying its own cmake config file.
option(CPPDOM_ATTEMPT_CMAKE_MODULE "Should we attempt to use CPPDOM's own CMake module for configuration?" ON)
mark_as_advanced(CPPDOM_ATTEMPT_CMAKE_MODULE)
if(NOT cppdom_FOUND)
	find_package(cppdom QUIET NO_MODULE)
	if(cppdom_FOUND)
		set(CPPDOM_LIBRARIES ${cppdom_LIBRARIES})
		set(CPPDOM_INCLUDE_DIRS ${cppdom_INCLUDE_DIRS})
		find_package_handle_standard_args(CPPDOM
			DEFAULT_MSG
			CPPDOM_LIBRARIES
			CPPDOM_INCLUDE_DIRS)
		return()
	endif()
endif()

if(CPPDOM_INCLUDE_DIRS AND CPPDOM_LIBRARIES)
	# in cache already
	set(CPPDOM_FIND_QUIETLY TRUE)
endif()

# Try flagpoll.
find_package(Flagpoll QUIET)

if(FLAGPOLL)
	flagpoll_get_include_dirs(${_FP_PKG_NAME} NO_DEPS)
	flagpoll_get_library_dirs(${_FP_PKG_NAME} NO_DEPS)
	flagpoll_get_library_names(${_FP_PKG_NAME} NO_DEPS)
endif()

set(CPPDOM_ROOT_DIR
	"${CPPDOM_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for CPPDOM")
if(DEFINED VRJUGGLER22_ROOT_DIR)
	mark_as_advanced(CPPDOM_ROOT_DIR)
endif()
if(NOT CPPDOM_ROOT_DIR)
	if(VRJUGGLER22_ROOT_DIR)
		set(CPPDOM_ROOT_DIR "${VRJUGGLER22_ROOT_DIR}")
	elseif(VRJUGGLER30_ROOT_DIR)
		set(CPPDOM_ROOT_DIR "${VRJUGGLER30_ROOT_DIR}")
	endif()
endif()

set(_ROOT_DIR "${CPPDOM_ROOT_DIR}")

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(_VRJ_LIBSUFFIXES lib64 lib)
	set(_VRJ_LIBDSUFFIXES
		debug
		lib64/x86_64/debug
		lib64/debug
		lib64
		lib/x86_64/debug
		lib/debug
		lib)
	set(_VRJ_LIBDSUFFIXES_ONLY
		debug
		lib64/x86_64/debug
		lib64/debug
		lib/x86_64/debug
		lib/debug)
else()
	set(_VRJ_LIBSUFFIXES lib)
	set(_VRJ_LIBDSUFFIXES debug lib/i686/debug lib/debug lib)
	set(_VRJ_LIBDSUFFIXES_ONLY debug lib/i686/debug lib/debug)
endif()

find_path(CPPDOM_INCLUDE_DIR
	${_HEADER}
	HINTS
	${_ROOT_DIR}
	${${_FP_PKG_NAME}_FLAGPOLL_INCLUDE_DIRS}
	PATHS
	PATH_SUFFIXES
	${CPPDOM_DIRS}
	${CPPDOM_HEADER_DIRS}
	include
	DOC
	"Path to ${_HUMAN} includes root")

find_library(CPPDOM_LIBRARY_RELEASE
	NAMES
	${CPPDOM_RELEASE_LIB_NAMES}
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_NAMES}
	HINTS
	${_ROOT_DIR}
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} library full path")

find_library(CPPDOM_LIBRARY_DEBUG
	NAMES
	${CPPDOM_DEBUG_LIB_NAMES}
	HINTS
	${_ROOT_DIR}
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

# Fallback to same library name but in the debug folder
if(NOT CPPDOM_LIBRARY_DEBUG)
	find_library(CPPDOM_LIBRARY_DEBUG
		NAMES
		${CPPDOM_LIB_NAMES}
		HINTS
		${CPPDOM_INCLUDE_DIR}/../
		${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
		PATH_SUFFIXES
		${_VRJ_LIBDSUFFIXES_ONLY}
		NO_DEFAULT_PATH
		DOC
		"${_HUMAN} debug library full path")
endif()

if(CPPDOM_LIBRARY_RELEASE OR CPPDOM_LIBRARY_DEBUG)
	select_library_configurations(CPPDOM)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
find_package_handle_standard_args(CPPDOM
	DEFAULT_MSG
	CPPDOM_LIBRARY
	CPPDOM_INCLUDE_DIR)

if(CPPDOM_FOUND)
	set(CPPDOM_INCLUDE_DIRS ${CPPDOM_INCLUDE_DIR})
	if(MSVC)
		set(CPPDOM_CXX_FLAGS "/wd4290")
	endif()

	mark_as_advanced(CPPDOM_ROOT_DIR)
endif()

mark_as_advanced(CPPDOM_LIBRARY_RELEASE
	CPPDOM_LIBRARY_DEBUG
	CPPDOM_INCLUDE_DIR)
