# - Build the Parasolid utility libraries as a part of your solution.
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file ../../LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


if(PARASOLID_fg_LIBRARY STREQUAL "parasolid_fg_nested_target")
	get_filename_component(_fgdir "${PARASOLID_FG_C}" PATH)
	include_directories("${_fgdir}")
	add_library(parasolid_fg_nested_target
		STATIC
		EXCLUDE_FROM_ALL
		"${PARASOLID_FG_C}")

	set_property(TARGET
		parasolid_fg_nested_target
		PROPERTY
		COMPILE_DEFINITIONS
		_CRT_SECURE_NO_DEPRECATE)
	if(MSVC)
		set_property(TARGET
			parasolid_fg_nested_target
			PROPERTY
			COMPILE_FLAGS
			"/Gs /GF /GS- /fp:fast")
	endif()

	set_property(TARGET
		parasolid_fg_nested_target
		PROPERTY
		PROJECT_LABEL
		"Parasolid Foreign Geometry Example Library")

endif()


if(PARASOLID_frustrum_LIBRARY
	STREQUAL
	"parasolid_frustrum_nested_target")
	get_filename_component(_frustrumdir "${PARASOLID_FRUSTRUM_C}" PATH)
	include_directories("${_frustrumdir}")
	add_library(parasolid_frustrum_nested_target
		STATIC
		EXCLUDE_FROM_ALL
		"${PARASOLID_FRUSTRUM_C}")

	set_property(TARGET
		parasolid_frustrum_nested_target
		PROPERTY
		COMPILE_DEFINITIONS
		_CRT_SECURE_NO_DEPRECATE)
	if(MSVC)
		set_property(TARGET
			parasolid_frustrum_nested_target
			PROPERTY
			COMPILE_FLAGS
			"/Gs /GF /GS- /fp:fast")
	endif()

	set_property(TARGET
		parasolid_frustrum_nested_target
		PROPERTY
		PROJECT_LABEL
		"Parasolid Frustrum Example Library")

endif()
