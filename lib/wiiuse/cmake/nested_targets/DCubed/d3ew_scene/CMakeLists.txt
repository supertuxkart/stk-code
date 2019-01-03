# - Build the DCubed wrapper library d3ew_scene as a part of your solution.
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
foreach(filename ansi.hxx
	body.hxx
	d3ew_clap_data.cpp
	d3ew_clap_data.hxx
	d3ew_new.hxx
	d3ew_stack.cpp
	d3ew_stack.hxx
	debload.hxx
	edge.cpp
	edge.hxx
	ehlc_timer.cpp
	ehlc_timer.hxx
	face.cpp
	face.hxx
	frustum.cpp
	frustum.hxx
	hldeb.hxx
	hljournal.hxx
	hlm_cdm_dummies.cpp
	hltable.hxx
	hltabledef.hxx
	journalw.cpp
	journalw.hxx
	modeller.cpp
	modeller.hxx
	nowarn.hxx
	scene.cpp
	scene.hxx
	scene_build.cpp
	scene_edge.cpp
	scene_face.cpp
	scene_misc.cpp
	scene_node.cpp
	scene_node.hxx
	scene_table.cpp
	scene_table.hxx
	secure_c.hxx
	segment.cpp
	segment.hxx
	table_dec.hxx
	wrapper.cpp
	wrapper_new.cpp)
	list(APPEND SOURCES "${DCUBED_WRAPPER_INCLUDE_DIR}/d3ew_scene/${filename}")
endforeach()
add_library(dcubed_wrapper_d3ew_scene_nested_target
	STATIC
	EXCLUDE_FROM_ALL
	${SOURCES})

set_property(TARGET
	dcubed_wrapper_d3ew_scene_nested_target
	PROPERTY
	PROJECT_LABEL
	"DCubed d3ew_scene Wrapper Library")
