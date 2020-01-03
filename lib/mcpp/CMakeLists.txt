cmake_minimum_required(VERSION 2.6)
if (UNIX OR MINGW)
    add_definitions(-O3)
    if (APPLE)
        add_definitions(-D_DARWIN_C_SOURCE)
    endif()
endif()
add_definitions(-DMCPP_LIB)
add_definitions(-DHAVE_CONFIG_H)
add_library(mcpp STATIC
    directive.c
    eval.c
    expand.c
    main.c
    main_libmcpp.c
    mbchar.c
    support.c
    system.c
)
