# - Copy/parse lua source files as a custom target
#
#  include(LuaTargets)
#  add_lua_target(<target_name> <directory to copy to> [<luafile> <luafile>])
#    Relative paths for the destination directory are considered with
#    with respect to CMAKE_CURRENT_BINARY_DIR
#
#  install_lua_target(<target_name> [arguments to INSTALL(PROGRAMS ...) ])
#
# Set this variable to specify location of luac, if it is not a target:
#  LUA_TARGET_LUAC_EXECUTABLE
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

if(__add_lua)
	return()
endif()
set(__add_lua YES)

include(FileCopyTargets)

function(add_lua_target _target _dest)

	if(NOT ARGN)
		message(WARNING
			"In add_lua_target call for target ${_target}, no source files were specified!")
		return()
	endif()

	if(NOT LUA_TARGET_LUAC_EXECUTABLE)
		if(TARGET luac)
			set(LUA_TARGET_LUAC_EXECUTABLE luac)
			mark_as_advanced(LUA_TARGET_LUAC_EXECUTABLE)
		else()
			find_program(LUA_TARGET_LUAC_EXECUTABLE NAMES luac)
		endif()
	endif()

	if(NOT LUA_TARGET_LUAC_EXECUTABLE)
		message(FATAL_ERROR
			"Can't find luac: please give LUA_TARGET_LUAC_EXECUTABLE a useful value - currently ${LUA_TARGET_LUAC_EXECUTABLE}")
	endif()
	mark_as_advanced(LUA_TARGET_LUAC_EXECUTABLE)

	set(ALLFILES)
	set(SOURCES)
	foreach(fn ${ARGN})
		# Produce an absolute path to the input file
		if(IS_ABSOLUTE "${fn}")
			get_filename_component(fullpath "${fn}" ABSOLUTE)
			get_filename_component(fn "${fn}" NAME)
		else()
			get_filename_component(fullpath
				"${CMAKE_CURRENT_SOURCE_DIR}/${fn}"
				ABSOLUTE)
		endif()

		# Clean up output file name
		get_filename_component(absout "${_dest}/${fn}" ABSOLUTE)

		add_custom_command(OUTPUT "${absout}"
			COMMAND
			${CMAKE_COMMAND}
			ARGS -E make_directory "${_dest}"
			COMMAND
			${CMAKE_COMMAND}
			ARGS -E copy "${fullpath}" "${_dest}"
			COMMAND
			"${LUA_TARGET_LUAC_EXECUTABLE}"
			ARGS -p "${fullpath}"
			MAIN_DEPENDENCY "${fullpath}"
			VERBATIM
			COMMENT "Copying ${fn} to ${absout} and parsing...")
		list(APPEND SOURCES "${fullpath}")
		list(APPEND ALLFILES "${absout}")
	endforeach()

	# Custom target depending on all the file copy commands
	add_custom_target(${_target}
		SOURCES ${SOURCES}
		DEPENDS ${ALLFILES})
	if(TARGET "${LUA_TARGET_LUAC_EXECUTABLE}")
		get_property(_luac_imported
			TARGET
			"${LUA_TARGET_LUAC_EXECUTABLE}"
			PROPERTY
			IMPORTED)
		if(NOT _luac_imported)
			add_dependencies(${_target} ${LUA_TARGET_LUAC_EXECUTABLE})
		endif()
	endif()

	set_property(TARGET ${_target} PROPERTY FILE_COPY_TARGET YES)
endfunction()

function(install_lua_target)
	install_file_copy_target(${ARGN})
endfunction()
