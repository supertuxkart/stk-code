# - Add an option that depends on one or more variables being true.
#
#  option_requires(<option_name> <description> <variable_name> [<variable_name>...])
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

function(option_requires name desc)
	set(args ${ARGN})

	set(OFF_BY_DEFAULT false)
	list(FIND args "OFF_BY_DEFAULT" _off_found)
	if(NOT _off_found EQUAL -1)
		list(REMOVE_AT args ${_off_found})
		set(OFF_BY_DEFAULT true)
	endif()

	set(found)
	set(missing)
	foreach(var ${args})
		if(${var})
			list(APPEND found ${var})
		else()
			list(APPEND missing ${var})
		endif()
	endforeach()

	if(NOT missing)
		set(OK TRUE)
	else()
		set(OK FALSE)
	endif()

	set(default ${OK})
	if(OFF_BY_DEFAULT)
		set(default OFF)
	endif()

	option(${name} "${desc}" ${default})

	if(${name} AND (NOT OK))
		message(FATAL_ERROR
			"${name} enabled but these dependencies were not valid: ${missing}")
	endif()

endfunction()
