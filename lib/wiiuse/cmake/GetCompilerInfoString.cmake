# - Utility function to return a human-useful-only string ID'ing the compiler
#
#  get_compiler_info_string(<resultvar>)
#
# and some helper functions:
#  get_gcc_version(<resultvar>)
#  get_vs_short_version_string(<generator> <resultvar>)
#
# You might consider using it when setting up CTest options, for example:
#  include(GetCompilerInfoString)
#  get_compiler_info_string(COMPILERID)
#  set(CTEST_BUILD_NAME "${CMAKE_SYSTEM}-${CMAKE_SYSTEM_PROCESSOR}-${COMPILERID}")
#
# Requires these CMake modules:
#  no additional modules required
#
# Original Author:
# 2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
# Some functions based on cmake-2.8.0 modules FindBoost.cmake and CTest.cmake
#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006-2008 Andreas Schneider <mail@cynapses.org>
# Copyright 2007      Wengo
# Copyright 2007      Mike Jackson
# Copyright 2008      Andreas Pakulat <apaku@gmx.de>
# Copyright 2008-2009 Philip Lowman <philip@yhbt.com>
# Copyright 2010      Iowa State University (Ryan Pavlik <abiryan@ryand.net>)
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
# All rights reserved.
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

if(__get_compiler_info_string)
	return()
endif()
set(__get_compiler_info_string YES)


function(get_compiler_info_string _var)
	set(_out)

	if(CTEST_CMAKE_GENERATOR AND NOT CMAKE_GENERATOR)
		# We're running in CTest - use that generator.
		set(CMAKE_GENERATOR ${CTEST_CMAKE_GENERATOR})
	endif()

	if(NOT CMAKE_CXX_COMPILER)
		# Also for use in CTest scripts
		include(CMakeDetermineCXXCompiler)
	endif()

	if(MSVC)
		# Parse version for Visual Studio
		get_vs_short_version_string("${CMAKE_GENERATOR}" _verstring)
		if(${CMAKE_GENERATOR} MATCHES "Win64")
			set(_verstring "${_verstring}win64")
		endif()

	elseif(CMAKE_COMPILER_IS_GNUCXX)
		# Parse version for GCC
		get_gcc_version(_gccver)
		set(_verstring "gcc${_gccver}")

	else()
		# Some other compiler we don't handle in more detail yet.
		string(REGEX REPLACE " " "_" _verstring "${CMAKE_GENERATOR}")
		set(_verstring "${CMAKE_CXX_COMPILER_ID}:generator:${_verstring}")
	endif()

	# Return _verstring
	set(${_var} "${_verstring}" PARENT_SCOPE)
endfunction()

## Based on a function in FindBoost.cmake from CMake 2.8.0
#-------------------------------------------------------------------------------
#
# Runs compiler with "-dumpversion" and parses major/minor
# version with a regex.
#
function(get_gcc_version _var)
	exec_program(${CMAKE_CXX_COMPILER}
		ARGS
		${CMAKE_CXX_COMPILER_ARG1}
		-dumpversion
		OUTPUT_VARIABLE
		_compilerinfo_COMPILER_VERSION)

	string(REGEX
		MATCH
		"([.0-9]+)"
		"\\1"
		_compilerinfo_COMPILER_VERSION
		"${_compilerinfo_COMPILER_VERSION}")

	set(${_var} ${_compilerinfo_COMPILER_VERSION} PARENT_SCOPE)
endfunction()

## Based on a function in CTest.cmake from CMake 2.8.0
#-------------------------------------------------------------------------------
#
# function to turn generator name into a version string
# like vs7 vs71 vs8 vs9
#
function(get_vs_short_version_string _generator _var)
	set(_ver_string)
	if("${_generator}" MATCHES "Visual Studio")
		string(REGEX
			REPLACE
			"Visual Studio ([0-9][0-9]?)($|.*)"
			"\\1"
			_vsver
			"${_generator}")
		if("${_generator}" MATCHES "Visual Studio 7 .NET 2003")
			# handle the weird one
			set(_ver_string "vs71")
		else()
			set(_ver_string "vs${_vsver}")
		endif()
	elseif(MSVC)
		if(MSVC71)
			set(_ver_string "vs71")
		else()
			foreach(_ver 6 7 8 9 10)
				if(MSVC${_ver}0)
					set(_ver_string "vs${_ver}")
					break()
				endif()
			endforeach()
		endif()
	endif()

	if(_ver_string)
		set(${_var} ${_ver_string} PARENT_SCOPE)
	endif()
endfunction()
