# - Provide access to the OpenSceneGraph runtime files for bundling in
# an installation or package.
#
# Sets these variables:
#  - OSGDB_PLUGINS_RELEASE
#  - OSGDB_PLUGINS_DEBUG
#  - OSGWRAPPER_PLUGINS_RELEASE
#  - OSGWRAPPER_PLUGINS_DEBUG
#  - OSG_RUNTIME_LIBRARY_DIR
#  - OSG_PATH_TO_PLUGINS
#
# Creates this function:
#  - install_osg_plugins( {varNameForOutputFilenames} )
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
		if("${name}" MATCHES "d$")
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
	set(${varprefix}_PLUGINS_RELEASE
		"${${varprefix}_PLUGINS_RELEASE}"
		PARENT_SCOPE)
	set(${varprefix}_PLUGINS_DEBUG
		"${${varprefix}_PLUGINS_DEBUG}"
		PARENT_SCOPE)
endfunction()

if(OPENSCENEGRAPH_FOUND)
	if(WIN32)
		get_filename_component(_osglibdir "${OSG_LIBRARY}" PATH)
		get_filename_component(_osgroot "${_osglibdir}/.." ABSOLUTE)
		set(OSG_RUNTIME_LIBRARY_DIR "${_osgroot}/bin")
		set(OSG_PATH_TO_PLUGINS "bin/osgPlugins-${OPENSCENEGRAPH_VERSION}/")
	else()
		get_filename_component(_osglibdir "${OSG_LIBRARY}" PATH)
		set(OSG_RUNTIME_LIBRARY_DIR "${_osglibdir}")
		set(OSG_PATH_TO_PLUGINS "lib/osgPlugins-${OPENSCENEGRAPH_VERSION}/")
	endif()
	# Find the osgDB plugins
	_osgbundle_find_plugins(OSGDB osgdb)
	_osgbundle_find_plugins(OSGWRAPPER osgwrapper)
endif()

function(install_osg_plugins var)
	set(INSTALLEDPLUGINS)
	foreach(plugin ${OSGDB_PLUGINS_RELEASE} ${OSGWRAPPER_PLUGINS_RELEASE})
		install(FILES "${plugin}"
			DESTINATION "${OSG_PATH_TO_PLUGINS}"
			CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
		get_filename_component(name "${plugin}" NAME)
		list(APPEND INSTALLEDPLUGINS "${OSG_PATH_TO_PLUGINS}/${name}")
	endforeach()
	foreach(plugin ${OSGDB_PLUGINS_DEBUG} ${OSGWRAPPER_PLUGINS_DEBUG})
		install(FILES
			"${plugin}"
			DESTINATION
			"${OSG_PATH_TO_PLUGINS}"
			CONFIGURATIONS
			Debug)
		#get_filename_component(name "${plugin}" NAME)
		#list(APPEND INSTALLEDPLUGINS "${OSG_PATH_TO_PLUGINS}/${name}")
	endforeach()
	set(${var} ${INSTALLEDPLUGINS} PARENT_SCOPE)
endfunction()
