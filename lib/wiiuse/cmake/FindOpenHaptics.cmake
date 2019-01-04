# - try to find OpenHaptics libraries
#
# Cache Variables: (probably not for direct use in your scripts)
#  HDAPI_INCLUDE_DIR
#  HDAPI_LIBRARY
#  HDAPI_LIBRARY_RELEASE
#  HDAPI_LIBRARY_DEBUG
#  HDAPI_HDU_INCLUDE_DIR
#  HDAPI_HDU_LIBRARY
#  HDAPI_HDU_LIBRARY_RELEASE
#  HDAPI_HDU_LIBRARY_DEBUG
#  HLAPI_INCLUDE_DIR
#  HLAPI_LIBRARY
#  HLAPI_LIBRARY_RELEASE
#  HLAPI_LIBRARY_DEBUG
#  HLAPI_HLU_INCLUDE_DIR
#  HLAPI_HLU_LIBRARY
#  HLAPI_HLU_LIBRARY_RELEASE
#  HLAPI_HLU_LIBRARY_DEBUG
#
# Non-cache variables you might use in your CMakeLists.txt:
#  OPENHAPTICS_FOUND
#  HDAPI_INCLUDE_DIRS
#  HDAPI_LIBRARIES
#  HDAPI_HDU_INCLUDE_DIRS
#  HDAPI_HDU_LIBRARIES
#  HLAPI_INCLUDE_DIRS
#  HLAPI_LIBRARIES
#  HLAPI_HLU_INCLUDE_DIRS
#  HLAPI_HLU_LIBRARIES
#  OPENHAPTICS_LIBRARIES - includes HD, HDU, HL, HLU
#  OPENHAPTICS_RUNTIME_LIBRARY_DIRS
#  OPENHAPTICS_ENVIRONMENT
#  OPENHAPTICS_LIBRARY_DIRS
#  OPENHAPTICS_INCLUDE_DIRS
#
# Requires these CMake modules:
#  CleanDirectoryList
#  CleanLibraryList
#  ListCombinations
#  ProgramFilesGlob
#  SelectLibraryConfigurations (included with CMake >=2.8.0)
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#  CMake 2.6.3 (uses "unset")
#
# Original Author:
# 2009-2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 2.6.3)

set(OPENHAPTICS_ROOT_DIR
	"${OPENHAPTICS_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for OpenHaptics")
option(OPENHAPTICS_NESTED_TARGETS
	"Whether we should compile HDU and HLU, if needed, as a part of the solution"
	ON)
mark_as_advanced(OPENHAPTICS_NESTED_TARGETS)

###
# Configure OpenHaptics
###

include(SelectLibraryConfigurations)
include(ListCombinations)
include(CleanDirectoryList)
include(CleanLibraryList)
include(ProgramFilesGlob)

set(_nest_targets)
set(_incsearchdirs)
set(_libsearchdirs)
set(OPENHAPTICS_ENVIRONMENT)
set(OPENHAPTICS_RUNTIME_LIBRARY_DIRS)

set(_dirs)
if(NOT "$ENV{OH_SDK_BASE}" STREQUAL "")
	list(APPEND _dirs "$ENV{OH_SDK_BASE}")
elseif(NOT "$ENV{3DTOUCH_BASE}" STREQUAL "")
	list(APPEND _dirs "$ENV{3DTOUCH_BASE}")
