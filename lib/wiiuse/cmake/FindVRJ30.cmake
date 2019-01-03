# - try to find VR Juggler 3.0 core library
# Requires JCCL 1.4, Gadgeteer 1.4, VPR 2.2, and Sonix 1.4
# (thus FindJCCL14.cmake, FindGadgeteer20.cmake, FindVPR22.cmake,
# and FindSonix14.cmake)
# Requires X11 if not on Mac or Windows.
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 3.0 - you probably want to use
# find_package(VRJuggler30) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler30.cmake for more information.
#
#  VRJ30_LIBRARY_DIR, library search path
#  VRJ30_INCLUDE_DIR, include search path
#  VRJ30_LIBRARY, the library to link against
#  VRJ30_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VRJ30_ROOT_DIR - A directory prefix to search
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
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


set(_HUMAN "VR Juggler 3.0 Core")
set(_FP_PKG_NAME vrjuggler)
set(_RELEASE_NAMES)
set(_DEBUG_NAMES)
foreach(VER 3_0 3_0_0 3_0_1 3_0_2)
	list(APPEND _RELEASE_NAMES vrj-${VER})
	list(APPEND _DEBUG_NAMES vrj_d-${VER})
endforeach()
set(_DIR vrjuggler-3.0)
set(_HEADER vrj/Kernel/Kernel.h)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(VRJ30_FIND_QUIETLY)
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

set(VRJ30_ROOT_DIR
	"${VRJ30_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VRJ")
if(DEFINED VRJUGGLER30_ROOT_DIR)
	mark_as_advanced(VRJ30_ROOT_DIR)
endif()
if(NOT VRJ30_ROOT_DIR)
	set(VRJ30_ROOT_DIR "${VRJUGGLER30_ROOT_DIR}")
endif()

set(_ROOT_DIR "${VRJ30_ROOT_DIR}")

find_path(VRJ30_INCLUDE_DIR
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

find_library(VRJ30_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(VRJ30_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(VRJ30)

# Dependencies
if(NOT JCCL14_FOUND)
	find_package(JCCL14 ${_FIND_FLAGS})
endif()

if(NOT GADGETEER20_FOUND)
	find_package(Gadgeteer20 ${_FIND_FLAGS})
endif()

if(NOT VPR22_FOUND)
	find_package(VPR22 ${_FIND_FLAGS})
endif()

if(NOT SONIX14_FOUND)
	find_package(Sonix14 ${_FIND_FLAGS})
endif()

if(UNIX AND NOT APPLE AND NOT WIN32)
	if(NOT X11_FOUND)
		find_package(X11 ${_FIND_FLAGS})
	endif()
	set(_CHECK_EXTRAS
		X11_FOUND
		X11_X11_LIB
		X11_ICE_LIB
		X11_SM_LIB
		X11_INCLUDE_DIR)
endif()
if(UNIX AND NOT WIN32)
	find_library(VRJ30_libm_LIBRARY m)
	mark_as_advanced(VRJ30_libm_LIBRARY)
	list(APPEND _CHECK_EXTRAS VRJ30_libm_LIBRARY)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRJ30
	DEFAULT_MSG
	VRJ30_LIBRARY
	VRJ30_INCLUDE_DIR
	JCCL14_FOUND
	JCCL14_LIBRARIES
	JCCL14_INCLUDE_DIR
	GADGETEER20_FOUND
	GADGETEER20_LIBRARIES
	GADGETEER20_INCLUDE_DIR
	VPR22_FOUND
	VPR22_LIBRARIES
	VPR22_INCLUDE_DIR
	SONIX14_FOUND
	SONIX14_LIBRARIES
	SONIX14_INCLUDE_DIR
	${_CHECK_EXTRAS})

if(VRJ30_FOUND)
	set(_DEPS
		${JCCL14_LIBRARIES}
		${GADGETEER20_LIBRARIES}
		${VPR22_LIBRARIES}
		${SONIX14_LIBRARIES})
	if(UNIX AND NOT APPLE AND NOT WIN32)
		list(APPEND _DEPS ${X11_X11_LIB} ${X11_ICE_LIB} ${X11_SM_LIB})
	endif()
	if(UNIX AND NOT WIN32)
		list(APPEND _DEPS ${VRJ30_libm_LIBRARY})
	endif()

	set(VRJ30_INCLUDE_DIRS ${VRJ30_INCLUDE_DIR})
	list(APPEND
		VRJ30_INCLUDE_DIRS
		${JCCL14_INCLUDE_DIRS}
		${GADGETEER20_INCLUDE_DIRS}
		${VPR22_INCLUDE_DIRS}
		${SONIX14_INCLUDE_DIRS})
	clean_directory_list(VRJ30_INCLUDE_DIRS)

	if(VRJUGGLER30_CREATE_IMPORTED_TARGETS)
		create_imported_target(VRJ30 ${_DEPS})
	else()
		clean_library_list(VRJ30_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(VRJ30_ROOT_DIR)
endif()

mark_as_advanced(VRJ30_LIBRARY_RELEASE
	VRJ30_LIBRARY_DEBUG
	VRJ30_INCLUDE_DIR)
