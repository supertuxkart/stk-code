# - Try to find GDB
#
# Once done, this will define:
#  GDB_FOUND - system has GDB
#  GDB_COMMAND - the command to run
#  GDB_VERSION - version
#  GDB_HAS_RETURN_CHILD_RESULT - if the --return-child-result flag is supported
#
# Useful configuration variables you might want to add to your cache:
#  GDB_ROOT_DIR - A directory prefix to search
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


set(GDB_ROOT_DIR
	"${GDB_ROOT_DIR}"
	CACHE
	PATH
	"Directory to start our search in")

find_program(GDB_COMMAND
	NAMES
	gdb
	HINTS
	"${GDB_ROOT_DIR}"
	PATH_SUFFIXES
	bin
	libexec)

if(GDB_COMMAND)
	execute_process(COMMAND
		gdb
		--version
		COMMAND
		head
		-n
		1
		OUTPUT_VARIABLE
		GDB_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX
		REPLACE
		"[^0-9]*([0-9]+[0-9.]*).*"
		"\\1"
		GDB_VERSION
		"${GDB_VERSION}")
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GDB
	DEFAULT_MSG
	GDB_COMMAND
	GDB_VERSION)

if(GDB_FOUND)
	mark_as_advanced(GDB_ROOT_DIR)
	if(GDB_VERSION VERSION_LESS 6.4)
		set(GDB_HAS_RETURN_CHILD_RESULT FALSE)
	else()
		set(GDB_HAS_RETURN_CHILD_RESULT TRUE)
	endif()
endif()

mark_as_advanced(GDB_COMMAND)
