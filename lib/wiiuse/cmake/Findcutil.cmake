# - find NVIDIA CUDA and source for the cutil library, building cutil if needed.
#
#  CUTIL_LIBRARIES - Libraries to link against to use CUTIL
#  CUTIL_INCLUDE_DIRS - Include directories to add before building a CUTIL app.
#
# Functions:
#  install_cutil({RUNTIME_LIBRARY_DESTINATION}) - Install the CUTIL shared lib if created.
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


find_package(CUDA QUIET)


file(TO_CMAKE_PATH "${CUDA_SDK_ROOT_DIR}/C/common" CUTIL_ROOT_DIR)

if(NOT EXISTS "${CUTIL_ROOT_DIR}/src/cutil.cpp")
	set(CUDA_SDK_ROOT_DIR
		SDKDIR-NOTFOUND
		CACHE
		PATH
		"NVIDIA GPU Computing SDK dir"
		FORCE)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cutil
	DEFAULT_MSG
	CUDA_SDK_ROOT_DIR
	CUDA_FOUND)

if(CUTIL_FOUND)
	get_filename_component(_moddir "${CMAKE_CURRENT_LIST_FILE}" PATH)
	add_subdirectory("${_moddir}/nested_targets/cutil")


	function(install_cutil dest)
		install(TARGETS cutil
			RUNTIME DESTINATION "${dest}"
			LIBRARY DESTINATION "${dest}")
	endfunction()
else()
	function(install_cutil)
		message(FATAL_ERROR "Can't install cutil - didn't find it!")
	endfunction()
endif()
