# - Run doxygen on source files as a custom target
#
#  include(DoxygenTargets)
#  add_doxygen(<doxyfile> [OUTPUT_DIRECTORY <outputdir>]
#   [EXTRA_INPUT <single path or quoted list of paths>]
#   [EXTRA_STRIP_FROM_PATH <single path or quoted list of paths>]
#   [EXTRA_STRIP_FROM_INC_PATH <single path or quoted list of paths>]
#   [INSTALL_DESTINATION <installdir>
#   [INSTALL_COMPONENT <installcomponent>]
#   [INSTALL_PDF_NAME <installpdfname>] ]
#   [DOC_TARGET <targetname>]
#   [PROJECT_NUMBER <versionnumber>]
#   [NO_WARNINGS]
#   [NO_PDF])
#
# Requires these CMake modules:
#  FindDoxygen
#
# Requires CMake 2.6 or newer (uses the 'function' command)
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

if(__add_doxygen)
	return()
endif()
set(__add_doxygen YES)

# We must run the following at "include" time, not at function call time,
# to find the path to this module rather than the path to a calling list file
get_filename_component(_doxygenmoddir ${CMAKE_CURRENT_LIST_FILE} PATH)

if(APPLE)
	list(APPEND CMAKE_PREFIX_PATH "/usr/texbin")
endif()

if(NOT DOXYGEN_FOUND)
	find_package(Doxygen QUIET)
endif()

set(DOXYGEN_LATEX "NO")
set(DOXYGEN_PDFLATEX "NO")
set(DOXYGEN_DOT "NO")

if(DOXYGEN_DOT_EXECUTABLE)
	set(DOXYGEN_DOT "YES")
endif()

find_package(LATEX QUIET)
if(LATEX_COMPILER AND MAKEINDEX_COMPILER)
	set(DOXYGEN_LATEX "YES")
endif()

if(PDFLATEX_COMPILER)
	set(DOXYGEN_PDFLATEX "YES")
endif()

set(_PF86 "ProgramFiles(x86)")
find_program(DOXYGEN_MSCGEN_EXECUTABLE
	mscgen
	PATHS
	"$ENV{ProgramFiles}/Mscgen"
	"$ENV{${_PF86}}/Mscgen"
	"$ENV{ProgramW6432}/Mscgen")
if(DOXYGEN_MSCGEN_EXECUTABLE)
	mark_as_advanced(DOXYGEN_MSCGEN_EXECUTABLE)
endif()

