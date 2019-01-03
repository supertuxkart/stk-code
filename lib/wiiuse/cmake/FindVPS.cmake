# - try to find VPS library
#
#  VPS_LIBRARY_DIR, library search path
#  VPS_INCLUDE_DIR, include search path
#  VPS_{component}_LIBRARY, the library to link against
#  VPS_FOUND, If false, do not try to use this library.
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  VPS_ROOT_DIR - A directory prefix to search
#                         (a path that contains include/ as a subdirectory)
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

include(SelectLibraryConfigurations)
include(ListCombinations)
include(CheckVersion)
include(ListFilter)

set(VPS_ROOT_DIR
	"${VPS_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for VPS")

# Try the config file mode.
find_package(VPS QUIET NO_MODULE)
if(VPS_FOUND)
	mark_as_advanced(VPS_DIR VPS_ROOT_DIR)
	return()
endif()

if(NOT BITS)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(BITS 64)
	else()
		set(BITS 32)
	endif()
endif()

set(_vpslibnames)
set(_grviewerlibnames)

###
# Cray MTA(Multi-Threaded Architecture) family: CMake build not tested
if("${CMAKE_SYSTEM_NAME}" STREQUAL "MTX")
	set(VPS_PLATFORM MTX)
	set(_VPS_FLAGS_32 "-pl all.pl -par")
	set(_VPS_FLAGS_64 "-pl all.pl -par")
###
# Linux
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(VPS_PLATFORM LINUX)
	set(_VPS_FLAGS_32 "-O2 -Wno-write-strings")
	set(_VPS_FLAGS_64 "-m64 -O3 -ffast-math -funroll-all-loops -Wno-write-strings")

###
# IBM-AIX: CMake build not tested
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "AIX")
	set(VPS_PLATFORM AIX)
	set(_VPS_FLAGS_32 "-q32")
	set(_VPS_FLAGS_64 "-q64")

###
# HP-UX: CMake build not tested
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "HP-UX")
	set(VPS_PLATFORM HPUX)
	set(_VPS_FLAGS_32 "-O")

###
# SunOS: CMake build not tested
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS")
	set(VPS_PLATFORM SOLARIS)
	set(_VPS_FLAGS_32 "-O")

###
# IRIX: CMake build not tested
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "IRIX")
	set(VPS_PLATFORM IRIX)
	set(_VPS_FLAGS_32 "-O2 -OPT")
	set(_VPS_FLAGS_64 "-64 -O2 -OPT")

