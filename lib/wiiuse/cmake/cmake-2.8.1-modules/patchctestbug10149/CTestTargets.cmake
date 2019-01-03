
#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
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

if(NOT RUN_FROM_CTEST_OR_DART)
	message(FATAL_ERROR "Do not incldue CTestTargets.cmake directly")
endif()

# make directories in the binary tree
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/Testing/Temporary)
get_filename_component(CMAKE_HOST_PATH ${CMAKE_COMMAND} PATH)
set(CMAKE_TARGET_PATH ${EXECUTABLE_OUTPUT_PATH})
find_program(CMAKE_CTEST_COMMAND
	ctest
	${CMAKE_HOST_PATH}
	${CMAKE_TARGET_PATH})
mark_as_advanced(CMAKE_CTEST_COMMAND)

# Use CTest
# configure files

if(CTEST_NEW_FORMAT)
	configure_file(${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
		${PROJECT_BINARY_DIR}/CTestConfiguration.ini)
else()
	configure_file(${CMAKE_ROOT}/Modules/DartConfiguration.tcl.in
		${PROJECT_BINARY_DIR}/DartConfiguration.tcl)
endif()

#
# Section 3:
#
# Custom targets to perform dashboard builds and submissions.
# These should NOT need to be modified from project to project.
#

set(__conf_types "")
if(CMAKE_CONFIGURATION_TYPES)
	# We need to pass the configuration type on the test command line.
	set(__conf_types -C "${CMAKE_CFG_INTDIR}")
endif()

# Add convenience targets.  Do this at most once in case of nested
# projects.
define_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED
	BRIEF_DOCS "Internal property used by CTestTargets module."
	FULL_DOCS "Set by the CTestTargets module to track addition of testing targets.")

get_property(_CTEST_TARGETS_ADDED GLOBAL PROPERTY CTEST_TARGETS_ADDED)
if(NOT _CTEST_TARGETS_ADDED)
	set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

	# For all generators add basic testing targets.
	foreach(mode Experimental Nightly Continuous NightlyMemoryCheck)
		add_custom_target(${mode}
			${CMAKE_CTEST_COMMAND}
			${__conf_types}
			-D
			${mode})
		set_property(TARGET ${mode} PROPERTY RULE_LAUNCH_CUSTOM "")
	endforeach()

	# For Makefile generators add more granular targets.
	if("${CMAKE_GENERATOR}" MATCHES Make)
		# Make targets for Experimental builds
		foreach(mode Nightly Experimental Continuous)
			foreach(testtype
				Start
				Update
				Configure
				Build
				Test
				Coverage
				MemCheck
				Submit)	# missing purify

				add_custom_target(${mode}${testtype}
					${CMAKE_CTEST_COMMAND}
					${__conf_types}
					-D
					${mode}${testtype})
				set_property(TARGET
					${mode}${testtype}
					PROPERTY
					RULE_LAUNCH_CUSTOM
					"")
			endforeach()
		endforeach()
	endif() # makefile generators

endif()
