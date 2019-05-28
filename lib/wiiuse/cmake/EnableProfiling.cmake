# - Add flags to compile with profiling support - currently only for GCC
#
#  enable_profiling(<targetname>)
#  globally_enable_profiling() - to modify CMAKE_CXX_FLAGS, etc
#    to change for all targets declared after the command, instead of per-command
#
#
# Original Author:
# 2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(__enable_profiling)
	return()
endif()
set(__enable_profiling YES)

macro(_enable_profiling_flags)
	set(_flags)
	if(MSVC)
		# TODO: what kind of flags are needed to profile on MSVC?
		#set(_flags /W4)
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		set(_flags "-p")
	endif()
endmacro()

function(enable_profiling _target)
	_enable_profiling_flags()
	get_target_property(_origflags ${_target} COMPILE_FLAGS)
	if(_origflags)
		set_property(TARGET
			${_target}
			PROPERTY
			COMPILE_FLAGS
			"${_flags} ${_origflags}")
	else()
		set_property(TARGET
			${_target}
			PROPERTY
			COMPILE_FLAGS
			"${_flags}")
	endif()

endfunction()

function(globally_enable_profiling)
	_enable_profiling_flags()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_flags}" PARENT_SCOPE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flags}" PARENT_SCOPE)
endfunction()
