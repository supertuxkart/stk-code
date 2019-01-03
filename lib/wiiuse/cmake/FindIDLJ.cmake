# - try to find Java's IDLJ Interface Definition Language compiler.
#
# Ideally used with CMake 2.8.5 or newer for Java support using FindJava.cmake
# and UseJava.cmake
#
# Variables:
#  Java_IDLJ_COMMAND, executable for idlj
#  IDLJ_FOUND, If false, do not try to use this
#
# Function:
#  java_idlj(varname idlfile [extra idlj args]) - Generates
#    the Java source files from the IDL file you indicate, and
#    appends filenames suitable to add to a add_jar() call to the
#    variable you specified.
#
# Because the files generated from an IDL file are not entirely predictable,
# java_idlj runs idlj in the cmake step, rather than the build step, and triggers
# a CMake re-run when an idl file is modified. Already up-to-date generated source
# is not re-generated, however.
#
# Files are generated in a directory created specifically for
# the particular IDL file and the particular call, in the build directory -
# there should be no worries about overwriting files or picking up too much
# with the wildcard.
#
# You may wish to add the IDL file to your list of sources if you want it
# to appear in your IDE, but it is not necessary.
#
# Original Author:
# 2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(NOT JAVA_FOUND)
	find_package(Java QUIET)
endif()

if(JAVA_FOUND)
	get_filename_component(JAVA_BIN_DIR "${Java_JAVAC_EXECUTABLE}" PATH)
	find_program(Java_IDLJ_COMMAND
		idlj
		HINTS
		${JAVA_BIN_DIR}
	)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IDLJ
	DEFAULT_MSG
	Java_IDLJ_COMMAND
	JAVA_FOUND)

if(IDLJ_FOUND)
	function(java_idlj _varname _idlfile)
		# Get some unique value we can use in a directory name
		# TODO would be better to somehow munge the full path relative to CMAKE_CURRENT_SOURCE_DIR
		# in case somebody has multiple idl files with the same name
		get_filename_component(_idl_name "${_idlfile}" NAME_WE)
		get_filename_component(_idl_abs "${_idlfile}" ABSOLUTE)

		# Compute directory name and stamp filename
		set(outdir "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/idlj/${_idl_name}.dir")
		set(stampfile "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/idlj/${_idl_name}.stamp")

		# Force re-cmake if idl file changes
		configure_file("${_idl_abs}" "${stampfile}" COPY_ONLY)

		if((NOT EXISTS "${outdir}") OR ("${_idl_abs}" IS_NEWER_THAN "${outdir}"))
			file(REMOVE_RECURSE "${outdir}")
			message(STATUS "Processing ${_idlfile} with Java's idlj")
			execute_process(COMMAND
				"${Java_IDLJ_COMMAND}" -fclient -fallTIE -td "${outdir}" ${ARGN} "${_idlfile}"
				WORKING_DIRECTORY
				"${CMAKE_CURRENT_SOURCE_DIR}")
		endif()
		file(GLOB_RECURSE _idl_output "${outdir}/*")

		set(${_varname} ${_idl_output} PARENT_SCOPE)

		# Clean up after ourselves on make clean
		set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${outdir}" "${stampfile}")
	endfunction()
endif()

mark_as_advanced(Java_IDLJ_COMMAND)
