# - try to find VRJuggler 3.0 OpenGL library
# Requires VRJ core 3.0 (thus FindVRJ30.cmake)
# Requires OpenGL.
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 3.0 - you probably want to use
# find_package(VRJuggler30) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler30.cmake for more information.
#
#  VRJOGL30_LIBRARY_DIR, library search path
#  VRJOGL30_INCLUDE_DIRS, include search path for dependencies
#  VRJOGL30_LIBRARY, the library to link against
#  VRJOGL30_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VRJOGL30_ROOT_DIR - A directory prefix to search
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
# Updated for VR Juggler 3.0 by:
# Brandon Newendorp <brandon@newendorp.com>


set(_HUMAN "VR Juggler 3.0 OpenGL Core")
set(_FP_PKG_NAME vrjuggler-opengl)
set(_RELEASE_NAMES)
set(_DEBUG_NAMES)
foreach(VER 3_0 3_0_0 3_0_1 3_0_2)
	list(APPEND _RELEASE_NAMES vrj_ogl-${VER})
	list(APPEND _DEBUG_NAMES vrj_ogl_d-${VER})
endforeach()
set(_DIR vrjuggler-3.0)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(VRJOGL30_FIND_QUIETLY)
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

set(VRJOGL30_ROOT_DIR
	"${VRJOGL30_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VRJOGL")
if(DEFINED VRJUGGLER30_ROOT_DIR)
	mark_as_advanced(VRJOGL30_ROOT_DIR)
endif()
if(NOT VRJOGL30_ROOT_DIR)
	set(VRJOGL30_ROOT_DIR "${VRJUGGLER30_ROOT_DIR}")
endif()

set(_ROOT_DIR "${VRJOGL30_ROOT_DIR}")

find_library(VRJOGL30_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(VRJOGL30_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(VRJOGL30)

# Dependency
if(NOT VRJ30_FOUND)
	find_package(VRJ30 ${_FIND_FLAGS})
endif()

if(NOT OPENGL_FOUND)
	find_package(OpenGL ${_FIND_FLAGS})
endif()

if(APPLE)
	set(VRJOGL30_AppKit_LIBRARY
		"-framework AppKit"
		CACHE
		STRING
		"AppKit framework for OSX")
	set(VRJOGL30_Cocoa_LIBRARY
		"-framework Cocoa"
		CACHE
		STRING
		"Cocoa framework for OSX")
	mark_as_advanced(VRJOGL30_AppKit_LIBRARY VRJOGL30_Cocoa_LIBRARY)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRJOGL30
	DEFAULT_MSG
	VRJOGL30_LIBRARY
	VRJ30_FOUND
	VRJ30_LIBRARIES
	VRJ30_INCLUDE_DIRS
	OPENGL_FOUND
	OPENGL_LIBRARIES)

if(VRJOGL30_FOUND)
	set(_DEPS ${VRJ30_LIBRARIES} ${OPENGL_LIBRARIES})
	if(APPLE)
		list(APPEND
			_DEPS
			${VRJOGL30_AppKit_LIBRARY}
			${VRJOGL30_Cocoa_LIBRARY})
	endif()

	set(VRJOGL30_INCLUDE_DIRS ${VRJ30_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})

	if(VRJUGGLER30_CREATE_IMPORTED_TARGETS)
		create_imported_target(VRJOGL30 ${_DEPS})
	else()
		clean_library_list(VRJOGL30_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(VRJOGL30_ROOT_DIR)
endif()

mark_as_advanced(VRJOGL30_LIBRARY_RELEASE VRJOGL30_LIBRARY_DEBUG)
