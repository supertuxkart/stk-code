# - try to find WiiUse library
#
# Cache Variables: (probably not for direct use in your scripts)
#  WIIUSE_INCLUDE_DIR
#  WIIUSE_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  WIIUSE_FOUND
#  WIIUSE_INCLUDE_DIRS
#  WIIUSE_LIBRARIES
#  WIIUSE_RUNTIME_LIBRARIES - aka the dll for installing
#  WIIUSE_RUNTIME_LIBRARY_DIRS
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

set(WIIUSE_ROOT_DIR
	"${WIIUSE_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search for WiiUse")

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(_LIBSUFFIXES /lib64 /lib)
else()
	set(_LIBSUFFIXES /lib)
endif()

find_library(WIIUSE_LIBRARY
	NAMES
	wiiuse
	PATHS
	"${WIIUSE_ROOT_DIR}"
	PATH_SUFFIXES
	"${_LIBSUFFIXES}")

get_filename_component(_libdir "${WIIUSE_LIBRARY}" PATH)

find_path(WIIUSE_INCLUDE_DIR
	NAMES
	wiiuse.h
	HINTS
	"${_libdir}"
	"${_libdir}/.."
	PATHS
	"${WIIUSE_ROOT_DIR}"
	PATH_SUFFIXES
	include/)

set(_deps_check)
if(WIN32)
	find_file(WIIUSE_RUNTIME_LIBRARY
		NAMES
		wiiuse.dll
		HINTS
		"${_libdir}"
		"${_libdir}/.."
		PATH_SUFFIXES
		bin)

	set(WIIUSE_RUNTIME_LIBRARIES "${WIIUSE_RUNTIME_LIBRARY}")
	get_filename_component(WIIUSE_RUNTIME_LIBRARY_DIRS
		"${WIIUSE_RUNTIME_LIBRARY}"
		PATH)
	list(APPEND _deps_check WIIUSE_RUNTIME_LIBRARY)
else()
	set(WIIUSE_RUNTIME_LIBRARY "${WIIUSE_LIBRARY}")
	set(WIIUSE_RUNTIME_LIBRARIES "${WIIUSE_RUNTIME_LIBRARY}")
	get_filename_component(WIIUSE_RUNTIME_LIBRARY_DIRS
		"${WIIUSE_LIBRARY}"
		PATH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WiiUse
	DEFAULT_MSG
	WIIUSE_LIBRARY
	WIIUSE_INCLUDE_DIR
	${_deps_check})

if(WIIUSE_FOUND)
	set(WIIUSE_LIBRARIES "${WIIUSE_LIBRARY}")
	set(WIIUSE_INCLUDE_DIRS "${WIIUSE_INCLUDE_DIR}")
	mark_as_advanced(WIIUSE_ROOT_DIR)
endif()

mark_as_advanced(WIIUSE_INCLUDE_DIR
	WIIUSE_LIBRARY
	WIIUSE_RUNTIME_LIBRARY)
