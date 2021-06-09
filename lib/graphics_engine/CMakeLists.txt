include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_engine/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/irrlicht/include")
find_path(SDL2_INCLUDEDIR NAMES SDL.h PATH_SUFFIXES SDL2 include/SDL2 include PATHS)
if (NOT SDL2_INCLUDEDIR)
    message(FATAL_ERROR "SDL2 not found.")
else()
    include_directories("${SDL2_INCLUDEDIR}")
endif()

if(UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()
add_library(graphics_engine STATIC
    src/gl.c
    src/vulkan.c
    src/ge_main.cpp
    src/ge_texture.cpp
    src/ge_dx9_texture.cpp
    src/ge_vulkan_driver.cpp
    src/ge_gl_texture.cpp
)
