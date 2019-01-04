# - Automatically fix CMAKE_INSTALL_PREFIX to be bit-appropriate on Win
#
# This is a workaround for CMake bug #9992 in <2.8.3 - see
# http://public.kitware.com/Bug/view.php?id=9992
#
# It runs automatically when included on a Windows build (passes if(WIN32)) -
# include after setting your project name (and your module search path,
# obviously)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file ../LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(WIN32)
	# caution - ENV{ProgramFiles} on Win64 is adjusted to point to the arch
	# of the running executable which, since CMake is 32-bit on Windows as
	# I write this, will always be = $ENV{ProgramFiles(x86)}.
	# Thus, we only use this environment variable if we are on a 32 machine

	# 32-bit dir on win32, useless to us on win64
	file(TO_CMAKE_PATH "$ENV{ProgramFiles}" _PROG_FILES)

	# 32-bit dir: only set on win64
    set(_PF86 "ProgramFiles(x86)")
	file(TO_CMAKE_PATH "$ENV{${_PF86}}" _PROG_FILES_X86)

	# 64-bit dir: only set on win64
	file(TO_CMAKE_PATH "$ENV{ProgramW6432}" _PROG_FILES_W6432)

	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64-bit build on win64
		set(_PROGFILESDIR "${_PROG_FILES_W6432}")
	else()
		if(_PROG_FILES_W6432)
			# 32-bit build on win64
			set(_PROGFILESDIR "${_PROG_FILES_X86}")
		else()
			# 32-bit build on win32
			set(_PROGFILESDIR "${_PROG_FILES}")
		endif()
	endif()

	if(NOT FIXWININSTALLPREFIX_PREFIX)
		set(_needsfix yes)
	elseif(NOT
		"${FIXWININSTALLPREFIX_PREFIX}"
		STREQUAL
		"${CMAKE_INSTALL_PREFIX}")
		set(_needsfix yes)
	else()
		set(_needsfix)
	endif()

	if(_needsfix)
		if("${CMAKE_INSTALL_PREFIX}"
			STREQUAL
			"${_PROG_FILES}/${CMAKE_PROJECT_NAME}")
			# The user didn't change this yet - it's the potentially broken default
			set(CMAKE_INSTALL_PREFIX
				"${_PROGFILESDIR}/${CMAKE_PROJECT_NAME}"
				CACHE
				PATH
				"Where to install the project - has been adjusted by FixWinInstallPrefix"
				FORCE)
		endif()
		set(FIXWININSTALLPREFIX_PREFIX
			"${CMAKE_INSTALL_PREFIX}"
			CACHE
			INTERNAL
			"We've fixed the prefix.")
	endif()
endif()
