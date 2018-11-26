# - Build the DCubed wrapper libraries as a part of your solution.
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


include_directories("${DCUBED_CORE_INCLUDE_DIR}/d3e_inc" "${DCUBED_WRAPPER_INCLUDE_DIR}")

include_directories(${PARASOLID_INCLUDE_DIRS})
if(MSVC)
	add_definitions(/D_CRT_SECURE_NO_DEPRECATE /arch:SSE2 /openmp /D_AFXDLL /DNDEBUG)
endif()
if(WIN32)
	add_definitions(/DDCM_NT /DWIN32 /D_WINDOWS)
endif()
add_definitions(/DD3EW_DUMMY_HLM)

if(DCUBED_WRAPPER_d3ew_scene_LIBRARY STREQUAL "dcubed_wrapper_d3ew_scene_nested_target")
	add_subdirectory(d3ew_scene)
endif()

if(DCUBED_WRAPPER_d3ew_p_LIBRARY STREQUAL "dcubed_wrapper_d3ew_p_nested_target")
	add_subdirectory(d3ew_p)
endif()
