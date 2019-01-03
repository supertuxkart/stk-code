# - try to find Windows HID support, part of the WDK/DDK
#
# Cache Variables: (probably not for direct use in your scripts)
#  WINHID_INCLUDE_DIR
#  WINHID_CRT_INCLUDE_DIR
#  WINHID_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  WINHID_FOUND
#  WINHID_INCLUDE_DIRS
#  WINHID_LIBRARIES
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#  PrefixListGlob
#  CleanDirectoryList
#  MinGWSearchPathExtras
#  FindWindowsSDK
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

if(NOT WIN32)
	find_package_handle_standard_args(WinHID
		"Skipping search for Windows HID on non-Windows platform"
		WIN32)
	return()
endif()

if(MSVC)
	if( (NOT WINHID_ROOT_DIR) AND (NOT ENV{DDKROOT} STREQUAL "") )
		set(WINHID_ROOT_DIR "$ENV{DDKROOT}")
	endif()
endif()

set(WINHID_ROOT_DIR
	"${WINHID_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search")

set(_deps_check)
set(_need_crt_dir)
if(MSVC)

	find_package(WindowsSDK)
	set(WINSDK_LIBDIRS)
	if(WINDOWSSDK_FOUND)
		get_windowssdk_library_dirs_multiple(WINSDK_LIBDIRS ${WINDOWSSDK_PREFERRED_FIRST_DIRS})
		foreach(WINSDKDIR ${WINDOWSSDK_DIRS})
			get_windowssdk_library_dirs(${WINSDKDIR} WINSDK_CURRENT_LIBDIRS)
			list(APPEND WINSDK_LIBDIRS ${WINSDK_CURRENT_LIBDIRS})
		endforeach()
	endif()

	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(_arch amd64) # what the architecture used to be called
		set(_arch8 x64) # what the WDK for Win8+ calls this architecture
	else()
		set(_arch i386) # what the architecture used to be called
		set(_arch8 x86) # what the WDK for Win8+ calls this architecture
	endif()
	include(PrefixListGlob)
	include(CleanDirectoryList)
	prefix_list_glob(_prefixed
		"*/"
		"$ENV{SYSTEMDRIVE}/WinDDK/"
		"$ENV{ProgramFiles}/Windows Kits/"
		"c:/WinDDK/")
	clean_directory_list(_prefixed)
	find_library(WINHID_LIBRARY
		NAMES
		hid
		libhid
		PATHS
		"${WINHID_ROOT_DIR}"
		${WINSDK_LIBDIRS}
		${_prefixed}
		PATH_SUFFIXES
		"lib/w2k/${_arch}" # Win2k min requirement
		"lib/wxp/${_arch}" # WinXP min requirement
		"lib/wnet/${_arch}" # Win Server 2003 min requirement
		"lib/wlh/${_arch}" # Win Vista ("Long Horn") min requirement
		"lib/wlh/um/${_arch8}" # Win Vista ("Long Horn") min requirement
		"lib/win7/${_arch}" # Win 7 min requirement
		"lib/win7/um/${_arch8}" # Win 7 min requirement
		"lib/win8/${_arch}" # Win 8 min requirement
		"lib/win8/um/${_arch8}" # Win 8 min requirement
		)
	# Might want to look close to the library first for the includes.
	if(WINHID_LIBRARY)
		get_filename_component(WINHID_LIBRARY_DIR "${WINHID_LIBRARY}" PATH)
		if(WINDOWSSDK_FOUND)
			get_windowssdk_from_component("${WINHID_LIBRARY}" _USED_WINSDK)
			set(WINHID_LIBRARY_FROM_WINDOWSSDK ON)
			get_windowssdk_include_dirs(${_USED_WINSDK} WINHID_INCLUDE_HINTS)
		endif()
	endif()

	find_library(WINHID_SETUPAPI_LIBRARY
		NAMES
		setupapi
		HINTS
		"${WINHID_LIBRARY_DIR}"
		PATHS
		"${WINHID_ROOT_DIR}"
		${WINSDK_LIBDIRS}
		${_prefixed}
		PATH_SUFFIXES
		"lib/w2k/${_arch}" # Win2k min requirement
		"lib/wxp/${_arch}" # WinXP min requirement
		"lib/wnet/${_arch}" # Win Server 2003 min requirement
		"lib/wlh/${_arch}" # Win Vista ("Long Horn") min requirement
		"lib/wlh/um/${_arch8}" # Win Vista ("Long Horn") min requirement
		"lib/win7/${_arch}" # Win 7 min requirement
		"lib/win7/um/${_arch8}" # Win 7 min requirement
		"lib/win8/${_arch}" # Win 8 min requirement
		"lib/win8/um/${_arch8}" # Win 8 min requirement
		)

	if(WINHID_LIBRARY AND NOT WINHID_LIBRARY_FROM_WINDOWSSDK)
		set(_basedir "${WINHID_LIBRARY_DIR}")
		set(_prevdir)
		while(NOT IS_DIRECTORY "${_basedir}/lib" AND NOT (_basedir STREQUAL _prevdir))
			set(_prevdir "${_basedir}")
			get_filename_component(_basedir "${_basedir}/.." ABSOLUTE)
		endwhile()

		set(WINHID_INCLUDE_HINTS "${_basedir}")

		if(EXISTS "${_basedir}/inc")
			find_path(WINHID_CRT_INCLUDE_DIR # otherwise you get weird compile errors
				NAMES
				stdio.h
				HINTS
				"${_basedir}"
				PATHS
				"${WINHID_ROOT_DIR}"
				PATH_SUFFIXES
				inc/crt
				NO_DEFAULT_PATH)
			list(APPEND _deps_check WINHID_CRT_INCLUDE_DIR)
			set(_need_crt_dir ON)
		endif()
	endif()
	find_path(WINHID_INCLUDE_DIR
		NAMES
		hidsdi.h
		HINTS
		${WINHID_INCLUDE_HINTS}
		PATHS
		"${WINHID_ROOT_DIR}"
		PATH_SUFFIXES
		inc/ddk
		inc/api
		inc/w2k
		inc/wxp
		inc/wnet
		include/shared)
