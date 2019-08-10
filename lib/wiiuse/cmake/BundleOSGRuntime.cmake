# - Include the OpenSceneGraph runtime files in an installation or built package.
#
#  OSGRUNTIME_BUNDLE - Set to "yes" to enable this behavior
#  OSGRUNTIME_zlib1dll - Must be set to the location of zlib1.dll on Windows
#  OSGRUNTIME_zlib1ddll - Can be set to the location of zlib1d.dll (debug) on Windows.
#                         If set, will be installed.
#
# Requires these CMake modules:
#  no additional modules required
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

function(_osgbundle_split_debug_versions releasevar debugvar)
	set(release)
	set(debug)
	foreach(fn ${ARGN})
		get_filename_component(name "${fn}" NAME_WE)
		if(${name} MATCHES "d$")
			list(APPEND debug "${fn}")
		else()
			list(APPEND release "${fn}")
		endif()
	endforeach()
	set(${releasevar} ${release} PARENT_SCOPE)
	set(${debugvar} ${debug} PARENT_SCOPE)
endfunction()

function(_osgbundle_find_plugins varprefix filenameprefix)
	file(GLOB
		all
		"${OSG_RUNTIME_LIBRARY_DIR}/osgPlugins-${OPENSCENEGRAPH_VERSION}/${filenameprefix}*${CMAKE_SHARED_LIBRARY_SUFFIX}")
	_osgbundle_split_debug_versions(${varprefix}_PLUGINS_RELEASE
		${varprefix}_PLUGINS_DEBUG
		${all})
endfunction()

if(OPENSCENEGRAPH_FOUND)
	if(WIN32)
		get_filename_component(_osglibdir "${OSG_LIBRARY}" PATH)
		get_filename_component(_osgroot "${_osglibdir}/.." ABSOLUTE)
		set(OSG_RUNTIME_LIBRARY_DIR "${_osgroot}/bin")
		find_file(OSGBUNDLE_zlib1dll
			zlib1.dll
			PATHS
			"${_osgroot}/bin"
			"${_osgroot}/lib")
		find_file(OSGBUNDLE_zlib1ddll
			zlib1d.dll
			PATHS
			"${_osgroot}/bin"
			"${_osgroot}/lib")
		mark_as_advanced(OSGBUNDLE_zlib1dll OSGBUNDLE_zlib1ddll)
		set(_osgbundle_required OSGBUNDLE_zlib1dll)
		set(_osgbundle_platformOK on)
	else()
		get_filename_component(_osglibdir "${OSG_LIBRARY}" PATH)
		set(OSG_RUNTIME_LIBRARY_DIR "${_osglibdir}")
		set(_osgbundle_platformOK on)
	endif()

	# Find the osgDB plugins

	_osgbundle_find_plugins(OSGDB osgdb)
	_osgbundle_find_plugins(OSGWRAPPER osgwrapper)
endif()



if(_osgbundle_platformOK)
	set(_osgbundle_caninstall on)
	foreach(_var ${_osgbundle_required})
		if(NOT ${_var})
			# If we are missing a single required file, cut out now.
			set(_osgbundle_caninstall off)
			option(OSGRUNTIME_BUNDLE
				"Install a local copy of the OpenSceneGraph runtime files with the project."
				off)
		endif()
	endforeach()
	if(_osgbundle_caninstall)
		option(OSGRUNTIME_BUNDLE
			"Install a local copy of the OpenSceneGraph runtime files with the project."
			on)
	endif()
endif()

mark_as_advanced(OSGRUNTIME_BUNDLE)

if(OSGRUNTIME_BUNDLE AND OPENSCENEGRAPH_FOUND AND _osgbundle_caninstall)
	if(WIN32)
		set(DESTINATION bin)
		install(FILES "${OSGBUNDLE_zlib1dll}"
			DESTINATION ${DESTINATION})

		if(OSGBUNDLE_zlib1ddll)
			install(FILES "${OSGBUNDLE_zlib1ddll}"
				DESTINATION ${DESTINATION})
		endif()

	else()
		set(DESTINATION lib)
	endif()

	install(DIRECTORY "${_osgroot}/bin/" "${_osgroot}/lib/"
		DESTINATION ${DESTINATION}
		FILES_MATCHING

		# Runtime files
		PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()
