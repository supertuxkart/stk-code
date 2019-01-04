# - Add tests using boost::test
#
# Add this line to your test files in place of including a basic boost test header:
#  #include <BoostTestTargetConfig.h>
#
# If you cannot do that and must use the included form for a given test,
# include the line
#  // OVERRIDE_BOOST_TEST_INCLUDED_WARNING
# in the same file with the boost test include.
#
#  include(BoostTestTargets)
#  add_boost_test(<testdriver_name> SOURCES <source1> [<more sources...>]
#   [FAIL_REGULAR_EXPRESSION <additional fail regex>]
#   [LAUNCHER <generic launcher script>]
#   [LIBRARIES <library> [<library>...]]
#   [RESOURCES <resource> [<resource>...]]
#   [TESTS <testcasename> [<testcasename>...]])
#
#  If for some reason you need access to the executable target created,
#  it can be found in ${${testdriver_name}_TARGET_NAME} as specified when
#  you called add_boost_test
#
# Requires CMake 2.6 or newer (uses the 'function' command)
#
# Requires:
# 	GetForceIncludeDefinitions
# 	CopyResourcesToBuildTree
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

if(__add_boost_test)
	return()
endif()
set(__add_boost_test YES)

set(BOOST_TEST_TARGET_PREFIX "boosttest")

if(NOT Boost_FOUND)
	find_package(Boost 1.34.0 QUIET)
endif()
if("${Boost_VERSION}0" LESS "1034000")
	set(_shared_msg
		"NOTE: boost::test-based targets and tests cannot "
		"be added: boost >= 1.34.0 required but not found. "
		"(found: '${Boost_VERSION}'; want >=103400) ")
	if(BUILD_TESTING)
		message(FATAL_ERROR
			${_shared_msg}
			"You may disable BUILD_TESTING to continue without the "
			"tests.")
	else()
		message(STATUS
			${_shared_msg}
			"BUILD_TESTING disabled, so continuing anyway.")
	endif()
endif()

include(GetForceIncludeDefinitions)
include(CopyResourcesToBuildTree)

