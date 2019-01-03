# - Removes duplicate entries and non-directories from a provided list
#
#  clean_directory_list(<listvar> [<additional list items>...])
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

if(__clean_directory_list)
	return()
endif()
set(__clean_directory_list YES)

function(clean_directory_list _var)
	# combine variable's current value with additional list items
	set(_in ${${_var}} ${ARGN})

	if(_in)
		# Initial list cleaning
		list(REMOVE_DUPLICATES _in)

		# Grab the absolute path of each actual directory
		set(_out)
		foreach(_dir ${_in})
			if(IS_DIRECTORY "${_dir}")
				get_filename_component(_dir "${_dir}" ABSOLUTE)
				file(TO_CMAKE_PATH "${_dir}" _dir)
				list(APPEND _out "${_dir}")
			endif()
		endforeach()

		if(_out)
			# Clean up the output list now
			list(REMOVE_DUPLICATES _out)
		endif()

		# return _out
		set(${_var} "${_out}" PARENT_SCOPE)
	endif()
endfunction()
