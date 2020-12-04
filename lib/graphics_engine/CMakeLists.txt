include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_engine/include")
add_library(graphics_engine STATIC
    src/gl.c
    src/vulkan.c
)
