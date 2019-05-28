# - try to find VR Juggler 2.2 core library
# Requires JCCL 1.2, Gadgeteer 1.2, VPR 2.0, and Sonix 1.2
# (thus FindJCCL12.cmake, FindGadgeteer12.cmake, FindVPR20.cmake,
# and FindSonix12.cmake)
# Requires X11 if not on Mac or Windows.
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 2.2 - you probably want to use
# find_package(VRJuggler22) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler22.cmake for more information.
#
#  VRJ22_LIBRARY_DIR, library search path
#  VRJ22_INCLUDE_DIR, include search path
#  VRJ22_LIBRARY, the library to link against
#  VRJ22_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VRJ22_ROOT_DIR - A directory prefix to search
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


set(_HUMAN "VR Juggler 2.2 Core")
set(_RELEASE_NAMES vrj-2_2 libvrj-2_2)
set(_DEBUG_NAMES vrj_d-2_2 libvrj_d-2_2)
set(_DIR vrjuggler-2.2)
set(_HEADER vrj/Kernel/Kernel.h)
set(_FP_PKG_NAME vrjuggler)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(VRJ22_FIND_QUIETLY)
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

set(VRJ22_ROOT_DIR
	"${VRJ22_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VRJ")
if(DEFINED VRJUGGLER22_ROOT_DIR)
	mark_as_advanced(VRJ22_ROOT_DIR)
endif()
if(NOT VRJ22_ROOT_DIR)
	set(VRJ22_ROOT_DIR "${VRJUGGLER22_ROOT_DIR}")
endif()

set(_ROOT_DIR "${VRJ22_ROOT_DIR}")

find_path(VRJ22_INCLUDE_DIR
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

find_library(VRJ22_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(VRJ22_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(VRJ22)

# Dependencies
if(NOT JCCL12_FOUND)
	find_package(JCCL12 ${_FIND_FLAGS})
endif()

if(NOT GADGETEER12_FOUND)
	find_package(Gadgeteer12 ${_FIND_FLAGS})
endif()

if(NOT VPR20_FOUND)
	find_package(VPR20 ${_FIND_FLAGS})
endif()

if(NOT SONIX12_FOUND)
	find_package(Sonix12 ${_FIND_FLAGS})
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
	find_library(VRJ22_libm_LIBRARY m)
	mark_as_advanced(VRJ22_libm_LIBRARY)
	list(APPEND _CHECK_EXTRAS VRJ22_libm_LIBRARY)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRJ22
	DEFAULT_MSG
	VRJ22_LIBRARY
	VRJ22_INCLUDE_DIR
	JCCL12_FOUND
	JCCL12_LIBRARIES
	JCCL12_INCLUDE_DIR
	GADGETEER12_FOUND
	GADGETEER12_LIBRARIES
	GADGETEER12_INCLUDE_DIR
	VPR20_FOUND
	VPR20_LIBRARIES
	VPR20_INCLUDE_DIR
	SONIX12_FOUND
	SONIX12_LIBRARIES
	SONIX12_INCLUDE_DIR
	${_CHECK_EXTRAS})

if(VRJ22_FOUND)
	set(_DEPS
		${JCCL12_LIBRARIES}
		${GADGETEER12_LIBRARIES}
		${VPR20_LIBRARIES}
		${SONIX12_LIBRARIES})
	if(UNIX AND NOT APPLE AND NOT WIN32)
		list(APPEND _DEPS ${X11_X11_LIB} ${X11_ICE_LIB} ${X11_SM_LIB})
	endif()
	if(UNIX AND NOT WIN32)
		list(APPEND _DEPS ${VRJ22_libm_LIBRARY})
	endif()

	set(VRJ22_INCLUDE_DIRS "${VRJ22_INCLUDE_DIR}")
	list(APPEND
		VRJ22_INCLUDE_DIRS
		${JCCL12_INCLUDE_DIRS}
		${GADGETEER12_INCLUDE_DIRS}
		${VPR20_INCLUDE_DIRS}
		${SONIX12_INCLUDE_DIRS})
	clean_directory_list(VRJ22_INCLUDE_DIRS)

	if(VRJUGGLER22_CREATE_IMPORTED_TARGETS)
		create_imported_target(VRJ22 ${_DEPS})
	else()
		clean_library_list(VRJ22_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(VRJ22_ROOT_DIR)
endif()

mark_as_advanced(VRJ22_LIBRARY_RELEASE
	VRJ22_LIBRARY_DEBUG
	VRJ22_INCLUDE_DIR)
