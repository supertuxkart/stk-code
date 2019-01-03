# - Returns a list of the file names of all files passed
#
#  get_file_list(<listvar> <file path> [<additional file paths>...])
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

if(__get_file_list)
	return()
endif()
set(__get_file_list YES)

function(get_file_list _var)
	# combine variable's current value with additional list items
	set(_in ${ARGN})

	if(_in)
		# Initial list cleaning
		list(REMOVE_DUPLICATES _in)

		# Grab the absolute path of each actual directory
		set(_out)
		foreach(_file ${_in})
			get_filename_component(_fn "${_file}" FILE)
			list(APPEND _out "${_fn}")
		endforeach()

		if(_out)
			# Clean up the output list now
			list(REMOVE_DUPLICATES _out)
		endif()

		# return _out
		set(${_var} "${_out}" PARENT_SCOPE)
	endif()
endfunction()
