# - Run cppcheck on c++ source files as a custom target and a test
#
#  include(CppcheckTargets)
#  add_cppcheck(<target-name> [UNUSED_FUNCTIONS] [STYLE] [POSSIBLE_ERROR] [FORCE] [FAIL_ON_WARNINGS]) -
#    Create a target to check a target's sources with cppcheck and the indicated options
#  add_cppcheck_sources(<target-name> [UNUSED_FUNCTIONS] [STYLE] [POSSIBLE_ERROR] [FORCE] [FAIL_ON_WARNINGS]) -
#    Create a target to check standalone sources with cppcheck and the indicated options
#
# Requires these CMake modules:
#  Findcppcheck
#
# Requires CMake 2.8 or newer (uses VERSION_LESS)
#
# Original Author:
# 2009-2010 Ryan Pavlik <ryan.pavlik@gmail.com> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Copyright Anthony Pesch 2014
# Copyright Stefan Eilemann 2014
# Copyright Nicholas Brown 2015
# Copyright Ryan Pavlik 2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(__add_cppcheck)
	return()
endif()
set(__add_cppcheck YES)

if(NOT CPPCHECK_FOUND)
	find_package(cppcheck QUIET)
endif()

if(NOT CPPCHECK_FOUND)
	add_custom_target(all_cppcheck
		COMMENT "cppcheck executable not found")
	set_target_properties(all_cppcheck PROPERTIES EXCLUDE_FROM_ALL TRUE)
elseif(CPPCHECK_VERSION VERSION_LESS 1.53.0)
	add_custom_target(all_cppcheck
		COMMENT "Need at least cppcheck 1.53, found ${CPPCHECK_VERSION}")
	set_target_properties(all_cppcheck PROPERTIES EXCLUDE_FROM_ALL TRUE)
	set(CPPCHECK_FOUND)
endif()

if(NOT TARGET all_cppcheck)
  add_custom_target(all_cppcheck)
endif()

function(add_cppcheck_sources _targetname)
	if(CPPCHECK_FOUND)
		set(_cppcheck_args -I ${CMAKE_SOURCE_DIR} ${CPPCHECK_EXTRA_ARGS})
		set(_input ${ARGN})
		list(FIND _input UNUSED_FUNCTIONS _unused_func)
		if("${_unused_func}" GREATER "-1")
			list(APPEND _cppcheck_args ${CPPCHECK_UNUSEDFUNC_ARG})
			list(REMOVE_AT _input ${_unused_func})
		endif()

		list(FIND _input STYLE _style)
		if("${_style}" GREATER "-1")
			list(APPEND _cppcheck_args ${CPPCHECK_STYLE_ARG})
			list(REMOVE_AT _input ${_style})
		endif()

		list(FIND _input POSSIBLE_ERROR _poss_err)
		if("${_poss_err}" GREATER "-1")
			list(APPEND _cppcheck_args ${CPPCHECK_POSSIBLEERROR_ARG})
			list(REMOVE_AT _input ${_poss_err})
		endif()

		list(FIND _input FORCE _force)
		if("${_force}" GREATER "-1")
			list(APPEND _cppcheck_args "--force")
			list(REMOVE_AT _input ${_force})
		endif()

		list(FIND _input FAIL_ON_WARNINGS _fail_on_warn)
		if("${_fail_on_warn}" GREATER "-1")
			list(APPEND
				CPPCHECK_FAIL_REGULAR_EXPRESSION
				${CPPCHECK_WARN_REGULAR_EXPRESSION})
			list(REMOVE_AT _input ${_fail_on_warn})
		endif()

		set(_files)
		foreach(_source ${_input})
			get_source_file_property(_cppcheck_loc "${_source}" LOCATION)
			if(_cppcheck_loc)
				# This file has a source file property, carry on.
				get_source_file_property(_cppcheck_lang "${_source}" LANGUAGE)
				if(("${_cppcheck_lang}" STREQUAL "C") OR ("${_cppcheck_lang}" STREQUAL "CXX"))
					list(APPEND _files "${_cppcheck_loc}")
				endif()
			else()
				# This file doesn't have source file properties - figure it out.
				get_filename_component(_cppcheck_loc "${_source}" ABSOLUTE)
				if(EXISTS "${_cppcheck_loc}")
					list(APPEND _files "${_cppcheck_loc}")
				else()
					message(FATAL_ERROR
						"Adding CPPCHECK for file target ${_targetname}: "
						"File ${_source} does not exist or needs a corrected path location "
						"since we think its absolute path is ${_cppcheck_loc}")
				endif()
			endif()
		endforeach()

		if("1.${CMAKE_VERSION}" VERSION_LESS "1.2.8.0")
			# Older than CMake 2.8.0
			add_test(${_targetname}_cppcheck_test
				"${CPPCHECK_EXECUTABLE}"
				${CPPCHECK_TEMPLATE_ARG}
				${_cppcheck_args}
				${_files})
		else()
			# CMake 2.8.0 and newer
			add_test(NAME
				${_targetname}_cppcheck_test
				COMMAND
				"${CPPCHECK_EXECUTABLE}"
				${CPPCHECK_TEMPLATE_ARG}
				${_cppcheck_args}
				${_files})
		endif()

		set_tests_properties(${_targetname}_cppcheck_test
			PROPERTIES
			FAIL_REGULAR_EXPRESSION
			"${CPPCHECK_FAIL_REGULAR_EXPRESSION}")

		add_custom_target(${_targetname}_cppcheck
			COMMAND
			${CPPCHECK_EXECUTABLE}
			${CPPCHECK_QUIET_ARG}
			${CPPCHECK_TEMPLATE_ARG}
			${_cppcheck_args}
			${_files}
			WORKING_DIRECTORY
			"${CMAKE_CURRENT_SOURCE_DIR}"
			COMMENT
			"${_targetname}_cppcheck: Running cppcheck on target ${_targetname}..."
			VERBATIM)
               add_dependencies(cppcheck ${_targetname}_cppcheck)
	endif()
