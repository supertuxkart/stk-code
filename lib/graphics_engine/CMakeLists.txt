# Distro package just use libastcenc.so, find it first
find_library(LIBASTCENC_LIBRARY NAMES libastcenc astcenc astcenc-avx2-static astcenc-sse4.1-static astcenc-sse2-static astcenc-neon-static astcenc-native-static)
find_path(LIBASTCENC_INCLUDEDIR NAMES astcenc.h PATHS)
if (LIBASTCENC_LIBRARY AND LIBASTCENC_INCLUDEDIR)
    set(ENABLE_LIBASTCENC 1)
    add_definitions(-DENABLE_LIBASTCENC)
    include_directories("${LIBASTCENC_INCLUDEDIR}")
    message(STATUS "Use libastcenc: ${LIBASTCENC_LIBRARY}")

    if (NOT MSVC)
        set(CMAKE_REQUIRED_FLAGS "-std=c++11")
    endif()
    set(CMAKE_REQUIRED_INCLUDES ${LIBASTCENC_INCLUDEDIR})
    set(CMAKE_REQUIRED_LIBRARIES ${LIBASTCENC_LIBRARY})
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("
    #define ASTCENC_DYNAMIC_LIBRARY 1
    #include <astcenc.h>
    int main()
    {
        astcenc_context_free(NULL);
        return 0;
    }
    " ASTCENC_DLL)
    if (NOT MSVC)
        unset(CMAKE_REQUIRED_FLAGS)
    endif()
    unset(CMAKE_REQUIRED_INCLUDES)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if (ASTCENC_DLL AND NOT IOS)
        message(STATUS "libastcenc: -DASTCENC_DYNAMIC_LIBRARY required")
        add_definitions(-DASTCENC_DYNAMIC_LIBRARY)
    endif()

else()
    message(WARNING "Missing astc-encoder for astc support, "
        "visit https://github.com/ARM-software/astc-encoder for details")
endif()

include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_engine/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_utils")
include_directories("${PROJECT_SOURCE_DIR}/lib/irrlicht/include")
include_directories("${PROJECT_SOURCE_DIR}/lib/bullet/src")
find_path(SDL2_INCLUDEDIR NAMES SDL.h PATH_SUFFIXES SDL2 include/SDL2 include PATHS)
if (NOT SDL2_INCLUDEDIR)
    message(FATAL_ERROR "SDL2 not found.")
else()
    include_directories("${SDL2_INCLUDEDIR}")
endif()

if(APPLE AND NOT DLOPEN_MOLTENVK)
    find_path(VULKAN_INCLUDEDIR NAMES vulkan/vulkan.h PATHS)
    if (NOT VULKAN_INCLUDEDIR)
        message(FATAL_ERROR "Vulkan not found.")
    else()
        include_directories("${VULKAN_INCLUDEDIR}")
    endif()
endif()

if(UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

set(GE_SOURCES
    src/gl.c
    src/ge_compressor_astc_4x4.cpp
    src/ge_compressor_bptc_bc7.cpp
    src/ge_compressor_s3tc_bc3.cpp
    src/ge_culling_tool.cpp
    src/ge_dx9_texture.cpp
    src/ge_main.cpp
    src/ge_material_manager.cpp
    src/ge_occlusion_culling.cpp
    src/ge_texture.cpp
    src/ge_vma.cpp
    src/ge_vulkan_2d_renderer.cpp
    src/ge_vulkan_array_texture.cpp
    src/ge_vulkan_animated_mesh_scene_node.cpp
    src/ge_vulkan_attachment_texture.cpp
    src/ge_vulkan_billboard_buffer.cpp
    src/ge_vulkan_camera_scene_node.cpp
    src/ge_vulkan_command_loader.cpp
    src/ge_vulkan_deferred_fbo.cpp
    src/ge_vulkan_driver.cpp
    src/ge_vulkan_draw_call.cpp
    src/ge_vulkan_dynamic_buffer.cpp
    src/ge_vulkan_dynamic_spm_buffer.cpp
    src/ge_vulkan_environment_map.cpp
    src/ge_vulkan_fbo_texture.cpp
    src/ge_vulkan_features.cpp
    src/ge_vulkan_hiz_depth.cpp
    src/ge_vulkan_light_handler.cpp
    src/ge_vulkan_mesh_cache.cpp
    src/ge_vulkan_mesh_scene_node.cpp
    src/ge_vulkan_scene_manager.cpp
    src/ge_vulkan_shader_manager.cpp
    src/ge_vulkan_skybox_renderer.cpp
    src/ge_vulkan_texture.cpp
    src/ge_vulkan_texture_descriptor.cpp
    src/ge_gl_texture.cpp
    src/ge_spm.cpp
    src/ge_spm_buffer.cpp
)

if(NOT APPLE OR DLOPEN_MOLTENVK)
    set(GE_SOURCES ${GE_SOURCES} src/vulkan.c)
endif()

option(BC7_ISPC "Enable BC7 (BPTC) support (requires ispc and cmake >= 3.19)" OFF)
if(BC7_ISPC AND NOT ISPC_ARCH MATCHES "unknown")
    add_definitions(-DBC7_ISPC)
    enable_language(ISPC)
    set(CMAKE_ISPC_FLAGS "${CMAKE_ISPC_FLAGS} --opt=fast-math --opt=disable-assertions --pic")
    if(ISPC_ARCH)
        set(CMAKE_ISPC_FLAGS "${CMAKE_ISPC_FLAGS} --arch=${ISPC_ARCH}")
    endif()
    if(WIN32)
        set(CMAKE_ISPC_FLAGS "${CMAKE_ISPC_FLAGS} --target-os=windows")
    elseif(IOS)
        set(CMAKE_ISPC_FLAGS "${CMAKE_ISPC_FLAGS} --target-os=ios")
    elseif(APPLE)
        set(CMAKE_ISPC_FLAGS "${CMAKE_ISPC_FLAGS} --target-os=macos")
    else()
        set(CMAKE_ISPC_FLAGS "${CMAKE_ISPC_FLAGS} --target-os=linux")
    endif()
    set(GE_SOURCES
        ${GE_SOURCES}
        src/bc7e.ispc
    )
    if (MINGW AND NOT ISPC_ARCH MATCHES "aarch64")
        enable_language(ASM)
        set(GE_SOURCES
            ${GE_SOURCES}
            src/libchkstk.S)
        if(LLVM_PREFIX)
            set_source_files_properties(src/libchkstk.S PROPERTIES COMPILE_FLAGS "-DCHKSTK")
            if(ISPC_ARCH MATCHES "x86" OR ISPC_ARCH MATCHES "x86-64")
                set_source_files_properties(src/ge_compressor_bptc_bc7.cpp PROPERTIES COMPILE_FLAGS "-DLLVM_MINGW_FLTUSED")
            endif()
        else()
            set_source_files_properties(src/libchkstk.S PROPERTIES COMPILE_FLAGS "-DCHKSTK -Wa,--no-pad-sections")
        endif()
        set_source_files_properties(src/libchkstk.S PROPERTIES LANGUAGE ASM)
    endif()
    include_directories("${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_library(graphics_engine STATIC ${GE_SOURCES})
if(BC7_ISPC)
    if(ISPC_ARCH MATCHES "aarch64")
        set_property(TARGET graphics_engine PROPERTY ISPC_INSTRUCTION_SETS neon)
    else()
        set_property(TARGET graphics_engine PROPERTY ISPC_INSTRUCTION_SETS avx2)
    endif()
endif()

target_link_libraries(graphics_engine ${SQUISH_LIBRARY})

if(ENABLE_LIBASTCENC)
    target_link_libraries(graphics_engine ${LIBASTCENC_LIBRARY})
endif()