if(Boost_FOUND AND NOT "${Boost_VERSION}0" LESS "1034000")
	set(_boosttesttargets_libs)
	set(_boostConfig "BoostTestTargetsIncluded.h")
	if(NOT Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
		find_package(Boost 1.34.0 QUIET COMPONENTS unit_test_framework)
	endif()
	if(Boost_UNIT_TEST_FRAMEWORK_LIBRARY)
		set(_boosttesttargets_libs "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
		if(Boost_USE_STATIC_LIBS)
			set(_boostConfig "BoostTestTargetsStatic.h")
		else()
			if(NOT APPLE)
				set(_boostConfig "BoostTestTargetsDynamic.h")
			endif()
		endif()
	endif()
	get_filename_component(_moddir ${CMAKE_CURRENT_LIST_FILE} PATH)
	configure_file("${_moddir}/${_boostConfig}"
		"${CMAKE_CURRENT_BINARY_DIR}/BoostTestTargetConfig.h"
		COPYONLY)
	include_directories("${CMAKE_CURRENT_BINARY_DIR}")
endif()

function(add_boost_test _name)
	if(NOT BUILD_TESTING)
		return()
	endif()

	# parse arguments
	set(_nowhere)
	set(_curdest _nowhere)
	set(_val_args
		SOURCES
		FAIL_REGULAR_EXPRESSION
		LAUNCHER
		LIBRARIES
		RESOURCES
		TESTS)
	set(_bool_args
		USE_COMPILED_LIBRARY)
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
		message(FATAL_ERROR "Syntax error in use of add_boost_test!")
	endif()

	if(NOT SOURCES)
		message(FATAL_ERROR
			"Syntax error in use of add_boost_test: at least one source file required!")
	endif()

	if(Boost_FOUND AND NOT "${Boost_VERSION}0" LESS "1034000")

		include_directories(${Boost_INCLUDE_DIRS})

		set(includeType)
		foreach(src ${SOURCES})
			file(READ ${src} thefile)
			if("${thefile}" MATCHES ".*BoostTestTargetConfig.h.*")
				set(includeType CONFIGURED)
				set(includeFileLoc ${src})
				break()
			elseif("${thefile}" MATCHES ".*boost/test/included/unit_test.hpp.*")
				set(includeType INCLUDED)
				set(includeFileLoc ${src})
				set(_boosttesttargets_libs)	# clear this out - linking would be a bad idea
				if(NOT
					"${thefile}"
					MATCHES
					".*OVERRIDE_BOOST_TEST_INCLUDED_WARNING.*")
					message("Please replace the include line in ${src} with this alternate include line instead:")
					message("  \#include <BoostTestTargetConfig.h>")
					message("Once you've saved your changes, re-run CMake. (See BoostTestTargets.cmake for more info)")
				endif()
				break()
			endif()
		endforeach()

		if(NOT _boostTestTargetsNagged${_name} STREQUAL "${includeType}")
			if("${includeType}" STREQUAL "CONFIGURED")
				message(STATUS
					"Test '${_name}' uses the CMake-configurable form of the boost test framework - congrats! (Including File: ${includeFileLoc})")
			elseif("${includeType}" STREQUAL "INCLUDED")
				message("In test '${_name}': ${includeFileLoc} uses the 'included' form of the boost unit test framework.")
			else()
				message("In test '${_name}': Didn't detect the CMake-configurable boost test include.")
				message("Please replace your existing boost test include in that test with the following:")
				message("  \#include <BoostTestTargetConfig.h>")
				message("Once you've saved your changes, re-run CMake. (See BoostTestTargets.cmake for more info)")
			endif()
		endif()
		set(_boostTestTargetsNagged${_name}
			"${includeType}"
			CACHE
			INTERNAL
			""
			FORCE)


		if(RESOURCES)
			list(APPEND SOURCES ${RESOURCES})
		endif()

		# Generate a unique target name, using the relative binary dir
		# and provided name. (transform all / into _ and remove all other
		# non-alphabet characters)
		file(RELATIVE_PATH
			targetpath
			"${CMAKE_BINARY_DIR}"
			"${CMAKE_CURRENT_BINARY_DIR}")
		string(REGEX REPLACE "[^A-Za-z/_]" "" targetpath "${targetpath}")
		string(REPLACE "/" "_" targetpath "${targetpath}")

		set(_target_name ${BOOST_TEST_TARGET_PREFIX}-${targetpath}-${_name})
		set(${_name}_TARGET_NAME "${_target_name}" PARENT_SCOPE)

		# Build the test.
		add_executable(${_target_name} ${SOURCES})

		list(APPEND LIBRARIES ${_boosttesttargets_libs})

		if(LIBRARIES)
			target_link_libraries(${_target_name} ${LIBRARIES})
		endif()

		if(RESOURCES)
			set_property(TARGET ${_target_name} PROPERTY RESOURCE ${RESOURCES})
			copy_resources_to_build_tree(${_target_name})
		endif()

		if(NOT Boost_TEST_FLAGS)
#			set(Boost_TEST_FLAGS --catch_system_error=yes --output_format=XML)
			set(Boost_TEST_FLAGS --catch_system_error=yes)
		endif()

		# TODO: Figure out why only recent boost handles individual test running properly

		if(LAUNCHER)
			set(_test_command ${LAUNCHER} "\$<TARGET_FILE:${_target_name}>")
		else()
			set(_test_command ${_target_name})
		endif()

		if(TESTS AND ( "${Boost_VERSION}" VERSION_GREATER "103799" ))
			foreach(_test ${TESTS})
				add_test(
					${_name}-${_test}
					${_test_command} --run_test=${_test} ${Boost_TEST_FLAGS}
				)
				if(FAIL_REGULAR_EXPRESSION)
					set_tests_properties(${_name}-${_test}
						PROPERTIES
						FAIL_REGULAR_EXPRESSION
						"${FAIL_REGULAR_EXPRESSION}")
				endif()
			endforeach()
		else()
			add_test(
				${_name}-boost_test
				${_test_command} ${Boost_TEST_FLAGS}
			)
			if(FAIL_REGULAR_EXPRESSION)
				set_tests_properties(${_name}-boost_test
					PROPERTIES
					FAIL_REGULAR_EXPRESSION
					"${FAIL_REGULAR_EXPRESSION}")
			endif()
		endif()

		# CppCheck the test if we can.
		if(COMMAND add_cppcheck)
			add_cppcheck(${_target_name} STYLE UNUSED_FUNCTIONS)
		endif()

	endif()
endfunction()
