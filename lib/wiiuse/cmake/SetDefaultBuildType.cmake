# - Set a developer-chosen default build type
#
# Requires these CMake modules:
#  no additional modules required
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
#


if(__set_default_build_type)
	return()
endif()
set(__set_default_build_type YES)

function(set_default_build_type _type)
	#if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE)
	if("${CMAKE_GENERATOR}" MATCHES "Makefiles" AND NOT CMAKE_BUILD_TYPE)
		if(NOT __DEFAULT_BUILD_TYPE_SET)
			set(CMAKE_BUILD_TYPE "${_type}" CACHE STRING "" FORCE)
			set(__DEFAULT_BUILD_TYPE_SET YES CACHE INTERNAL "")
		endif()
	endif()
endfunction()