###
# Mac OS X
elseif(APPLE AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	set(VPS_PLATFORM MACOSX)
	set(_VPS_FLAGS_32 "-O2 -lm -lobjc -lstdc++ -Wno-write-strings")
	set(_VPS_FLAGS_64 "-m64 -O3 -ffast-math -funroll-all-loops -lm -lobjc -lstdc++ -Wno-write-strings")

###
# Windows
elseif(WIN32)
	set(VPS_PLATFORM WINDOWS)
	set(_VPS_FLAGS_32 "-O2")
	set(_VPS_FLAGS_64 "-O2")

	if(MSVC)
		set(DEFS_32 -D_CRT_SECURE_NO_DEPRECATE)
		set(DEFS_64 -D_CRT_SECURE_NO_DEPRECATE)
		if(MSVC60)
			set(VPS_CRT "VC6")
		elseif(MSVC70)
			set(VPS_CRT "VC7")
		elseif(MSVC71)
			set(VPS_CRT "VC71")
		elseif(MSVC80)
			set(VPS_CRT "VC8")
		elseif(MSVC90)
			set(VPS_CRT "VC9")
		elseif(MSVC10)
			set(VPS_CRT "VC10")
		else()
			set(VPS_CRT "VC")
		endif()
	endif()
endif()

if(WIN32 AND MSVC)
	set(PLATFORM win${BITS})
	set(_threadsuffix Mt)
	if(MSVC71)
		set(VC_VER vc71)
		set(VC_VER_LONG vc71)
	elseif(MSVC80)
		set(VC_SHORT VC8)
		set(VC_LONG MSVC80)
	elseif(MSVC90)
		set(VC_SHORT VC9)
		set(VC_LONG MSVC90)
	endif()
	list(APPEND
		_vpslibnames
		"Vps${VC_SHORT}_${BITS}"
		"Vps${VC_SHORT}_${BITS}${_threadsuffix}")
endif()

list(APPEND _vpslibnames "Vps${VPS_PLATFORM}${VPS_CRT}_${BITS}")
list(APPEND
	_grviewerlibnames
	"Viewer"
	"GrViewer${VPS_PLATFORM}${VPS_CRT}_${BITS}")

###
# Configure VPS
###

set(_incsearchdirs)
set(_libsearchdirs)

if(WIN32)
	include(ProgramFilesGlob)
	program_files_glob(_dirs "/VPS*/")
	program_files_glob(_dirs2 "/VPS/*/")
	list(APPEND _dirs ${_dirs2})
endif()

list_combinations(_libsearchdirs
	PREFIXES
	"${VPS_ROOT_DIR}"
	"${_dirs}"
	SUFFIXES
	"/lib"
	"/Viewer")
list_combinations(_libsearchdirs2
	PREFIXES
	${_libsearchdirs}
	SUFFIXES
	"/Release"
	"/RelWithDebInfo"
	"/MinSizeRel"
	"/Debug")
clean_directory_list(_libsearchdirs ${_libsearchdirs2})

list_combinations(_incsearchdirs
	PREFIXES
	"${VPS_ROOT_DIR}"
	"${_dirs}"
	SUFFIXES
	"/include"
	"/include/vps"
	"/src"
	"/../src"
	"/Viewer"
	"/../Viewer")
clean_directory_list(_incsearchdirs)

# If a version was specified, the least we can do is remove any directories
# from our search that contain too low of versions
if(VPS_FIND_VERSION)

	set(_badversions)
	foreach(_dir ${_libsearchdirs})
		string(REGEX MATCH "([0-9]).([0-9]).([0-9])" _ver "${_dir}")
		if(_ver)
			string(REGEX
				REPLACE
				"([0-9]).([0-9]).([0-9])"
				"\\1.\\2.\\3"
				_verstd
				"${_ver}")
			check_version(_result VPS "${_verstd}")
			if(NOT _result)
				list(APPEND _badversions "${_verstd}")
			endif()
		endif()
	endforeach()

	foreach(_badver ${_badversions})
		list_filter_out(_libsearchdirs ${_badver} ${_libsearchdirs})
		list_filter_out(_incsearchdirs ${_badver} ${_incsearchdirs})
	endforeach()
endif()
if(_libsearchdirs)
	list(SORT _libsearchdirs)
	list(REVERSE _libsearchdirs)
endif()

if(_incsearchdirs)
	list(SORT _incsearchdirs)
	list(REVERSE _incsearchdirs)
endif()

find_library(VPS_vps_LIBRARY
	NAMES
	${_vpslibnames}
	PATH_SUFFIXES
	LP
	HINTS
	${_libsearchdirs}
	PATHS
	${VPS_ROOT_DIR}
	${VPS_ROOT_DIR}/src
	${VPS_ROOT_DIR}/lib)

find_path(VPS_vps_INCLUDE_DIR
	NAMES
	Vps.h
	HINTS
	${_incsearchdirs}
	PATHS
	${VPS_ROOT_DIR}
	PATH_SUFFIXES
	include
	include/vps
	src)

find_library(VPS_grviewer_LIBRARY
	NAMES
	${_grviewerlibnames}
	HINTS
	${_libsearchdirs}
	PATHS
	${VPS_ROOT_DIR}
	PATH_SUFFIXES
	lib
	Viewer)

find_path(VPS_grviewer_INCLUDE_DIR
	NAMES
	grViewerLib.h
	HINTS
	${_incsearchdirs}
	PATHS
	${VPS_ROOT_DIR}
	PATH_SUFFIXES
	include
	include/vps
	Viewer)

mark_as_advanced(VPS_vps_LIBRARY
	VPS_vps_INCLUDE_DIR
	VPS_grviewer_LIBRARY
	VPS_grviewer_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VPS
	DEFAULT_MSG
	VPS_vps_LIBRARY
	VPS_vps_INCLUDE_DIR)

if(VPS_FOUND)
	set(VPS_vps_INCLUDE_DIRS "${VPS_vps_INCLUDE_DIR}")
	set(VPS_INCLUDE_DIRS "${VPS_vps_INCLUDE_DIR}")
	set(VPS_grviewer_INCLUDE_DIRS
		"${VPS_vps_INCLUDE_DIR}"
		"${VPS_grviewer_INCLUDE_DIR}")
	set(VPS_LIBRARIES "${VPS_vps_LIBRARY}")
	set(VPS_grviewer_LIBRARIES
		"${VPS_vps_LIBRARY}"
		"${VPS_grviewer_LIBRARY}")
	mark_as_advanced(VPS_ROOT_DIR VPS_DIR)
endif()
