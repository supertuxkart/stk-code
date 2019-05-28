# - try to find Adrienne Electronics Corporation timecode card library
#
# SDK available from the manufacturer: http://www.adrielec.com/
#
# Cache Variables: (probably not for direct use in your scripts)
#  ADRIENNE_INCLUDE_DIR
#  ADRIENNE_LIBRARY
#  ADRIENNE_RUNTIME_LIBRARY
#  ADRIENNE_INCLUDE_FILE
#
# Variables you might use in your CMakeLists.txt:
#  ADRIENNE_FOUND
#  ADRIENNE_INCLUDE_DIRS
#  ADRIENNE_LIBRARIES
#  ADRIENNE_RUNTIME_LIBRARIES - the AEC_NTTC.dll file
#  ADRIENNE_RUNTIME_LIBRARY_DIRS
#
#  ADRIENNE_INCLUDE_FILENAME - this is probably AEC_NTTC.h, but it might also
#    be AECINTTC.H.
#
#  ADRIENNE_INCLUDE_HAS_EXTERN_C - Some (most) versions of the header already
#    wrap their definitions in extern "C" { }, but some do not.
#
#  ADRIENNE_DEFINITIONS - defines a quoted ADRIENNE_INCLUDE_FILENAME as above,
#    so you can write a line like #include ADRIENNE_INCLUDE_FILENAME
#    Also defines ADRIENNE_BEFORE_INCLUDE and ADRIENNE_AFTER_INCLUDE to handle
#    adding extern "C" { and } if the header file doesn't do so itself.
#
# Variables that might be set by the user in the gui/command line to help
# find the library:
#  ADRIENNE_ROOT_DIR - root of an Adrienne CD, disk, or extracted/copied contents
#    thereof.
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2012 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2012.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

