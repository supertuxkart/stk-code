include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_engine/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/irrlicht/include")
add_library(graphics_engine STATIC
    src/gl.c
    src/vulkan.c
    src/ge_main.cpp
    src/ge_texture.cpp
)
