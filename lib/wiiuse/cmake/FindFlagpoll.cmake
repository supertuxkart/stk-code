# - try to find Flagpoll application, and offer package-finding services
#  FLAGPOLL, the executable: if not defined, do not try to use Flagpoll.
#
# Useful configuration variables you might want to add to your cache:
#  FLAGPOLL_ROOT_DIR  - A directory prefix to search for the app
#                       (a path that contains bin/ as a subdirectory)
#
# VR Juggler requires this package, so this Find script takes that into
# account when determining where to search for the desired files.
# The VJ_BASE_DIR environment variable is searched (preferentially)
# when searching for this package, so most sane VR Juggler build environments
# should "just work."  Note that you need to manually re-run CMake if you
# change this environment variable, because it cannot auto-detect this change
# and trigger an automatic re-run.
#
# You can use Flagpoll to provide directories to use as HINTS for find_*
# These are the provided macros:
#  flagpoll_get_include_dirs
#  flagpoll_get_library_dirs
#  flagpoll_get_library_names
#  flagpoll_get_extra_libs
# All take the name of the desired package, optionally NO_DEPS to pass --no-deps
# to Flagpoll, and return yourpkgname_FLAGPOLL_INCLUDE_DIRS(etc. for the other
# macros).
#
# Example usage:
# 	flagpoll_get_include_dirs(vpr NO_DEPS)
# 	find_path(VPR20_INCLUDE_DIRS vpr/vpr.h
#   	  HINTS  ${vpr_FLAGPOLL_INCLUDE_DIRS})
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


###
# Flagpoll detection
###
set(Flagpoll_FIND_QUIETLY true)
find_program(FLAGPOLL
	NAMES
	flagpoll
	flagpoll.exe
	PATHS
	"${FLAGPOLL_ROOT_DIR}"
	"${VRJUGGLER22_ROOT_DIR}"
	PATH_SUFFIXES
	bin)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Flagpoll DEFAULT_MSG FLAGPOLL)

mark_as_advanced(FLAGPOLL)

###
# Macro for internal use - shared workings between all the public macros below.
###
macro(_flagpoll_get_results _package _arg _flag _output)
	if(FLAGPOLL)

		# If the CMakeLists that called the flagpoll macro passed NO_DEPS,
		# we won't return the results for dependencies
		if("${ARGN}" MATCHES "NO_DEPS")
			set(_FLAGPOLL_NODEP "--no-deps")
		else()
			set(_FLAGPOLL_NODEP "")
		endif()

		# Run flagpoll
		execute_process(COMMAND
			${FLAGPOLL}
			${_package}
			${_arg}
			${_FLAGPOLL_NODEP}
			OUTPUT_VARIABLE
			_FLAGPOLL_OUTPUT
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)

		if(_FLAGPOLL_OUTPUT)
			# Remove -I and /I(or equivalent for other flags
			string(REGEX
				REPLACE
				"[-/]${_flag}"
				""
				_FLAGPOLL_OUTPUT
				${_FLAGPOLL_OUTPUT})

			# Remove extra spaces
			string(REGEX REPLACE " +" " " _FLAGPOLL_OUTPUT ${_FLAGPOLL_OUTPUT})

			# Make a CMake list, standardize paths, and append only what we want to our final list
			separate_arguments(_FLAGPOLL_OUTPUT)
			foreach(_RESULT ${_FLAGPOLL_OUTPUT})
				string(REGEX MATCH "^-" _BAD ${_RESULT})
				if(_RESULT AND NOT _BAD)
					file(TO_CMAKE_PATH "${_RESULT}" _RESULT_CLEAN)
					list(APPEND ${_output} ${_RESULT_CLEAN})
				endif()
			endforeach()
		endif()

	endif()
endmacro()

###
# "Public" macros - to use flagpoll to give you HINTS directories when finding things
###
macro(flagpoll_get_include_dirs _package)
	# Passing ARGN along so if they specified NO_DEPS we actually do it.
	_flagpoll_get_results(${_package}
		"--cflags-only-I"
		I
		${_package}_FLAGPOLL_INCLUDE_DIRS
		${ARGN})
endmacro()

macro(flagpoll_get_library_dirs _package)
	# Passing ARGN along so if they specified NO_DEPS we actually do it.
	_flagpoll_get_results(${_package}
		"--libs-only-L"
		L
		${_package}_FLAGPOLL_LIBRARY_DIRS
		${ARGN})
endmacro()

macro(flagpoll_get_library_names _package)
	# Passing ARGN along so if they specified NO_DEPS we actually do it.
	_flagpoll_get_results(${_package}
		"--libs-only-l"
		l
		${_package}_FLAGPOLL_LIBRARY_NAMES
		${ARGN})
endmacro()

macro(flagpoll_get_extra_libs _package)
	# Passing ARGN along so if they specified NO_DEPS we actually do it.
	_flagpoll_get_results(${_package}
		"--get-extra-libs"
		l
		${_package}_FLAGPOLL_EXTRA_LIBS
		${ARGN})
endmacro()
