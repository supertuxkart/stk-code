# - Issue an error if the source tree is in or equal to the build tree
#
#  include(RequireOutOfSourceBuild)
#
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file ../LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

get_filename_component(_src "${CMAKE_SOURCE_DIR}" ABSOLUTE)
get_filename_component(_cur_src "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
get_filename_component(_bin "${CMAKE_BINARY_DIR}" ABSOLUTE)

string(LENGTH "${_src}" _src_len)
string(LENGTH "${_cur_src}" _cur_src_len)
string(LENGTH "${_bin}" _bin_len)

set(_test)

if(NOT "${_bin_len}" GREATER "${_src_len}")
	list(APPEND _test _src)
	#message(STATUS "Checking ${_src}")
endif()

if(NOT "${_bin_len}" GREATER "${_cur_src_len}")
	list(APPEND _test _cur_src)
	#message(STATUS "Checking ${_cur_src}")
endif()

foreach(_var ${_test})
	string(SUBSTRING "${${_var}}" 0 ${_bin_len} _chopped)
	#message(STATUS "comparing ${_bin} and ${_chopped}")
	if("${_bin}" STREQUAL "${_chopped}")
		get_filename_component(_parent "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
		message(FATAL_ERROR
			"You must set a binary directory that is different from your source directory.  You might consider ${CMAKE_SOURCE_DIR}/build or ${_parent}/build-${CMAKE_PROJECT_NAME}")
	endif()
endforeach()
