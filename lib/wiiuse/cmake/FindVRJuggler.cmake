# - try to find VR Juggler-related packages (combined finder)
#  VRJUGGLER_INCLUDE_DIRS, include search paths
#  VRJUGGLER_LIBRARIES, the libraries to link against
#  VRJUGGLER_ENVIRONMENT
#  VRJUGGLER_RUNTIME_LIBRARY_DIRS
#  VRJUGGLER_CXX_FLAGS
#  VRJUGGLER_DEFINITIONS
#  VRJUGGLER_FOUND, If false, do not try to use VR Juggler.
#
# Components available to search for (uses "VRJOGL" by default):
#  VRJOGL
#  VRJ
#  Gadgeteer
#  JCCL
#  VPR
#  Sonix
#  Tweek
#
# Additionally, a full setup requires these packages and their Find_.cmake scripts
#  CPPDOM
#  GMTL
#
# Optionally uses Flagpoll (and FindFlagpoll.cmake)
#
# Notes on components:
#  - All components automatically include their dependencies.
#  - If you do not specify a component, VRJOGL (the OpenGL view manager)
#    will be used by default.
#  - Capitalization of component names does not matter, but it's best to
#    pretend it does and use the above capitalization.
#  - Since this script calls find_package for your requested components and
#    their dependencies, you can use any of the variables specified in those
#    files in addition to the "summary" ones listed here, for more finely
#    controlled building and linking.
#
# This CMake script requires all of the Find*.cmake scripts for the
# components listed above, as it is only a "meta-script" designed to make
# using those scripts more developer-friendly.
#
# Useful configuration variables you might want to add to your cache:
#  (CAPS COMPONENT NAME)_ROOT_DIR - A directory prefix to search
#                         (a path that contains include/ as a subdirectory)
#
# The VJ_BASE_DIR environment variable is also searched (preferentially)
# when seeking any of the above components, as well as Flagpoll, CPPDOM,
# and Boost (from within VPR), so most sane build environments should
# "just work."
#
# IMPORTANT: Note that you need to manually re-run CMake if you change
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

include(CleanLibraryList)
include(CleanDirectoryList)
include(FindPackageMessage)

if(NOT VRJUGGLER_ROOT_DIR)
	file(TO_CMAKE_PATH "$ENV{VJ_BASE_DIR}" VRJUGGLER_ROOT_DIR)
endif()

set(VRJUGGLER_ROOT_DIR
	"${VRJUGGLER_ROOT_DIR}"
	CACHE
	PATH
	"Additional root directory to search for VR Juggler and its dependencies.")
if(NOT VRJUGGLER_ROOT_DIR)
	file(TO_CMAKE_PATH "$ENV{VJ_BASE_DIR}" VRJUGGLER30_ROOT_DIR)
endif()

# Default required components
if(NOT VRJuggler_FIND_COMPONENTS)
	set(VRJuggler_FIND_COMPONENTS vrjogl)
endif()

if(VRJuggler30_FIND_QUIETLY)
	set(_FIND_FLAGS "QUIET")
else()
	set(_FIND_FLAGS "")
endif()

set(VRJUGGLER_FIND_22 TRUE)
set(VRJUGGLER_FIND_30 TRUE)
if(VRJuggler_FIND_VERSION)
	if(VRJuggler_FIND_VERSION_EXACT)
		if(VRJuggler_FIND_VERSION MATCHES "2.2" OR VRJuggler_FIND_VERSION MATCHES "22")
			set(VRJUGGLER_FIND_30 FALSE)
		elseif(VRJuggler_FIND_VERSION MATCHES "3.0" OR VRJuggler_FIND_VERSION MATCHES "30")
			set(VRJUGGLER_FIND_22 FALSE)
		endif()
	else()
		if(VRJuggler_FIND_VERSION MATCHES "3.0" OR VRJuggler_FIND_VERSION MATCHES "30")
			set(VRJUGGLER_FIND_22 FALSE)
		endif()
	endif()
endif()