# Apparently Windows only.
if(WIN32 OR CYGWIN)
	set(ADRIENNE_ROOT_DIR
		"${ADRIENNE_ROOT_DIR}"
		CACHE
		PATH
		"Directory to search for Adrienne Electronics Timecode data - root of a software distribution or extracted download from http://www.adrielec.com/")

	set(ADRIENNE_DEFINITIONS)

	set(ADRIENNE_INCLUDE_SEARCH
		# from AecPCIeTC_8-12a: contains AEC_NTTC.h with extern "C" and three extra functions:
		# AEC_PCTC_OPEN_ALL, AEC_PCTC_CLOSE_ALL,  AEC_PCTC_INITIALIZE_EVENT
		"SDK_SourceCode"

		# from AecPci6_02_CD - called AECINTTC.H but otherwise essentially identical to earlier versions
		"TestPrograms_WithDLL/DLL_API_INFO"
		# A zipped development project contains essentially the same, named AEC_NTTC.h so we'll add this in case it's unzipped.
		"TestPrograms_WithDLL/ZippedDevelopmentProjects/AecMfc32_Rel504"

		# from pc-ltc - called AECINTTC.H and lacks extern "C"
		"NT-CODE/C40-APP1"
	)
	set(ADRIENNE_HEADER_NAMES
		AEC_NTTC.H
		AEC_NTTC.h
		Aec_nttc.h
		AECINTTC.H)

	set(ADRIENNE_LIB_SEARCH)
	set(ADRIENNE_DLL_SEARCH)

	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		# 64 bit code - apparently initially packaged only in the
		# PCIe version of their software
		list(APPEND ADRIENNE_LIB_SEARCH
			# from AecPCIeTC_8-12a
			"64BitOS_files/DLL Versions")

		list(APPEND ADRIENNE_DLL_SEARCH
			# from AecPCIeTC_8-12a
			"64BitOS_files/DLL Versions")
	else()
		# 32-bit code, much more prevalent.
		list(APPEND ADRIENNE_LIB_SEARCH
			# from AecPCIeTC_8-12a
			"32BitOS_files/DLLversions"

			# from AecPci6_02_CD
			"TestPrograms_WithDLL/DLL_API_INFO"

			# from pc-ltc
			"NT-CODE/DLL"
			)

		list(APPEND ADRIENNE_DLL_SEARCH
			# from AecPCIeTC_8-12a
			"32BitOS_files/DLLversions"

			# from AecPci6_02_CD
			"TestPrograms_WithDLL"

			# from pc-ltc
			"NT-CODE/DLL")
	endif()

	find_library(ADRIENNE_LIBRARY
		NAMES
		AEC_NTTC
		PATHS
		"${ADRIENNE_ROOT_DIR}"
		PATH_SUFFIXES
		${ADRIENNE_LIB_SEARCH})

	find_path(ADRIENNE_INCLUDE_DIR
		NAMES
		${ADRIENNE_HEADER_NAMES}
		PATHS
		"${ADRIENNE_ROOT_DIR}"
		PATH_SUFFIXES
		${ADRIENNE_INCLUDE_SEARCH})

	if(ADRIENNE_INCLUDE_DIR)
		find_file(ADRIENNE_INCLUDE_FILE
			NAMES
			${ADRIENNE_HEADER_NAMES}
			HINTS
			${ADRIENNE_INCLUDE_DIR})

		# Get include filename
		get_filename_component(ADRIENNE_INCLUDE_FILENAME
			"${ADRIENNE_INCLUDE_FILE}"
			NAME)
		list(APPEND ADRIENNE_DEFINITIONS -DADRIENNE_INCLUDE_FILENAME="ADRIENNE_INCLUDE_FILENAME")

		# Find out if it has extern "C" in it.
		file(STRINGS "${ADRIENNE_INCLUDE_FILE}" strings)
		set(ADRIENNE_INCLUDE_HAS_EXTERN_C OFF)
		foreach(_line ${strings})
			if("${_line}" STREQUAL "extern \"C\"")
				set(ADRIENNE_INCLUDE_HAS_EXTERN_C ON)
				break()
			endif()
		endforeach()

		if(ADRIENNE_INCLUDE_HAS_EXTERN_C)
			list(APPEND ADRIENNE_DEFINITIONS -DADRIENNE_BEFORE_INCLUDE -DADRIENNE_AFTER_INCLUDE)
		else()
			list(APPEND ADRIENNE_DEFINITIONS "-DADRIENNE_BEFORE_INCLUDE=extern \"C\" {" "-DADRIENNE_AFTER_INCLUDE=}")
		endif()
	endif()

	get_filename_component(_adrienne_libdir "${ADRIENNE_LIBRARY}" PATH)
	find_file(ADRIENNE_RUNTIME_LIBRARY
		NAMES
		AEC_NTTC.dll
		HINTS
		"${_adrienne_libdir}"
		"${_adrienne_libdir}/.."
		PATHS
		"${ADRIENNE_ROOT_DIR}"
		PATH_SUFFIXES
		${ADRIENNE_DLL_SEARCH})


	set(ADRIENNE_RUNTIME_LIBRARIES "${ADRIENNE_RUNTIME_LIBRARY}")
	get_filename_component(ADRIENNE_RUNTIME_LIBRARY_DIRS
		"${ADRIENNE_RUNTIME_LIBRARY}"
		PATH)

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(Adrienne
		DEFAULT_MSG
		ADRIENNE_LIBRARY
		ADRIENNE_RUNTIME_LIBRARY
		ADRIENNE_INCLUDE_DIR
		ADRIENNE_INCLUDE_FILENAME)

	if(ADRIENNE_FOUND)
		set(ADRIENNE_LIBRARIES "${ADRIENNE_LIBRARY}")
		set(ADRIENNE_INCLUDE_DIRS "${ADRIENNE_INCLUDE_DIR}")
		mark_as_advanced(ADRIENNE_ROOT_DIR)
	endif()

	mark_as_advanced(
		ADRIENNE_LIBRARY
		ADRIENNE_RUNTIME_LIBRARY
		ADRIENNE_INCLUDE_DIR
		ADRIENNE_INCLUDE_FILE)
else()
	set(ADRIENNE_FOUND OFF)
	set(ADRIENNE_SDK_IS_WINDOWS_ONLY NOTFOUND)
	find_package_handle_standard_args(Adrienne
		DEFAULT_MSG
		ADRIENNE_SDK_IS_WINDOWS_ONLY)
endif()
