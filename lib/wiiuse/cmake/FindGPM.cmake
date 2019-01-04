# - try to find GPM library
#
# Cache Variables: (probably not for direct use in your scripts)
#  GPM_INCLUDE_DIR
#  GPM_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  GPM_FOUND
#  GPM_INCLUDE_DIRS
#  GPM_LIBRARIES
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
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

find_library(GPM_LIBRARY
	NAMES gpm)

find_path(GPM_INCLUDE_DIR
	NAMES gpm.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GPM
	DEFAULT_MSG
	GPM_LIBRARY
	GPM_INCLUDE_DIR)

if(GPM_FOUND)
	set(GPM_LIBRARIES "${GPM_LIBRARY}")

	set(GPM_INCLUDE_DIRS "${GPM_INCLUDE_DIR}")
endif()

mark_as_advanced(GPM_INCLUDE_DIR GPM_LIBRARY)
