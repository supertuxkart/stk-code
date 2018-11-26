# - Find Microsoft DirectShow sample files, library, and headers.
#
#  DIRECTSHOW_INCLUDE_DIRS - where to find needed include file
#  DIRECTSHOW_BASECLASS_DIR- Directory containing the DirectShow baseclass sample code.
#  DIRECTSHOW_FOUND        - True if DirectShow found.
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Initially in VRPN - Distributed under the Boost Software License, Version 1.0.
#
# Almost entirely re-written by:
# 2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Look for one of the sample files.

set(_ds_quiet)
if(DirectShow_FIND_QUIETLY)
	set(_ds_quiet QUIET)
endif()
find_package(WindowsSDK ${_ds_quiet})
find_package(DirectX ${_ds_quiet})

set(DIRECTSHOW_WINDOWSSDK_ROOT
	"${DIRECTSHOW_WINDOWSSDK_ROOT}"
	CACHE
	PATH
	"A specific Windows SDK to use for DirectShow.")

if(MSVC AND CMAKE_CL_64)
	set(DIRECTSHOW_LIB_SUBDIR /x64)
else()
	set(DIRECTSHOW_LIB_SUBDIR)
endif()

set(_acceptable_winsdk)
if(WINDOWSSDK_FOUND)
	foreach(_sdkdir ${WINDOWSSDK_DIRS})
		if(EXISTS "${_sdkdir}/Samples/Multimedia/DirectShow/BaseClasses/streams.h"
			AND EXISTS "${_sdkdir}/Lib${DIRECTSHOW_LIB_SUBDIR}/strmiids.lib"
			AND EXISTS "${_sdkdir}/Include/DShow.h")
			list(APPEND _acceptable_winsdk "${_sdkdir}")
		endif()
	endforeach()
endif()

find_path(DIRECTSHOW_BASECLASS_DIR
	NAMES
	streams.h
	HINTS
	"${DIRECTSHOW_WINDOWSSDK_ROOT}"
	PATHS
	${_acceptable_winsdk}
	PATH_SUFFIXES
	"Samples/Multimedia/DirectShow/BaseClasses")

find_path(DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR
	NAMES
	DShow.h
	HINTS
	"${DIRECTSHOW_WINDOWSSDK_ROOT}"
	PATHS
	${_acceptable_winsdk}
	PATH_SUFFIXES
	"Include")

###
# Begin code dedicated to finding a proper qedit.h file...
###

# Checks to see if a directory has qedit.h and if that file contains ISampleGrabber.
function(_directshow_check_qedit _dir _var)
	set(fn "${_dir}/qedit.h")
	if(NOT EXISTS "${fn}")
		set(${_var} FALSE PARENT_SCOPE)
		return()
	endif()
	file(STRINGS "${fn}" samplegrabber REGEX ISampleGrabber)
	if(NOT samplegrabber)
		set(${_var} FALSE PARENT_SCOPE)
		return()
	endif()
	set(${_var} TRUE PARENT_SCOPE)
endfunction()

function(_directshow_check_current_qedit)
	if(DIRECTSHOW_QEDIT_INCLUDE_DIR)
		_directshow_check_qedit("${DIRECTSHOW_QEDIT_INCLUDE_DIR}" _ds_ok)
		if(NOT _ds_ok)
			message(STATUS "FindDirectShow: qedit.h in ${DIRECTSHOW_QEDIT_INCLUDE_DIR} lacks ISampleGrabber, unsetting DIRECTSHOW_QEDIT_INCLUDE_DIR")
			set(DIRECTSHOW_QEDIT_INCLUDE_DIR "" CACHE PATH "" FORCE)
		endif()
	endif()
endfunction()

# Check before deciding if we should make our list of possible locations.
_directshow_check_current_qedit()

