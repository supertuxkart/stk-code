# - Copy the resources your app needs to the build tree.
#
#  copy_resources_to_build_tree(<target_name>)
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

if(__copy_resources_to_build_tree)
	return()
endif()
set(__copy_resources_to_build_tree YES)

function(copy_resources_to_build_tree _target)
	get_target_property(_resources ${_target} RESOURCE)
	if(NOT _resources)
		# Bail if no resources
		message(STATUS
			"Told to copy resources for target ${_target}, but "
			"no resources are set!")
		return()
	endif()

	get_target_property(_path ${_target} LOCATION)
	get_filename_component(_path "${_path}" PATH)

	if(NOT MSVC AND NOT "${CMAKE_GENERATOR}" MATCHES "Makefiles")
		foreach(_config ${CMAKE_CONFIGURATION_TYPES})
			get_target_property(_path${_config} ${_target} LOCATION_${_config})
			get_filename_component(_path${_config} "${_path${_config}}" PATH)
			add_custom_command(TARGET ${_target}
						POST_BUILD
						COMMAND
						${CMAKE_COMMAND}
						ARGS -E make_directory "${_path${_config}}/"
						COMMENT "Creating directory ${_path${_config}}/")
		endforeach()
	endif()

	foreach(_res ${_resources})
		if(NOT IS_ABSOLUTE "${_res}")
			get_filename_component(_res "${_res}" ABSOLUTE)
		endif()
		get_filename_component(_name "${_res}" NAME)

		if(MSVC)
			# Working dir is solution file dir, not exe file dir.
			add_custom_command(TARGET ${_target}
				POST_BUILD
				COMMAND
				${CMAKE_COMMAND}
				ARGS -E copy "${_res}" "${CMAKE_BINARY_DIR}/"
				COMMENT "Copying ${_name} to ${CMAKE_BINARY_DIR}/ for MSVC")
		else()
			if("${CMAKE_GENERATOR}" MATCHES "Makefiles")
				add_custom_command(TARGET ${_target}
					POST_BUILD
					COMMAND
					${CMAKE_COMMAND}
					ARGS -E copy "${_res}" "${_path}/"
					COMMENT "Copying ${_name} to ${_path}/")
			else()
				foreach(_config ${CMAKE_CONFIGURATION_TYPES})
					add_custom_command(TARGET ${_target}
						POST_BUILD
						COMMAND
						${CMAKE_COMMAND}
						ARGS -E copy "${_res}" "${_path${_config}}"
						COMMENT "Copying ${_name} to ${_path${_config}}")
				endforeach()

			endif()
		endif()
	endforeach()
endfunction()
