# select_library_configurations( basename )
#
# This macro takes a library base name as an argument, and will choose good
# values for basename_LIBRARY, basename_LIBRARIES, basename_LIBRARY_DEBUG, and
# basename_LIBRARY_RELEASE depending on what has been found and set.  If only
# basename_LIBRARY_RELEASE is defined, basename_LIBRARY, basename_LIBRARY_DEBUG,
# and basename_LIBRARY_RELEASE will be set to the release value.  If only
# basename_LIBRARY_DEBUG is defined, then basename_LIBRARY,
# basename_LIBRARY_DEBUG and basename_LIBRARY_RELEASE will take the debug value.  
#
# If the generator supports configuration types, then basename_LIBRARY and
# basename_LIBRARIES will be set with debug and optimized flags specifying the
# library to be used for the given configuration.  If no build type has been set
# or the generator in use does not support configuration types, then
# basename_LIBRARY and basename_LIBRARIES will take only the release values.

#=============================================================================
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Will Dicharry <wdicharry@stellarscience.com>
# Copyright 2005-2009 Kitware, Inc.
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
# This macro was adapted from the FindQt4 CMake module and is maintained by Will
# Dicharry <wdicharry@stellarscience.com>.

# Utility macro to check if one variable exists while another doesn't, and set
# one that doesn't exist to the one that exists.
macro( _set_library_name basename GOOD BAD )
    if( ${basename}_LIBRARY_${GOOD} AND NOT ${basename}_LIBRARY_${BAD} )
        set( ${basename}_LIBRARY_${BAD} ${${basename}_LIBRARY_${GOOD}} )
        set( ${basename}_LIBRARY ${${basename}_LIBRARY_${GOOD}} )
        set( ${basename}_LIBRARIES ${${basename}_LIBRARY_${GOOD}} )
    endif( ${basename}_LIBRARY_${GOOD} AND NOT ${basename}_LIBRARY_${BAD} )
endmacro( _set_library_name )

macro( select_library_configurations basename )
    # if only the release version was found, set the debug to be the release
    # version.
    _set_library_name( ${basename} RELEASE DEBUG )
    # if only the debug version was found, set the release value to be the
    # debug value.
    _set_library_name( ${basename} DEBUG RELEASE )
    if (${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE )
        # if the generator supports configuration types or CMAKE_BUILD_TYPE
        # is set, then set optimized and debug options.
        if( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE )
            set( ${basename}_LIBRARY 
                optimized ${${basename}_LIBRARY_RELEASE}
                debug ${${basename}_LIBRARY_DEBUG} )
            set( ${basename}_LIBRARIES 
                optimized ${${basename}_LIBRARY_RELEASE}
                debug ${${basename}_LIBRARY_DEBUG} )
        else( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE )
            # If there are no configuration types or build type, just use
            # the release version
            set( ${basename}_LIBRARY ${${basename}_LIBRARY_RELEASE} )
            set( ${basename}_LIBRARIES ${${basename}_LIBRARY_RELEASE} )
        endif( CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE )
    endif( ${basename}_LIBRARY_DEBUG AND ${basename}_LIBRARY_RELEASE )

    set( ${basename}_LIBRARY ${${basename}_LIBRARY} CACHE FILEPATH 
        "The ${basename} library" )

    if( ${basename}_LIBRARY )
        set( ${basename}_FOUND TRUE )
    endif( ${basename}_LIBRARY )

    mark_as_advanced( ${basename}_LIBRARY 
        ${basename}_LIBRARY_RELEASE
        ${basename}_LIBRARY_DEBUG
    )
endmacro( select_library_configurations )

