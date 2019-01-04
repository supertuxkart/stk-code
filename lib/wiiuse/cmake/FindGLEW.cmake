#.rst:
# FindGLEW
# --------
#
# Find the OpenGL Extension Wrangler Library (GLEW)
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the :prop_tgt:`IMPORTED` target ``GLEW::GLEW``,
# if GLEW has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   GLEW_INCLUDE_DIRS - include directories for GLEW
#   GLEW_LIBRARIES - libraries to link against GLEW
#   GLEW_FOUND - true if GLEW has been found and can be used

#=============================================================================
# Copyright 2012 Benjamin Eikel
# Copyright 2016 Ryan Pavlik
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

find_path(GLEW_INCLUDE_DIR GL/glew.h)
if(WIN32)
  # TODO how to make this exclude arm?
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(GLEW_ARCH Win32)
  else()
    set(GLEW_ARCH x64)
  endif()
  set(GLEW_EXTRA_SUFFIXES lib/Release/${GLEW_ARCH} bin/Release/${GLEW_ARCH})
endif()

if(WIN32 AND GLEW_INCLUDE_DIR)
  get_filename_component(GLEW_LIB_ROOT_CANDIDATE "${GLEW_INCLUDE_DIR}/.." ABSOLUTE)
endif()

find_library(GLEW_LIBRARY
  NAMES GLEW glew32 glew glew32s
  PATH_SUFFIXES lib64 ${GLEW_EXTRA_SUFFIXES}
  HINTS "${GLEW_LIB_ROOT_CANDIDATE}")

if(WIN32 AND GLEW_LIBRARY AND NOT GLEW_LIBRARY MATCHES ".*s.lib")
  get_filename_component(GLEW_LIB_DIR "${GLEW_LIBRARY}" DIRECTORY)
  get_filename_component(GLEW_BIN_ROOT_CANDIDATE1 "${GLEW_LIB_DIR}/.." ABSOLUTE)
  get_filename_component(GLEW_BIN_ROOT_CANDIDATE2 "${GLEW_LIB_DIR}/../../.." ABSOLUTE)
  find_file(GLEW_RUNTIME_LIBRARY
    NAMES glew32.dll
    PATH_SUFFIXES bin ${GLEW_EXTRA_SUFFIXES}
    HINTS
    "${GLEW_BIN_ROOT_CANDIDATE1}"
    "${GLEW_BIN_ROOT_CANDIDATE2}")
endif()

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
set(GLEW_LIBRARIES ${GLEW_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW
                                  REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)

if(GLEW_FOUND AND NOT TARGET GLEW::GLEW)
  if(WIN32 AND GLEW_LIBRARY MATCHES ".*s.lib")
    # Windows, known static library.
    add_library(GLEW::GLEW STATIC IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
      IMPORTED_LOCATION "${GLEW_LIBRARY}"
      PROPERTY INTERFACE_COMPILE_DEFINITIONS GLEW_STATIC)

  elseif(WIN32 AND GLEW_RUNTIME_LIBRARY)
    # Windows, known dynamic library and we have both pieces
    # TODO might be different for mingw
    add_library(GLEW::GLEW SHARED IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
      IMPORTED_LOCATION "${GLEW_RUNTIME_LIBRARY}"
      IMPORTED_IMPLIB "${GLEW_LIBRARY}")
  else()

    # Anything else - previous behavior.
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
      IMPORTED_LOCATION "${GLEW_LIBRARY}")
  endif()

  set_target_properties(GLEW::GLEW PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")

endif()

mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY GLEW_RUNTIME_LIBRARY)
