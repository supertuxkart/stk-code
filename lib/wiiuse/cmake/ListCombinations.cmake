# - Combine lists of prefixes and suffixes in all combinations
#
#  list_combinations(var PREFIXES listitems... SUFFIXES listitems...) -
#   where var is the name of your desired output variable and PREFIXES
#   and SUFFIXES are special arguments that indicate the start of your
#   list of prefixes or suffixes respectively.
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

if(__list_combinations)
	return()
endif()
set(__list_combinations YES)

function(list_combinations var)
	# Parse arguments
	set(_prefixes)
	set(_suffixes)
	set(_nowhere)
	set(_curdest _nowhere)
	foreach(_element ${ARGN})
		if("${_element}" STREQUAL "PREFIXES")
			set(_curdest _prefixes)
		elseif("${_element}" STREQUAL "SUFFIXES")
			set(_curdest _suffixes)
		else()
			list(APPEND ${_curdest} "${_element}")
		endif()
	endforeach()
	if(_nowhere)
		message(STATUS "_prefixes ${_prefixes}")
		message(STATUS "_prefixes ${_suffixes}")
		message(STATUS "_prefixes ${_nowhere}")
		message(FATAL_ERROR
			"Syntax error in use of ${CMAKE_CURRENT_LIST_FILE}")
	endif()

	foreach(_prefix ${_prefixes})
		foreach(_suffix ${_suffixes})
			list(APPEND _out "${_prefix}${_suffix}")
		endforeach()
	endforeach()

	set(${var} "${_out}" PARENT_SCOPE)
endfunction()
