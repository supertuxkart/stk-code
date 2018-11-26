# - try to find JTTK library
#
#  JTTK_LIBRARY_DIRS, library search path
#  JTTK_INCLUDE_DIRS, include search path
#  JTTK_{component}_LIBRARY, the library to link against
#  JTTK_ENVIRONMENT, environment variables to set
#  JTTK_RUNTIME_LIBRARY_DIRS
#  JTTK_FOUND, If false, do not try to use this library.
#
# If you have license issues, you might run this command on each JtTk-using target:
#  jttk_stamp_binary(<targetname>)
#
# Plural versions refer to this library and its dependencies, and
# are recommended to be used instead, unless you have a good reason.
#
# Useful configuration variables you might want to add to your cache:
#  JTTK_ROOT_DIR - A directory prefix to search
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

include(ListCombinations)
include(CheckVersion)
include(GetDirectoryList)
include(PrefixListGlob)
include(GetCompilerInfoString)
if(WIN32)
	include(ProgramFilesGlob)
endif()

set(JTTK_ROOT_DIR
	"${JTTK_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for JtTk")

if(NOT JTTK_CUSTOMER_ID)
	set(JTTK_CUSTOMER_ID "$ENV{JTTK_CUSTOMER_ID}")
endif()

set(JTTK_CUSTOMER_ID
	"${JTTK_CUSTOMER_ID}"
	CACHE
	STRING
	"JtTk customer ID, to place in the environment")


get_filename_component(_jttk_mod_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)

if(NOT BITS)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(BITS 64)
	else()
		set(BITS 32)
	endif()
endif()

set(JTTK_ENVIRONMENT "JTTK_DEV_PLATFORM=${BITS}")
if(JTTK_CUSTOMER_ID)
	list(APPEND JTTK_ENVIRONMENT "JTTK_CUSTOMER_ID=${JTTK_CUSTOMER_ID}")
endif()

if(WIN32 AND MSVC)
	if(MSVC90)
		set(VC_VER vc9)
		set(VC_VER_LONG vc90)
	elseif(MSVC80)
		set(VC_VER vc8)
		set(VC_VER_LONG vc80)
	elseif(MSVC71)
		set(VC_VER vc71)
		set(VC_VER_LONG vc71)
	endif()

	if(BITS EQUAL 32)
		set(PLATFORM win32)
	else()
		set(PLATFORM win64)
	endif()
endif()

if(NOT "${3RDPARTYROOT}")
	set(3RDPARTYROOT ${CMAKE_SOURCE_DIR}/third-party)
endif()

set(libsearchdirs)
set(includesearchdirs)
set(_jttklibs)
set(_libsuffixes)
if(WIN32)
	program_files_fallback_glob(_dirs "/UGS/JTOpenToolkit/*/dev")
	program_files_fallback_glob(_dirs2 "/Siemens/JTOpenToolkit/*/dev")
	list(APPEND _dirs ${_dirs2})

	file(TO_CMAKE_PATH "$ENV{JTTK_DEV_PATH}" _envloc)
	list(APPEND _dirs "${_envloc}")

	if(MSVC90)
		prefix_list_glob(_vc9_libdirs
			"/lib/win_${BITS}vc9/JtTk*.dll"
			"${JTTK_ROOT_DIR}"
			${_dirs})
		list(APPEND _jttklibs ${_vc9_libdirs})
		prefix_list_glob(_vc9_libdirs
			"/lib/win_${BITS}_vc9/JtTk*.dll"
			"${JTTK_ROOT_DIR}"
			${_dirs})
		list(APPEND _jttklibs ${_vc9_libdirs})
		list(APPEND _libsuffixes "/lib/win_${BITS}vc9" "/lib/win_${BITS}_vc9")
	endif()
	if(MSVC80)
		prefix_list_glob(_vc8_libdirs
			"/lib/win_${BITS}/JtTk*.dll"
			"${JTTK_ROOT_DIR}"
			${_dirs})
		list(APPEND _jttklibs ${_vc8_libdirs})
		list(APPEND _libsuffixes "/lib/win_${BITS}")
	endif()
	if(MSVC71)
		prefix_list_glob(_vc71_libdirs
			"/lib/win_${BITS}vs7/JtTk*.dll"
			"${dirs}")
		list(APPEND _jttklibs "${_vc71_libdirs}")
		list(APPEND _libsuffixes "/lib/win_${BITS}vs7")
	endif()

