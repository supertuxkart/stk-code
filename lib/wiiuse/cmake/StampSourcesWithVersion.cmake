# - When enabled, stamp the current version on C/C++ sources
#
# To set up your source code for proper stamping, start your file
# with a Doxygen-style comment block, starting with /* and ending with */
# On a line by itself, with unimportant whitespace, add the standard Doxygen
# "version" command:
#  @version xxx
# or
#  \version xxx
#
# To make sure it works, please do actually put xxx as the current version:
# when you save, add one of the command below to your cmake build, and run
# cmake, it should replace xxx with the current version. (It replaces anything
# between the end of the whitespace after \version and the end of the line
# with the version that you pass in your build script, so put nothing else
# on that line!)
#
# For <version>, I recommend passing the value of a CMake variable like
#  ${CPACK_PACKAGE_VERSION}
# Remember, reduced duplication of information means reduced errors!
#
# WARNING!
# This does edit your source directory, but will only execute if the
# (hidden/advanced, default OFF) variable ENABLE_VERSION_STAMPING is on.
#
# Additionally, it tries to be very careful:
# - It will not edit files that are outside your source tree
# - It will only attempt a substitution within the first C-style comment block
# of your code (that is, the first /* */), but only if // is not found first
#
#  stamp_target_with_version(<version> <target_name> [HEADERS_ONLY | <source>...]) -
#   If no source file is specified, all will be processed.
#
#  stamp_sources_with_version(<version> <source> [<source> ...]) -
#   Use for files not directly associated with a target.
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

if(__stamp_sources_with_version)
	return()
endif()
set(__stamp_sources_with_version YES)

if(NOT APPLE)
	option(ENABLE_VERSION_STAMPING
		"Modify source files to update the version in the comment header. Maintainers only!"
		OFF)
	mark_as_advanced(ENABLE_VERSION_STAMPING)
endif()

# Stash where our data is, at include() time
get_filename_component(_sswv_mod_dir ${CMAKE_CURRENT_LIST_FILE} PATH)


# Internal utility function - not for outside use
function(_stamp_file_with_version version filename)
	if(NOT SED_EXECUTABLE)
		find_program(SED_EXECUTABLE sed)
		mark_as_advanced(SED_EXECUTABLE)
	endif()
	# TODO: fix the sed script on Mac
	if(SED_EXECUTABLE AND ENABLE_VERSION_STAMPING AND NOT APPLE)
		get_source_file_property(_abs "${filename}" LOCATION)
		if(NOT _abs)
			get_filename_component(_abs "${filename}" ABSOLUTE)
		endif()
		file(RELATIVE_PATH _rel "${CMAKE_SOURCE_DIR}" "${_abs}")
		if(NOT "${_rel}" MATCHES "[.][.]")
			# Only if this file is in the source tree
			get_filename_component(_name "${filename}" NAME)
			set(_in_source_dir YES)
			# Create the sed script
			configure_file("${_sswv_mod_dir}/StampSourcesWithVersion.sed.in"
				"${CMAKE_CURRENT_BINARY_DIR}/stamp-${_name}.sed"
				@ONLY)

			if(APPLE)
				set(extendedre_arg -E)
			else()
				set(extendedre_arg -r)
			endif()

			set(sedargs
				${extendedre_arg}
				-f
				"${CMAKE_CURRENT_BINARY_DIR}/stamp-${_name}.sed"
				${filename})

			# Run the sed script
			execute_process(COMMAND
				${SED_EXECUTABLE}
				${sedargs}
				OUTPUT_FILE
				"${CMAKE_CURRENT_BINARY_DIR}/stampedoutput-${_name}.out"
				WORKING_DIRECTORY
				"${CMAKE_CURRENT_SOURCE_DIR}")

			# Check to see if changes were made
			execute_process(COMMAND
				${CMAKE_COMMAND}
				-E
				compare_files
				"${CMAKE_CURRENT_BINARY_DIR}/stampedoutput-${_name}.out"
				${filename}
				WORKING_DIRECTORY
				"${CMAKE_CURRENT_SOURCE_DIR}"
				RESULT_VARIABLE
				files_different
				OUTPUT_QUIET
				ERROR_QUIET)

			# if so, run it again, but in-place this time
			if(files_different)
				message(STATUS "Stamping file ${_rel} with version ${version}")
				execute_process(COMMAND
					${SED_EXECUTABLE}
					-i
					${sedargs}
					OUTPUT_FILE
					"${CMAKE_CURRENT_BINARY_DIR}/stampedoutput-${_name}.out"
					WORKING_DIRECTORY
					"${CMAKE_CURRENT_SOURCE_DIR}")
			else()
				message(STATUS "Version stamp up-to-date on file ${_rel}")
			endif()
		endif()
	endif()
endfunction()

function(stamp_sources_with_version version)
	foreach(_file ${ARGN})
		_stamp_file_with_version("${version}" "${_file}")
	endforeach()
endfunction()

function(stamp_target_with_version version target_name)

	set(_target_stampables)

	get_target_property(_target_sources ${target_name} SOURCES)
	foreach(_source ${_target_sources})
		get_source_file_property(_lang "${_source}" LANGUAGE)
		get_source_file_property(_loc "${_source}" LOCATION)
		if("${_lang}" MATCHES "CXX" OR "${_lang}" MATCHES "C")
			list(APPEND _target_stampables "${_loc}")
		endif()
	endforeach()

	set(_src_to_stamp)
	if("${ARGN}" STREQUAL "HEADERS_ONLY")
		# We were passed HEADERS_ONLY
		foreach(_file ${_target_stampables})
			get_filename_component(_ext "${_file}" EXT)
			if("${_ext}" MATCHES "[.]([hH]|hpp|HPP|hxx|HXX)$" OR NOT _ext)
				list(APPEND _src_to_stamp "${_file}")
			endif()
		endforeach()

	elseif(ARGN)
		# We were passed a list of files
		set(_src_to_stamp ${ARGN})

	else()
		# We were passed only a target - process all source in the source tree.
		set(_src_to_stamp ${_target_stampables})
	endif()

	stamp_sources_with_version(${version} ${_src_to_stamp})
endfunction()