else()
	# This is the non-MSVC path.
	if(MINGW)
		include(MinGWSearchPathExtras)

		find_library(WINHID_LIBRARY
			NAMES
			libhid
			HINTS
			"${WINHID_ROOT_DIR}"
			${MINGWSEARCH_LIBRARY_DIRS}
			/mingw
			PATH_SUFFIXES
			lib
			lib/w32api)
		find_library(WINHID_SETUPAPI_LIBRARY
			NAMES
			libsetupapi
			HINTS
			"${WINHID_ROOT_DIR}"
			${MINGWSEARCH_LIBRARY_DIRS}
			/mingw
			PATH_SUFFIXES
			lib
			lib/w32api)
	else()
		find_library(WINHID_LIBRARY
			NAMES
			hid
			libhid
			HINTS
			"${WINHID_ROOT_DIR}"
			/mingw
			PATH_SUFFIXES
			lib
			lib/w32api)
		find_library(WINHID_SETUPAPI_LIBRARY
			NAMES
			setupapi
			libsetupapi
			HINTS
			"${WINHID_ROOT_DIR}"
			/mingw
			PATH_SUFFIXES
			lib
			lib/w32api)
	endif()
	find_path(WINHID_INCLUDE_DIR
		NAMES
		hidsdi.h
		PATHS
		"${WINHID_ROOT_DIR}"
		${MINGWSEARCH_INCLUDE_DIRS}
		/mingw
		PATH_SUFFIXES
		include/w32api/ddk
		include/ddk
		ddk)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WinHID
	DEFAULT_MSG
	WINHID_LIBRARY
	WINHID_SETUPAPI_LIBRARY
	WINHID_INCLUDE_DIR
	${_deps_check})

if(WINHID_FOUND)
	if(MSVC)
		set(_winreq)
		if(WINHID_LIBRARY MATCHES "[Ll]ib/w2k")
			set(_winreq "Windows 2000")
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/wxp")
			set(_winreq "Windows XP")
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/wnet")
			set(_winreq "Windows Server 2003")
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/wlh")
			set(_winreq "Windows Vista")
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/win7")
			set(_winreq "Windows 7")
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/win8")
			set(_winreq "Windows 8")
			set(_winreq_uncertain ON)
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/winv6.3")
			set(_winreq "Windows 8.1")
			set(_winreq_uncertain ON)
		elseif(WINHID_LIBRARY MATCHES "[Ll]ib/10.0")
			set(_winreq "Windows 10")
			set(_winreq_uncertain ON)
		endif()
		if(NOT "${WINHID_MIN_WINDOWS_VER}" STREQUAL "${_winreq}")
			if(NOT WinHID_FIND_QUIETLY)
				if(NOT _winreq)
					message("Couldn't determine if the WINHID_LIBRARY would result in a minimum version compatibility requirement.")
				elseif(_winreq_uncertain)
					message(STATUS
					"Found WINHID_LIBRARY in the Windows SDK for ${_winreq} , which may or may not affect minimum compatible Windows version.")
				else()
					message(STATUS
						"Linking against WINHID_LIBRARY will enforce this minimum version: ${_winreq}")
				endif()
			endif()
			set(WINHID_MIN_WINDOWS_VER "${_winreq}" CACHE INTERNAL "" FORCE)
		endif()
	endif()
	set(WINHID_LIBRARIES "${WINHID_LIBRARY}" "${WINHID_SETUPAPI_LIBRARY}")
	if(_need_crt_dir)
		set(WINHID_INCLUDE_DIRS
			"${WINHID_CRT_INCLUDE_DIR}"
			"${WINHID_INCLUDE_DIR}")
	else()
		# Don't need that CRT include dir for WDK 8+
		set(WINHID_INCLUDE_DIRS
			"${WINHID_INCLUDE_DIR}")
	endif()
	mark_as_advanced(WINHID_ROOT_DIR)
endif()

mark_as_advanced(WINHID_INCLUDE_DIR
	WINHID_CRT_INCLUDE_DIR
	WINHID_LIBRARY)
