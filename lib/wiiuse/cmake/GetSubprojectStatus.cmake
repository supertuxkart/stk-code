# - Returns whether the current project is on its own or within another project's build
#
#  get_subproject_status(<resultvar>) - resultvar will be YES if we are
#   included in another project, or NO if we are being built separately
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

if(__get_subproject_status)
	return()
endif()
set(__get_subproject_status YES)

function(get_subproject_status _var)
	if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
		# Base source dir is our source dir - we are top-level
		set(${_var} NO PARENT_SCOPE)
	else()
		# Base source dir is not our source dir - we are a subproject
		set(${_var} YES PARENT_SCOPE)
	endif()
endfunction()
