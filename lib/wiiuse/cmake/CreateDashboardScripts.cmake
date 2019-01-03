# - Create ctest -S scripts to use to run dashboard builds
#
#  include(CreateDashboardScripts)
#  create_dashboard_scripts([<initialcachetemplatefilename>])
#
# If you need additional settings to persist from the "parent" CMake instance
# to the initial cache created by the dashboard script,
# you may pass a filename which will be configured into the initial cache.
#
# In the resulting DASHBOARDSCRIPT_BASE_DIRECTORY, an end-user
# may optionally create a file named
#  CustomInitialCache.${DASHBOARDSCRIPT_SCRIPT_NAME}
# (by default, CustomInitialCache.go.cmake) containing set commands that use
# the CACHE option, to set up additional site-local cache variable values.
#
# Requires these CMake modules:
#  GetCompilerInfoString
#
# Requires CMake 2.6 or newer (uses the 'function' command),
# as well as FindGit.cmake (included with 2.8.2, may be backported)
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

# Only do any of the prep work if not already in a dashboard script
if(NOT IN_DASHBOARD_SCRIPT)

	# Hide a CTest variable
	mark_as_advanced(DART_TESTING_TIMEOUT)

	include(GetCompilerInfoString)

	get_compiler_info_string(_COMPILERID)

	# We must run the following at "include" time, not at function call time,
	# to find the path to this module rather than the path to a calling list file
	get_filename_component(_dashboardmoddir
		${CMAKE_CURRENT_LIST_FILE}
		PATH)

	if(NOT "$ENV{USER}" MATCHES "^$")
		set(_user "$ENV{USER}")
	elseif(NOT "$ENV{USERNAME}" MATCHES "^$")
		set(_user "$ENV{USERNAME}")
	endif()

	if(NOT _dashboardscript_machine)
		if(NOT SITE)
			site_name(SITE)
		endif()
		set(_dashboardscript_machine "${SITE}" CACHE INTERNAL "")
		set(SITE
			"${_user}@${_dashboardscript_machine}"
			CACHE
			STRING
			"Human-readable site name"
			FORCE)
	endif()

	set(DASHBOARDSCRIPT_BASE_DIRECTORY
		"${CMAKE_BINARY_DIR}/Dashboards-${_dashboardscript_machine}-${_user}"
		CACHE
		PATH
		"Directory to use as the root of all dashboard work")
	mark_as_advanced(DASHBOARDSCRIPT_BASE_DIRECTORY)

	set(DASHBOARDSCRIPT_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}")

	set(BUILDNAME
		"${CMAKE_SYSTEM}-${CMAKE_SYSTEM_PROCESSOR}-${_COMPILERID}"
		CACHE
		STRING
		"Human-readable build ID info")

	set(DASHBOARDSCRIPT_CMAKE_COMMAND
		"${CMAKE_COMMAND}"
		CACHE
		FILEPATH
		"The cmake binary to use when configuring a dashboard build")
	mark_as_advanced(DASHBOARDSCRIPT_CMAKE_COMMAND)

	# Try to find CTest, preferably right next to the chosen CMake
	if(DASHBOARDSCRIPT_CMAKE_COMMAND)
		get_filename_component(_cmake_dir
			${DASHBOARDSCRIPT_CMAKE_COMMAND}
			PATH)
	else()
		get_filename_component(_cmake_dir ${CMAKE_COMMAND} PATH)
	endif()
	find_program(DASHBOARDSCRIPT_CTEST_EXECUTABLE
		NAMES
		ctest
		HINTS
		"${_cmake_dir}"
		NO_DEFAULT_PATH)
	find_program(DASHBOARDSCRIPT_CTEST_EXECUTABLE
		NAMES
		ctest
		HINTS
		"${_cmake_dir}")


	set(DASHBOARDSCRIPT_CTEST_EXECUTABLE
		"${DASHBOARDSCRIPT_CTEST_EXECUTABLE}"
		CACHE
		FILEPATH
		"Path to the CTest executable to use for dashboard builds.")
	mark_as_advanced(DASHBOARDSCRIPT_CTEST_EXECUTABLE)


	# Optionals

	if(NOT "1.${CMAKE_VERSION}" VERSION_LESS "1.2.8.0")
		if(IS_DIRECTORY "${CMAKE_SOURCE_DIRECTORY}/.git")
			if(NOT GIT_FOUND)
				find_package(Git QUIET)
			endif()
			# If we have a valid git we found ourselves in older version of the module,
			# let the regular FindGit module (since 2.8.2) know.
			if(DASHBOARDSCRIPT_GIT_EXECUTABLE AND EXISTS "${DASHBOARDSCRIPT_GIT_EXECUTABLE}" AND NOT GIT_FOUND)
				set(GIT_EXECUTABLE "${DASHBOARDSCRIPT_GIT_EXECUTABLE}" CACHE FILEPATH "Git executable" FORCE)
				find_package(Git QUIET)
			endif()
			unset(DASHBOARDSCRIPT_GIT_EXECUTABLE)
			unset(DASHBOARDSCRIPT_GIT_EXECUTABLE CACHE)
			if(GIT_FOUND)
				set(UPDATE_TYPE "git")
				set(UPDATE_COMMAND "${GIT_EXECUTABLE}")
				set(UPDATE_OPTIONS "")
			endif()
		endif()
	endif()

