# - Try to find LyX, and define some custom commands to export from LyX
#
# Once done, this will define:
#  LYX_FOUND - system has LyX
#  LYX_COMMAND - the command to run
#
# and the following new functions:
#  lyx_export(<format> <extension-without-leading-dot> <output-variable>
#    INPUT <lyx-file> [...]
#    [OUTPUT_TO_SOURCE_DIR]
#    [ EXTRA_DEPS <bibtex-or-other-file> [...] ]) - the base function
#
# These shortcut functions all have the same syntax:
#  lyx_export_to_XXX(<output-variable>
#    INPUT <lyx-file> [...]
#    [OUTPUT_TO_SOURCE_DIR]
#    [ EXTRA_DEPS <bibtex-or-other-file> [...] ])
#
# Available shortcuts:
#  lyx_export_to_docbook_xml
#  lyx_export_to_docbook
#  lyx_export_to_pdf
#  lyx_export_to_pdf_via_pdflatex
#  lyx_export_to_pdf_via_dvi
#
# Useful configuration variables you might want to add to your cache:
#  LYX_ROOT_DIR - A directory prefix to search
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


set(LYX_ROOT_DIR
	"${LYX_ROOT_DIR}"
	CACHE
	PATH
	"Directory to start our search in")

find_program(LYX_COMMAND
	NAMES
	lyx
	HINTS
	"${LYX_ROOT_DIR}"
	PATH_SUFFIXES
	bin)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LyX DEFAULT_MSG LYX_COMMAND)

if(LYX_FOUND)
	mark_as_advanced(LYX_ROOT_DIR)
endif()

mark_as_advanced(LYX_COMMAND)

function(lyx_export _format _extension _outvar)
	set(_nowhere)
	set(_curdest _nowhere)
	set(_val_args EXTRA_DEPS INPUT)
	set(_bool_args OUTPUT_TO_SOURCE_DIR)
	foreach(_arg ${_val_args} ${_bool_args})
		set(${_arg})
	endforeach()
	foreach(_element ${ARGN})
		list(FIND _val_args "${_element}" _val_arg_find)
		list(FIND _bool_args "${_element}" _bool_arg_find)
		if("${_val_arg_find}" GREATER "-1")
			set(_curdest "${_element}")
		elseif("${_bool_arg_find}" GREATER "-1")
			set("${_element}" ON)
			set(_curdest _nowhere)
		else()
			list(APPEND ${_curdest} "${_element}")
		endif()
	endforeach()

	if(_nowhere)
		message(FATAL_ERROR "Syntax error in use of a lyx_export command!")
	endif()

	set(_out)
	set(_outname)
	foreach(_file ${INPUT})
		get_filename_component(_base "${_file}" NAME_WE)

		if(NOT OUTPUT_TO_SOURCE_DIR)
			set(_outname "${CMAKE_CURRENT_BINARY_DIR}/${_base}.${_extension}")
		else()
			set(_outname "${CMAKE_CURRENT_SOURCE_DIR}/${_base}.${_extension}")
		endif()

		list(APPEND _out "${_outname}")
		if(LYX_COMMAND)
			add_custom_command(OUTPUT "${_outname}"
				COMMAND ${CMAKE_COMMAND} -E remove "${_outname}"
				#COMMAND ${LYX_COMMAND} "${_file}" --export ${_format}
				COMMAND ${LYX_COMMAND} "${_file}"
				--execute
				"buffer-export-custom ${_format} ${CMAKE_COMMAND} -E copy '$$$$FName' '${_outname}'"
				--execute
				"lyx-quit"
				MAIN_DEPENDENCY "${_file}"
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				DEPENDS "${_file}" ${EXTRA_DEPS}
				COMMENT "Exporting ${_file} to ${_format}...")
		endif()
	endforeach()

	set(${_outvar} ${_out} PARENT_SCOPE)
endfunction()

function(lyx_export_to_docbook_xml _outvar)
	lyx_export(docbook-xml xml ${_outvar} ${ARGN})
	set(${_outvar} ${${_outvar}} PARENT_SCOPE)
endfunction()

function(lyx_export_to_docbook _outvar)
	lyx_export(docbook sgml ${_outvar} ${ARGN})
	set(${_outvar} ${${_outvar}} PARENT_SCOPE)
endfunction()

function(lyx_export_to_pdf _outvar)
	lyx_export(pdf pdf ${_outvar} ${ARGN})
	set(${_outvar} ${${_outvar}} PARENT_SCOPE)
endfunction()

function(lyx_export_to_pdf_via_pdflatex _outvar)
	lyx_export(pdf2 pdf ${_outvar} ${ARGN})
	set(${_outvar} ${${_outvar}} PARENT_SCOPE)
endfunction()

function(lyx_export_to_pdf_via_dvi _outvar)
	lyx_export(pdf3 pdf ${_outvar} ${ARGN})
	set(${_outvar} ${${_outvar}} PARENT_SCOPE)
endfunction()
