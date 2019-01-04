# - try to find VPR 2.2 library
# Requires Boost 1.33.1 or greater (including filesystem and signals libraries)
# (and thus FindBoost.cmake from 2.8rc3 or newer, preferably)
# Requires NSPR4 (and PLC4) on Windows
# Requires pthreads on Unix (Mac or Linux)
# Requires libuuid on Linux
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 3.0 - you probably want to use
# find_package(VRJuggler30) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler30.cmake for more information.
#
#  VPR22_LIBRARY_DIR, library search path
#  VPR22_INCLUDE_DIR, include search path
#  VPR22_LIBRARY, the library to link against
#  VPR22_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VPR22_ROOT_DIR - A directory prefix to search
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
# Updated for VR Juggler 3.0 by:
# Brandon Newendorp <brandon@newendorp.com>

set(_HUMAN "VPR 2.2")
set(_FP_PKG_NAME vpr)
set(_RELEASE_NAMES)
set(_DEBUG_NAMES)
foreach(VER 2_2 2_2_0 2_2_1 2_2_2)
	list(APPEND _RELEASE_NAMES ${_FP_PKG_NAME}-${VER})
	list(APPEND _DEBUG_NAMES ${_FP_PKG_NAME}_d-${VER})
endforeach()
set(_DIR vpr-2.2)
set(_HEADER vpr/vpr.h)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(VPR22_FIND_QUIETLY)
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

set(VPR22_ROOT_DIR
	"${VPR22_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VPR")
if(DEFINED VRJUGGLER30_ROOT_DIR)
	mark_as_advanced(VPR22_ROOT_DIR)
endif()
if(NOT VPR22_ROOT_DIR)
	set(VPR22_ROOT_DIR "${VRJUGGLER30_ROOT_DIR}")
endif()

set(_ROOT_DIR "${VPR22_ROOT_DIR}")

find_path(VPR22_INCLUDE_DIR
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

find_library(VPR22_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(VPR22_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(VPR22)

# Dependencies
set(_deps_libs)
set(_deps_includes)
set(_deps_check)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0011 NEW)
	cmake_policy(SET CMP0012 NEW)
endif()
if((NOT Boost_FOUND)
	OR (NOT Boost_FILESYSTEM_FOUND)
	OR (NOT Boost_SIGNALS_FOUND)
	OR (NOT Boost_SYSTEM_FOUND)
	OR (NOT Boost_PROGRAM_OPTIONS_FOUND)
	OR (NOT Boost_DATE_TIME_FOUND)
	OR (NOT Boost_REGEX_FOUND))
	if(VPR22_LIBRARY_RELEASE)
		# Find Boost in the same place as VPR
		get_filename_component(VPR22_LIBRARY_DIR
			${VPR22_LIBRARY_RELEASE}
			PATH)
		set(BOOST_ROOT ${VPR22_LIBRARY_DIR}/../)

		find_package(Boost
			1.40.0
			${_FIND_FLAGS}
			COMPONENTS
			filesystem
			system
			signals
			program_options
			date_time
			regex)

		mark_as_advanced(Boost_LIB_DIAGNOSTIC_DEFINITIONS)

	endif()

endif()

list(APPEND
	_deps_libs
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_SIGNALS_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
	${Boost_REGEX_LIBRARY})
list(APPEND _deps_includes ${Boost_INCLUDE_DIRS})
list(APPEND
	_deps_check
	Boost_FILESYSTEM_LIBRARY
	Boost_SYSTEM_LIBRARY
	Boost_SIGNALS_LIBRARY
	Boost_PROGRAM_OPTIONS_LIBRARY
	Boost_DATE_TIME_LIBRARY
	Boost_REGEX_LIBRARY
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
		find_library(VPR22_libuuid_LIBRARY NAMES uuid)
		mark_as_advanced(VPR22_libuuid_LIBRARY)
		if(VPR22_libuuid_LIBRARY)
			list(APPEND _deps_libs ${VPR22_libuuid_LIBRARY})
		endif()
	endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPR22
	DEFAULT_MSG
	VPR22_LIBRARY
	VPR22_INCLUDE_DIR
	${_deps_check})

if(VPR22_FOUND)

	set(VPR22_INCLUDE_DIRS ${VPR22_INCLUDE_DIR} ${_deps_includes})

	clean_directory_list(VPR22_INCLUDE_DIRS)

	if(VRJUGGLER30_CREATE_IMPORTED_TARGETS)
		create_imported_target(VPR22 ${_deps_libs})
	else()
		clean_library_list(VPR22_LIBRARIES ${VPR22_LIBRARY} ${_deps_libs})
	endif()

	mark_as_advanced(VPR22_ROOT_DIR)
endif()

mark_as_advanced(VPR22_LIBRARY_RELEASE
	VPR22_LIBRARY_DEBUG
	VPR22_INCLUDE_DIR)
