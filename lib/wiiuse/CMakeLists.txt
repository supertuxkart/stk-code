# 2009-2011 Ryan Pavlik <rpavlik@iastate.edu>
# http://academic.cleardefinition.com/
# Iowa State University HCI Graduate Program/VRAC

cmake_minimum_required(VERSION 2.8.0)

# Added for STK, silences CMake warning
# See https://github.com/supertuxkart/stk-code/commit/b0ff15873ee7fa8901672672b47def9039a5534b#diff-a3e272598233e89e4b577a434cc2a89d
if(NOT (CMAKE_MAJOR_VERSION VERSION_LESS 3))
  cmake_policy(SET CMP0048 OLD)
endif()

# Set package properties
project(WiiUse)

###
# Set up options
###

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(SUBPROJECT YES)
endif()

include("./ParseVersion.cmake")
message(STATUS "Configuring WiiUse version ${CPACK_PACKAGE_VERSION}")

option(BUILD_EXAMPLE "Should we build the example app?" YES)
option(BUILD_EXAMPLE_SDL "Should we build the SDL-based example app?" YES)
# Modified for STK, see https://github.com/supertuxkart/stk-code/commit/aea2739e9de42f566ceb6ba0c6396a9f97fe90d4#diff-a3e272598233e89e4b577a434cc2a89d
option(BUILD_WIIUSE_SHARED_LIB "Should we build as a shared library (dll/so)?" NO)
option(INSTALL_EXAMPLES "Should we install the example apps?" YES)

option(CPACK_MONOLITHIC_INSTALL "Only produce a single component installer, rather than multi-component." NO)

###
# Perform build configuration of dependencies
###

# Locally-developed modules dist'ed with this app - always have this first.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseBackportedModules)
include(DoxygenTargets)

if(BUILD_WIIUSE_SHARED_LIB)
	set(WIIUSE_LIB_TYPE SHARED)
else()
	set(WIIUSE_LIB_TYPE STATIC)
	add_definitions(-DWIIUSE_STATIC)
endif()

if(NOT WIN32 AND NOT APPLE)
	set(LINUX YES)
	find_package(Bluez REQUIRED)
	include_directories(${BLUEZ_INCLUDE_DIRS})

        include("GNUInstallDirs")
else()
        set(LINUX NO)
        set(CMAKE_INSTALL_LIBDIR "lib")
endif()

if(WIN32)
	find_package(WinHID REQUIRED)
	include_directories(${WINHID_INCLUDE_DIRS})
	if(MSVC)
		# Minimum requirement is WinXP
		add_definitions(-D_WIN32_WINNT=0x0501)
	endif()
endif()

###
# Build the project
###

# The lib is in the "src" subdirectory
add_subdirectory(src)

if(NOT SUBPROJECT)
	# Example apps
	if(BUILD_EXAMPLE)
		add_subdirectory(example)
	endif()

	if(BUILD_EXAMPLE_SDL)
		add_subdirectory(example-sdl)
	endif()
endif()

if(SUBPROJECT)
	set(WIIUSE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)
	set(WIIUSE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)
	set(WIIUSE_LIBRARY wiiuse PARENT_SCOPE)
	set(WIIUSE_LIBRARIES wiiuse PARENT_SCOPE)
	set(WIIUSE_FOUND ON PARENT_SCOPE)
endif()

if(NOT SUBPROJECT)
###
# Set packaging options (for CPack)
###

	if(WIN32)
		set(DOC_DIR .)
	else()
		set(DOC_DIR share/doc/wiiuse)
	endif()

	# Documentation
	add_doxygen(Doxyfile
		INSTALL_DESTINATION ${DOC_DIR}
		INSTALL_COMPONENT docs
		NO_PDF)

	install(FILES
		CHANGELOG.mkd
		LICENSE
		README.mkd
		DESTINATION ${DOC_DIR} COMPONENT docs)

	if(INSTALL_EXAMPLES)
		install(FILES example/example.c
			DESTINATION ${DOC_DIR}/example COMPONENT examples)
		install(FILES example-sdl/sdl.c
			DESTINATION ${DOC_DIR}/example-sdl COMPONENT examples)
	endif()

	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.mkd")

	include(GetCompilerInfoString)
	get_compiler_info_string(_compiler)

	set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${_compiler}")

	# Include the packaging system now that we have it all set up
	include(CPack)

	cpack_add_component(docs HIDDEN)

	cpack_add_component(development
		DISPLAY_NAME "Development Files")

	cpack_add_component(examples
		DISPLAY_NAME "Examples")

	cpack_add_component(runtime
		DISPLAY_NAME "Runtime Library"
		REQUIRED)
endif()
