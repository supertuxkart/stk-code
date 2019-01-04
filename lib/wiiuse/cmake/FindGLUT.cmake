# - try to find glut library and include files
#  GLUT_INCLUDE_DIRS, where to find GL/glut.h, etc.
#  GLUT_LIBRARIES, the libraries to link against
#  GLUT_FOUND, If false, do not try to use GLUT.
#  GLUT_RUNTIME_LIBRARY_DIRS, path to DLL on Windows for runtime use.
#  GLUT_RUNTIME_LIBRARY, dll on Windows, for installation purposes
#
# Also defined, but not for general use are:
#  GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  GLUT_glut_LIBRARY = the full path to the glut library.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
# Copyright 2009-2010 Iowa State University
#                     (Author: Ryan Pavlik <abiryan@ryand.net> )
#
# Distributed under the OSI-approved BSD License (the "License");
# see below.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# 
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

if(GLUT_FIND_QUIETLY)
	find_package(OpenGL QUIET)
else()
	find_package(OpenGL)
endif()

if(OPENGL_FOUND)
	get_filename_component(_ogl_libdir ${OPENGL_gl_LIBRARY} PATH)
	find_path(GLUT_INCLUDE_DIR
		NAMES
		GL/glut.h
		GLUT/glut.h
		glut.h
		PATHS
		${_ogl_libdir}/../include
		${GLUT_ROOT_PATH}
		${GLUT_ROOT_PATH}/include
		/usr/include/GL
		/usr/openwin/share/include
		/usr/openwin/include
		/opt/graphics/OpenGL/include
		/opt/graphics/OpenGL/contrib/libglut)

	find_library(GLUT_glut_LIBRARY
		NAMES
		glut
		glut32
		GLUT
		freeglut
		PATHS
		${_ogl_libdir}
		${GLUT_ROOT_PATH}
		${GLUT_ROOT_PATH}/Release
		/usr/openwin/lib)

endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLUT
	DEFAULT_MSG
	GLUT_glut_LIBRARY
	GLUT_INCLUDE_DIR
	OPENGL_FOUND)

if(GLUT_FOUND)
	set(GLUT_LIBRARIES ${GLUT_glut_LIBRARY} ${OPENGL_LIBRARIES})
	set(GLUT_INCLUDE_DIRS ${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

	if(WIN32)
		get_filename_component(_basename "${GLUT_glut_LIBRARY}" NAME_WE)
		get_filename_component(_libpath "${GLUT_glut_LIBRARY}" PATH)
		find_path(GLUT_RUNTIME_LIBRARY
			NAMES
			${_basename}.dll
			glut.dll
			glut32.dll
			freeglut.dll
			HINTS
			${_libpath}
			${_libpath}/../bin)
		if(GLUT_RUNTIME_LIBRARY)
			get_filename_component(GLUT_RUNTIME_LIBRARY_DIRS
				"${GLUT_RUNTIME_LIBRARY}"
				PATH)
		else()
			set(GLUT_RUNTIME_LIBRARY_DIRS)
		endif()
	endif()

	#The following deprecated settings are for backwards compatibility with CMake1.4
	set(GLUT_LIBRARY ${GLUT_LIBRARIES})
	set(GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIR})
endif()

mark_as_advanced(GLUT_INCLUDE_DIR
	GLUT_glut_LIBRARY
	GLUT_RUNTIME_LIBRARY)
