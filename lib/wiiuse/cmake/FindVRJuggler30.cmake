# - try to find VRJuggler 3.0-related packages (main finder)
#  VRJUGGLER30_LIBRARY_DIRS, library search paths
#  VRJUGGLER30_INCLUDE_DIRS, include search paths
#  VRJUGGLER30_LIBRARIES, the libraries to link against
#  VRJUGGLER30_ENVIRONMENT
#  VRJUGGLER30_RUNTIME_LIBRARY_DIRS
#  VRJUGGLER30_CXX_FLAGS
#  VRJUGGLER30_DEFINITIONS
#  VRJUGGLER30_FOUND, If false, do not try to use VR Juggler 3.0.
#
# Components available to search for (uses "VRJOGL30" by default):
#  VRJOGL30
#  VRJ30
#  Gadgeteer20
#  JCCL14
#  VPR22
#  Sonix14
#  Tweek14
#
# Additionally, a full setup requires these packages and their Find_.cmake scripts
#  CPPDOM
#  GMTL
#
# Optionally uses Flagpoll (and FindFlagpoll.cmake)
#
# Notes on components:
#  - All components automatically include their dependencies.
#  - You can search for the name above with or without the version suffix.
#  - If you do not specify a component, VRJOGL30(the OpenGL view manager)
#    will be used by default.
#  - Capitalization of component names does not matter, but it's best to
#    pretend it does and use the above capitalization.
#  - Since this script calls find_package for your requested components and
#    their dependencies, you can use any of the variables specified in those
#    files in addition to the "summary" ones listed here, for more finely
#    controlled building and linking.
#
# This CMake script requires all of the Find*.cmake scripts for the
# components listed above, as it is only a "meta-script" designed to make
# using those scripts more developer-friendly.
#
# Useful configuration variables you might want to add to your cache:
#  (CAPS COMPONENT NAME)_ROOT_DIR - A directory prefix to search
#                         (a path that contains include/ as a subdirectory)
#
# The VJ_BASE_DIR environment variable is also searched (preferentially)
# when seeking any of the above components, as well as Flagpoll, CPPDOM,
# and Boost (from within VPR22), so most sane build environments should
# "just work."
#
# IMPORTANT: Note that you need to manually re-run CMake if you change
# this environment variable, because it cannot auto-detect this change
# and trigger an automatic re-run.
#
# Original Author:
# 2009-2011 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
# Updated for VR Juggler 3.0 by:
# Brandon Newendorp <brandon@newendorp.com> and Ryan Pavlik
#
# Copyright Iowa State University 2009-2011.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

include(CleanLibraryList)
include(CleanDirectoryList)
include(FindPackageMessage)

set(VRJUGGLER30_ROOT_DIR
	"${VRJUGGLER30_ROOT_DIR}"
	CACHE
	PATH
	"Additional root directory to search for VR Juggler and its dependencies.")
if(NOT VRJUGGLER30_ROOT_DIR)
	file(TO_CMAKE_PATH "$ENV{VJ_BASE_DIR}" VRJUGGLER30_ROOT_DIR)
endif()

# Default required components
if(NOT VRJuggler30_FIND_COMPONENTS)
	set(VRJuggler30_FIND_COMPONENTS vrjogl30)
endif()

if(VRJuggler30_FIND_QUIETLY)
	set(_FIND_FLAGS "QUIET")
else()
	set(_FIND_FLAGS "")
endif()

set(VRJUGGLER30_SUBMODULES
	VRJ30
	VRJOGL30
	Gadgeteer20
	JCCL14
	VPR22
	Sonix14
	Tweek14)
string(TOUPPER "${VRJUGGLER30_SUBMODULES}" VRJUGGLER30_SUBMODULES_UC)
string(TOUPPER
	"${VRJuggler30_FIND_COMPONENTS}"
	VRJUGGLER30_FIND_COMPONENTS_UC)

# Turn a potentially messy components list into a nice one with versions.
set(VRJUGGLER30_REQUESTED_COMPONENTS)
foreach(VRJUGGLER30_LONG_NAME ${VRJUGGLER30_SUBMODULES_UC})
	# Look at requested components
	foreach(VRJUGGLER30_REQUEST ${VRJUGGLER30_FIND_COMPONENTS_UC})
		string(REGEX
			MATCH
			"${VRJUGGLER30_REQUEST}"
			VRJUGGLER30_MATCHING
			"${VRJUGGLER30_LONG_NAME}")
		if(VRJUGGLER30_MATCHING)
			list(APPEND
				VRJUGGLER30_REQUESTED_COMPONENTS
				${VRJUGGLER30_LONG_NAME})
			list(APPEND
				VRJUGGLER30_COMPONENTS_FOUND
				${VRJUGGLER30_LONG_NAME}_FOUND)
		endif()
	endforeach()
