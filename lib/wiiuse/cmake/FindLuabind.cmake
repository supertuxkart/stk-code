# - try to find Luabind
#
# Users may optionally supply:
#  LUABIND_ROOT_DIR - a prefix to start searching
#
# Non-cache variables you might use in your CMakeLists.txt:
#  LUABIND_FOUND
#  LUABIND_DEFINITIONS
#  LUABIND_INCLUDE_DIRS
#  LUABIND_LIBRARIES
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

set(LUABIND_ROOT_DIR
	"${LUABIND_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for Luabind")

###
# Dependencies
###
find_package(Lua51 QUIET)

###
# Configure Luabind
###
find_path(LUABIND_INCLUDE_DIR
	NAMES
	luabind/luabind.hpp
	HINTS
	"${LUABIND_ROOT_DIR}"
	PATH_SUFFIXES
	include)
mark_as_advanced(LUABIND_INCLUDE_DIR)

find_library(LUABIND_LIBRARY
	NAMES
	luabind
	HINTS
	"${LUABIND_ROOT_DIR}"
	PATH_SUFFIXES
	lib64
	lib)
mark_as_advanced(LUABIND_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Luabind
	DEFAULT_MSG
	LUABIND_LIBRARY
	LUABIND_INCLUDE_DIR
	LUA_LIBRARIES
	LUA_INCLUDE_DIR)

if(LUABIND_FOUND)
	set(LUABIND_INCLUDE_DIRS "${LUABIND_INCLUDE_DIR}" "${LUA_INCLUDE_DIR}")
	set(LUABIND_LIBRARIES "${LUABIND_LIBRARY}" ${LUA_LIBRARIES})
	set(LUABIND_DEFINITIONS "-DLUABIND_DYNAMIC_LINK")
	mark_as_advanced(LUABIND_ROOT_DIR)
endif()
