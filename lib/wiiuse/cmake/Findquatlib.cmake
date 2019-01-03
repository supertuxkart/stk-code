# - Find quatlib
# Find the quatlib headers and libraries.
#
#  QUATLIB_INCLUDE_DIRS - where to find quat.h
#  QUATLIB_LIBRARIES    - List of libraries when using quatlib.
#  QUATLIB_FOUND        - True if quatlib found.
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

if(TARGET quat)
	# Look for the header file.
	find_path(QUATLIB_INCLUDE_DIR NAMES quat.h
			PATHS ${quatlib_SOURCE_DIR})

	set(QUATLIB_LIBRARY "quat")

else()
	set(QUATLIB_ROOT_DIR
		"${QUATLIB_ROOT_DIR}"
		CACHE
		PATH
		"Root directory to search for quatlib")
	if(DEFINED VRPN_ROOT_DIR AND NOT QUATLIB_ROOT_DIR)
		set(QUATLIB_ROOT_DIR "${VRPN_ROOT_DIR}")
		mark_as_advanced(QUATLIB_ROOT_DIR)
	endif()

	if("${CMAKE_SIZEOF_VOID_P}" MATCHES "8")
		set(_libsuffixes lib64 lib)

		# 64-bit dir: only set on win64
		file(TO_CMAKE_PATH "$ENV{ProgramW6432}" _progfiles)
	else()
		set(_libsuffixes lib)
		set(_PF86 "ProgramFiles(x86)")
		if(NOT "$ENV{${_PF86}}" STREQUAL "")
			# 32-bit dir: only set on win64
			file(TO_CMAKE_PATH "$ENV{${_PF86}}" _progfiles)
		else()
			# 32-bit dir on win32, useless to us on win64
			file(TO_CMAKE_PATH "$ENV{ProgramFiles}" _progfiles)
		endif()
	endif()

	# Look for the header file.
	find_path(QUATLIB_INCLUDE_DIR
		NAMES
		quat.h
		HINTS
		"${QUATLIB_ROOT_DIR}"
		PATH_SUFFIXES
		include
		PATHS
		"${_progfiles}/VRPN"
		"${_progfiles}/quatlib"
		C:/usr/local
		/usr/local)

	# Look for the library.
	find_library(QUATLIB_LIBRARY
		NAMES
		quat.lib
		libquat.a
		HINTS
		"${QUATLIB_ROOT_DIR}"
		PATH_SUFFIXES
		${_libsuffixes}
		PATHS
		"${_progfiles}/VRPN"
		"${_progfiles}/quatlib"
		C:/usr/local
		/usr/local)
endif()

# handle the QUIETLY and REQUIRED arguments and set QUATLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(quatlib
	DEFAULT_MSG
	QUATLIB_LIBRARY
	QUATLIB_INCLUDE_DIR)

if(QUATLIB_FOUND)
	set(QUATLIB_LIBRARIES ${QUATLIB_LIBRARY})
	if(NOT WIN32)
		list(APPEND QUATLIB_LIBRARIES m)
	endif()
	set(QUATLIB_INCLUDE_DIRS ${QUATLIB_INCLUDE_DIR})

	mark_as_advanced(QUATLIB_ROOT_DIR)
else()
	set(QUATLIB_LIBRARIES)
	set(QUATLIB_INCLUDE_DIRS)
endif()

mark_as_advanced(QUATLIB_LIBRARY QUATLIB_INCLUDE_DIR)
