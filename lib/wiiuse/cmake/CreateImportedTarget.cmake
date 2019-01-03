# - A smarter replacement for list(REMOVE_DUPLICATES) for library lists
#
#  create_imported_target(<libname> [SHARED|STATIC|MODULE] [<library dependency>...]) - where
#  ${libname}_LIBRARIES is set to this library's paths.
#
# Removes duplicates from the list then sorts while preserving "optimized",
# "debug", and "general" labeling
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

if(__create_imported_target)
	return()
endif()
set(__create_imported_target YES)

function(create_imported_target _libname)
	if(ARGN)
		list(FIND ARGN SHARED _target_shared)
		list(FIND ARGN STATIC _target_static)
		list(FIND ARGN MODULE _target_module)

		if(${_target_shared} GREATER -1)
			set(_target_type SHARED)
		elseif(${_target_static} GREATER -1)
			set(_target_type STATIC)
		elseif(${_target_module} GREATER -1)
			set(_target_type MODULE)
		else()
			set(_target_type UNKNOWN)
		endif()

		set(_deps ${ARGN})
		list(REMOVE_ITEM _deps SHARED STATIC MODULE UNKNOWN)
	else()
		set(_target_type UNKNOWN)
		set(_deps)
	endif()

	if(${_libname}_LIBRARIES AND NOT TARGET ${_libname}_imported)
		add_library(${_libname}_imported ${_target_type} IMPORTED)
		#message(STATUS "Library ${_libname}: lib ${${_libname}_LIBRARIES}")
		#message(STATUS "Deps: ${_deps}")
		set_target_properties(${_libname}_imported
			PROPERTIES
			IMPORTED_LOCATION
			"${${_libname}_LIBRARIES}"
			IMPORTED_LINK_INTERFACE_LIBRARIES
			"${_deps}")
	endif()

	if(TARGET ${_libname}_imported)
		set(${_libname}_LIBRARIES ${_libname}_imported PARENT_SCOPE)
	endif()
endfunction()
