# - Get the platform-appropriate flags to add to force inclusion of a file
#
# The most common use of this is to use a generated config.h-type file
# placed out of the source tree in all files.
#
#  get_force_include_definitions(var forcedincludefiles...) -
#   where var is the name of  your desired output variable, and everything
#   else is a source file to forcibly include.
#   a list item to be filtered.
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

if(__get_force_include_definitions)
	return()
endif()
set(__get_force_include_definitions YES)

function(get_force_include_definitions var)
	set(_flagprefix)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(_flag "-include")
	elseif(MSVC)
		set(_flag "/FI")
	else()
		message(SEND_ERROR "You don't seem to be using MSVC or GCC, but")
		message(SEND_ERROR "the project called get_force_include_definitions.")
		message(SEND_ERROR "Contact this project with the name of your")
		message(FATAL_ERROR "compiler and preferably the flag to force includes")
	endif()

	set(_out)
	foreach(_item ${ARGN})
		list(APPEND _out "${_flag} \"${_item}\"")
	endforeach()
	set(${var} "${_out}" PARENT_SCOPE)
endfunction()
