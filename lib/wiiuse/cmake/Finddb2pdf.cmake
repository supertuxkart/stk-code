# - Try to find db2pdf, and define a custom command to turn docbook into pdf
#
# Once done, this will define:
#  DB2PDF_FOUND - system has LyX
#  DB2PDF_COMMAND - the command to run
#
# and the following function:
#  docbook_to_pdf(<output-variable> <docbook files>)
#
# Useful configuration variables you might want to add to your cache:
#  DB2PDF_ROOT_DIR - A directory prefix to search
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


set(DB2PDF_ROOT_DIR
	"${DB2PDF_ROOT_DIR}"
	CACHE
	PATH
	"Directory to start our search in")

find_program(DB2PDF_COMMAND
	NAMES
	db2pdf
	HINTS
	"${DB2PDF_ROOT_DIR}"
	PATH_SUFFIXES
	bin)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(db2pdf DEFAULT_MSG DB2PDF_COMMAND)

if(DB2PDF_FOUND)
	mark_as_advanced(DB2PDF_ROOT_DIR)
endif()

mark_as_advanced(DB2PDF_COMMAND)

function(docbook_to_pdf _outvar)
	set(INPUT ${ARGN})
	set(_out)
	set(_outname)

	foreach(_file ${INPUT})
		get_filename_component(_base "${_file}" NAME_WE)
		set(_outname "${CMAKE_CURRENT_BINARY_DIR}/${_base}.pdf")
		list(APPEND _out "${_outname}")
		if(DB2PDF_COMMAND)
			add_custom_command(OUTPUT
				"${_outname}"
				COMMAND
				${DB2PDF_COMMAND}
				-o
				"${CMAKE_CURRENT_BINARY_DIR}"
				"${_file}"
				WORKING_DIRECTORY
				"${CMAKE_CURRENT_SOURCE_DIR}"
				MAIN_DEPENDENCY
				"${_file}")
		endif()
	endforeach()
	set(${_outvar} ${_out} PARENT_SCOPE)
endfunction()
