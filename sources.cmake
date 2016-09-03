# Modify this file to change the last-modified date when you add/remove a file. 
# This will then trigger a new cmake run automatically. 
file(GLOB_RECURSE STK_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE STK_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
if(EXISTS "../data/shaders_optimized/" AND IS_DIRECTORY "../data/shaders_optimized/")
    file(GLOB_RECURSE STK_SHADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "data/shaders_optimized/*")
    MESSAGE("-- Using optimized GLSL shaders")
else()
    file(GLOB_RECURSE STK_SHADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "data/shaders/*")
    MESSAGE("-- NOT using optimized GLSL shaders. To optimize the shader run tools/optimize_shaders.sh")
endif()

file(GLOB_RECURSE STK_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/tmp/*.rc")
