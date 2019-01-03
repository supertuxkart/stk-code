# - Script to compile Win32-developed sources using tchar without modifying the code
# Requires that ${CMAKE_SOURCE_DIR}/cmake/workarounds/tchar/ be present.
#
#	TCHAR_WORKAROUND, automatically set to on when not on win32
#	TCHAR_INCLUDE_DIR, location of our fake tchar.h file
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

if(NOT WIN32)
	option(TCHAR_WORKAROUND "Work around missing tchar error" on)
else()
	option(TCHAR_WORKAROUND "Work around missing tchar error" off)
endif()

mark_as_advanced(TCHAR_WORKAROUND)

if(TCHAR_WORKAROUND)
	find_path(TCHAR_INCLUDE_DIR
		tchar.h
		PATHS
		${CMAKE_SOURCE_DIR}/cmake/workarounds/tchar/
		./workarounds/tchar/
		PATH_SUFFIXES
		workarounds/
		workarounds/tchar/)
	if(TCHAR_INCLUDE_DIR)
		include_directories(${TCHAR_INCLUDE_DIR})
		mark_as_advanced(TCHAR_INCLUDE_DIR)
	endif()
endif()
