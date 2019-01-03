# - try to find Sensable GHOST library and include files
#  GHOST_INCLUDE_DIRS, where to find GL/glut.h, etc.
#  GHOST_LIBRARIES, the libraries to link against
#  GHOST_FOUND, If false, do not try to use GLUT.
#  GHOST_RUNTIME_LIBRARY_DIRS, path to DLL on Windows for runtime use.
#
# Requires these CMake modules:
#  no additional modules required
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

set(GHOST_ROOT_DIR
	"${GHOST_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for GHOST")

set(_dirs)
if(WIN32)
	include(ProgramFilesGlob)
	program_files_fallback_glob(_dirs "/Sensable/GHOST/v*/")
endif()

find_path(GHOST_INCLUDE_DIR
	gstPHANToM.h
	PATHS
	${_dirs}
	HINTS
	"${GHOST_ROOT_DIR}"
	PATH_SUFFIXES
	include)

find_library(GHOST_LIBRARY
	GHOST40
	GHOST31
	PATHS
	${_dirs}
	HINTS
	"${GHOST_ROOT_DIR}"
	PATH_SUFFIXES
	lib)

if(MSVC)
	if(MSVC_VERSION GREATER 1300)
		# .NET and newer: fake the STL headers
		get_filename_component(_moddir "${CMAKE_CURRENT_LIST_FILE}" PATH)
		set(GHOST_STL_INCLUDE_DIR "${_moddir}/ghost-fake-stl")
	else()
		# 6.0 and earlier - use GHOST-provided STL
		find_path(GHOST_STL_INCLUDE_DIR
			vector.h
			PATHS
			${_dirs}
			HINTS
			"${GHOST_ROOT_DIR}"
			"${GHOST_INCLUDE_DIR}"
			PATH_SUFFIXES
			external/stl
			stl)
	endif()
	set(_deps_check GHOST_STL_INCLUDE_DIR)
else()
	set(_deps_check)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GHOST
	DEFAULT_MSG
	GHOST_LIBRARY
	${_deps_check}
	GHOST_INCLUDE_DIR)

if(GHOST_FOUND)
	set(GHOST_LIBRARIES "${GHOST_LIBRARY}")
	set(GHOST_INCLUDE_DIRS "${GHOST_INCLUDE_DIR}")

	mark_as_advanced(GHOST_ROOT_DIR)
endif()

mark_as_advanced(GHOST_LIBRARY GHOST_STL_INCLUDE_DIR GHOST_INCLUDE_DIR)