if(VRJUGGLER_FIND_30)
	if(NOT VRJUGGLER30_ROOT_DIR)
		set(VRJUGGLER30_ROOT_DIR ${VRJUGGLER_ROOT_DIR})
	endif()
	find_package(VRJuggler30 COMPONENTS ${VRJuggler_FIND_COMPONENTS})
	if(VRJUGGLER30_FOUND)
		set(VRJUGGLER_FOUND TRUE)

		set(VRJUGGLER_LIBRARIES ${VRJUGGLER30_LIBRARIES})
		set(VRJUGGLER_INCLUDE_DIRS ${VRJUGGLER30_INCLUDE_DIRS})
		set(VRJUGGLER_LIBRARY_DIRS ${VRJUGGLER30_LIBRARY_DIRS})

		set(VRJUGGLER_ENVIRONMENT ${VRJUGGLER30_ENVIRONMENT})
		set(VRJUGGLER_RUNTIME_LIBRARY_DIRS ${VRJUGGLER30_RUNTIME_LIBRARY_DIRS})

		set(VRJUGGLER_CXX_FLAGS ${VRJUGGLER30_CXX_FLAGS})
		set(VRJUGGLER_DEFINITIONS ${VRJUGGLER30_DEFINITIONS})
		set(VRJUGGLER_BUNDLE_PLUGINS ${VRJUGGLER30_BUNDLE_PLUGINS})
		set(VRJUGGLER_VJ_BASE_DIR ${VRJUGGLER30_VJ_BASE_DIR})
		set(VRJUGGLER_VERSION 3.0)

		macro(install_vrjuggler_data_files prefix)
			install_vrjuggler30_data_files("${prefix}" ${ARGN})
		endmacro()

		macro(install_vrjuggler_plugins prefix varForFilenames)
			install_vrjuggler30_plugins("${prefix}" ${varForFilenames} ${ARGN})
		endmacro()
	endif()
endif()

if(VRJUGGLER_FIND_22 AND (NOT VRJUGGLER_FOUND))

	if(NOT VRJUGGLER22_ROOT_DIR)
		set(VRJUGGLER22_ROOT_DIR ${VRJUGGLER_ROOT_DIR})
	endif()
	find_package(VRJuggler22 COMPONENTS ${VRJuggler_FIND_COMPONENTS})
	if(VRJUGGLER22_FOUND)
		set(VRJUGGLER_FOUND TRUE)

		set(VRJUGGLER_LIBRARIES ${VRJUGGLER22_LIBRARIES})
		set(VRJUGGLER_INCLUDE_DIRS ${VRJUGGLER22_INCLUDE_DIRS})
		set(VRJUGGLER_LIBRARY_DIRS ${VRJUGGLER22_LIBRARY_DIRS})

		set(VRJUGGLER_ENVIRONMENT ${VRJUGGLER22_ENVIRONMENT})
		set(VRJUGGLER_RUNTIME_LIBRARY_DIRS ${VRJUGGLER22_RUNTIME_LIBRARY_DIRS})

		set(VRJUGGLER_CXX_FLAGS ${VRJUGGLER22_CXX_FLAGS})
		set(VRJUGGLER_DEFINITIONS ${VRJUGGLER22_DEFINITIONS})
		set(VRJUGGLER_BUNDLE_PLUGINS ${VRJUGGLER22_BUNDLE_PLUGINS})
		set(VRJUGGLER_VJ_BASE_DIR ${VRJUGGLER22_VJ_BASE_DIR})
		set(VRJUGGLER_VERSION 2.2)

		macro(install_vrjuggler_data_files prefix)
			install_vrjuggler22_data_files("${prefix}" ${ARGN})
		endmacro()

		macro(install_vrjuggler_plugins prefix varForFilenames)
			install_vrjuggler22_plugins("${prefix}" ${varForFilenames} ${ARGN})
		endmacro()
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRJuggler
	REQUIRED_VARS
	VRJUGGLER_LIBRARIES
	VERSION_VAR
	VRJUGGLER_VERSION)

if(VRJUGGLER_FOUND)
	mark_as_advanced(VRJUGGLER_ROOT_DIR)

	# Set generic component variables, like VPR_LIBRARIES
	if(VRJUGGLER_VERSION VERSION_EQUAL 2.2)
		set(_components VRJ22 VRJOGL22 VPR20 TWEEK12 SONIX12 JCCL12)
	else()
		set(_components VRJ30 VRJOGL30 VPR22 TWEEK14 SONIX14 JCCL14)
	endif()

	foreach(comp ${_components})
		string(LENGTH "${comp}" len)
		math(EXPR complen "${len} - 2")
		string(SUBSTRING "${comp}" 0 ${complen} compshort)
		set(${compshort}_LIBRARIES ${${comp}_LIBRARIES})
		set(${compshort}_INCLUDE_DIRS ${${comp}_INCLUDE_DIRS})
		set(${compshort}_LIBRARY_DIRS ${${comp}_LIBRARY_DIRS})
		set(${compshort}_CXX_FLAGS ${${comp}_CXX_FLAGS})
	endforeach()
endif()