else()
	# IN_DASHBOARD_SCRIPT is YES
	message(STATUS
		"CreateDashboardScripts detected that we're in a dashboard script already.")
endif()

function(create_dashboard_scripts)
	# Only create the script if we have all the required variables
	# and are not already in it, and are at least 2.8.0.
	if(DASHBOARDSCRIPT_BASE_DIRECTORY AND
		DASHBOARDSCRIPT_SOURCE_DIRECTORY AND
		DASHBOARDSCRIPT_BASE_DIRECTORY AND
		BUILDNAME AND
		DASHBOARDSCRIPT_CMAKE_COMMAND AND
		DASHBOARDSCRIPT_CTEST_EXECUTABLE AND
		NOT IN_DASHBOARD_SCRIPT AND
		NOT "${CMAKE_VERSION}" VERSION_LESS "2.8.0")

		set(_Continuous_cron "15 * * * * ")
		set(_Nightly_cron "15 0 * * * ")
		set(_Experimental_cron
			"\nor run this command for an one-off experimental test build:\n")
		set(_Experimental_flags "-VV")

		message(STATUS
			"Dashboard scripts have been generated for automatic nightly and continuous builds.")
		if(WIN32)
			set(_Continuous_cron)
			set(_Nightly_cron)
			message(STATUS
				"You can set up scheduled tasks to run these command lines:")
		else()
			message(STATUS "You can add these sample lines to your crontab:")
		endif()

		set(_msg)

		if(NOT DASHBOARDSCRIPT_BUILD_CONFIGURATION)
			set(DASHBOARDSCRIPT_BUILD_CONFIGURATION "RelWithDebInfo")
		endif()
		set(DASHBOARDSCRIPT_BUILD_CONFIGURATION "${DASHBOARDSCRIPT_BUILD_CONFIGURATION}" CACHE STRING "Build configuration to use for dashboard builds by default")
		mark_as_advanced(DASHBOARDSCRIPT_BUILD_CONFIGURATION)

		foreach(DASHBOARDSCRIPT_DASH_TYPE Nightly Continuous Experimental)
			# If given a cache template, configure it
			if(ARGN)
				configure_file(${ARGN}
					"${DASHBOARDSCRIPT_BASE_DIRECTORY}/GeneratedInitialCache.run${DASHBOARDSCRIPT_DASH_TYPE}.cmake"
					@ONLY)
			endif()

			# Actually create the script file
			configure_file(${_dashboardmoddir}/DashboardScript.cmake.in
				"${DASHBOARDSCRIPT_BASE_DIRECTORY}/run${DASHBOARDSCRIPT_DASH_TYPE}.cmake"
				@ONLY)

			set(_msg
				"${_msg}\n${_${DASHBOARDSCRIPT_DASH_TYPE}_cron}\"${DASHBOARDSCRIPT_CTEST_EXECUTABLE}\" -S \"${DASHBOARDSCRIPT_BASE_DIRECTORY}/run${DASHBOARDSCRIPT_DASH_TYPE}.cmake\" ${_${DASHBOARDSCRIPT_DASH_TYPE}_flags}")

		endforeach()
		message(STATUS "\n${_msg}\n")
		message(STATUS "")

	endif()
endfunction()
