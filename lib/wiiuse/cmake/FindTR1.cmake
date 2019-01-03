# - Try to find C++ TR1 headers and libraries
# Once done, this will define
#
#  TR1_USE_FILE, which you may "include" in your CMake file to be able
#   to use TR1 features transparently
#  TR1_INCLUDE_DIRS, any directories needed to access TR1 headers
#  TR1_LIBRARY_DIRS, any directories needed to access (auto-link) TR1 libraries
#  TR1_FOUND, If false, do not try to use TR1 features.
#
# If TR1 features are not built-in, we will try to use Boost to
# substitute for them.
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

# If we were sought quietly, any dependencies should be quiet as well
if(TR1_FIND_QUIETLY)
	set(_findflags QUIET)
else()
	set(_findflags)
endif()

set(_check)
set(TR1_INCLUDE_DIRS)
set(TR1_LIBRARY_DIRS)

get_filename_component(_findtr1moddir
	${CMAKE_CURRENT_LIST_FILE}
	PATH)
set(TR1_USE_FILE "${_findtr1moddir}/UseTR1.cmake")

if(WIN32)
	if(MSVC)
		set(PLATFORM "Visual Studio - no workarounds")
	else()
		set(PLATFORM "Unknown Windows platform - no workarounds set")
	endif()

	if(MSVC_VERSION LESS 1600)
		# Earlier than VS 2010
		# Missing stdint.h/cstdint
		set(PLATFORM "Visual Studio older than Visual Studio 2010")
	endif()

	if(MSVC_VERSION LESS 1500)
		# Earlier than VS 2008
		# Missing all of TR1
		# (The feature pack or SP1 is required for VS2008 TR support)
		set(PLATFORM "Visual Studio older than Visual Studio 2008")
		list(APPEND _check Boost_FOUND)
		find_package(Boost COMPONENTS math_c99 math_tr1 ${_findflags})
		list(APPEND
			TR1_INCLUDE_DIRS
			"${Boost_INCLUDE_DIR}/boost/tr1/tr1"
			"${Boost_INCLUDE_DIR}/boost/tr1"
			"${Boost_INCLUDE_DIR}")
		list(APPEND TR1_LIBRARY_DIRS ${Boost_LIBRARY_DIRS})
	endif()
else()
	set(PLATFORM "Non-Windows Platform - no workarounds set")
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TR1 DEFAULT_MSG PLATFORM ${_check})
