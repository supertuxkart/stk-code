cmake_minimum_required(VERSION 2.6)
if (UNIX OR MINGW)
    add_definitions(-O3 -msse -msse2 -msse3 -mssse3 -msse4.1 -mavx -ffast-math)
endif()
add_library(graphics_utils STATIC
    mipmap/cpusimd.c
    mipmap/img.c
    mipmap/imgresize.c
)
