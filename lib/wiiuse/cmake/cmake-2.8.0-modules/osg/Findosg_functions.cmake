#
# This CMake file contains two macros to assist with searching for OSG
# libraries and nodekits.
#

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
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

#
# OSG_FIND_PATH
#
function(OSG_FIND_PATH module header)
   string(TOUPPER ${module} module_uc)

   # Try the user's environment request before anything else.
   find_path(${module_uc}_INCLUDE_DIR ${header}
       HINTS
            $ENV{${module_uc}_DIR}
            $ENV{OSG_DIR}
            $ENV{OSGDIR}
            $ENV{OSG_ROOT}
       PATH_SUFFIXES include
       PATHS
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            /usr/freeware
   )
endfunction(OSG_FIND_PATH module header)


#
# OSG_FIND_LIBRARY
#
function(OSG_FIND_LIBRARY module library)
   string(TOUPPER ${module} module_uc)

   find_library(${module_uc}_LIBRARY
       NAMES ${library}
       HINTS
            $ENV{${module_uc}_DIR}
            $ENV{OSG_DIR}
            $ENV{OSGDIR}
            $ENV{OSG_ROOT}
       PATH_SUFFIXES lib64 lib
       PATHS
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            /usr/freeware
   )

   find_library(${module_uc}_LIBRARY_DEBUG
       NAMES ${library}d
       HINTS
            $ENV{${module_uc}_DIR}
            $ENV{OSG_DIR}
            $ENV{OSGDIR}
            $ENV{OSG_ROOT}
       PATH_SUFFIXES lib64 lib
       PATHS
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            /usr/freeware
    )

   if(NOT ${module_uc}_LIBRARY_DEBUG)
      # They don't have a debug library
      set(${module_uc}_LIBRARY_DEBUG ${${module_uc}_LIBRARY} PARENT_SCOPE)
      set(${module_uc}_LIBRARIES ${${module_uc}_LIBRARY} PARENT_SCOPE)
   else()
      # They really have a FOO_LIBRARY_DEBUG
      set(${module_uc}_LIBRARIES 
          optimized ${${module_uc}_LIBRARY}
          debug ${${module_uc}_LIBRARY_DEBUG}
          PARENT_SCOPE
      )
   endif()
endfunction(OSG_FIND_LIBRARY module library)

#
# OSG_MARK_AS_ADVANCED
# Just a convenience function for calling MARK_AS_ADVANCED
#
function(OSG_MARK_AS_ADVANCED _module)
   string(TOUPPER ${_module} _module_UC)
   mark_as_advanced(${_module_UC}_INCLUDE_DIR)
   mark_as_advanced(${_module_UC}_LIBRARY)
   mark_as_advanced(${_module_UC}_LIBRARY_DEBUG)
endfunction()
