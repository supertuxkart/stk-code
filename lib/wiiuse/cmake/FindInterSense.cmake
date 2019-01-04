# - try to find InterSense library
#
# Cache Variables: (probably not for direct use in your scripts)
#  INTERSENSE_INCLUDE_DIR
#  INTERSENSE_ISENSEC_DIR - location of the isense.c "import library" substitute.
#  INTERSENSE_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  INTERSENSE_FOUND
#  INTERSENSE_INCLUDE_DIRS
#  INTERSENSE_LIBRARIES
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Author:
# 2013 Eric Marsh <bits@wemarsh.com>
# http://wemarsh.com/
# Kognitiv Neuroinformatik, Universität Bremen
#
# (building on Ryan Pavlik's templates)
#
# 2013 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(INTERSENSE_ROOT_DIR
	"${INTERSENSE_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search for the Intersense SDK")

if(APPLE)
	set(_ARCH UniversalLib)
else()
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(_IS_ARCH x86_64)
	else()
		set(_IS_ARCH x86_32)
	endif()
endif()

set(_IS_INSTALLDIRS)
if(APPLE)
	set(_IS_SDKDIR MacOSX)
elseif(WIN32)
	set(_IS_SDKDIR Windows)
	# Default locations, as well as registry places it records install locations,
	# if you installed from a (actual or downloaded) product "CD"
	foreach(_IS_PROD "IS-900 Software" "InertiaCube Software")
		get_filename_component(_IS_REGPATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\InterSense\\${_IS_PROD};Path]" ABSOLUTE)
		if(_IS_REGPATH AND (NOT "${_IS_REGPATH}" STREQUAL "/registry"))
			list(APPEND _IS_INSTALLDIRS "${_IS_REGPATH}/SDK")
		endif()
		list(APPEND _IS_INSTALLDIRS "C:/InterSense/${_IS_PROD}/SDK")
	endforeach()
else() # Assume Linux, since that's the only other platform supported by this library
	set(_IS_SDKDIR Linux)
endif()

find_path(INTERSENSE_INCLUDE_DIR
	NAMES isense.h
	PATHS "${INTERSENSE_ROOT_DIR}" "${INTERSENSE_ROOT_DIR}/SDK" ${_IS_INSTALLDIRS})

find_path(INTERSENSE_ISENSEC_DIR
	NAMES isense.c
	PATHS "${INTERSENSE_ROOT_DIR}" "${INTERSENSE_ROOT_DIR}/SDK" ${_IS_INSTALLDIRS}
	PATH_SUFFIXES
	"Windows/Sample/Visual C++ 2005"
	"Windows/Sample/Visual C++ 2005 (single tracker)"
	Linux/Sample
	MacOSX/Sample)

include(FindPackageHandleStandardArgs)

# This is a weird one - no import library is supplied, and instead, at least on Windows, they use
# an isense.c file to call into the DLL. Not sure if MinGW can link right against the dll in this case.
if(WIN32)
	find_package_handle_standard_args(InterSense
		DEFAULT_MSG
		INTERSENSE_INCLUDE_DIR
		INTERSENSE_ISENSEC_DIR)
	if(INTERSENSE_FOUND)
		set(INTERSENSE_LIBRARIES "")
		set(INTERSENSE_INCLUDE_DIRS "${INTERSENSE_INCLUDE_DIR}" "${INTERSENSE_ISENSEC_DIR}")
	endif()
else() # Only MSVC on Windows theoretically needs import libraries, so...
	find_library(INTERSENSE_LIBRARY
		NAMES isense
		PATHS "${INTERSENSE_ROOT_DIR}" "${INTERSENSE_ROOT_DIR}/SDK" ${_IS_INSTALLDIRS}
		PATH_SUFFIXES "${_IS_SDKDIR}/${_IS_ARCH}")

	find_package_handle_standard_args(InterSense
		DEFAULT_MSG
		INTERSENSE_LIBRARY
		INTERSENSE_INCLUDE_DIR)

	if(INTERSENSE_FOUND)
		set(INTERSENSE_LIBRARIES "${INTERSENSE_LIBRARY}" ${CMAKE_DL_LIBS})
		set(INTERSENSE_INCLUDE_DIRS "${INTERSENSE_INCLUDE_DIR}")
	endif()
endif()

mark_as_advanced(INTERSENSE_INCLUDE_DIR INTERSENSE_ISENSEC_DIR INTERSENSE_LIBRARY)
