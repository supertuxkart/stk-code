# CMake cross-platform build system
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu>
# http://academic.cleardefinition.com/
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)


###
# Build the project
###

include_directories("${CUTIL_ROOT_DIR}/inc")
set(SOURCES
	"${CUTIL_ROOT_DIR}/src/bank_checker.cpp"
	"${CUTIL_ROOT_DIR}/src/cmd_arg_reader.cpp"
	"${CUTIL_ROOT_DIR}/src/cutil.cpp"
	"${CUTIL_ROOT_DIR}/src/stopwatch.cpp"
	"${CUTIL_ROOT_DIR}/src/multithreading.cpp")
	
if(WIN32)
	list(APPEND SOURCES
		"${CUDA_SDK_ROOT_DIR}/C/common/src/stopwatch_win.cpp")
	add_definitions(-DWIN32 -D_WINDOWS -DBUILD_DLL)
	set(TYPE SHARED)
	
else()
	list(APPEND SOURCES
		"${CUTIL_ROOT_DIR}/src/stopwatch_linux.cpp")
	set(TYPE STATIC)
	
endif()

cuda_add_library(cutil ${TYPE} EXCLUDE_FROM_ALL ${SOURCES})

set(CUTIL_INCLUDE_DIRS "${CUTIL_ROOT_DIR}/inc" "${CUDA_INCLUDE_DIRS}" PARENT_SCOPE)
set(CUTIL_LIBRARIES cutil PARENT_SCOPE)