endif()
if(WIN32)
	program_files_fallback_glob(_pfdirs "/Sensable/3DTouch*/")
	foreach(_OH_DEFAULT_LOCATION "C:/OpenHaptics/3.1" "C:/OpenHaptics/Academic/3.1")
		if(EXISTS "${_OH_DEFAULT_LOCATION}")
			list(APPEND _dirs "${_OH_DEFAULT_LOCATION}")
		endif()
	endforeach()
	set(_dirs "${_dirs};${_pfdirs}")
	if(MSVC60)
		set(_vc "vc6")
	elseif(MSVC70 OR MSVC71)
		set(_vc "vc7")
	elseif(MSVC80)
		set(_vc "vc8")
	endif()
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64-bit
		list_combinations(_libsearch
			PREFIXES
			"${OPENHAPTICS_ROOT_DIR}"
			${_dirs}
			SUFFIXES
			"/lib/x64")
		list_combinations(_libsearch2
			PREFIXES
			"${OPENHAPTICS_ROOT_DIR}"
			${_dirs}
			SUFFIXES
			"/utilities/lib/x64")
	else()
		# 32-bit
		list_combinations(_libsearch
			PREFIXES
			"${OPENHAPTICS_ROOT_DIR}"
			${_dirs}
			SUFFIXES
			"/lib"
			"/lib/win32")
		list_combinations(_libsearch2
			PREFIXES
			"${OPENHAPTICS_ROOT_DIR}"
			${_dirs}
			SUFFIXES
			"/utilities/lib/Win32"
			"/utilities/lib"
			"/utilities/lib/${_vc}")
	endif()

	clean_directory_list(_libsearchdirs ${_libsearch} ${_libsearch2})
endif()

list_combinations(_incsearch
	PREFIXES
	"${OPENHAPTICS_ROOT_DIR}"
	${_dirs}
	SUFFIXES
	"/include")
list_combinations(_incsearch2
	PREFIXES
	"${OPENHAPTICS_ROOT_DIR}"
	${_dirs}
	SUFFIXES
	"/utilities/include")
clean_directory_list(_incsearchdirs ${_incsearch} ${_incsearch2})

set(_deps_check)
set(_deps_libs)

###
# HDAPI: HD
###

if(UNIX)
	find_library(HDAPI_PHANToMIO_LIBRARY
		NAMES
		PHANToMIO
		HINTS
		${_libsearchdirs})
	mark_as_advanced(HDAPI_PHANToMIO_LIBRARY)
	list(APPEND _deps_check HDAPI_PHANToMIO_LIBRARY)
	list(APPEND _deps_libs "${HDAPI_PHANToMIO_LIBRARY}")
endif()

find_path(HDAPI_INCLUDE_DIR
	NAMES
	HD/hd.h
	HINTS
	${_incsearchdirs})

find_library(HDAPI_LIBRARY_RELEASE
	NAMES
	HD
	PATH_SUFFIXES
	ReleaseAcademicEdition
	Release
	HINTS
	${_libsearchdirs})

find_library(HDAPI_LIBRARY_DEBUG
	NAMES
	HD
	PATH_SUFFIXES
	DebugAcademicEdition
	Debug
	HINTS
	${_libsearchdirs})

select_library_configurations(HDAPI)

###
# HDAPI: HDU
###
find_path(HDAPI_HDU_INCLUDE_DIR
	NAMES
	HDU/hdu.h
	HINTS
	${_incsearchdirs})

find_library(HDAPI_HDU_LIBRARY_RELEASE
	NAMES
	HDU
	PATH_SUFFIXES
	ReleaseAcademicEdition
	Release
	HINTS
	${_libsearchdirs})

find_library(HDAPI_HDU_LIBRARY_DEBUG
	NAMES
	HDU
	PATH_SUFFIXES
	DebugAcademicEdition
	Debug
	HINTS
	${_libsearchdirs})

# Fallback
find_library(HDAPI_HDU_LIBRARY_DEBUG
	NAMES
	HDUD
	PATH_SUFFIXES
	DebugAcademicEdition
	Debug
	HINTS
	${_libsearchdirs})

select_library_configurations(HDAPI_HDU)

if(OPENHAPTICS_NESTED_TARGETS OR NOT HDAPI_HDU_LIBRARY)
	if(HDAPI_HDU_SOURCE_DIR AND NOT EXISTS "${HDAPI_HDU_SOURCE_DIR}/hdu.cpp")
		unset(HDAPI_HDU_SOURCE_DIR)
	endif()
	find_path(HDAPI_HDU_SOURCE_DIR
		NAMES
		hdu.cpp
		PATH_SUFFIXES
		src
		src/HDU
		src/HDU/src
		libsrc/HDU
		HINTS
		"${HDAPI_HDU_INCLUDE_DIR}/.."
		"${HDAPI_HDU_INCLUDE_DIR}/../share/3DTouch")
	list(APPEND _deps_check HDAPI_HDU_SOURCE_DIR)
	if(HDAPI_HDU_SOURCE_DIR)
		mark_as_advanced(HDAPI_HDU_SOURCE_DIR)
		set(_nest_targets YES)
		set(HDAPI_HDU_LIBRARY
			"openhaptics_hdu_nested_target"
			CACHE
			STRING
			"We will build the OpenHaptics HDU lib."
			FORCE)
		set(HDAPI_HDU_LIBRARIES ${HDAPI_HDU_LIBRARY})
	endif()
