# - try to find Bluez
#
# Cache Variables: (probably not for direct use in your scripts)
#  BLUEZ_INCLUDE_DIR
#  BLUEZ_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  BLUEZ_FOUND
#  BLUEZ_INCLUDE_DIRS
#  BLUEZ_LIBRARIES
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

if(WIN32 OR APPLE OR NOT UNIX)
	if(NOT Bluez_FIND_QUIETLY)
		message(STATUS "Platform not supported by Bluez - skipping search")
	endif()
else()
	set(BLUEZ_ROOT_DIR
		"${BLUEZ_ROOT_DIR}"
		CACHE
		PATH
		"Directory to search")

	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(_LIBSUFFIXES lib64 lib)
	else()
		set(_LIBSUFFIXES lib)
	endif()

	find_library(BLUEZ_LIBRARY
		NAMES
		bluetooth
		HINTS
		"${BLUEZ_ROOT_DIR}"
		PATH_SUFFIXES
		"${_LIBSUFFIXES}")

	# Might want to look close to the library first for the includes.
	get_filename_component(_libdir "${BLUEZ_LIBRARY}" PATH)

	find_path(BLUEZ_INCLUDE_DIR
		NAMES
		bluetooth/bluetooth.h
		HINTS
		"${_libdir}/.."
		PATHS
		"${BLUEZ_ROOT_DIR}"
		PATH_SUFFIXES
		include/)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Bluez
	DEFAULT_MSG
	BLUEZ_LIBRARY
	BLUEZ_INCLUDE_DIR)

if(BLUEZ_FOUND)
	set(BLUEZ_LIBRARIES "${BLUEZ_LIBRARY}")
	set(BLUEZ_INCLUDE_DIRS "${BLUEZ_INCLUDE_DIR}")
	mark_as_advanced(BLUEZ_ROOT_DIR)
endif()

mark_as_advanced(BLUEZ_INCLUDE_DIR
	BLUEZ_LIBRARY)