endforeach()

if(VRJUGGLER30_REQUESTED_COMPONENTS)
	list(REMOVE_DUPLICATES VRJUGGLER30_REQUESTED_COMPONENTS)
endif()

if(VRJUGGLER30_COMPONENTS_FOUND)
	list(REMOVE_DUPLICATES VRJUGGLER30_COMPONENTS_FOUND)
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(_VRJ_LIBSUFFIXES lib64 lib)
	set(_VRJ_LIBDSUFFIXES
		debug
		lib64/x86_64/debug
		lib64/debug
		lib64
		lib/x86_64/debug
		lib/debug
		lib)
	set(_VRJ_LIBDSUFFIXES_ONLY
		debug
		lib64/x86_64/debug
		lib64/debug
		lib/x86_64/debug
		lib/debug)
else()
	set(_VRJ_LIBSUFFIXES lib)
	set(_VRJ_LIBDSUFFIXES debug lib/i686/debug lib/debug lib)
	set(_VRJ_LIBDSUFFIXES_ONLY debug lib/i686/debug lib/debug)
endif()

if(NOT VRJUGGLER30_FIND_QUIETLY
	AND NOT VRJUGGLER30_FOUND
	AND NOT "${_VRJUGGLER30_SEARCH_COMPONENTS}"	STREQUAL "${VRJUGGLER30_REQUESTED_COMPONENTS}")
	message(STATUS
		"Searching for these requested VR Juggler 3.0 components and their dependencies: ${VRJUGGLER30_REQUESTED_COMPONENTS}")
endif()

# Find components
if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "VRJOGL30" AND NOT VRJOGL30_FOUND)
	find_package(VRJOGL30 ${_FIND_FLAGS})
endif()

if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "VRJ30" AND NOT VRJ30_FOUND)
	find_package(VRJ30 ${_FIND_FLAGS})
endif()

if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "JCCL14" AND NOT JCCL14_FOUND)
	find_package(JCCL14 ${_FIND_FLAGS})
endif()

if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "GADGETEER20" AND NOT GADGETEER20_FOUND)
	find_package(Gadgeteer20 ${_FIND_FLAGS})
endif()

if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "SONIX14" AND NOT SONIX14_FOUND)
	find_package(Sonix14 ${_FIND_FLAGS})
endif()

if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "TWEEK14" AND NOT TWEEK14_FOUND)
	find_package(Tweek14 ${_FIND_FLAGS})
endif()

if("${VRJUGGLER30_REQUESTED_COMPONENTS}" MATCHES "VPR22" AND NOT VPR22_FOUND)
	find_package(VPR22 ${_FIND_FLAGS})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VRJuggler30
	DEFAULT_MSG
	${VRJUGGLER30_COMPONENTS_FOUND})