endif()


###
# HLAPI: HL
###
find_path(HLAPI_INCLUDE_DIR
	NAMES
	HL/hl.h
	HINTS
	${_incsearchdirs})

find_library(HLAPI_LIBRARY_RELEASE
	NAMES
	HL
	PATH_SUFFIXES
	ReleaseAcademicEdition
	Release
	HINTS
	${_libsearchdirs})

find_library(HLAPI_LIBRARY_DEBUG
	NAMES
	HL
	PATH_SUFFIXES
	DebugAcademicEdition
	Debug
	HINTS
	${_libsearchdirs})

select_library_configurations(HLAPI)

###
# HLAPI: HLU
###
find_path(HLAPI_HLU_INCLUDE_DIR
	NAMES
	HLU/hlu.h
	HINTS
	${_incsearchdirs})

find_library(HLAPI_HLU_LIBRARY_RELEASE
	NAMES
	HLU
	PATH_SUFFIXES
	ReleaseAcademicEdition
	Release
	HINTS
	${_libsearchdirs})

find_library(HLAPI_HLU_LIBRARY_DEBUG
	NAMES
	HLU
	PATH_SUFFIXES
	DebugAcademicEdition
	Debug
	HINTS
	${_libsearchdirs})

# fallback
find_library(HLAPI_HLU_LIBRARY_DEBUG
	NAMES
	HLUD
	PATH_SUFFIXES
	DebugAcademicEdition
	Debug
	HINTS
	${_libsearchdirs})

select_library_configurations(HLAPI_HLU)

if(OPENHAPTICS_NESTED_TARGETS OR NOT HLAPI_HLU_LIBRARY)
	if(HLAPI_HLU_SOURCE_DIR AND NOT EXISTS "${HLAPI_HLU_SOURCE_DIR}/hlu.cpp")
		unset(HLAPI_HLU_SOURCE_DIR)
	endif()
	find_path(HLAPI_HLU_SOURCE_DIR
		NAMES
		hlu.cpp
		PATH_SUFFIXES
		src
		src/HLU
		src/HLU/src
		libsrc/HLU
		HINTS
		"${HLAPI_HLU_INCLUDE_DIR}/.."
		"${HLAPI_HLU_INCLUDE_DIR}/../share/3DTouch")
	list(APPEND _deps_check HLAPI_HLU_SOURCE_DIR)
	if(HLAPI_HLU_SOURCE_DIR)
		mark_as_advanced(HLAPI_HLU_SOURCE_DIR)
		set(_nest_targets YES)
		set(HLAPI_HLU_LIBRARY
			"openhaptics_hlu_nested_target"
			CACHE
			STRING
			"We will build the OpenHaptics HLU lib."
			FORCE)
		set(HLAPI_HLU_LIBRARIES ${HLAPI_HLU_LIBRARY})
	endif()
endif()

###
# Add dependencies: Libraries
###
set(HDAPI_LIBRARIES "${HDAPI_LIBRARY}" ${_deps_libs})

if(HDAPI_HDU_LIBRARIES AND HDAPI_LIBRARIES)
	list(APPEND HDAPI_HDU_LIBRARIES ${HDAPI_LIBRARIES})
else()
	set(HDAPI_HDU_LIBRARIES)
endif()

if(HLAPI_LIBRARY AND HDAPI_LIBRARIES)
	set(HLAPI_LIBRARIES ${HLAPI_LIBRARY} ${HDAPI_LIBRARIES})
else()
	set(HLAPI_LIBRARIES)
endif()

if(HLAPI_HLU_LIBRARIES AND HLAPI_LIBRARIES)
	list(APPEND HLAPI_HLU_LIBRARIES ${HLAPI_LIBRARIES})
