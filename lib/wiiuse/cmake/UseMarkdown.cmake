# - Convert markdown source files to HTML as a custom target
#
#  include(UseMarkdown)
#  add_markdown_target(<target_name> <directory to copy to> <markdownfile> [<markdownfile>...] [RENAME <newname>])
#    Relative paths for the destination directory are considered with
#    with respect to CMAKE_CURRENT_BINARY_DIR. The RENAME argument is only
#    valid with a single markdown file as input.
#
#
#  install_markdown_target(<target_name> [extra arguments to INSTALL(FILES ...) ])
#
#
# Requires CMake 2.6 or newer (uses the 'function' command)
#
# Original Author:
# 2011 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2011-2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(__add_markdown_target)
	return()
endif()
set(__add_markdown_target YES)

define_property(TARGET
	PROPERTY
	MARKDOWN_TARGET_OUTPUTS
	BRIEF_DOCS
	"Markdown target outputs"
	FULL_DOCS
	"Output files of a target created by add_markdown_target")

function(add_markdown_target _target _dest)

	if(NOT ARGN)
		message(WARNING
			"In add_markdown_target call for target ${_target}, no source files were specified!")
		return()
	endif()

	find_package(Markdown QUIET)
	if(NOT MARKDOWN_EXECUTABLE)
		message(FATAL_ERROR "Can't find a markdown conversion tool!")
	endif()

	set(NEW_NAME)
	list(FIND ARGN "RENAME" _renameloc)
	if(_renameloc GREATER -1)
		list(LENGTH ARGN _len)
		if(NOT _len EQUAL 3)
			message(FATAL_ERROR
				"Specifying RENAME requires 1 input file and 1 output name!")
		endif()
		list(GET ARGN 2 NEW_NAME)
		list(GET ARGN 0 ARGN)
	endif()

	set(ALLFILES)
	set(SOURCES)
	foreach(fn ${ARGN})
		# Produce an absolute path to the input file
		if(IS_ABSOLUTE "${fn}")
			get_filename_component(fullpath "${fn}" ABSOLUTE)
			get_filename_component(fn "${fn}" NAME)
		else()
			get_filename_component(fullpath
				"${CMAKE_CURRENT_SOURCE_DIR}/${fn}"
				ABSOLUTE)
		endif()
		get_filename_component(fn_noext "${fn}" NAME_WE)

		# Clean up output file name
		if(NEW_NAME)
			get_filename_component(absout "${_dest}/${NEW_NAME}" ABSOLUTE)
		else()
			get_filename_component(absout "${_dest}/${fn_noext}.html" ABSOLUTE)
		endif()

		add_custom_command(OUTPUT "${absout}"
			COMMAND
			${CMAKE_COMMAND}
			ARGS -E make_directory "${_dest}"
			COMMAND
			${MARKDOWN_EXECUTABLE}
			ARGS "${fullpath}" > "${absout}"
			MAIN_DEPENDENCY "${fullpath}"
			VERBATIM
			COMMENT "Converting Markdown ${fn} to HTML in ${absout}...")
		list(APPEND SOURCES "${fullpath}")
		list(APPEND ALLFILES "${absout}")
	endforeach()

	# Custom target depending on all the file copy commands
	add_custom_target(${_target}
		ALL
		SOURCES ${SOURCES}
		DEPENDS ${ALLFILES})
	set_property(TARGET ${_target} PROPERTY MARKDOWN_TARGET_OUTPUTS "${ALLFILES}")
endfunction()

function(install_markdown_target _target)
	get_target_property(_mtoutputs ${_target} MARKDOWN_TARGET_OUTPUTS)
	if(NOT _mtoutputs)
		message(WARNING
			"install_markdown_target called on a target not created with add_markdown_target!")
		return()
	endif()

	# Forward the call to install
	install(FILES ${_mtoutputs} ${ARGN})
endfunction()
