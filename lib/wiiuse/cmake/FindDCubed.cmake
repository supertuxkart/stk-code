# - try to find DCUBED library
#
#  DCUBED_LIBRARY_DIR, library search path
#  DCUBED_INCLUDE_DIR, include search path
#  DCUBED_{component}_LIBRARY, the library to link against
#  DCUBED_ENVIRONMENT
#  DCUBED_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  DCUBED_ROOT_DIR - A directory prefix to search
#                         (a path that contains include/ as a subdirectory)
#
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(NOT BITS)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(BITS 64)
	else()
		set(BITS 32)
	endif()
endif()

if(WIN32 AND MSVC)
	include(CMakeDetermineVSServicePack)
	determinevsservicepack(_sp)
	if(MSVC71)
		set(VC_VER vc71)
		set(VC_VER_LONG vc71)
	elseif(MSVC80)
		set(VC_VER vc8)
		set(VC_VER_LONG vc80)
		# FIXME TODO provide more options here
		set(D3BUILD nt8)
		if("${_sp}" STREQUAL "vc80sp1")
			set(_verstring nt8s1)
		else()
			set(_verstring nt8)
		endif()
	elseif(MSVC90)
		set(VC_VER vc9)
		set(VC_VER_LONG vc90)
		set(_verstring nt9)
	endif()

	if(BITS EQUAL 32)
		set(PLATFORM win32)
	else()
		set(PLATFORM win64)
	endif()
endif()

if(NOT DCUBED_ROOT_DIR)
	if(EXISTS "$ENV{DCUBED}" AND IS_DIRECTORY "$ENV{DCUBED}")
		set(DCUBED_ROOT_DIR "$ENV{DCUBED}")
	endif()
endif()

file(TO_CMAKE_PATH "${DCUBED_ROOT_DIR}" DCUBED_ROOT_DIR)

set(DCUBED_ROOT_DIR
	"${DCUBED_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for DCubed")

# Do this by default
if(NOT DEFINED DCUBED_NESTED_TARGETS)
	set(DCUBED_NESTED_TARGETS TRUE)
endif()

set(DCUBED_NESTED_TARGETS
	"${DCUBED_NESTED_TARGETS}"
	CACHE
	BOOL
	"Whether we should compile the wrappers as a part of the solution")
mark_as_advanced(DCUBED_NESTED_TARGETS)

###
# Configure DCubed
###


find_path(DCUBED_CORE_INCLUDE_DIR
	d3ew_inc/modelgate.hxx
	PATHS
	"${DCUBED_ROOT_DIR}/inc")

foreach(lib aem cdmwp d3e_base d3e_cd dcm dcm3 g3)
	find_library(DCUBED_${lib}_LIBRARY
		${lib}
		PATHS
		"${DCUBED_ROOT_DIR}/lib/${_verstring}")
	if(DCUBED_${lib}_LIBRARY)
		list(APPEND DCUBED_LIBRARIES ${DCUBED_${lib}_LIBRARY})
		list(APPEND DCUBED_CORE_LIBRARIES ${DCUBED_${lib}_LIBRARY})
	endif()
	mark_as_advanced(DCUBED_${lib}_LIBRARY)
endforeach()

find_path(DCUBED_WRAPPER_INCLUDE_DIR
	d3ew_p/p_utils.hxx
	PATHS
	"${DCUBED_ROOT_DIR}/source/wrapper_source/")

foreach(lib d3ew_p d3ew_scene)
	find_library(DCUBED_WRAPPER_${lib}_LIBRARY
		${lib}_${D3BUILD}
		PATHS
		"${DCUBED_ROOT_DIR}/wrappers/cdmwp/${lib}")
	mark_as_advanced(DCUBED_WRAPPER_${lib}_LIBRARY)
endforeach()

set(_nest_targets)
if(DCUBED_WRAPPER_INCLUDE_DIR)
	foreach(lib d3ew_p d3ew_scene)
		if(DCUBED_NESTED_TARGETS OR NOT DCUBED_WRAPPER_${lib}_LIBRARY)
			if(PARASOLID_FOUND)
				set(_nest_targets YES)
				set(DCUBED_WRAPPER_${lib}_LIBRARY
					"dcubed_wrapper_${lib}_nested_target"
					CACHE
					STRING
					"We will build the DCubed wrapper ${lib} lib."
					FORCE)
			else()
				set(DCUBED_WRAPPER_${lib}_LIBRARY
					"NESTED_TARGET_REQUIRES_PARASOLID-NOTFOUND"
					CACHE
					STRING
					"Can't build the DCubed wrapper ${lib} without first finding Parasolid."
					FORCE)
			endif()
		endif()
	endforeach()
endif()

foreach(lib d3ew_p d3ew_scene)
	if(DCUBED_WRAPPER_${lib}_LIBRARY)
		list(APPEND DCUBED_WRAPPER_LIBRARIES ${DCUBED_WRAPPER_${lib}_LIBRARY})
	endif()
endforeach()

if(NOT DCUBED_ROOT_DIR)
	get_filename_component(_path "${DCUBED_dcm_LIBRARY}" PATH)
	get_filename_component(_path "${_path}/../.." ABSOLUTE)
	set(DCUBED_ROOT_DIR
		"${_path}"
		CACHE
		PATH
		"Root directory to search for DCubed"
		FORCE)
endif()

#file(TO_NATIVE_PATH "${DCUBED_ROOT_DIR}" _d3envdir)
set(DCUBED_ENVIRONMENT "DCUBED=${DCUBED_ROOT_DIR}")

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DCubed
	DEFAULT_MSG
	DCUBED_ROOT_DIR
	DCUBED_LIBRARIES
	DCUBED_CORE_LIBRARIES
	DCUBED_CORE_INCLUDE_DIR
	DCUBED_WRAPPER_INCLUDE_DIR
	DCUBED_WRAPPER_LIBRARIES)

if(DCUBED_FOUND)
	if(_nest_targets)
		get_filename_component(_moddir "${CMAKE_CURRENT_LIST_FILE}" PATH)
		add_subdirectory("${_moddir}/nested_targets/DCubed")
	endif()
	set(DCUBED_INCLUDE_DIRS
		"${DCUBED_CORE_INCLUDE_DIR}"
		"${DCUBED_CORE_INCLUDE_DIR}/if3"
		"${DCUBED_CORE_INCLUDE_DIR}/.."
		"${DCUBED_WRAPPER_INCLUDE_DIR}")
	mark_as_advanced(DCUBED_ROOT_DIR)
endif()

mark_as_advanced(DCUBED_CORE_INCLUDE_DIR DCUBED_WRAPPER_INCLUDE_DIR)
