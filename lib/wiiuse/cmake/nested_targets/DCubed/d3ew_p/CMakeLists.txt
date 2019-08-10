# - Build the DCubed wrapper library d3ew_p as a part of your solution.
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


set(SOURCES)
foreach(filename d3ew_p.rc
	p_dll.cpp
	p_dll.hxx
	p_dump.cpp
	p_frustrum.cpp
	p_gate.cpp
	p_gate.hxx
	p_lib.cpp
	p_modeller.cpp
	p_new.hxx
	p_rollmark.cpp
	p_utils.cpp
	p_utils.hxx
	res_defs.hxx)
	list(APPEND SOURCES "${DCUBED_WRAPPER_INCLUDE_DIR}/d3ew_p/${filename}")
endforeach()
add_library(dcubed_wrapper_d3ew_p_nested_target
	STATIC
	EXCLUDE_FROM_ALL
	${SOURCES})
target_link_libraries(dcubed_wrapper_d3ew_p_nested_target ${DCUBED_LIBRARIES} ${PARASOLID_LIBRARIES} "${DCUBED_WRAPPER_d3ew_scene_LIBRARY}")

set_property(TARGET
	dcubed_wrapper_d3ew_p_nested_target
	PROPERTY
	PROJECT_LABEL
	"DCubed d3ew_p Wrapper Library")
