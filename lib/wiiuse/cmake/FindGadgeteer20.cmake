# - try to find Gadgeteer 2.0 library
# Requires JCCL 1.4 and VPR 2.2 (thus FindJCCL14.cmake and FindVPR22.cmake)
# Requires X11 if not on Mac or Windows.
# Optionally uses Flagpoll and FindFlagpoll.cmake
#
# This library is a part of VR Juggler 3.0 - you probably want to use
# find_package(VRJuggler30) instead, for an easy interface to this and
# related scripts.  See FindVRJuggler30.cmake for more information.
#
#  GADGETEER20_LIBRARY_DIR, library search path
#  GADGETEER20_INCLUDE_DIR, include search path
#  GADGETEER20_LIBRARY, the library to link against
#  GADGETEER20_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  GADGETEER20_ROOT_DIR - A directory prefix to search
#                         (a path that contains include/ as a subdirectory)
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

set(_HUMAN "Gadgeteer 2.0")
set(_FP_PKG_NAME gadgeteer)
set(_RELEASE_NAMES)
set(_DEBUG_NAMES)
foreach(VER 2_0 2_0_0 2_0_1 2_0_2)
	list(APPEND _RELEASE_NAMES gadget-${VER})
	list(APPEND _DEBUG_NAMES gadget_d-${VER})
endforeach()
set(_DIR gadgeteer-2.0)
set(_HEADER gadget/gadgetConfig.h)

include(SelectLibraryConfigurations)
include(CreateImportedTarget)
include(CleanLibraryList)
include(CleanDirectoryList)

if(GADGETEER20_FIND_QUIETLY)
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

set(GADGETEER20_ROOT_DIR
	"${GADGETEER20_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for Gadgeteer")
if(DEFINED VRJUGGLER30_ROOT_DIR)
	mark_as_advanced(GADGETEER20_ROOT_DIR)
endif()
if(NOT GADGETEER20_ROOT_DIR)
	set(GADGETEER20_ROOT_DIR "${VRJUGGLER30_ROOT_DIR}")
endif()

set(_ROOT_DIR "${GADGETEER20_ROOT_DIR}")

find_path(GADGETEER20_INCLUDE_DIR
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

find_library(GADGETEER20_LIBRARY_RELEASE
	NAMES
	${_RELEASE_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBSUFFIXES}
	DOC
	"${_HUMAN} release library full path")

find_library(GADGETEER20_LIBRARY_DEBUG
	NAMES
	${_DEBUG_NAMES}
	HINTS
	"${_ROOT_DIR}"
	${${_FP_PKG_NAME}_FLAGPOLL_LIBRARY_DIRS}
	PATH_SUFFIXES
	${_VRJ_LIBDSUFFIXES}
	DOC
	"${_HUMAN} debug library full path")

select_library_configurations(GADGETEER20)

# Dependencies
foreach(package JCCL14 VPR22 GMTL)
	if(NOT ${PACKAGE}_FOUND)
		find_package(${package} ${_FIND_FLAGS})
	endif()
endforeach()

if(UNIX AND NOT APPLE AND NOT WIN32)
	# We need X11 if not on Mac or Windows
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

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GADGETEER20
	DEFAULT_MSG
	GADGETEER20_LIBRARY
	GADGETEER20_INCLUDE_DIR
	JCCL14_FOUND
	JCCL14_LIBRARIES
	JCCL14_INCLUDE_DIR
	VPR22_FOUND
	VPR22_LIBRARIES
	VPR22_INCLUDE_DIR
	GMTL_FOUND
	GMTL_INCLUDE_DIR
	${_CHECK_EXTRAS})

if(GADGETEER20_FOUND)
	set(_DEPS ${JCCL14_LIBRARIES} ${VPR22_LIBRARIES})

	set(GADGETEER20_INCLUDE_DIRS ${GADGETEER20_INCLUDE_DIR})
	list(APPEND
		GADGETEER20_INCLUDE_DIRS
		${JCCL14_INCLUDE_DIRS}
		${VPR22_INCLUDE_DIRS}
		${GMTL_INCLUDE_DIRS})

	if(UNIX AND NOT APPLE AND NOT WIN32)
		# We need X11 if not on Mac or Windows
		list(APPEND _DEPS ${X11_X11_LIB} ${X11_ICE_LIB} ${X11_SM_LIB})
		list(APPEND GADGETEER20_INCLUDE_DIRS ${X11_INCLUDE_DIR})
	endif()

	clean_directory_list(GADGETEER20_INCLUDE_DIRS)

	if(VRJUGGLER30_CREATE_IMPORTED_TARGETS)
		create_imported_target(GADGETEER20 ${_DEPS})
	else()
		clean_library_list(GADGETEER20_LIBRARIES ${_DEPS})
	endif()

	mark_as_advanced(GADGETEER20_ROOT_DIR)
endif()

mark_as_advanced(GADGETEER20_LIBRARY_RELEASE
	GADGETEER20_LIBRARY_DEBUG
	GADGETEER20_INCLUDE_DIR)
