# - try to find PARASOLID library
# Important note: If you are also using JtTk, do your
#  find_package(JtTk)
# first, to avoid runtime PK_* errors!
#
#  PARASOLID_LIBRARY_DIR, library search path
#  PARASOLID_INCLUDE_DIR, include search path
#  PARASOLID_{component}_LIBRARY, the library to link against
#  PARASOLID_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Requires these CMake modules:
#  CheckVersion
#  ListCombinations
#  ProgramFilesGlob
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

include(ListCombinations)
include(CheckVersion)
include(PrefixListGlob)
include(CleanDirectoryList)
if(WIN32)
	include(ProgramFilesGlob)
endif()

set(PARASOLID_ROOT_DIR
	"${PARASOLID_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for Parasolid")

file(TO_CMAKE_PATH "${PARASOLID_ROOT_DIR}" PARASOLID_ROOT_DIR)

# Do this by default
if(NOT DEFINED PARASOLID_NESTED_TARGETS)
	set(PARASOLID_NESTED_TARGETS TRUE)
endif()

set(PARASOLID_NESTED_TARGETS
	"${PARASOLID_NESTED_TARGETS}"
	CACHE
	BOOL
	"Whether we should compile fg and frustrum as a part of the solution")
mark_as_advanced(PARASOLID_NESTED_TARGETS)

set(_nest_targets)

###
# Configure Parasolid
###

string(TOLOWER "${CMAKE_SYSTEM_NAME}" _lcsystem)
set(libsearchdirs)
if(WIN32)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64-bit
		program_files_fallback_glob(dirs "/Parasolid*/kernel/x64_win/base")
		program_files_fallback_glob(dirs2 "/Parasolid/kernel/*/x64_win/base")
		list(APPEND dirs ${dirs2})
	else()
		# 32-bit
		program_files_glob(dirs "/Parasolid*/kernel/intel_nt/base")
		program_files_fallback_glob(dirs2 "/Parasolid/kernel/*/intel_nt/base")
		list(APPEND dirs ${dirs2})
	endif()

	list_combinations(libsearchdirs
		PREFIXES
		${dirs}
		"${PARASOLID_ROOT_DIR}"
		SUFFIXES
		"/dll")
	list(APPEND libsearchdirs ${dirs} "${PARASOLID_ROOT_DIR}")
elseif("${_lcsystem}" MATCHES "linux")
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64-bit
		prefix_list_glob(libsearchdirs
			"/Parasolid*/kernel/intel_linux/base_lx64"
			"${PARASOLID_ROOT_DIR}"
			"/usr"
			"/usr/local"
			"/usr/local/ugs")
	else()
		# 32-bit
		prefix_list_glob(libsearchdirs
			"/Parasolid*/kernel/intel_linux/base_lx32"
			"${PARASOLID_ROOT_DIR}"
			"/usr"
			"/usr/local"
			"/usr/local/ugs")
	endif()
endif()

###
# Find the link library
###
find_library(PARASOLID_pskernel_LIBRARY
	NAMES
	pskernel
	PATH_SUFFIXES
	dll
	shared_object
	HINTS
	${libsearchdirs}
	PATHS
	"${PARASOLID_ROOT_DIR}")


# Don't add this library to the default list of libraries
find_library(PARASOLID_pskernel_archive_LIBRARY
	NAMES
	pskernel_archive
	pskernel_archive.lib
	HINTS
	${libsearchdirs}
	PATHS
	"${PARASOLID_ROOT_DIR}")
mark_as_advanced(PARASOLID_pskernel_archive_LIBRARY)


###
# Prepare for the rest of our search based off of where we found the link library
###
get_filename_component(PARASOLID_LIBRARY_DIR
	"${PARASOLID_pskernel_LIBRARY}"
	PATH)

# Setup include search path
get_filename_component(_includedir
	"${PARASOLID_LIBRARY_DIR}/../include"
	ABSOLUTE)
get_filename_component(_includedir2
	"${PARASOLID_LIBRARY_DIR}/.."
	ABSOLUTE)
set(includesearchdirs
	"${PARASOLID_LIBRARY_DIR}"
	"${_includedir}"
	"${_includedir2}")
clean_directory_list(includesearchdirs)

###
# Find the headers
###
find_path(PARASOLID_INCLUDE_DIR
	NAMES
	parasolid_kernel.h
	HINTS
	${includesearchdirs}
	PATHS
	"${PARASOLID_ROOT_DIR}")

###
# Find remaining libs
###

