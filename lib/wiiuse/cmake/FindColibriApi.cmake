# - try to find Trivisio Colibri API library
#
# Cache Variables:
#  COLIBRIAPI_LIBRARY
#  COLIBRIAPI_INCLUDE_DIR
#
# Non-cache variables you might use in your CMakeLists.txt:
#  COLIBRIAPI_FOUND
#  COLIBRIAPI_SERVER_LIBRARIES - server libraries
#  COLIBRIAPI_LIBRARIES - client libraries
#  COLIBRIAPI_CLIENT_DEFINITIONS - definitions if you only use the client library
#  COLIBRIAPI_DEFINITIONS - Client-only definition if all we found was the client library.
#  COLIBRIAPI_INCLUDE_DIRS
#
# COLIBRIAPI_ROOT_DIR is searched preferentially for these files
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2009-2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(COLIBRIAPI_ROOT_DIR
	"${COLIBRIAPI_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for Colibri API")

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

###
# Configure COLIBRIAPI
###

find_path(COLIBRIAPI_INCLUDE_DIR
	NAMES
	colibri_api.h
	PATH_SUFFIXES
	include
	HINTS
	"${COLIBRIAPI_ROOT_DIR}"
	PATHS
	"${_progfiles}/ColibriAPI 1.1.20140925 (64-bit)"
	"$ENV{HOME}"
	"$ENV{HOME}/ColibriAPI-1.1.20140710-Linux-amd64"
	C:/usr/local
	/usr/local)

find_library(COLIBRIAPI_LIBRARY
	NAMES
	colibri-api
	PATH_SUFFIXES
	${_libsuffixes}
	HINTS
	"${COLIBRIAPI_ROOT_DIR}"
	PATHS
	"${_progfiles}/ColibriAPI 1.1.20140925 (64-bit)"
	"$ENV{HOME}"
	"$ENV{HOME}/ColibriAPI-1.1.20140710-Linux-amd64"
	C:/usr/local
	/usr/local)

###
# Dependencies
###
set(_deps_libs)
set(_deps_includes)
set(_deps_check)

find_package(quatlib)
list(APPEND _deps_libs ${QUATLIB_LIBRARIES})
list(APPEND _deps_includes ${QUATLIB_INCLUDE_DIRS})
list(APPEND _deps_check QUATLIB_FOUND)

if(NOT WIN32)
	find_package(Threads)
	list(APPEND _deps_libs ${CMAKE_THREAD_LIBS_INIT})
	list(APPEND _deps_check CMAKE_HAVE_THREADS_LIBRARY)
endif()


# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ColibriApi
	DEFAULT_MSG
	COLIBRIAPI_LIBRARY
	COLIBRIAPI_INCLUDE_DIR
	${_deps_check})

if(COLIBRIAPI_FOUND)
	set(COLIBRIAPI_INCLUDE_DIRS "${COLIBRIAPI_INCLUDE_DIR}" ${_deps_includes})
	set(COLIBRIAPI_LIBRARIES "${COLIBRIAPI_LIBRARY}" ${_deps_libs})

	mark_as_advanced(COLIBRIAPI_ROOT_DIR)
endif()

mark_as_advanced(COLIBRIAPI_LIBRARY COLIBRIAPI_INCLUDE_DIR)
