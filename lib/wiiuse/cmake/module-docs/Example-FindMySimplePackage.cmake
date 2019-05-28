# - try to find MySimplePackage library
#
# Example-MySimplePackage.cmake
#
# This example is for a pretty simple library but that is still fairly
# common in its complexity.
#
# Cache Variables: (probably not for direct use in your scripts)
#  MYSIMPLEPACKAGE_INCLUDE_DIR
#  MYSIMPLEPACKAGE_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  MYSIMPLEPACKAGE_FOUND
#  MYSIMPLEPACKAGE_INCLUDE_DIRS
#  MYSIMPLEPACKAGE_LIBRARIES
#  MYSIMPLEPACKAGE_RUNTIME_LIBRARIES - aka the dll for installing
#  MYSIMPLEPACKAGE_RUNTIME_LIBRARY_DIRS
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
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

set(MYSIMPLEPACKAGE_ROOT_DIR
	"${MYSIMPLEPACKAGE_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search")

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(_LIBSUFFIXES /lib64 /lib)
else()
	set(_LIBSUFFIXES /lib)
endif()

find_library(MYSIMPLEPACKAGE_LIBRARY
	NAMES
	mysimplepackage
	PATHS
	"${MYSIMPLEPACKAGE_ROOT_DIR}"
	PATH_SUFFIXES
	"${_LIBSUFFIXES}")

# Might want to look close to the library first for the includes.
get_filename_component(_libdir "${MYSIMPLEPACKAGE_LIBRARY}" PATH)

find_path(MYSIMPLEPACKAGE_INCLUDE_DIR
	NAMES
	mysimplepackage.h
	HINTS
	"${_libdir}" # the library I based this on was sometimes bundled right next to its include
	"${_libdir}/.."
	PATHS
	"${MYSIMPLEPACKAGE_ROOT_DIR}"
	PATH_SUFFIXES
	include/)

# There's a DLL to distribute on Windows - find where it is.
set(_deps_check)
if(WIN32)
	find_file(MYSIMPLEPACKAGE_RUNTIME_LIBRARY
		NAMES
		mysimplepackage.dll
		HINTS
		"${_libdir}")
	set(MYSIMPLEPACKAGE_RUNTIME_LIBRARIES
		"${MYSIMPLEPACKAGE_RUNTIME_LIBRARY}")
	get_filename_component(MYSIMPLEPACKAGE_RUNTIME_LIBRARY_DIRS
		"${MYSIMPLEPACKAGE_RUNTIME_LIBRARY}"
		PATH)
	list(APPEND _deps_check MYSIMPLEPACKAGE_RUNTIME_LIBRARY)
else()
	get_filename_component(MYSIMPLEPACKAGE_RUNTIME_LIBRARY_DIRS
		"${MYSIMPLEPACKAGE_LIBRARY}"
		PATH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MySimplePackage
	DEFAULT_MSG
	MYSIMPLEPACKAGE_LIBRARY
	MYSIMPLEPACKAGE_INCLUDE_DIR
	${_deps_check})

if(MYSIMPLEPACKAGE_FOUND)
	set(MYSIMPLEPACKAGE_LIBRARIES "${MYSIMPLEPACKAGE_LIBRARY}")
	set(MYSIMPLEPACKAGE_INCLUDE_DIRS "${MYSIMPLEPACKAGE_INCLUDE_DIR}")
	mark_as_advanced(MYSIMPLEPACKAGE_ROOT_DIR)
endif()

mark_as_advanced(MYSIMPLEPACKAGE_INCLUDE_DIR
	MYSIMPLEPACKAGE_LIBRARY
	MYSIMPLEPACKAGE_RUNTIME_LIBRARY)
