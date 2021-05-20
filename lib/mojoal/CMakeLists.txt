include_directories("${SDL2_INCLUDEDIR}")
include_directories("${PROJECT_SOURCE_DIR}/lib/mojoal")
add_library(mojoal STATIC
    mojoal.c
)
