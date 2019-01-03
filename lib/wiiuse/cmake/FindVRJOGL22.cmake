# - try to find VRJuggler 2.2 OpenGL library
# Requires VRJ core 2.2 (thus FindVRJ22.cmake)
# Requires OpenGL.
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 2.2 - you probably want to use
# find_package(VRJuggler22) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler22.cmake for more information.
#
#  VRJOGL22_LIBRARY_DIR, library search path
#  VRJOGL22_INCLUDE_DIRS, include search path for dependencies
#  VRJOGL22_LIBRARY, the library to link against
#  VRJOGL22_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VRJOGL22_ROOT_DIR - A directory prefix to search
#                      (a path that contains include/ as a subdirectory)
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


set(_HUMAN "VR Juggler 2.2 OpenGL Core")
set(_RELEASE_NAMES vrj_ogl-2_2 libvrj_ogl-2_2)
set(_DEBUG_NAMES vrj_ogl_d-2_2 libvrj_ogl_d-2_2)
set(_DIR vrjuggler-2.2)
set(_FP_PKG_NAME vrjuggler-opengl)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(VRJOGL22_FIND_QUIETLY)
	set(_FIND_FLAGS "QUIET")
else()
	set(_FIND_FLAGS "")
endif()

# Try flagpoll.
find_package(Flagpoll QUIET)

if(FLAGPOLL)
	flagpoll_get_library_dirs(${_FP_PKG_NAME} NO_DEPS)
	flagpoll_get_library_names(${_FP_PKG_NAME} NO_DEPS)
endif()

set(VRJOGL22_ROOT_DIR
	"${VRJOGL22_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VRJOGL")
if(DEFINED VRJUGGLER22_ROOT_DIR)
	mark_as_advanced(VRJOGL22_ROOT_DIR)
endif()
if(NOT VRJOGL22_ROOT_DIR)
	set(VRJOGL22_ROOT_DIR "${VRJUGGLER22_ROOT_DIR}")
endif()

set(_ROOT_DIR "${VRJOGL22_ROOT_DIR}")

find_library(VRJOGL22_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(VRJOGL22_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(VRJOGL22)

# Dependency
if(NOT VRJ22_FOUND)
	find_package(VRJ22 ${_FIND_FLAGS})
endif()

if(NOT OPENGL_FOUND)
	find_package(OpenGL ${_FIND_FLAGS})
endif()

if(APPLE)
	set(VRJOGL22_AppKit_LIBRARY
		"-framework AppKit"
		CACHE
		STRING
		"AppKit framework for OSX")
	set(VRJOGL22_Cocoa_LIBRARY
		"-framework Cocoa"
		CACHE
		STRING
		"Cocoa framework for OSX")
	mark_as_advanced(VRJOGL22_AppKit_LIBRARY VRJOGL22_Cocoa_LIBRARY)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRJOGL22
	DEFAULT_MSG
	VRJOGL22_LIBRARY
	VRJ22_FOUND
	VRJ22_LIBRARIES
	VRJ22_INCLUDE_DIRS
	OPENGL_FOUND
	OPENGL_LIBRARIES)

if(VRJOGL22_FOUND)
	set(_DEPS ${VRJ22_LIBRARIES} ${OPENGL_LIBRARIES})
	if(APPLE)
		list(APPEND
			_DEPS
			${VRJOGL22_AppKit_LIBRARY}
			${VRJOGL22_Cocoa_LIBRARY})
	endif()

	set(VRJOGL22_INCLUDE_DIRS ${VRJ22_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})

	if(VRJUGGLER22_CREATE_IMPORTED_TARGETS)
		create_imported_target(VRJOGL22 ${_DEPS})
	else()
		clean_library_list(VRJOGL22_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(VRJOGL22_ROOT_DIR)
endif()

mark_as_advanced(VRJOGL22_LIBRARY_RELEASE VRJOGL22_LIBRARY_DEBUG)