# An optional single-file install that supports cmake older than 2.8.0
# For internal use
function(_dt_install_file target filename dest rename)
	if(CMAKE_VER VERSION_LESS 2.8.0)
		set(INSTALL_CODE  "
			if(EXISTS \"${filename}\")
				message(STATUS \"Found: ${filename}\")
				file(INSTALL
					DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${dest}\"
					TYPE FILE
					RENAME \"${rename}\"
					FILES \"${filename}\")
			else()
				message(STATUS \"Skipping (build '${target}' to create): ${filename}\")
			endif()
			")
		if(NOT ARGN STREQUAL "")
			set(INSTALL_COMPONENT "${ARGN}")
			set(INSTALL_CODE "
			if(NOT CMAKE_INSTALL_COMPONENT OR \"\${CMAKE_INSTALL_COMPONENT}\" STREQUAL \"${INSTALL_COMPONENT}\")
				${INSTALL_CODE}
			endif()
			")
		endif()
		install(CODE "${INSTALL_CODE}")
	else()
		set(COMPONENT_ARGS)
		if(NOT ARGN STREQUAL "")
			set(COMPONENT_ARGS COMPONENT "${ARGN}")
		endif()
		install(FILES
			"${filename}"
			DESTINATION
			"${dest}"
			RENAME "${rename}"
			${COMPONENT_ARGS}
			OPTIONAL)
	endif()

endfunction()

# An optional single-directory install that supports cmake older than 2.8.0
# For internal use
function(_dt_install_dir target dir dest)
	if(CMAKE_VER VERSION_LESS 2.8.0)
		set(INSTALL_CODE  "
			if(EXISTS \"${dir}\")
				message(STATUS \"Found: ${dir}\")
				file(INSTALL
					DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${dest}\"
					TYPE DIRECTORY
					FILES \"${dir}\")
			else()
				message(STATUS \"Skipping (build '${target}' to create): ${dir}\")
			endif()
			")
		if(NOT ARGN STREQUAL "")
			set(INSTALL_COMPONENT "${ARGN}")
			set(INSTALL_CODE "

			if(NOT CMAKE_INSTALL_COMPONENT OR \"\${CMAKE_INSTALL_COMPONENT}\" STREQUAL \"${INSTALL_COMPONENT}\")
				${INSTALL_CODE}
			endif()
			")
		endif()
		install(CODE "${INSTALL_CODE}")
	else()
		set(COMPONENT_ARGS)
		if(NOT ARGN STREQUAL "")
			set(COMPONENT_ARGS COMPONENT "${ARGN}")
		endif()
		install(DIRECTORY
			"${dir}"
			DESTINATION
			"${dest}"
			${COMPONENT_ARGS}
			OPTIONAL)
	endif()

endfunction()

function(add_doxygen _doxyfile)
	# parse arguments
	set(WARNINGS YES)
	set(_nowhere)
	set(_curdest _nowhere)
	set(_val_args
		OUTPUT_DIRECTORY
		EXTRA_INPUT
		EXTRA_STRIP_FROM_PATH
		EXTRA_STRIP_FROM_INC_PATH
		DOC_TARGET
		INSTALL_DESTINATION
		INSTALL_COMPONENT
		INSTALL_PDF_NAME
		PROJECT_NUMBER)
	set(_bool_args
		NO_WARNINGS
		NO_PDF)
	foreach(_arg ${_val_args} ${_bool_args})
		set(${_arg})
	endforeach()
	foreach(_element ${ARGN})
		list(FIND _val_args "${_element}" _val_arg_find)
		list(FIND _bool_args "${_element}" _bool_arg_find)
		if("${_val_arg_find}" GREATER "-1")
			set(_curdest "${_element}")
		elseif("${_bool_arg_find}" GREATER "-1")
			set("${_element}" ON)
			set(_curdest _nowhere)
		else()
			list(APPEND ${_curdest} "${_element}")
		endif()
	endforeach()

	if(_nowhere)
		message(FATAL_ERROR "Syntax error in use of add_doxygen!")
	endif()

	if(NO_WARNINGS)
		set(WARNINGS NO)
	endif()

	if(NOT DOC_TARGET)
		set(DOC_TARGET doc)
	endif()

	if(NOT OUTPUT_DIRECTORY)
		set(OUTPUT_DIRECTORY "docs-generated")
	endif()

	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_DIRECTORY}")

	if(NOT INSTALL_PDF_NAME)
		set(INSTALL_PDF_NAME "docs-generated.pdf")
	endif()

	if(NOT PROJECT_NUMBER)
		set(PROJECT_NUMBER "${CPACK_PACKAGE_VERSION}")
	endif()

	if(DOXYGEN_FOUND)
		if(TARGET ${DOC_TARGET})
			message(FATAL_ERROR "Documentation target ${DOC_TARGET} already exists!")
		endif()

		if(NOT IS_ABSOLUTE "${OUTPUT_DIRECTORY}")
			get_filename_component(OUTPUT_DIRECTORY
				"${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_DIRECTORY}"
				ABSOLUTE)
		endif()

		set_property(DIRECTORY
			APPEND
			PROPERTY
			ADDITIONAL_MAKE_CLEAN_FILES
			"${OUTPUT_DIRECTORY}/html"
			"${OUTPUT_DIRECTORY}/latex")

		if(NOT TARGET ${DOC_TARGET}_open)
			# Create a target to open the generated HTML file.
			if(WIN32)
				set(DOXYGEN_LAUNCHER_COMMAND start)
			elseif(NOT APPLE)
				set(DOXYGEN_LAUNCHER_COMMAND xdg-open)
			endif()
			if(DOXYGEN_LAUNCHER_COMMAND)
				add_custom_target(${DOC_TARGET}_open
					COMMAND ${DOXYGEN_LAUNCHER_COMMAND} "${OUTPUT_DIRECTORY}/html/index.html"
					VERBATIM)
				set_target_properties(${DOC_TARGET}_open
					PROPERTIES
					EXCLUDE_FROM_ALL
					TRUE)
				set_target_properties(${DOC_TARGET}_open
					PROPERTIES
					EXCLUDE_FROM_DEFAULT_BUILD
					TRUE)
				add_dependencies(${DOC_TARGET}_open ${DOC_TARGET})
			endif()
		endif()

		get_filename_component(_doxyfileabs "${_doxyfile}" ABSOLUTE)
		get_filename_component(INCLUDE_FILE "${_doxyfileabs}" NAME)
		get_filename_component(INCLUDE_PATH "${_doxyfileabs}" PATH)

		# Doesn't currently work on Windows, so don't bother
		if(DOXYGEN_LATEX AND NOT NO_PDF AND NOT WIN32)
			set(MAKE_PDF YES)
			set(GENERATE_LATEX YES)
		else()
			set(MAKE_PDF NO)
			set(GENERATE_LATEX NO)
		endif()

		if(DOXYGEN_PDFLATEX AND MAKE_PDF)
			set(USE_PDFLATEX YES)
		else()
			set(USE_PDFLATEX NO)
		endif()

		if(DOXYGEN_DOT)
			set(HAVE_DOT YES)
			set(DOT_PATH ${DOXYGEN_DOT_PATH})
		else()
			set(HAVE_DOT NO)
			set(DOT_PATH)
		endif()

		if(DOXYGEN_MSCGEN_EXECUTABLE)
			get_filename_component(MSCGEN_PATH "${DOXYGEN_MSCGEN_EXECUTABLE}" PATH)
		endif()

		# See http://www.cmake.org/pipermail/cmake/2006-August/010786.html
		# for info on this variable
		if("${CMAKE_BUILD_TOOL}" MATCHES "(msdev|devenv)")
			set(WARN_FORMAT "\"$file($line) : $text \"")
		else()
			set(WARN_FORMAT "\"$file:$line: $text \"")
		endif()

		configure_file("${_doxygenmoddir}/DoxygenTargets.doxyfile.in"
			"${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.${DOC_TARGET}.additional"
			@ONLY)

		if(IN_DASHBOARD_SCRIPT)
			set(ALL_IN_DASHBOARD ALL)
		else()
			set(ALL_IN_DASHBOARD)
		endif()

		add_custom_target(${DOC_TARGET} ${ALL_IN_DASHBOARD}
			COMMAND
			"${DOXYGEN_EXECUTABLE}"
			"${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.${DOC_TARGET}.additional"
			WORKING_DIRECTORY
			"${CMAKE_CURRENT_SOURCE_DIR}"
			#MAIN_DEPENDENCY ${DOC_TARGET}
			COMMENT
			"Running Doxygen with configuration ${_doxyfile}..."
			VERBATIM)

		if(NOT IN_DASHBOARD_SCRIPT)
			set_target_properties(${DOC_TARGET}
				PROPERTIES
				EXCLUDE_FROM_ALL
				TRUE)
			set_target_properties(${DOC_TARGET}
				PROPERTIES
				EXCLUDE_FROM_DEFAULT_BUILD
				TRUE)
		endif()
		if(MAKE_PDF)
			add_custom_command(TARGET
				${DOC_TARGET}
				POST_BUILD
				COMMAND
				${CMAKE_MAKE_PROGRAM}
				WORKING_DIRECTORY
				"${OUTPUT_DIRECTORY}/latex"
				COMMENT
				"Generating PDF using PDFLaTeX..."
				VERBATIM)
		endif()

		if(INSTALL_DESTINATION)
			if(INSTALL_COMPONENT)
				_dt_install_dir("${DOC_TARGET}" "${OUTPUT_DIRECTORY}/html" "${INSTALL_DESTINATION}" "${INSTALL_COMPONENT}")
				if(MAKE_PDF)
					_dt_install_file("${DOC_TARGET}" "${OUTPUT_DIRECTORY}/latex/refman.pdf" "${INSTALL_DESTINATION}" "${INSTALL_PDF_NAME}" "${INSTALL_COMPONENT}")
				endif()

			else()
				_dt_install_dir("${DOC_TARGET}" "${OUTPUT_DIRECTORY}/html" "${INSTALL_DESTINATION}")
				if(MAKE_PDF)
					_dt_install_file("${DOC_TARGET}" "${OUTPUT_DIRECTORY}/latex/refman.pdf" "${INSTALL_DESTINATION}" "${INSTALL_PDF_NAME}")
				endif()
			endif()
		endif()

	endif()
endfunction()
