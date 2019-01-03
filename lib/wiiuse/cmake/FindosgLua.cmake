# - try to find osgLua
#
# Users may optionally supply:
#  OSGLUA_ROOT_DIR - a prefix to start searching
#
# Non-cache variables you might use in your CMakeLists.txt:
#  OSGLUA_FOUND
#  OSGLUA_INCLUDE_DIRS
#  OSGLUA_LIBRARIES
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

set(OSGLUA_ROOT_DIR
	"${OSGLUA_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for osgLua")

###
# Dependencies
###
find_package(Lua51 QUIET)

###
# Configure Luabind
###
find_path(OSGLUA_INCLUDE_DIR
	NAMES
	osgLua/Script
	HINTS
	"${OSGLUA_ROOT_DIR}"
	PATH_SUFFIXES
	include)
mark_as_advanced(OSGLUA_INCLUDE_DIR)

find_library(OSGLUA_LIBRARY
	NAMES
	osgLua
	HINTS
	"${OSGLUA_ROOT_DIR}"
	PATH_SUFFIXES
	lib64
	lib)
mark_as_advanced(OSGLUA_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(osgLua
	DEFAULT_MSG
	OSGLUA_LIBRARY
	OSGLUA_INCLUDE_DIR
	LUA_LIBRARIES
	LUA_INCLUDE_DIR)

if(OSGLUA_FOUND)
	set(OSGLUA_INCLUDE_DIRS "${OSGLUA_INCLUDE_DIR}" "${LUA_INCLUDE_DIR}")
	set(OSGLUA_LIBRARIES "${OSGLUA_LIBRARY}" ${LUA_LIBRARIES})
	mark_as_advanced(OSGLUA_ROOT_DIR)
endif()
