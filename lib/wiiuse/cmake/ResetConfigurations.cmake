# - Re-set the available configurations to just RelWithDebInfo, Release, and Debug
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


if(__reset_configurations)
	return()
endif()
set(__reset_configurations YES)

if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo;Release;Debug")
	set(CMAKE_CONFIGURATION_TYPES
		"${CMAKE_CONFIGURATION_TYPES}"
		CACHE
		STRING
		"Reset the configurations to what we need"
		FORCE)
endif()