else()
	set(HLAPI_HLU_LIBRARIES)
endif()

###
# Add dependencies: Include dirs
###
if(HDAPI_INCLUDE_DIR)
	set(HDAPI_INCLUDE_DIRS ${HDAPI_INCLUDE_DIR})

	if(HDAPI_HDU_INCLUDE_DIR)
		set(HDAPI_HDU_INCLUDE_DIRS
			${HDAPI_INCLUDE_DIRS}
			${HDAPI_HDU_INCLUDE_DIR})

		if(HDAPI_HDU_INCLUDE_DIR)
			set(HLAPI_INCLUDE_DIRS ${HDAPI_INCLUDE_DIRS} ${HLAPI_INCLUDE_DIR})

			if(HLAPI_HLU_INCLUDE_DIR)
				set(HLAPI_HLU_INCLUDE_DIRS
					${HLAPI_INCLUDE_DIRS}
					${HLAPI_HLU_INCLUDE_DIR})

			endif()
		endif()
	endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenHaptics
	DEFAULT_MSG
	HDAPI_LIBRARY
	HDAPI_INCLUDE_DIR
	HDAPI_HDU_INCLUDE_DIR
	HDAPI_HDU_LIBRARY
	HLAPI_INCLUDE_DIR
	HLAPI_LIBRARY
	HLAPI_HLU_INCLUDE_DIR
	HLAPI_HLU_LIBRARY
	${_deps_check})

if(OPENHAPTICS_FOUND)
	# Recurse into the nested targets subdirectory if needed
	if(_nest_targets)
		get_filename_component(_moddir "${CMAKE_CURRENT_LIST_FILE}" PATH)
		add_subdirectory("${_moddir}/nested_targets/OpenHaptics" "OpenHapticsNestedTargets")
	endif()

	set(OPENHAPTICS_LIBRARIES
		${HDAPI_LIBRARY}
		${HDAPI_HDU_LIBRARY}
		${HLAPI_LIBRARY}
		${HLAPI_HLU_LIBRARY})
	set(OPENHAPTICS_LIBRARY_DIRS)
	foreach(_lib
		${_deps_check}
		HDAPI_LIBRARY_RELEASE
		HDAPI_LIBRARY_DEBUG
		HDAPI_HDU_LIBRARY_RELEASE
		HDAPI_HDU_LIBRARY_DEBUG
		HLAPI_LIBRARY_RELEASE
		HLAPI_LIBRARY_DEBUG
		HLAPI_HLU_LIBRARY_RELEASE
		HLAPI_HLU_LIBRARY_DEBUG)
		get_filename_component(_libdir ${${_lib}} PATH)
		list(APPEND OPENHAPTICS_LIBRARY_DIRS ${_libdir})
	endforeach()

	set(OPENHAPTICS_INCLUDE_DIRS
		${HLAPI_HLU_INCLUDE_DIRS}
		${HDAPI_HDU_INCLUDE_DIRS})

	clean_directory_list(OPENHAPTICS_LIBRARY_DIRS)
	clean_directory_list(OPENHAPTICS_INCLUDE_DIRS)

	list(APPEND
		OPENHAPTICS_RUNTIME_LIBRARY_DIRS
		${OPENHAPTICS_LIBRARY_DIRS})

	clean_library_list(OPENHAPTICS_LIBRARIES)

	mark_as_advanced(OPENHAPTICS_ROOT_DIR)
endif()

mark_as_advanced(HDAPI_INCLUDE_DIR
	HDAPI_LIBRARY_RELEASE
	HDAPI_LIBRARY_DEBUG
	HDAPI_HDU_INCLUDE_DIR
	HDAPI_HDU_LIBRARY_RELEASE
	HDAPI_HDU_LIBRARY_DEBUG
	HLAPI_INCLUDE_DIR
	HLAPI_LIBRARY_RELEASE
	HLAPI_LIBRARY_DEBUG
	HLAPI_HLU_INCLUDE_DIR
	HLAPI_HLU_LIBRARY_RELEASE
	HLAPI_HLU_LIBRARY_DEBUG)
