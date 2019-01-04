# - try to find Glove5DT libraries
#
# Cache Variables: (probably not for direct use in your scripts)
#  GLOVE5DT_INCLUDE_DIR
#  GLOVE5DT_LIBRARY
#  GLOVE5DT_LIBRARY_RELEASE
#  GLOVE5DT_LIBRARY_DEBUG
#  GLOVE5DT_RUNTIME_LIBRARY_RELEASE
#  GLOVE5DT_RUNTIME_LIBRARY_DEBUG
#
# Non-cache variables you might use in your CMakeLists.txt:
#  GLOVE5DT_FOUND
#  GLOVE5DT_INCLUDE_DIRS
#  GLOVE5DT_LIBRARIES
#  GLOVE5DT_RUNTIME_LIBRARY_DIRS
#
# Requires these CMake modules:
#  CleanDirectoryList
#  CleanLibraryList
#  ListCombinations
#  ProgramFilesGlob
#  SelectLibraryConfigurations (included with CMake >=2.8.0)
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

set(GLOVE5DT_ROOT_DIR
	"${GLOVE5DT_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for Glove5DT SDK")

###
# Configure Glove5DT
###

include(SelectLibraryConfigurations)
include(ListCombinations)
include(CleanDirectoryList)
include(ProgramFilesGlob)

if(WIN32)
	# Data Glove 5 and 16 use these directories for includes, lib, and runtime
	program_files_glob(_dirs516 "/5DT/*/Driver")
	set(_dirs516
		"${GLOVE5DT_ROOT_DIR}/Driver"
		"${GLOVE5DT_ROOT_DIR}"
		${_dirs516})

	# Data Glove Ultra uses this directory as the base for a dll, inc, and lib directory
	program_files_glob(_dirsultra "/5DT/*/SDK")

	list_combinations(_libsearchultra
		PREFIXES
		"${GLOVE5DT_ROOT_DIR}"
		"${_dirsultra}"
		SUFFIXES
		"/lib")
	list_combinations(_incsearchultra
		PREFIXES
		"${GLOVE5DT_ROOT_DIR}"
		"${_dirsultra}"
		SUFFIXES
		"/inc")
	list_combinations(_dllsearchultra
		PREFIXES
		"${GLOVE5DT_ROOT_DIR}"
		"${_dirsultra}"
		SUFFIXES
		"/dll")
endif()


###
# First search for the Ultra (2.0) SDK
###
find_path(GLOVE5DT_INCLUDE_DIR
	NAMES
	fglove.h
	HINTS
	${_incsearchultra}
	${GLOVE5DT_ROOT_DIR})

find_library(GLOVE5DT_LIBRARY_RELEASE
	NAMES
	fglove
	HINTS
	${_libsearchultra}
	${GLOVE5DT_ROOT_DIR})

find_library(GLOVE5DT_LIBRARY_DEBUG
	NAMES
	fgloved
	HINTS
	${_libsearchultra}
	${GLOVE5DT_ROOT_DIR})


select_library_configurations(GLOVE5DT)
# As a post condition, either both LIBRARY_RELEASE and LIBRARY_DEBUG are set, or
# neither is.


###
# Ultra (2.0) SDK Runtime Libraries
###
if(WIN32)
	find_file(GLOVE5DT_RUNTIME_LIBRARY_RELEASE
		NAMES
		fglove.dll
		HINTS
		${_dllsearchultra})

	find_file(GLOVE5DT_RUNTIME_LIBRARY_DEBUG
		NAMES
		fgloved.dll
		HINTS
		${_dllsearchultra})
else()
	# the library is the runtime library
	set(GLOVE5DT_RUNTIME_LIBRARY_RELEASE "${GLOVE5DT_LIBRARY_RELEASE}")
	set(GLOVE5DT_RUNTIME_LIBRARY_DEBUG "${GLOVE5DT_LIBRARY_DEBUG}")
endif()


select_library_configurations(GLOVE5DT_RUNTIME)


###
# Fallback to the 5/16 (1.0) SDK
###
find_path(GLOVE5DT_INCLUDE_DIR
	NAMES
	fglove.h
	HINTS
	${_dirs516})

find_library(GLOVE5DT_LIBRARY_RELEASE
	NAMES
	fglove
	HINTS
	${_dirs516})

select_library_configurations(GLOVE5DT)

###
# 5/16 (1.0) SDK Runtime Libraries
###

if(WIN32)
	find_file(GLOVE5DT_RUNTIME_LIBRARY_RELEASE
		NAMES
		fglove.dll
		HINTS
		${_dirs516})
else()
	# the library is the runtime library
	set(GLOVE5DT_RUNTIME_LIBRARY_RELEASE "${GLOVE5DT_LIBRARY_RELEASE}")
endif()

select_library_configurations(GLOVE5DT_RUNTIME)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glove5DT
	DEFAULT_MSG
	GLOVE5DT_LIBRARY_RELEASE
	GLOVE5DT_RUNTIME_LIBRARY_RELEASE
	GLOVE5DT_INCLUDE_DIR)


if(GLOVE5DT_FOUND)
	set(GLOVE5DT_RUNTIME_LIBRARY_DIRS)
	foreach(_lib
		GLOVE5DT_RUNTIME_LIBRARY_RELEASE
		GLOVE5DT_RUNTIME_LIBRARY_DEBUG)
		if(${_lib})
			get_filename_component(_libdir ${${_lib}} PATH)
			list(APPEND GLOVE5DT_RUNTIME_LIBRARY_DIRS "${_libdir}")
		endif()
	endforeach()

	clean_directory_list(GLOVE5DT_RUNTIME_LIBRARY_DIRS)
	set(GLOVE5DT_INCLUDE_DIRS "${GLOVE5DT_INCLUDE_DIR}")
	set(GLOVE5DT_LIBRARIES ${GLOVE5DT_LIBRARY})
	mark_as_advanced(GLOVE5DT_ROOT_DIR)
endif()


mark_as_advanced(GLOVE5DT_INCLUDE_DIR
	GLOVE5DT_LIBRARY_RELEASE
	GLOVE5DT_LIBRARY_DEBUG
	GLOVE5DT_RUNTIME_LIBRARY_RELEASE
	GLOVE5DT_RUNTIME_LIBRARY_DEBUG)
