# - try to find Markdown tool
#
# Cache Variables:
#  MARKDOWN_EXECUTABLE
#
# Non-cache variables you might use in your CMakeLists.txt:
#  MARKDOWN_FOUND
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2011 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2011.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

file(TO_CMAKE_PATH "${MARKDOWN_ROOT_DIR}" MARKDOWN_ROOT_DIR)
set(MARKDOWN_ROOT_DIR
	"${MARKDOWN_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for Markdown")

if(MARKDOWN_EXECUTABLE AND NOT EXISTS "${MARKDOWN_EXECUTABLE}")
	set(MARKDOWN_EXECUTABLE "notfound" CACHE PATH FORCE "")
endif()

# If we have a custom path, look there first.
if(MARKDOWN_ROOT_DIR)
	find_program(MARKDOWN_EXECUTABLE
		NAMES
		markdown
		PATHS
		"${MARKDOWN_ROOT_DIR}"
		PATH_SUFFIXES
		bin
		NO_DEFAULT_PATH)
endif()

find_program(MARKDOWN_EXECUTABLE NAMES markdown)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Markdown
	DEFAULT_MSG
	MARKDOWN_EXECUTABLE)

if(MARKDOWN_FOUND)
	mark_as_advanced(MARKDOWN_ROOT_DIR)
endif()

mark_as_advanced(MARKDOWN_EXECUTABLE)
