# - Add flags to compile with extra warnings
#
#  enable_extra_compiler_warnings(<targetname>)
#  globally_enable_extra_compiler_warnings() - to modify CMAKE_CXX_FLAGS, etc
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

if(__enable_extra_compiler_warnings)
	return()
endif()
set(__enable_extra_compiler_warnings YES)

macro(_enable_extra_compiler_warnings_flags)
	set(_flags)
	if(MSVC)
		option(COMPILER_WARNINGS_EXTREME
			"Use compiler warnings that are probably overkill."
			off)
		mark_as_advanced(COMPILER_WARNINGS_EXTREME)
		set(_flags "/W4")
		if(COMPILER_WARNINGS_EXTREME)
			set(_flags "${_flags} /Wall /wd4619 /wd4668 /wd4820 /wd4571 /wd4710")
		endif()
	else()
		include(CheckCXXCompilerFlag)
		set(_flags)

		check_cxx_compiler_flag(-W SUPPORTS_W_FLAG)
		if(SUPPORTS_W_FLAG)
			set(_flags "${_flags} -W")
		endif()

		check_cxx_compiler_flag(-Wall SUPPORTS_WALL_FLAG)
		if(SUPPORTS_WALL_FLAG)
			set(_flags "${_flags} -Wall")
		endif()

		check_cxx_compiler_flag(-Wextra SUPPORTS_WEXTRA_FLAG)
		if(SUPPORTS_WEXTRA_FLAG)
			set(_flags "${_flags} -Wextra")
		endif()

		if(SUPPORTS_WALL_FLAG)
			# At least GCC includes -Wmaybe-uninitialized in -Wall, which
			# unneccesarily whines about boost::optional (by it's nature
			# it's a "maybe" warning - prone to noisy false-positives)
			check_cxx_compiler_flag(-Wno-maybe-uninitialized SUPPORTS_WNO_MAYBE_UNINITIALIZED_FLAG)
			if(SUPPORTS_WNO_MAYBE_UNINITIALIZED_FLAG)
				set(_flags "${_flags} -Wno-maybe-uninitialized")
			endif()
		endif()
	endif()
endmacro()

function(enable_extra_compiler_warnings _target)
	_enable_extra_compiler_warnings_flags()
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

function(globally_enable_extra_compiler_warnings)
	_enable_extra_compiler_warnings_flags()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_flags}" PARENT_SCOPE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flags}" PARENT_SCOPE)
endfunction()
