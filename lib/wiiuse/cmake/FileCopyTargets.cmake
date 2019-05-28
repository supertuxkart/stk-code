# - Add a target for files that just need to be copied
#
#  include(FileCopyTargets)
#  add_file_copy_target(<target_name> <directory to copy to> <filename> [<filename>...])
#    Creates a custom target that copies the files to a directory, if needed.
#    Relative paths for the destination directory are considered with
#    with respect to CMAKE_CURRENT_BINARY_DIR
#    You can use this target in all the usual ways, including
#    add_dependencies(some_other_target this_target) to specify that another
#    target depends on this.
#
#  install_file_copy_target(<target_name> [arguments to INSTALL(PROGRAMS ...) ])
#    Works just the same as INSTALL(PROGRAMS ...) because it wraps it to install
#    the files you specified in add_file_copy_target
#
#
# Requires CMake 2.6 or newer (uses the 'function' command)
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

if(__add_file_copy_target)
	return()
endif()
set(__add_file_copy_target YES)

define_property(TARGET
	PROPERTY
	FILE_COPY_TARGET
	BRIEF_DOCS
	"File Copy target"
	FULL_DOCS
	"Is this a target created by add_file_copy_target?")

function(add_file_copy_target _target _dest)
	if(NOT ARGN)
		message(WARNING
			"In add_file_copy_target call for target ${_target}, no source files were specified!")
		return()
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

		# Clean up output file name
		get_filename_component(absout "${_dest}/${fn}" ABSOLUTE)

		add_custom_command(OUTPUT "${absout}"
			COMMAND
			${CMAKE_COMMAND}
			ARGS -E make_directory "${_dest}"
			COMMAND
			${CMAKE_COMMAND}
			ARGS -E copy "${fullpath}" "${_dest}"
			MAIN_DEPENDENCY "${fullpath}"
			VERBATIM
			COMMENT "Copying ${fn} to ${absout}")
		list(APPEND SOURCES "${fullpath}")
		list(APPEND ALLFILES "${absout}")
	endforeach()

	# Custom target depending on all the file copy commands
	add_custom_target(${_target}
		SOURCES ${SOURCES}
		DEPENDS ${ALLFILES})

	set_property(TARGET ${_target} PROPERTY FILE_COPY_TARGET YES)
endfunction()

function(install_file_copy_target _target)
	get_target_property(_isFCT ${_target} FILE_COPY_TARGET)
	if(NOT _isFCT)
		message(WARNING
			"install_file_copy_target called on a target not created with add_file_copy_target!")
		return()
	endif()

	# Get sources
	get_target_property(_srcs ${_target} SOURCES)

	# Remove the "fake" file forcing build
	list(REMOVE_AT _srcs 0)

	# Forward the call to install
	install(PROGRAMS ${_srcs} ${ARGN})
endfunction()
