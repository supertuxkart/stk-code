cmake_minimum_required(VERSION 2.6)
if (UNIX OR MINGW)
    add_definitions(-std=gnu++0x -O3)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(webm STATIC
    mkvmuxer/mkvmuxer.cc
    mkvmuxer/mkvmuxerutil.cc
    mkvmuxer/mkvwriter.cc
)