if(VRJUGGLER30_FOUND)
	foreach(VRJUGGLER30_REQUEST ${VRJUGGLER30_REQUESTED_COMPONENTS})
		list(APPEND VRJUGGLER30_LIBRARIES ${${VRJUGGLER30_REQUEST}_LIBRARIES})
		list(APPEND
			VRJUGGLER30_INCLUDE_DIRS
			${${VRJUGGLER30_REQUEST}_INCLUDE_DIRS})
	endforeach()

	clean_library_list(VRJUGGLER30_LIBRARIES)

	clean_directory_list(VRJUGGLER30_INCLUDE_DIRS)

	set(_vjbase)
	set(_vjbaseclean)
	foreach(_component VPR22 VRJ30 VRJOGL30 JCCL14 GADGETEER20)

		# Find the common parent directory of the include dir and the library dir
		get_filename_component(_absincpath "${${_component}_INCLUDE_DIR}" PATH)
		if(${_component}_INCLUDE_DIR)
			get_filename_component(_curpath "${${_component}_LIBRARY_RELEASE}" PATH)
			get_filename_component(_nextpath "${_curpath}/.." ABSOLUTE)
			while(NOT _curpath STREQUAL _nextpath)
				set(_curpath "${_nextpath}")
				string(LENGTH "${_curpath}" _pathlen)
				if("${_absincpath}" MATCHES "^${_curpath}.*")
					list(APPEND _vjbase "${_curpath}")
					break()
				endif()
				get_filename_component(_nextpath "${_curpath}/.." ABSOLUTE)
			endwhile()
		endif()
	endforeach()

	clean_directory_list(_vjbase)

	set(_vrj30_have_base_dir NO)
	list(LENGTH _vjbase _vjbaselen)
	if("${_vjbaselen}" EQUAL 1 AND NOT VRJUGGLER30_VJ_BASE_DIR)
		list(GET _vjbase 0 VRJUGGLER30_VJ_BASE_DIR)
		mark_as_advanced(VRJUGGLER30_VJ_BASE_DIR)
		if(NOT VRJUGGLER30_VJ_BASE_DIR STREQUAL _vrj30_base_dir)
			unset(VRJUGGLER30_VJ_CFG_DIR)
		endif()
		set(_vrj30_have_base_dir YES)
	else()
		list(GET _vjbase 0 _calculated_base_dir)
		if(NOT
			"${_calculated_base_dir}"
			STREQUAL
			"${VRJUGGLER30_VJ_BASE_DIR}")
			message("It looks like you might be mixing VR Juggler versions... ${_vjbaselen} ${_vjbase}")
			message("If you are, fix your libraries then remove the VRJUGGLER30_VJ_BASE_DIR variable in CMake, then configure again")
			message("If you aren't, set the VRJUGGLER30_VJ_BASE_DIR variable to the desired VJ_BASE_DIR to use when running")
		else()
			if(NOT VRJUGGLER30_VJ_BASE_DIR STREQUAL _vrj30_base_dir)
				unset(VRJUGGLER30_VJ_CFG_DIR)
			endif()
			set(_vrj30_have_base_dir YES)
		endif()
	endif()

	set(_vrj30_base_dir "${VRJUGGLER30_VJ_BASE_DIR}")
	set(_vrj30_base_dir "${_vrj30_base_dir}" CACHE INTERNAL "" FORCE)

	if(_vrj30_have_base_dir)
		find_path(VRJUGGLER30_VJ_CFG_DIR
			standalone.jconf
			PATHS
			${VRJUGGLER30_VJ_BASE_DIR}/share/vrjuggler-3.0/data/configFiles
			${VRJUGGLER30_VJ_BASE_DIR}/share/vrjuggler/data/configFiles
			NO_DEFAULT_PATH)
		mark_as_advanced(VRJUGGLER30_VJ_CFG_DIR)
	endif()

	set(VRJUGGLER30_VJ_BASE_DIR
		"${VRJUGGLER30_VJ_BASE_DIR}"
		CACHE
		PATH
		"Base directory to use as VJ_BASE_DIR when running your app."
		FORCE)
	set(VRJUGGLER30_ENVIRONMENT
		"VJ_BASE_DIR=${VRJUGGLER30_VJ_BASE_DIR}"
		"JCCL_BASE_DIR=${VRJUGGLER30_VJ_BASE_DIR}"
		"SONIX_BASE_DIR=${VRJUGGLER30_VJ_BASE_DIR}"
		"TWEEK_BASE_DIR=${VRJUGGLER30_VJ_BASE_DIR}"
		"VJ_CFG_DIR=${VRJUGGLER30_VJ_CFG_DIR}")

	include(GetDirectoryList)

	get_directory_list(VRJUGGLER30_RUNTIME_LIBRARY_DIRS
		${VRJUGGLER30_LIBRARIES})
	if(WIN32)
		foreach(dir ${VRJUGGLER30_RUNTIME_LIBRARY_DIRS})
			list(APPEND VRJUGGLER30_RUNTIME_LIBRARY_DIRS "${dir}/../bin")
		endforeach()
	endif()

	if(MSVC)
		# BOOST_ALL_DYN_LINK
		set(VRJUGGLER30_DEFINITIONS
			"-DBOOST_ALL_DYN_LINK"
			"-DCPPDOM_DYN_LINK"
			"-DCPPDOM_AUTO_LINK")

		# Disable these annoying warnings
		# 4275: non dll-interface class used as base for dll-interface class
		# 4251: needs to have dll-interface to be used by clients of class
		# 4100: unused parameter
		# 4512: assignment operator could not be generated
		# 4127: (Not currently disabled) conditional expression in loop evaluates to constant

		set(VRJUGGLER30_CXX_FLAGS "/wd4275 /wd4251 /wd4100 /wd4512")
	elseif(CMAKE_COMPILER_IS_GNUCXX)
		# Silence annoying warnings about deprecated hash_map.
		set(VRJUGGLER30_CXX_FLAGS "-Wno-deprecated")

		set(VRJUGGLER30_DEFINITIONS "")
	endif()
	set(VRJUGGLER30_CXX_FLAGS
		"${VRJUGGLER30_CXX_FLAGS} ${CPPDOM_CXX_FLAGS}")

	set(_VRJUGGLER30_SEARCH_COMPONENTS
		"${VRJUGGLER30_REQUESTED_COMPONENTS}"
		CACHE
		INTERNAL
		"Requested components, used as a flag.")



	set(_plugin_dirs)
	foreach(_libdir ${VRJUGGLER30_RUNTIME_LIBRARY_DIRS})
		# Find directories of Gadgeteer plugins and drivers
		if(EXISTS "${_libdir}/gadgeteer")
			list(APPEND
				_plugin_dirs
				"${_libdir}/gadgeteer/drivers"
				"${_libdir}/gadgeteer/plugins")
		elseif(EXISTS "${_libdir}/gadgeteer-1.4")
			list(APPEND
				_plugin_dirs
				"${_libdir}/gadgeteer-1.4/drivers"
				"${_libdir}/gadgeteer-1.4/plugins")
		endif()

		# Find directories of Sonix plugins
		if(EXISTS "${_libdir}/sonix")
			list(APPEND _plugin_dirs "${_libdir}/sonix/plugins/dbg")
			list(APPEND _plugin_dirs "${_libdir}/sonix/plugins/opt")
		elseif(EXISTS "${_libdir}/sonix-1.4")
			list(APPEND _plugin_dirs "${_libdir}/sonix-1.4/plugins/dbg")
			list(APPEND _plugin_dirs "${_libdir}/sonix-1.4/plugins/opt")
		endif()

		# Find directories of JCCL plugins
		if(EXISTS "${_libdir}/jccl/plugins")
			list(APPEND _plugin_dirs "${_libdir}/jccl/plugins")
		elseif(EXISTS "${_libdir}/jccl-1.4/plugins")
			list(APPEND _plugin_dirs "${_libdir}/jccl-1.4/plugins")
		endif()

		# Find directories of VR Juggler plugins
		if(EXISTS "${_libdir}/vrjuggler/plugins")
			list(APPEND _plugin_dirs "${_libdir}/vrjuggler/plugins")
		elseif(EXISTS "${_libdir}/vrjuggler-3.0/plugins")
			list(APPEND _plugin_dirs "${_libdir}/vrjuggler-3.0/plugins")
		endif()
	endforeach()

	# Grab the actual plugins
	foreach(_libdir ${_plugin_dirs})
		if(EXISTS "${_libdir}")
			list(APPEND VRJUGGLER30_RUNTIME_LIBRARY_DIRS "${_libdir}")
			file(GLOB _plugins "${_libdir}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
			foreach(_plugin ${_plugins})
				if("${_plugin}" MATCHES "_d${CMAKE_SHARED_LIBRARY_SUFFIX}")
					list(APPEND VRJUGGLER30_BUNDLE_PLUGINS_DEBUG ${_plugin})
				else()
					list(APPEND VRJUGGLER30_BUNDLE_PLUGINS ${_plugin})
				endif()
			endforeach()
		endif()
	endforeach()

	mark_as_advanced(VRJUGGLER30_ROOT_DIR)
endif()

mark_as_advanced(VRJUGGLER30_DEFINITIONS)

function(install_vrjuggler30_data_files prefix)
	set(base "${VRJUGGLER30_VJ_CFG_DIR}/..")
	get_filename_component(base "${base}" ABSOLUTE)
	file(RELATIVE_PATH reldest "${VRJUGGLER30_VJ_BASE_DIR}" "${base}")
	if(prefix STREQUAL "" OR prefix STREQUAL "." OR prefix STREQUAL "./")
		set(DEST "${reldest}")
	else()
		set(DEST "${prefix}/${reldest}")
	endif()

	# configFiles *.jconf
	file(GLOB
		_vj_config_files
		"${base}/configFiles/*.jconf")
	install(FILES ${_vj_config_files} DESTINATION "${DEST}/configFiles/")

	# definitions *.jdef
	file(GLOB
		_vj_defs_files
		"${base}/definitions/*.jdef")
	install(FILES ${_vj_defs_files} DESTINATION "${DEST}/definitions/")

	# models *.flt
	file(GLOB
		_vj_model_files
		"${base}/models/*.flt")
	install(FILES ${_vj_model_files} DESTINATION "${DEST}/models/")

	# sounds *.wav
	file(GLOB
		_vj_sound_files
		"${base}/sounds/*.wav")
	install(FILES ${_vj_sound_files} DESTINATION "${DEST}/sounds/")
endfunction()

macro(_vrjuggler30_plugin_install _VAR)
	foreach(plugin ${${_VAR}})
		get_filename_component(full "${plugin}" ABSOLUTE)
		file(RELATIVE_PATH relloc "${VRJUGGLER30_VJ_BASE_DIR}" "${full}")
		set(filedest "${DEST}/${relloc}")
		get_filename_component(path "${filedest}" PATH)
		list(APPEND out "${filedest}")
		install(FILES "${full}" DESTINATION "${path}" ${ARGN})
	endforeach()
endmacro()

function(install_vrjuggler30_plugins prefix varForFilenames)
	set(DEST "${prefix}")

	set(out)
	_vrjuggler30_plugin_install(VRJUGGLER30_BUNDLE_PLUGINS)
	_vrjuggler30_plugin_install(VRJUGGLER30_BUNDLE_PLUGINS_DEBUG CONFIGURATIONS DEBUG)
	set(${varForFilenames} ${out} PARENT_SCOPE)

endfunction()

function(get_vrjuggler_bundle_sources _target_sources)
	if(APPLE)
		set(_bundledir "${VRJUGGLER30_VJ_CFG_DIR}/../bundle")
		get_filename_component(_bundledir "${_bundledir}" ABSOLUTE)

		set(_vj_base_dir .)
		set(_vj_data_dir ${vj_base_dir}/share/vrjuggler-3.0)

		# Append Mac-specific sources to source list
		set(_vj_bundle_src
			${_bundledir}/vrjuggler.icns
			${_bundledir}/vrjuggler.plist
			${_bundledir}/en.lproj/MainMenu.nib/classes.nib
			${_bundledir}/MainMenu.nib/info.nib
			${_bundledir}/MainMenu.nib/keyedobjects.nib)

		message(STATUS "vjbundlesrc: ${_vj_bundle_src}")
		set(${_target_sources}
			${${_target_sources}}
			${_vj_bundle_src}
			PARENT_SCOPE)

		# Set destination of nib files
		set_source_files_properties(${_bundledir}/MainMenu.nib/classes.nib
			${_bundledir}/MainMenu.nib/info.nib
			${_bundledir}/MainMenu.nib/keyedobjects.nib
			PROPERTIES
			MACOSX_PACKAGE_LOCATION
			Resources/en.lproj/MainMenu.nib/)

		# Set destination of Resources
		set_source_files_properties(${_bundledir}/vrjuggler.icns
			${_bundledir}/vrjuggler.plist
			PROPERTIES
			MACOSX_PACKAGE_LOCATION
			Resources/)
	endif()
endfunction()

get_filename_component(_vrjuggler30moddir
	${CMAKE_CURRENT_LIST_FILE}
	PATH)
function(fixup_vrjuggler_app_bundle
	_target
	_targetInstallDest
	_extralibs
	_libdirs)

	if(NOT VRJUGGLER30_FOUND)
		return()
	endif()


	set(PACKAGE_DIR ${_vrjuggler30moddir}/package)
	set(MACOSX_PACKAGE_DIR ${PACKAGE_DIR}/macosx)

	set(TARGET_LOCATION
		"${_targetInstallDest}/${_target}${CMAKE_EXECUTABLE_SUFFIX}")
	if(APPLE)
		set(TARGET_LOCATION "${TARGET_LOCATION}.app")
	endif()

	set_target_properties(${_target}
		PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST
		${MACOSX_PACKAGE_DIR}/VRJuggler30BundleInfo.plist.in
		MACOSX_BUNDLE_ICON_FILE
		vrjuggler.icns
		MACOSX_BUNDLE_INFO_STRING
		"${PROJECT_NAME} (VR Juggler Application) version ${CPACK_PACKAGE_VERSION}, created by ${CPACK_PACKAGE_VENDOR}"
		MACOSX_BUNDLE_GUI_IDENTIFIER
		org.vrjuggler.${PROJECT_NAME}
		MACOSX_BUNDLE_SHORT_VERSION_STRING
		${CPACK_PACKAGE_VERSION}
		MACOSX_BUNDLE_BUNDLE_VERSION
		${CPACK_PACKAGE_VERSION})

	if(WIN32)
		list(APPEND _libdirs "${VRJUGGLER30_VJ_BASE_DIR}/bin")
	endif()

	set(BUNDLE_LIBS ${_extralibs})
	set(BUNDLE_LIB_DIRS "${VRJUGGLER30_VJ_BASE_DIR}" ${_libdirs})

	configure_file(${PACKAGE_DIR}/fixupbundle.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/${_target}-fixupbundle-juggler.cmake
		@ONLY)
	install(SCRIPT
		"${CMAKE_CURRENT_BINARY_DIR}/${_target}-fixupbundle-juggler.cmake")
endfunction()