# Compose a list of possible directories that might hold a qedit.h file.
set(DIRECTSHOW_QEDIT_SEARCH)
if(WINDOWSSDK_FOUND AND NOT DIRECTSHOW_QEDIT_INCLUDE_DIR)
	foreach(_sdk ${WINDOWSSDK_DIRS})
		windowssdk_build_lookup("${_sdk}" _build)
		if(_build AND ("${_build}" VERSION_LESS 6.2))
		get_windowssdk_include_dirs("${_sdk}" _dirs)
		if(_dirs)
			list(APPEND DIRECTSHOW_QEDIT_SEARCH ${_dirs})
		endif()
		endif()
	endforeach()
endif()

# This one we can grab from another SDK version.
find_path(DIRECTSHOW_QEDIT_INCLUDE_DIR
	NAMES
	qedit.h
	HINTS
	"${DIRECTSHOW_WINDOWSSDK_ROOT}"
	PATHS
	${DIRECTSHOW_QEDIT_SEARCH}
	PATH_SUFFIXES
	"Include")

# Check if the directory is OK after the search.
_directshow_check_current_qedit()

# If we didn't find a proper qedit, manually look through the possibilities.
if(NOT DIRECTSHOW_QEDIT_INCLUDE_DIR)
	foreach(_dir "${DIRECTSHOW_WINDOWSSDK_ROOT}/Include" ${DIRECTSHOW_QEDIT_SEARCH})
		if(NOT DIRECTSHOW_QEDIT_INCLUDE_DIR)
			_directshow_check_qedit("${_dir}" _ds_ok)
			if(_ds_ok)
				set(DIRECTSHOW_QEDIT_INCLUDE_DIR "${_dir}" CACHE PATH "" FORCE)
			endif()
		endif()
	endforeach()
endif()

###
# End qedit.h section.
###

set(DIRECTSHOW_STRMIIDS_SEARCH)
if(WINDOWSSDK_FOUND AND NOT DIRECTSHOW_STRMIIDS_LIBRARY)
	foreach(_sdk ${WINDOWSSDK_DIRS})
		get_windowssdk_library_dirs("${_sdk}" _dirs)
		if(_dirs)
			list(APPEND DIRECTSHOW_STRMIIDS_SEARCH ${_dirs})
		endif()
	endforeach()
endif()

find_library(DIRECTSHOW_STRMIIDS_LIBRARY
	NAMES
	strmiids
	HINTS
	"${DIRECTSHOW_WINDOWSSDK_ROOT}"
	PATHS
	${DIRECTSHOW_STRMIIDS_SEARCH}
	PATH_SUFFIXES
	"Lib${DIRECTSHOW_LIB_SUBDIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DirectShow
	DEFAULT_MSG
	DIRECTSHOW_BASECLASS_DIR
	DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR
	DIRECTSHOW_QEDIT_INCLUDE_DIR
	DIRECTX_INCLUDE_DIR
	DIRECTSHOW_STRMIIDS_LIBRARY)

if(DIRECTSHOW_FOUND)
	set(DIRECTSHOW_INCLUDE_DIRS
		# Baseclass must be before SDK so it gets the correct refclock.h
		"${DIRECTSHOW_BASECLASS_DIR}"
		"${DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR}"
		"${DIRECTX_INCLUDE_DIR}"
	)
	if(EXISTS "${DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR}/atl/atlbase.h")
		list(APPEND
			DIRECTSHOW_INCLUDE_DIRS
			"${DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR}/atl")
	endif()
	if(NOT "${DIRECTSHOW_QEDIT_INCLUDE_DIR}" STREQUAL "${DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR}")
		# QEdit include dir might be an older SDK, so put it last.
		list(APPEND DIRECTSHOW_INCLUDE_DIRS "${DIRECTSHOW_QEDIT_INCLUDE_DIR}")
	endif()

	set(DIRECTSHOW_LIBRARIES "${DIRECTSHOW_STRMIIDS_LIBRARY}")

	mark_as_advanced(DIRECTSHOW_WINDOWSSDK_ROOT)
endif()

mark_as_advanced(DIRECTSHOW_BASECLASS_DIR
	DIRECTSHOW_WINDOWSSDK_INCLUDE_DIR
	DIRECTSHOW_QEDIT_INCLUDE_DIR
	DIRECTSHOW_STRMIIDS_LIBRARY)
