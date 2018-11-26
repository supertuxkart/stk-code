# - try to find Arrington Research ViewPoint EyeTracker SDK
#
# Cache Variables: (probably not for direct use in your scripts)
#  VIEWPOINT_INCLUDE_DIR
#  VIEWPOINT_LIBRARY
#  VIEWPOINT_RUNTIME_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  VIEWPOINT_FOUND
#  VIEWPOINT_INCLUDE_DIRS
#  VIEWPOINT_LIBRARIES
#  VIEWPOINT_RUNTIME_LIBRARIES - aka the dll for installing
#  VIEWPOINT_RUNTIME_LIBRARY_DIRS
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(VIEWPOINT_ROOT_DIR
	"${VIEWPOINT_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search for Arrington Research ViewPoint EyeTracker SDK")

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(_LIBSUFFIXES /lib64 /lib)
else()
	set(_LIBSUFFIXES /lib)
endif()

find_library(VIEWPOINT_LIBRARY
	NAMES
	VPX_InterApp
	PATHS
	"${VIEWPOINT_ROOT_DIR}"
	PATH_SUFFIXES
	"${_LIBSUFFIXES}")

get_filename_component(_libdir "${VIEWPOINT_LIBRARY}" PATH)

find_path(VIEWPOINT_INCLUDE_DIR
	NAMES
	vpx.h
	HINTS
	"${_libdir}"
	PATHS
	"${VIEWPOINT_ROOT_DIR}"
	PATH_SUFFIXES
	include/)

set(_deps_check)
if(WIN32)
	find_file(VIEWPOINT_RUNTIME_LIBRARY
		NAMES
		VPX_InterApp.dll
		HINTS
		"${_libdir}")

	set(VIEWPOINT_RUNTIME_LIBRARIES "${VIEWPOINT_RUNTIME_LIBRARY}")
	get_filename_component(VIEWPOINT_RUNTIME_LIBRARY_DIRS
		"${VIEWPOINT_RUNTIME_LIBRARY}"
		PATH)
	list(APPEND _deps_check VIEWPOINT_RUNTIME_LIBRARY)
else()
	get_filename_component(VIEWPOINT_RUNTIME_LIBRARY_DIRS
		"${VIEWPOINT_LIBRARY}"
		PATH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ViewPoint
	DEFAULT_MSG
	VIEWPOINT_LIBRARY
	VIEWPOINT_INCLUDE_DIR
	${_deps_check})

if(VIEWPOINT_FOUND)
	set(VIEWPOINT_LIBRARIES "${VIEWPOINT_LIBRARY}")
	set(VIEWPOINT_INCLUDE_DIRS "${VIEWPOINT_INCLUDE_DIR}")
	mark_as_advanced(VIEWPOINT_ROOT_DIR)
endif()

mark_as_advanced(VIEWPOINT_INCLUDE_DIR
	VIEWPOINT_LIBRARY
	VIEWPOINT_RUNTIME_LIBRARY)
