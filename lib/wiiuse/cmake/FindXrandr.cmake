# - try to find the Xrandr library
#
# Cache Variables: (probably not for direct use in your scripts)
#  XRANDR_INCLUDE_DIR
#  XRANDR_SOURCE_DIR
#  XRANDR_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  XRANDR_FOUND
#  XRANDR_INCLUDE_DIRS
#  XRANDR_LIBRARIES
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2014 Kevin M. Godby <kevin@godby.org>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(XRANDR_ROOT_DIR
    "${XRANDR_ROOT_DIR}"
	CACHE
	PATH
    "Directory to search for Xrandr")

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(PC_LIBXRANDR xrandr)
endif()

find_library(XRANDR_LIBRARY
	NAMES
	Xrandr
	PATHS
	${PC_LIBXRANDR_LIBRARY_DIRS}
	${PC_LIBXRANDR_LIBDIR}
	HINTS
	"${XRANDR_ROOT_DIR}"
	PATH_SUFFIXES
	lib
	)

get_filename_component(_libdir "${XRANDR_LIBRARY}" PATH)

find_path(XRANDR_INCLUDE_DIR
	NAMES
	Xrandr.h
	PATHS
	${PC_LIBXRANDR_INCLUDE_DIRS}
	${PC_LIBXRANDR_INCLUDEDIR}
	HINTS
	"${_libdir}"
	"${_libdir}/.."
	"${XRANDR_ROOT_DIR}"
	PATH_SUFFIXES
	X11
	X11/extensions
	)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XRANDR
	DEFAULT_MSG
	XRANDR_LIBRARY
	XRANDR_INCLUDE_DIR
	)

if(XRANDR_FOUND)
	list(APPEND XRANDR_LIBRARIES ${XRANDR_LIBRARY})
	list(APPEND XRANDR_INCLUDE_DIRS ${XRANDR_INCLUDE_DIR})
	mark_as_advanced(XRANDR_ROOT_DIR)
endif()

mark_as_advanced(XRANDR_INCLUDE_DIR
	XRANDR_LIBRARY)

