# - Add appropriate linker flags to show link details on Visual Studio
#
#  include(MSVCVerboseLinking) - to add the flags automaticlly if applicable
#
# Be sure to include this module _BEFORE_ adding your targets, or the targets
# won't pick up the updated flags.
#
# Requires these CMake modules:
#  - none
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

if(MSVC)
	if(NOT DEFINED MSVC_LINK_REALLY_VERBOSE)
		if(IN_DASHBOARD_SCRIPT)
			set(MSVC_LINK_REALLY_VERBOSE TRUE)
		else()
			set(MSVC_LINK_REALLY_VERBOSE FALSE)
		endif()
	endif()
	set(MSVC_LINK_REALLY_VERBOSE
		"${MSVC_LINK_REALLY_VERBOSE}"
		CACHE
		BOOL
		"Provide maximum linker messages?")
	mark_as_advanced(MSVC_LINK_REALLY_VERBOSE)

	if(MSVC_LINK_REALLY_VERBOSE)
		set(_verbose_flag "/VERBOSE")
	else()
		set(_verbose_flag "/VERBOSE:LIB")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS
		"${CMAKE_EXE_LINKER_FLAGS} ${_verbose_flag}")
	set(CMAKE_MODULE_LINKER_FLAGS
		"${CMAKE_MODULE_LINKER_FLAGS} ${_verbose_flag}")
	set(CMAKE_SHARED_LINKER_FLAGS
		"${CMAKE_SHARED_LINKER_FLAGS} ${_verbose_flag}")
endif()
