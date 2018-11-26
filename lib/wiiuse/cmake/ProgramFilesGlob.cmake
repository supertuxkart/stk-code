# - Find bit-appropriate program files directories matching a given pattern
#
# Requires these CMake modules:
#  CleanDirectoryList
#  PrefixListGlob
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

include(PrefixListGlob)
include(CleanDirectoryList)

if(__program_files_glob)
	return()
endif()
set(__program_files_glob YES)

macro(_program_files_glob_var_prep)
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
endmacro()

function(program_files_glob var pattern)
	_program_files_glob_var_prep()
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64-bit build on win64
		set(_PROGFILESDIRS "${_PROG_FILES_W6432}")
	else()
		if(_PROG_FILES_W6432)
			# 32-bit build on win64
			set(_PROGFILESDIRS "${_PROG_FILES_X86}")
		else()
			# 32-bit build on win32
			set(_PROGFILESDIRS "${_PROG_FILES}")
		endif()
	endif()

	prefix_list_glob(_prefixed "${pattern}" ${_PROGFILESDIRS})
	clean_directory_list(_prefixed)
	set(${var} ${_prefixed} PARENT_SCOPE)
endfunction()

function(program_files_fallback_glob var pattern)
	_program_files_glob_var_prep()
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64-bit build on win64
		# look in the "32 bit" (c:\program files (x86)\) directory as a
		# fallback in case of weird/poorly written installers, like those
		# that put both 64- and 32-bit libs in the same program files directory
		set(_PROGFILESDIRS "${_PROG_FILES_W6432}" "${_PROG_FILES_X86}")
	else()
		if(_PROG_FILES_W6432)
			# 32-bit build on win64
			# look in the "64 bit" (c:\program files\) directory as a fallback
			# in case of old/weird/poorly written installers
			set(_PROGFILESDIRS "${_PROG_FILES_X86}" "${_PROG_FILES_W6432}")
		else()
			# 32-bit build on win32
			set(_PROGFILESDIRS "${_PROG_FILES}")
		endif()
	endif()

	prefix_list_glob(_prefixed "${pattern}" ${_PROGFILESDIRS})
	clean_directory_list(_prefixed)
	set(${var} ${_prefixed} PARENT_SCOPE)
endfunction()
