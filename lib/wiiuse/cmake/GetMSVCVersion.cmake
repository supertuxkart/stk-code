#.rst:
# GetMSVCVersion
# --------------
#
#
#
# get_msvc_major_version(<var>)
# get_msvc_minor_version(<var>)
# get_msvc_combined_major_minor_version(<var>)
# get_msvc_major_minor_version(<major_var> <minor_var>)
# get_msvc_unique_decimal_version(<var>)
#
# This family of functions is designed to be used to convert
# MSVC_VERSION from the compiler version number to the Visual Studio
# decimal version number (2012 is 11.0, 2015 is 14.0). All take a name
# of a variable in <var> to return their results in.
#
# Consider Visual Studio 2013, which reports 1800 in MSVC_VERSION (and
# the _MSC_VER preprocessor macro). It is also known as VS 12 or 12.0.
# (Minor versions are rarely used, except in the case of 7.1 aka
# VS.NET 2003) The functions would return this output for 2013:
#
# get_msvc_major_version: 12
# get_msvc_minor_version: 0
# get_msvc_combined_major_minor_version: 120
# get_msvc_major_minor_version: 12 in <major_var>, 0 in <minor_var>
# get_msvc_unique_decimal_version: 12  (this returns with a decimal and
# minor when needed to be precise, e.g. 7.1)
#
# The variable is not modified if not building with MSVC.

#=============================================================================
# Copyright 2015 Ryan Pavlik <ryan.pavlik@gmail.com>
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

# This function serves as the main implementation, with the others just
# tweaking the result.
function(get_msvc_combined_major_minor_version _var)
    if(NOT MSVC)
        return()
    endif()
    math(EXPR _vs_ver "${MSVC_VERSION} / 10 - 60")

    # VS 2015 changed the pattern because they skipped VS 13
    if(_vs_ver GREATER 120)
        math(EXPR _vs_ver "${_vs_ver} + 10")
    endif()
    set(${_var} ${_vs_ver} PARENT_SCOPE)
endfunction()

# This function is also used as backend to some implementation, though
# its contents are simpler, no real business logic to speak of.
function(get_msvc_major_minor_version _major_var _minor_var)
    if(NOT MSVC)
        return()
    endif()
    get_msvc_combined_major_minor_version(_vs_ver)

    math(EXPR _vs_minor "${_vs_ver} % 10")
    math(EXPR _vs_major "(${_vs_ver} - ${_vs_minor}) / 10")
    set(${_major_var} ${_vs_major} PARENT_SCOPE)
    set(${_minor_var} ${_vs_minor} PARENT_SCOPE)
endfunction()

function(get_msvc_major_version _var)
    if(NOT MSVC)
        return()
    endif()
    get_msvc_major_minor_version(_vs_ver _dummy)
    set(${_var} ${_vs_ver} PARENT_SCOPE)
endfunction()

function(get_msvc_minor_version _var)
    if(NOT MSVC)
        return()
    endif()
    get_msvc_major_minor_version(_dummy _vs_ver)
    set(${_var} ${_vs_ver} PARENT_SCOPE)
endfunction()

function(get_msvc_unique_decimal_version _var)
    if(NOT MSVC)
        return()
    endif()
    get_msvc_major_minor_version(_vs_major _vs_minor)
    set(_vs_ver ${_vs_major})
    if(_vs_minor GREATER 0)
        set(_vs_ver "${_vs_ver}.${_vs_minor}")
    endif()
    set(${_var} ${_vs_ver} PARENT_SCOPE)
endfunction()