elseif(UNIX)

	get_gcc_version(_gccver)
	if("${_gccver}" VERSION_LESS "4.1.0")
		set(_compiler "")
	else()
		set(_compiler "_gcc41")
	endif()

	string(TOLOWER "${CMAKE_SYSTEM_NAME}" _sysname)
	file(TO_CMAKE_PATH "$ENV{JTTK_DEV_PATH}" _envloc)
	prefix_list_glob(_jttklibs
		"/lib/${_sysname}_${BITS}${_compiler}/libJtTk*"
		"${JTTK_ROOT_DIR}"
		"/usr/"
		"/usr/local/"
		"/usr/local/siemens/"
		"/usr/local/ugs/")
	prefix_list_glob(_jttklibs2
		"/dev/lib/${_sysname}_${BITS}${_compiler}/libJtTk*"
		"${JTTK_ROOT_DIR}"
		"/usr/"
		"/usr/local/"
		"/usr/local/siemens/"
		"/usr/local/ugs/"
		"${_envloc}/")
	list(APPEND _jttklibs ${_jttklibs2})

	list(APPEND _libsuffixes "/lib/${_sysname}_${BITS}${_compiler}")
endif()

foreach(_lib ${_jttklibs})
	string(REGEX MATCH "JtTk[0-9][0-9]" _jttkver "${_lib}")
	if(_jttkver)
		string(REGEX
			REPLACE
			"JtTk([0-9])([0-9])"
			"\\1.\\2"
			_verstd
			"${_jttkver}")
		string(REGEX
			REPLACE
			"JtTk([0-9])([0-9])"
			"\\1\\2"
			_vernodot
			"${_jttkver}")
	endif()
	check_version(_result JtTk "${_verstd}")
	if(_result)
		get_filename_component(_libpath "${_lib}" PATH)
		list(APPEND JTTK_JTTK_VERSIONS ${_vernodot})
		list(APPEND JTTK_DEV_PATHS "${_libpath}")
	else()
		#message(STATUS "Found JtTk version ${ver}, does not meet requirements")
	endif()
endforeach()

if(JTTK_JTTK_VERSIONS)
	list(SORT JTTK_JTTK_VERSIONS)
	list(REVERSE JTTK_JTTK_VERSIONS)
endif()

###
# Configure JtTk
###

###
# Find the link library
###
list_combinations(names PREFIXES "JtTk" SUFFIXES ${JTTK_JTTK_VERSIONS})
find_library(JTTK_JtTk_LIBRARY
	NAMES
	${names}
	HINTS
	${JTTK_DEV_PATHS}
	PATH_SUFFIXES
	${_libsuffixes})
set(JTTK_LIBRARY "${JTTK_JtTk_LIBRARY}")
set(JTTK_LIBRARIES "${JTTK_JtTk_LIBRARY}")

###
# Prepare for the rest of our search based off of where we found the link library
###
get_filename_component(JTTK_LIBRARY_DIR "${JTTK_LIBRARY}" PATH)
get_filename_component(JTTK_DEV_PATH
	"${JTTK_LIBRARY_DIR}/../.."
	ABSOLUTE)

# Grab JtTk version
string(REGEX MATCH "JtTk[0-9]*" _ver "${JTTK_LIBRARY}")
string(REGEX
	REPLACE
	"JtTk([0-9])([0-9])"
	"\\1.\\2"
	JTTK_JTTK_VERSION
	"${_ver}")
string(REGEX
	REPLACE
	"JtTk([0-9])([0-9])"
	"\\1\\2"
	JTTK_JTTK_VERNODOT
	"${_ver}")

