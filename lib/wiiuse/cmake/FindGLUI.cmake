# - Try to find GLUI (GL User Interface)
# Requires OpenGL and GLUT - searches for them using find_package
# Once done, this will define
#
#	GLUI_INCLUDE_DIR, where to find GL/glui.h (or GLUI/glui.h on mac)
#	GLUI_LIBRARY, the libraries to link against
#	GLUI_FOUND, If false, do not try to use GLUI.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#   GLUI_ROOT_DIR - A directory prefix to search
#                  (usually a path that contains include/ as a subdirectory)
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

if(GLUI_FIND_QUIETLY)
	find_package(OpenGL QUIET)
	find_package(GLUT QUIET)
else()
	find_package(OpenGL)
	find_package(GLUT)
endif()

if(OPENGL_FOUND AND GLUT_FOUND)
	if(WIN32)
		find_path(GLUI_INCLUDE_DIR
			NAMES
			GL/glui.h
			PATHS
			${GLUI_ROOT_PATH}/include
			DOC
			"GLUI include directory")
		find_library(GLUI_LIBRARY
			NAMES
			glui
			${GLUI_ROOT_DIR}/lib
			${GLUI_ROOT_DIR}/Release
			HINTS
			${OPENGL_LIBRARY_DIR}
			${OPENGL_INCLUDE_DIR}/../lib
			DOC
			"GLUI library")
		find_library(GLUI_DEBUG_LIBRARY
			NAMES
			glui32
			${GLUI_ROOT_DIR}/lib
			${GLUI_ROOT_DIR}/Debug
			HINTS
			${OPENGL_LIBRARY_DIR}
			${OPENGL_INCLUDE_DIR}/../lib
			DOC
			"GLUI debug library")
	else()
		find_library(GLUI_LIBRARY
			NAMES
			GLUI
			glui
			PATHS
			${GLUI_ROOT_DIR}/lib64
			${GLUI_ROOT_DIR}/lib
			${GLUI_ROOT_DIR}
			/usr/openwin/lib
			HINTS
			${OPENGL_LIBRARY_DIR}
			${OPENGL_INCLUDE_DIR}/../lib64
			${OPENGL_INCLUDE_DIR}/../lib
			DOC
			"GLUI library")

		if(APPLE)
			find_path(GLUI_INCLUDE_DIR
				GLUI/glui.h
				HINTS
				${OPENGL_INCLUDE_DIR}
				DOC
				"GLUI include directory")
		else()
			find_path(GLUI_INCLUDE_DIR
				GL/glui.h
				PATHS
				${GLUI_ROOT_DIR}/include
				/usr/include/GL
				/usr/openwin/share/include
				/usr/openwin/include
				/opt/graphics/OpenGL/include
				/opt/graphics/OpenGL/contrib/libglui
				DOC
				"GLUI include directory")
		endif()
	endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLUI
	DEFAULT_MSG
	GLUI_INCLUDE_DIR
	GLUI_LIBRARY
	GLUT_FOUND
	OPENGL_FOUND)

if(GLUI_FOUND)
	if(WIN32 AND GLUI_LIBRARY AND GLUI_DEBUG_LIBRARY)
		set(GLUI_LIBRARIES
			optimized
			${GLUI_LIBRARY}
			debug
			${GLUI_DEBUG_LIBRARY}
			${GLUT_LIBRARIES}
			${OPENGL_LIBRARIES})
	else()
		set(GLUI_LIBRARIES
			${GLUI_LIBRARY}
			${GLUT_LIBRARIES}
			${OPENGL_LIBRARIES})
	endif()
	set(GLUI_INCLUDE_DIRS
		${GLUI_INCLUDE_DIR}
		${GLUT_INCLUDE_DIR}
		${OPENGL_INCLUDE_DIR})
endif()

if(GLUI_LIBRARY AND GLUI_INCLUDE_DIR)
	mark_as_advanced(GLUI_INCLUDE_DIR GLUI_LIBRARY GLUI_DEBUG_LIBRARY)
endif()
