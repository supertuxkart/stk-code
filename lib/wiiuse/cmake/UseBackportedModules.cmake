# - Do a version-dependent check and auto-include backported modules dirs
#
# Name your module directories cmake-*-modules where * is the full
# (maj.min.patch) version number that they came from.  You can use
# subdirectories within those directories, if you like - all directories
# inside of a cmake-*-modules dir for a newer version of CMake that what
# we're running, that contain one or more .cmake files, will be appended
# to the CMAKE_MODULE_PATH.
#
# When backporting modules, be sure to test them and follow copyright
# instructions (usually updating copyright notices)
#
# Requires these CMake modules:
#  CleanDirectoryList
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

if(NOT CMAKE_VERSION)	# defined in >=2.6.3
	set(_cmver
		"${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
else()
	set(_cmver "${CMAKE_VERSION}")
endif()

include(CleanDirectoryList)

# No debugging output please
set(USE_BACKPORTED_MODULES_VERBOSE NO)

get_filename_component(_moddir ${CMAKE_CURRENT_LIST_FILE} PATH)
file(GLOB _globbed "${_moddir}/cmake-*-modules")

if(USE_BACKPORTED_MODULES_VERBOSE)
	message(STATUS
		"UseBackportedModules: Detected use of CMake version ${_cmver}")
	message(STATUS "Checking these base directories: ${_globbed}")
endif()

foreach(_dir ${_globbed})
	string(REGEX
		MATCH
		"cmake-[0-9].[0-9].[0-9]-modules"
		_dirname
		"${_dir}")
	string(REGEX
		REPLACE
		"cmake-([0-9].[0-9].[0-9])-modules"
		"\\1"
		_ver
		"${_dirname}")
	string(REGEX
		REPLACE
		"cmake-([0-9]).([0-9]).([0-9])-modules"
		"\\1_\\2_\\3"
		_ver_clean
		"${_dirname}")

	if(USE_BACKPORTED_MODULES_VERBOSE)
		message(STATUS "${_dir}: ${_ver} ${_ver_clean}")
	endif()

	if("${_cmver}" VERSION_LESS "${_ver}")
		list(APPEND _upgradever "${_ver_clean}")
		file(GLOB_RECURSE _modules "${_dir}/*.cmake")

		foreach(_mod ${_modules})
			get_filename_component(_path "${_mod}" PATH)
			list(APPEND _paths_${_ver_clean} "${_path}")
		endforeach()

	endif()
endforeach()


# Autoinclude files from oldest version to newest version
if(_upgradever)
	set(_save_cmake_module_path ${CMAKE_MODULE_PATH})
	list(REMOVE_DUPLICATES _upgradever)
	list(SORT _upgradever)
	foreach(_ver_clean ${_upgradever})
		clean_directory_list(_paths_${_ver_clean})
		foreach(_dir ${_paths_${_ver_clean}})
			set(CMAKE_MODULE_PATH ${_dir} ${_save_cmake_module_path})
			include("${_dir}/autoinclude.cmake" OPTIONAL RESULT_VARIABLE _result)
			if(USE_BACKPORTED_MODULES_VERBOSE)
				message(STATUS "${_dir} - Autoincluded: ${_result}")
			endif()
		endforeach()
	endforeach()
	set(CMAKE_MODULE_PATH ${_save_cmake_module_path})
endif()

# Add the module path from newest version to oldest version
set(_added_module_path)
if(_upgradever)
	list(REVERSE _upgradever)
	foreach(_ver_clean ${_upgradever})
		list(APPEND _added_module_path ${_paths_${_ver_clean}})
	endforeach()
endif()

list(APPEND CMAKE_MODULE_PATH ${_added_module_path})

if(USE_BACKPORTED_MODULES_VERBOSE)
	message(STATUS "New module path: ${CMAKE_MODULE_PATH}")
endif()
