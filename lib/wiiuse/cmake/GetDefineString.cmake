# - Script to get the value of a preprocessor definition that is a string,
# after including the given files
# Requires that the associated source file be present: GetDefineString.cpp.in.
#
#   get_define_string(NAME <define_name> [INCLUDES <files>] RESULT <variable>
#                     [FLAGS <compile-flags>]
#                     [INCLUDE_DIRS <include-dirs>]
#                     [DEFINES] <-Ddefinitions>)
# Original Author:
# 2014 Ryan Pavlik <ryan@sensics.com> <abiryan@ryand.net>
#
# Copyright Sensics, Inc. 2014.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


get_filename_component(_getdefinestring_moddir "${CMAKE_CURRENT_LIST_FILE}" PATH)

function(get_define_string)
	include(CMakeParseArguments)
	cmake_parse_arguments(_gds "" "NAME;RESULT" "INCLUDES;FLAGS;INCLUDE_DIRS;DEFINES" ${ARGN})
	if(NOT _gds_NAME)
		message(FATAL_ERROR "Must pass NAME to get_define_string!")
	endif()
	if(NOT _gds_RESULT)
		message(FATAL_ERROR "Must pass RESULT to get_define_string!")
	endif()
	if(NOT DEFINED "${_gds_RESULT}_CHECKED" OR (NOT "${${_gds_RESULT}_CHECKED}" STREQUAL "${ARGN}"))
		set(${_gds_RESULT}_CHECKED "${ARGN}" CACHE INTERNAL "" FORCE)
		set(GET_DEFINE_STRING_NAME ${_gds_NAME})
		set(_INCLUDES)
		if(_gds_INCLUDE_DIRS)
			set(_INCLUDES "-DINCLUDE_DIRECTORIES:STRING=${_gds_INCLUDE_DIRS}")
		endif()
		set(GET_DEFINE_STRING_INCLUDES)
		foreach(_file ${_gds_INCLUDES})
			set(GET_DEFINE_STRING_INCLUDES "${GET_DEFINE_STRING_INCLUDES}#include <${_file}>\n")
		endforeach()
		set(_src "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/GetDefineString${_gds_RESULT}.cpp")
		set(_out "${CMAKE_CURRENT_BINARY_DIR}/GetDefineString${_gds_RESULT}.out")
		configure_file("${_getdefinestring_moddir}/GetDefineString.cpp.in" "${_src}")
		try_compile(_result "${CMAKE_CURRENT_BINARY_DIR}" SOURCES "${_src}"
			CMAKE_FLAGS ${_INCLUDES}
			COMPILE_DEFINITIONS ${_gds_DEFINES}
			OUTPUT_VARIABLE OUTPUT
			COPY_FILE "${_out}")
		if(_result)
			file(STRINGS "${_out}" _result_string REGEX "INFO:[A-Za-z0-9_]+\\[[^]]*\\]")
			if("${_result_string}" MATCHES "INFO:define\\[([^]\"]*)\\]")
				set(${_gds_RESULT}_CACHED "${CMAKE_MATCH_1}" CACHE INTERNAL "" FORCE)
			endif()
		else()
			set(${_gds_RESULT}_CACHED NOTFOUND)
		endif()
	endif()
	set(${_gds_RESULT} ${${_gds_RESULT}_CACHED} PARENT_SCOPE)
endfunction()