# Grab JT version
file(GLOB _jtdll "${JTTK_LIBRARY_DIR}/*JtBrep*")
string(REGEX MATCH "JtBrep[0-9]*" _jtver "${_jtdll}")
string(REGEX
	REPLACE
	"JtBrep([0-9])([0-9])"
	"\\1\\2"
	JTTK_JT_VERNODOT
	"${_jtver}")

# Setup dev path
get_filename_component(JTTK_DEV_PATH
	"${JTTK_LIBRARY_DIR}/../../"
	ABSOLUTE)

list(APPEND JTTK_ENVIRONMENT "JTTK_DEV_PATH=${JTTK_DEV_PATH}")
set(ENV{JTTK_DEV_PLATFORM} ${BITS})
set(ENV{JTTK_DEV_PATH} "${JTTK_DEV_PATH}")

set(_deps_libs)
set(_deps_includes)
set(_deps_check)

###
# Find the headers
###
find_path(JTTK_INCLUDE_DIR
	JtTk/JtkEntity.h
	HINTS
	${JTTK_DEV_PATH}/include)

if(WIN32)
	###
	# Find the DLL's
	###

	# Find the versioned DLL's
	foreach(dll Jt JtBrep JtLibra JtSimp JtSupt JtXTBrep ParaSupt)
		find_file(JTTK_${dll}_DLL
			NAMES
			"${dll}${JTTK_JT_VERNODOT}.dll"
			HINTS
			"${JTTK_LIBRARY_DIR}")
		list(APPEND JTTK_DLLS ${JTTK_${dll}_DLL})
		mark_as_advanced(JTTK_${dll}_DLL)
	endforeach()

	# Find the unversioned DLL's and the matching JtTk dll
	foreach(dll psbodyshop pskernel psxttoolkit JtTk${JTTK_JTTK_VERNODOT})
		list_combinations(names PREFIXES "${dll}" SUFFIXES ".dll")
		find_file(JTTK_${dll}_DLL
			NAMES
			${names}
			HINTS
			"${JTTK_LIBRARY_DIR}")
		list(APPEND JTTK_DLLS ${JTTK_${dll}_DLL})
		mark_as_advanced(JTTK_${dll}_DLL)
	endforeach()

	get_directory_list(JTTK_RUNTIME_LIBRARY_DIRS ${JTTK_DLLS})