endfunction()

function(add_cppcheck _name)
	if(NOT TARGET ${_name})
		message(FATAL_ERROR
			"add_cppcheck given a target name that does not exist: '${_name}' !")
	endif()
	if(CPPCHECK_FOUND)
		set(_cppcheck_args -I ${CMAKE_SOURCE_DIR} ${CPPCHECK_EXTRA_ARGS})

		list(FIND ARGN UNUSED_FUNCTIONS _unused_func)
		if("${_unused_func}" GREATER "-1")
			list(APPEND _cppcheck_args ${CPPCHECK_UNUSEDFUNC_ARG})
		endif()

		list(FIND ARGN STYLE _style)
		if("${_style}" GREATER "-1")
			list(APPEND _cppcheck_args ${CPPCHECK_STYLE_ARG})
		endif()

		list(FIND ARGN POSSIBLE_ERROR _poss_err)
		if("${_poss_err}" GREATER "-1")
			list(APPEND _cppcheck_args ${CPPCHECK_POSSIBLEERROR_ARG})
		endif()

		list(FIND ARGN FORCE _force)
		if("${_force}" GREATER "-1")
			list(APPEND _cppcheck_args "--force")
		endif()

		list(FIND _input FAIL_ON_WARNINGS _fail_on_warn)
		if("${_fail_on_warn}" GREATER "-1")
			list(APPEND
				CPPCHECK_FAIL_REGULAR_EXPRESSION
				${CPPCHECK_WARN_REGULAR_EXPRESSION})
			list(REMOVE_AT _input ${_unused_func})
		endif()

		get_target_property(_cppcheck_includes "${_name}" INCLUDE_DIRECTORIES)
		set(_includes)
		foreach(_include ${_cppcheck_includes})
			list(APPEND _includes "-I${_include}")
		endforeach()

		get_target_property(_cppcheck_sources "${_name}" SOURCES)
		set(_files)
		foreach(_source ${_cppcheck_sources})
			get_source_file_property(_cppcheck_lang "${_source}" LANGUAGE)
			get_source_file_property(_cppcheck_loc "${_source}" LOCATION)
			if(("${_cppcheck_lang}" STREQUAL "C") OR ("${_cppcheck_lang}" STREQUAL "CXX"))
				list(APPEND _files "${_cppcheck_loc}")
			endif()
		endforeach()

		if("1.${CMAKE_VERSION}" VERSION_LESS "1.2.8.0")
			# Older than CMake 2.8.0
			add_test(${_name}_cppcheck_test
				"${CPPCHECK_EXECUTABLE}"
				${CPPCHECK_TEMPLATE_ARG}
				${_cppcheck_args}
				${_files})
		else()
			# CMake 2.8.0 and newer
			add_test(NAME
				${_name}_cppcheck_test
				COMMAND
				"${CPPCHECK_EXECUTABLE}"
				${CPPCHECK_TEMPLATE_ARG}
				${_cppcheck_args}
				${_files})
		endif()

		set_tests_properties(${_name}_cppcheck_test
			PROPERTIES
			FAIL_REGULAR_EXPRESSION
			"${CPPCHECK_FAIL_REGULAR_EXPRESSION}")

		add_custom_target(${_name}_cppcheck
			COMMAND
			${CPPCHECK_EXECUTABLE}
			${CPPCHECK_QUIET_ARG}
			${CPPCHECK_TEMPLATE_ARG}
			${_cppcheck_args}
			${_includes}
			${_files}
			WORKING_DIRECTORY
			"${CMAKE_CURRENT_SOURCE_DIR}"
			COMMENT
			"${_name}_cppcheck: Running cppcheck on target ${_name}..."
			VERBATIM)
		add_dependencies(all_cppcheck ${_name}_cppcheck)
	endif()

endfunction()
