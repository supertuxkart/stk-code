cmake_minimum_required(VERSION 3.6.0...4.0)
include_directories("${PROJECT_SOURCE_DIR}/lib/simd_wrapper")
if (UNIX OR MINGW)
    add_definitions(-O3)
endif()
add_library(graphics_utils STATIC
    mipmap/cpusimd.c
    mipmap/imgresize.c
)