elseif(UNIX)

	foreach(_lib Jt JtBrep JtLibra JtSimp JtSupt JtXTBrep ParaSupt)
		find_library(JTTK_${_lib}_LIBRARY
			NAMES
			"${_lib}${JTTK_JT_VERNODOT}"
			HINTS
			"${JTTK_LIBRARY_DIR}")
		list(APPEND _deps_libs "${JTTK_${_lib}_LIBRARY}")
		list(APPEND _deps_check JTTK_${_lib}_LIBRARY)
		mark_as_advanced(JTTK_${_lib}_LIBRARY)
	endforeach()

	# Find the unversioned libs
	foreach(_lib psbodyshop pskernel psxttoolkit eaiunicode)
		find_library(JTTK_${_lib}_LIBRARY
			NAMES
			${_lib}
			HINTS
			"${JTTK_LIBRARY_DIR}")
		list(APPEND _deps_libs "${JTTK_${_lib}_LIBRARY}")
		list(APPEND _deps_check JTTK_${_lib}_LIBRARY)
		mark_as_advanced(JTTK_${_lib}_LIBRARY)
	endforeach()

	# Find stamper
	#list(APPEND _deps_check JTTK_KEYS)

	find_program(JTTK_STAMP_COMMAND
		stampkey
		HINTS
		"${JTTK_DEV_PATH}/../bin")
	list(APPEND _deps_check JTTK_STAMP_COMMAND)

	find_program(JTTK_STAMP_PLATFORM_COMMAND
		stampkey
		HINTS
		"${JTTK_DEV_PATH}/../bin/${_sysname}")
	list(APPEND _deps_check JTTK_STAMP_PLATFORM_COMMAND)


	if("${JTTK_KEYS}" STREQUAL "${JTTK_KEYS_AUTO}" OR NOT JTTK_KEYS)
		find_file(JTTK_INSTALL_LOG
			install.log
			HINTS
			"${JTTK_DEV_PATH}/.."
			NO_DEFAULT_PATH)
		#list(APPEND _deps_check JTTK_INSTALL_LOG)
		mark_as_advanced(JTTK_INSTALL_LOG)

		if(JTTK_INSTALL_LOG)
			file(READ "${JTTK_INSTALL_LOG}" _log)
			string(REGEX MATCHALL "..key ([0-9A-Z])+" _keylines "${_log}")
			set(JTTK_KEYS)
			foreach(_keyline ${_keylines})
				string(REGEX
					REPLACE
					"..key (([0-9A-Z])+)$"
					"\\1"
					_key
					"${_keyline}")
				list(APPEND JTTK_KEYS "${_key}")
				message(STATUS "Found JtTk key: ${_key}")
			endforeach()
			set(JTTK_KEYS
				"${JTTK_KEYS}"
				CACHE
				STRING
				"A semi-colon separated list of JtTk keys to stamp on the binaries."
				FORCE)
			set(JTTK_KEYS_AUTO
				"${JTTK_KEYS}"
				CACHE
				INTERNAL
				"The keys we auto-detected"
				FORCE)
		endif()
	else()
		foreach(_key ${JTTK_KEYS})
			message(STATUS "Using cached JtTk key: ${_key}")
		endforeach()
		set(JTTK_KEYS
			"${JTTK_KEYS}"
			CACHE
			STRING
			"A semi-colon separated list of JtTk keys to stamp on the binaries.")
	endif()

	# Find dependencies
	find_library(JTTK_MATH_LIBRARY m)
	mark_as_advanced(JTTK_MATH_LIBRARY)
	list(APPEND _deps_check JTTK_MATH_LIBRARY)
	list(APPEND _deps_libs ${JTTK_MATH_LIBRARY})

	if(NOT X11_FOUND)
		find_package(X11)
	endif()
	list(APPEND _deps_check X11_FOUND)
	list(APPEND _deps_libs ${X11_LIBRARIES})
	list(APPEND _deps_includes ${X11_INCLUDE_DIRS})

	if(NOT OPENGL_FOUND)
		find_package(OpenGL)
	endif()
	list(APPEND _deps_check OPENGL_FOUND)
	list(APPEND _deps_libs ${OPENGL_LIBRARIES})
	list(APPEND _deps_includes ${OPENGL_INCLUDE_DIR})

	if(NOT THREADS_FOUND)
		find_package(Threads)
	endif()
	list(APPEND _deps_check THREADS_FOUND)
	list(APPEND _deps_libs ${CMAKE_THREAD_LIBS_INIT})

	get_directory_list(JTTK_RUNTIME_LIBRARY_DIRS ${_deps_libs})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JtTk
	DEFAULT_MSG
	JTTK_JtTk_LIBRARY
	JTTK_CUSTOMER_ID
	JTTK_INCLUDE_DIR
	${_deps_check})

if(JTTK_FOUND)
	set(JTTK_INCLUDE_DIRS "${JTTK_INCLUDE_DIR}" ${_deps_includes})
	set(JTTK_LIBRARIES "${JTTK_LIBRARY}" ${_deps_libs})
	mark_as_advanced(JTTK_CUSTOMER_ID JTTK_ROOT_DIR)
endif()

function(jttk_stamp_binary _target)
	if(UNIX)
		get_target_property(_binary "${_target}" LOCATION)
		configure_file("${_jttk_mod_dir}/FindJtTk.stampkey.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/${_target}.stampkey.cmake"
			@ONLY)
		add_custom_command(TARGET
			"${_target}"
			POST_BUILD
			COMMAND
			"${CMAKE_COMMAND}"
			-P
			"${CMAKE_CURRENT_BINARY_DIR}/${_target}.stampkey.cmake"
			COMMENT
			"Stamping executable ${_binary} with JtTk keys..."
			VERBATIM)
	endif()
endfunction()

mark_as_advanced(JTTK_JtTk_LIBRARY
	JTTK_INCLUDE_DIR
	JTTK_KEYS
	JTTK_STAMP_COMMAND
	JTTK_STAMP_PLATFORM_COMMAND)