# Default libs
foreach(lib fg frustrum)
	find_library(PARASOLID_${lib}_LIBRARY
		NAMES
		${lib}
		PATH_SUFFIXES
		dll
		HINTS
		"${PARASOLID_LIBRARY_DIR}"
		${libsearchdirs}
		PATHS
		"${PARASOLID_ROOT_DIR}")

endforeach()

if(PARASOLID_pskernel_LIBRARY OR PARASOLID_INCLUDE_DIR)
	get_filename_component(_libdir "${PARASOLID_pskernel_LIBRARY}" PATH)
	get_filename_component(_incdir "${PARASOLID_INCLUDE_DIR}" PATH)

	if(PARASOLID_NESTED_TARGETS OR NOT PARASOLID_fg_LIBRARY)
		find_file(PARASOLID_FG_C
			NAMES
			fg.c
			HINTS
			"${_libdir}"
			"${_libdir}/.."
			"${_incdir}")
		if(PARASOLID_FG_C)
			mark_as_advanced(PARASOLID_FG_C)
			set(_nest_targets YES)
			set(PARASOLID_fg_LIBRARY
				"parasolid_fg_nested_target"
				CACHE
				STRING
				"We will build the Parasolid fg lib."
				FORCE)
		endif()
	endif()

	if(PARASOLID_NESTED_TARGETS OR NOT PARASOLID_frustrum_LIBRARY)
		find_file(PARASOLID_FRUSTRUM_C
			NAMES
			frustrum.c
			HINTS
			"${_libdir}"
			"${_libdir}/.."
			"${_incdir}")
		if(PARASOLID_FRUSTRUM_C)
			mark_as_advanced(PARASOLID_FRUSTRUM_C)
			set(_nest_targets YES)
			set(PARASOLID_frustrum_LIBRARY
				"parasolid_frustrum_nested_target"
				CACHE
				STRING
				"We will build the Parasolid frustrum lib."
				FORCE)
		endif()
	endif()

endif()

# Non-default libs
foreach(lib testfr)
	find_library(PARASOLID_${lib}_LIBRARY
		NAMES
		${lib}
		PATH_SUFFIXES
		dll
		HINTS
		${PARASOLID_LIBRARY_DIR}
		${libsearchdirs}
		PATHS
		"${PARASOLID_ROOT_DIR}")
	mark_as_advanced(PARASOLID_${lib}_LIBRARY)
endforeach()

###
# Find the DLL's
###

if(JTTK_FOUND AND JTTK_pskernel_DLL)
	# If we have JtTk, must use the dll there or we'll have weird runtime errors
	# in parasolid
	set(PARASOLID_pskernel_DLL "${JTTK_pskernel_DLL}")
else()
	# Find the unversioned DLL
	set(dll pskernel)
	find_file(PARASOLID_${dll}_DLL
		NAMES
		${dll}.dll
		PATH_SUFFIXES
		dll
		HINTS
		${PARASOLID_LIBRARY_DIR}
		${libsearchdirs}
		PATHS
		"${PARASOLID_ROOT_DIR}")
	list(APPEND PARASOLID_DLLS ${PARASOLID_${dll}_DLL})
	mark_as_advanced(PARASOLID_${dll}_DLL)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Parasolid
	DEFAULT_MSG
	PARASOLID_pskernel_LIBRARY
	PARASOLID_fg_LIBRARY
	PARASOLID_frustrum_LIBRARY
	PARASOLID_INCLUDE_DIR)

if(PARASOLID_FOUND)
	# Recurse into the nested targets subdirectory if needed
	if(_nest_targets)
		get_filename_component(_moddir "${CMAKE_CURRENT_LIST_FILE}" PATH)
		add_subdirectory("${_moddir}/nested_targets/Parasolid")
	endif()

	set(PARASOLID_INCLUDE_DIRS "${PARASOLID_INCLUDE_DIR}")
	set(PARASOLID_LIBRARIES
		"${PARASOLID_pskernel_LIBRARY}"
		"${PARASOLID_fg_LIBRARY}"
		"${PARASOLID_frustrum_LIBRARY}")
	if(PARASOLID_pskernel_DLL)
		get_filename_component(PARASOLID_RUNTIME_LIBRARY_DIRS
			"${PARASOLID_pskernel_DLL}"
			PATH)
	endif()
	mark_as_advanced(PARASOLID_ROOT_DIR)
endif()

mark_as_advanced(PARASOLID_pskernel_LIBRARY
	PARASOLID_pskernel_archive_LIBRARY
	PARASOLID_fg_LIBRARY
	PARASOLID_frustrum_LIBRARY
	PARASOLID_INCLUDE_DIR
	PARASOLID_FRUSTRUM_C
	PARASOLID_FG_C)
