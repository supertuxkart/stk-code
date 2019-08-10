# - try to find VPR 2.0 library
# Requires Boost 1.33.1 or greater (including filesystem and signals libraries)
# (and thus FindBoost.cmake from 2.8rc3 or newer, preferably)
# Requires NSPR4 (and PLC4) on Windows
# Requires pthreads on Unix (Mac or Linux)
# Requires libuuid on Linux
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 2.2 - you probably want to use
# find_package(VRJuggler22) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler22.cmake for more information.
#
#  VPR20_LIBRARY_DIR, library search path
#  VPR20_INCLUDE_DIR, include search path
#  VPR20_LIBRARY, the library to link against
#  VPR20_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VPR20_ROOT_DIR - A directory prefix to search
#                   (a path that contains include/ as a subdirectory)
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

set(_HUMAN "VPR 2.0")
set(_RELEASE_NAMES vpr-2_0 libvpr-2_0)
set(_DEBUG_NAMES vpr_d-2_0 libvpr_d-2_0)
set(_DIR vpr-2.0)
set(_HEADER vpr/vpr.h)
set(_FP_PKG_NAME vpr)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(VPR20_FIND_QUIETLY)
	set(_FIND_FLAGS "QUIET")
else()
	set(_FIND_FLAGS "")
endif()

# Try flagpoll.
find_package(Flagpoll QUIET)

if(FLAGPOLL)
	flagpoll_get_include_dirs(${_FP_PKG_NAME} NO_DEPS)
	flagpoll_get_library_dirs(${_FP_PKG_NAME} NO_DEPS)
	flagpoll_get_extra_libs(${_FP_PKG_NAME} NO_DEPS)
endif()

set(VPR20_ROOT_DIR
	"${VPR20_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VPR")
if(DEFINED VRJUGGLER22_ROOT_DIR)
	mark_as_advanced(VPR20_ROOT_DIR)
endif()
if(NOT VPR20_ROOT_DIR)
	set(VPR20_ROOT_DIR "${VRJUGGLER22_ROOT_DIR}")
endif()

set(_ROOT_DIR "${VPR20_ROOT_DIR}")

find_path(VPR20_INCLUDE_DIR
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

find_library(VPR20_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(VPR20_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(VPR20)

# Dependencies
set(_deps_libs)
set(_deps_includes)
set(_deps_check)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0011 NEW)
	cmake_policy(SET CMP0012 NEW)
endif()
if((NOT "${Boost_FOUND}")
	OR (NOT "${Boost_FILESYSTEM_FOUND}")
	OR (NOT "${Boost_SIGNALS_FOUND}")
	OR (Boost_VERSION GREATER 103401 AND NOT Boost_SYSTEM_FOUND))
	if(VPR20_LIBRARY_RELEASE)
		# Find Boost in the same place as VPR
		get_filename_component(VPR20_LIBRARY_DIR
			${VPR20_LIBRARY_RELEASE}
			PATH)
		set(BOOST_ROOT ${VPR20_LIBRARY_DIR}/../)

		if(APPLE)
			# VR Juggler 2.2.1 binaries for Mac are built against single-threaded boost.
			set(Boost_USE_STATIC_LIBS ON)
			#set(Boost_USE_MULTITHREADED OFF)
		endif()

		find_package(Boost
			1.33.1
			${_FIND_FLAGS}
			COMPONENTS
			filesystem
			signals)

		mark_as_advanced(Boost_LIB_DIAGNOSTIC_DEFINITIONS)

		if(WIN32 AND NOT Boost_FOUND)
			if(NOT VPR20_FIND_QUIETLY)
				message(STATUS
					"Searching for Boost using forced '-vc80' override...")
			endif()
			set(Boost_COMPILER "-vc80")
			find_package(Boost
				1.33.1
				${_FIND_FLAGS}
				COMPONENTS
				filesystem
				signals)
		endif()

		if(Boost_VERSION GREATER 103401)
			find_package(Boost
				${_FIND_FLAGS}
				COMPONENTS
				filesystem
				system
				signals)
		endif()
	endif()

endif()

list(APPEND
	_deps_libs
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_SIGNALS_LIBRARY})
list(APPEND _deps_includes ${Boost_INCLUDE_DIRS})
list(APPEND
	_deps_check
	Boost_FILESYSTEM_LIBRARY
	Boost_SIGNALS_LIBRARY
	Boost_INCLUDE_DIRS)

if(NOT CPPDOM_FOUND)
	find_package(CPPDOM ${_FIND_FLAGS})
endif()

list(APPEND _deps_libs ${CPPDOM_LIBRARIES})
list(APPEND _deps_includes ${CPPDOM_INCLUDE_DIRS})
list(APPEND _deps_check CPPDOM_LIBRARIES CPPDOM_INCLUDE_DIRS)

if(UNIX AND NOT WIN32)
	if(NOT THREADS_FOUND)
		find_package(Threads ${_FIND_FLAGS})
	endif()

	list(APPEND _deps_check THREADS_FOUND)
	list(APPEND _deps_libs ${CMAKE_THREAD_LIBS_INIT})

	if(NOT APPLE)
		find_library(VPR20_libuuid_LIBRARY NAMES uuid)
		mark_as_advanced(VPR20_libuuid_LIBRARY)
		list(APPEND _deps_check VPR20_libuuid_LIBRARY)
		list(APPEND _deps_libs ${VPR20_libuuid_LIBRARY})
	endif()
endif()

if(WIN32)
	find_library(VPR20_libnspr4_LIBRARY
		NAMES
		nspr4
		libnspr4
		HINTS
		${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
		"${_ROOT_DIR}"
		PATH_SUFFIXES
		${_VRJ_LIBSUFFIXES}
		DOC
		"${_HUMAN} NSPR4 library full path")

	find_library(VPR20_libplc4_LIBRARY
		NAMES
		plc4
		libplc4
		HINTS
		${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
		"${_ROOT_DIR}"
		PATH_SUFFIXES
		${_VRJ_LIBDSUFFIXES}
		DOC
		"${_HUMAN} PLC4 library full path")
	mark_as_advanced(VPR20_libnspr4_LIBRARY VPR20_libplc4_LIBRARY)
	list(APPEND _deps_check VPR20_libnspr4_LIBRARY VPR20_libplc4_LIBRARY)
	list(APPEND
		_deps_libs
		${VPR20_libnspr4_LIBRARY}
		${VPR20_libplc4_LIBRARY})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPR20
	DEFAULT_MSG
	VPR20_LIBRARY
	VPR20_INCLUDE_DIR
	${_deps_check})

if(VPR20_FOUND)

	set(VPR20_INCLUDE_DIRS "${VPR20_INCLUDE_DIR}" ${_deps_includes})

	clean_directory_list(VPR20_INCLUDE_DIRS)

	if(VRJUGGLER22_CREATE_IMPORTED_TARGETS)
		create_imported_target(VPR20 ${_deps_libs})
	else()
		clean_library_list(VPR20_LIBRARIES ${VPR20_LIBRARY} ${_deps_libs})
	endif()

	mark_as_advanced(VPR20_ROOT_DIR)
endif()

mark_as_advanced(VPR20_LIBRARY_RELEASE
	VPR20_LIBRARY_DEBUG
	VPR20_INCLUDE_DIR)
