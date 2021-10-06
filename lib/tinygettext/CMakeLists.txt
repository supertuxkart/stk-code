# tinygettext - A gettext replacement that works directly on .po files
# Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgement in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

#
# INSTRUCTIONS:
# -------------
#
# Create a directory build/ and change to it. Run
#
#   cmake ..
#
# This creates a set of Makefiles to build the project. Run
#
#   make
#

cmake_policy(SET CMP0005 NEW)

## Project name to use as command prefix

project(tinygettext)
set(VERSION "0.1.0")

### CMake configuration

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

## Reveal library type choice to users
option(BUILD_SHARED_LIBS "Produce dynamic library instead of static archive" OFF)

# STK addition: disable libiconv
option(USE_ICONV "Use libiconv" OFF)
if (USE_ICONV)
## Add iconv to include directories

find_package(ICONV REQUIRED)
include_directories(${ICONV_INCLUDE_DIR})

## Check iconv_const

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${ICONV_INCLUDE_DIR})
check_cxx_source_compiles(
	"
	#include <iconv.h>
	// this declaration will fail when there already exists a non const char** version which returns size_t
	double iconv(iconv_t cd,  char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
	int main() { return 0; }
	"
	HAVE_ICONV_CONST
)

# TODO: better way of config

if(HAVE_ICONV_CONST)
  add_definitions(-DHAVE_ICONV_CONST)
else(HAVE_ICONV_CONST)
  remove_definitions(-DHAVE_ICONV_CONST)
endif(HAVE_ICONV_CONST)
else()
add_definitions(-DDISABLE_ICONV)
endif()

if (UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

## TinyGetText library compilation

## build list of source files

file(GLOB TINYGETTEXT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
file(GLOB TINYGETTEXT_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/tinygettext/*.hpp)

## define a target for building the library

add_library(tinygettext STATIC ${TINYGETTEXT_SOURCES})

## Add tinygettext dir to search path

include_directories(include/)

add_definitions(-DVERSION=${VERSION})

# EOF #
