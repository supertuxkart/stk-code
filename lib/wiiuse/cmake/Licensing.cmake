# - Building a licensing description file
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

define_property(GLOBAL
	PROPERTY
	PROPRIETARY_LICENSES
	BRIEF_DOCS
	"Text for proprietary licenses"
	FULL_DOCS
	"Text for proprietary licenses")
define_property(GLOBAL
	PROPERTY
	SHAREALIKE_LICENSES
	BRIEF_DOCS
	"Text for share-alike licenses"
	FULL_DOCS
	"Text for share-alike licenses (e.g. GPL)")
define_property(GLOBAL
	PROPERTY
	PERMISSIVE_LICENSES
	BRIEF_DOCS
	"Text for permissive licenses"
	FULL_DOCS
	"Text for permissive licenses (e.g. BSD, MIT, X11)")
define_property(GLOBAL
	PROPERTY
	PACKAGES_LICENSED
	BRIEF_DOCS
	"List of all packages whose licenses are registered here"
	FULL_DOCS
	"List of all packages whose licenses are registered here")
define_property(GLOBAL
	PROPERTY
	REDISTRIBUTION_WARNINGS
	BRIEF_DOCS
	"Text for important redistribution warnings"
	FULL_DOCS
	"Text for important redistribution warnings, such as 'This is not redistributable!'")


function(add_license LICENSE_TYPE_PROPERTY package license)
	get_property(def GLOBAL PROPERTY LICENSE_PACKAGE_${package} DEFINED)
	if(NOT def)
		define_property(GLOBAL
			PROPERTY
			LICENSE_PACKAGE_${package}
			BRIEF_DOCS
			"Flag to indicate the addition of the license for ${package}"
			FULL_DOCS
			"Flag to indicate the addition of the license for ${package}")
		get_property(existing GLOBAL PROPERTY ${LICENSE_TYPE_PROPERTY})
		set_property(GLOBAL
			PROPERTY
			${LICENSE_TYPE_PROPERTY}
			"${existing}${license}\n\n")

		if(ARGN)
			define_property(GLOBAL
				PROPERTY
				LICENSE_EXTRAS_PACKAGE_${package}
				BRIEF_DOCS
				"Extras (like URL) for the license for ${package}"
				FULL_DOCS
				"Extras (like URL) for the license for ${package}")
			set_property(GLOBAL
				PROPERTY
				LICENSE_EXTRAS_PACKAGE_${package}
				"${ARGN}")
		endif()

		get_property(allpackages GLOBAL PROPERTY PACKAGES_LICENSED)
		list(APPEND allpackages "${package}")
		set_property(GLOBAL PROPERTY PACKAGES_LICENSED "${allpackages}")
	endif()
endfunction()

function(add_proprietary_license package license)
	add_license(PROPRIETARY_LICENSES "${package}" "${license}" ${ARGN})
endfunction()

function(add_sharealike_license package license)
	add_license(SHAREALIKE_LICENSES "${package}" "${license}" ${ARGN})
endfunction()

function(add_permissive_license package license)
	add_license(PERMISSIVE_LICENSES "${package}" "${license}" ${ARGN})
endfunction()

function(add_redistribution_warning warning)
	get_property(existing GLOBAL PROPERTY REDISTRIBUTION_WARNINGS)
	set_property(GLOBAL
		PROPERTY
		REDISTRIBUTION_WARNINGS
		"${warning}  ${existing}")
endfunction()

function(configure_license_file input output)
	get_property(PACKAGES_LICENSED GLOBAL PROPERTY PACKAGES_LICENSED)
	if(PACKAGES_LICENSED)
		list(SORT PACKAGES_LICENSED)
		set(PACKAGES_LICENSED_BULLETED)
		foreach(package ${PACKAGES_LICENSED})
			set(EXTRAS)
			get_property(package_extras
				GLOBAL
				PROPERTY
				"LICENSE_EXTRAS_PACKAGE_${package}")
			if(package_extras)
				set(EXTRAS " ${package_extras}")
			endif()
			set(PACKAGES_LICENSED_BULLETED
				"${PACKAGES_LICENSED_BULLETED} * ${package}${EXTRAS}\n")
		endforeach()
	endif()
	get_property(REDISTRIBUTION_WARNINGS
		GLOBAL
		PROPERTY
		REDISTRIBUTION_WARNINGS)
	get_property(PROPRIETARY_LICENSES GLOBAL PROPERTY PROPRIETARY_LICENSES)
	get_property(SHAREALIKE_LICENSES GLOBAL PROPERTY SHAREALIKE_LICENSES)
	get_property(PERMISSIVE_LICENSES GLOBAL PROPERTY PERMISSIVE_LICENSES)
	configure_file("${input}" "${output}" ${ARGN})
endfunction()

