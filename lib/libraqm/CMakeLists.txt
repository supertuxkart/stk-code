cmake_minimum_required(VERSION 2.6)

if (NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

include(CheckFunctionExists)
include(CheckCSourceCompiles)
set(CMAKE_REQUIRED_INCLUDES ${HARFBUZZ_INCLUDEDIR})
set(CMAKE_REQUIRED_LIBRARIES ${HARFBUZZ_LIBRARY})

CHECK_FUNCTION_EXISTS(hb_ft_font_create_referenced HAVE_hb_ft_font_create_referenced)
if (HAVE_hb_ft_font_create_referenced)
    add_definitions(-DHAVE_HB_FT_FONT_CREATE_REFERENCED)
endif()

CHECK_FUNCTION_EXISTS(hb_ft_font_set_load_flags HAVE_hb_ft_font_set_load_flags)
if (HAVE_hb_ft_font_set_load_flags)
    add_definitions(-DHAVE_HB_FT_FONT_SET_LOAD_FLAGS)
endif()

CHECK_FUNCTION_EXISTS(hb_buffer_set_invisible_glyph HAVE_hb_buffer_set_invisible_glyph)
if (HAVE_hb_buffer_set_invisible_glyph)
    add_definitions(-DHAVE_HB_BUFFER_SET_INVISIBLE_GLYPH)
endif()

CHECK_C_SOURCE_COMPILES(
"
#include <harfbuzz/hb.h>
int main ()
{
#ifndef HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES
    (void)HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES;
#endif
    return 0;
}
"
HAVE_DECL_HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES)
if (HAVE_DECL_HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES)
    add_definitions(-DHAVE_DECL_HB_BUFFER_FLAG_REMOVE_DEFAULT_IGNORABLES)
endif()

add_library(raqm STATIC raqm.c)
