# Browser build entry point. The normal desktop configuration remains separate.
# Emscripten ports supply these libraries, including their transitive dependencies.
set(WEB_PORT_FLAGS -sUSE_SDL=2 -sUSE_ZLIB=1 -sUSE_LIBPNG=1
    -sUSE_LIBJPEG=1 -sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1
    -sUSE_OGG=1 -sUSE_VORBIS=1)
add_compile_options(${WEB_PORT_FLAGS} -pthread -fwasm-exceptions)
add_link_options(${WEB_PORT_FLAGS} -pthread -fwasm-exceptions)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web")
set(WEB_PREFIX "${PROJECT_SOURCE_DIR}/build-web-tools/prefix" CACHE PATH
    "WebAssembly-only curl and OpenSSL installation")
set(WEB_ASSETS "${PROJECT_SOURCE_DIR}/build-web-tools/data" CACHE PATH
    "Merged game data produced by web/prepare_assets.py")
if(NOT EXISTS "${WEB_ASSETS}/supertuxkart.git")
    message(FATAL_ERROR "Prepare browser assets with web/prepare_assets.py first")
endif()
if(NOT EXISTS "${WEB_PREFIX}/lib/libcurl.a")
    message(FATAL_ERROR "Build WebAssembly dependencies with web/build.py first")
endif()

# Ports install their headers in this SDK's sysroot. No Homebrew libraries here.
execute_process(COMMAND em-config CACHE OUTPUT_VARIABLE WEB_CACHE
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
set(WEB_INCLUDE "${WEB_CACHE}/sysroot/include")
include_directories(src lib/irrlicht/include lib/bullet/src
    lib/graphics_engine/include lib/graphics_utils lib/mcpp
    lib/angelscript/include lib/enet/include lib/sheenbidi/Headers
    lib/tinygettext/include "${WEB_INCLUDE}/SDL2" "${WEB_INCLUDE}/freetype2"
    "${WEB_INCLUDE}/harfbuzz" "${WEB_PREFIX}/include")

# Use the existing legacy GLES renderer. Vulkan and desktop GL are unavailable.
set(USE_GLES2 ON)
set(SERVER_ONLY OFF)
set(SQUISH_LIBRARY squish)
add_compile_definitions(USE_GLES2 _IRR_COMPILE_WITH_OGLES2_
    NO_IRR_COMPILE_WITH_X11_ NO_IRR_COMPILE_WITH_WAYLAND_DEVICE_
    NO_IRR_COMPILE_WITH_DIRECT3D_9_ NO_IRR_COMPILE_WITH_VULKAN_
    NO_IRR_COMPILE_WITH_OPENGL_ DISABLE_SHADERC DISABLE_RPC
    ENABLE_SOUND ENABLE_CRYPTO_OPENSSL DNS_C
    SUPERTUXKART_DATADIR="/data"
    $<$<CONFIG:Debug>:DEBUG> $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

# Irrlicht's Find modules require these variables even though Emscripten's
# port flags already arrange the actual library linking.
set(ZLIB_INCLUDE_DIR "${WEB_INCLUDE}")
set(ZLIB_LIBRARY z)
set(ZLIB_VERSION_STRING 1.3.1)
set(PNG_PNG_INCLUDE_DIR "${WEB_INCLUDE}")
set(PNG_LIBRARY png)
set(JPEG_INCLUDE_DIR "${WEB_INCLUDE}")
set(JPEG_LIBRARY jpeg)

add_subdirectory(lib/bullet)
add_subdirectory(lib/enet)
add_subdirectory(lib/dnsc)
add_subdirectory(lib/libsquish)
add_subdirectory(lib/graphics_utils)
add_subdirectory(lib/graphics_engine)
add_subdirectory(lib/irrlicht)
add_subdirectory(lib/angelscript/projects/cmake)
add_subdirectory(lib/mcpp)
add_subdirectory(lib/sheenbidi)
add_subdirectory(lib/tinygettext)
include(sources.cmake)
add_executable(supertuxkart ${STK_SOURCES})
set_target_properties(supertuxkart PROPERTIES SUFFIX ".js")
target_link_libraries(supertuxkart bulletdynamics bulletcollision bulletmath
    enet dnsc stkirrlicht angelscript mcpp sheenbidi tinygettext
    graphics_utils graphics_engine squish
    "${WEB_PREFIX}/lib/libcurl.a" "${WEB_PREFIX}/lib/libcrypto.a"
    openal)

# A worker pool lets existing background tasks start without waiting for the
# browser main thread. Shared memory requires the headers in web/serve.py.
# FULL_ES2 emulates client-side vertex arrays used by Irrlicht's legacy driver.
target_link_options(supertuxkart PRIVATE
    -sENVIRONMENT=web,worker -sPTHREAD_POOL_SIZE=8
    -sINITIAL_MEMORY=536870912 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=2147483648
    -sSTACK_SIZE=8388608 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
    -sFULL_ES2=1 -sFULL_ES3=1 -sEXIT_RUNTIME=1
    -sEXPORTED_RUNTIME_METHODS=callMain,FS -sINVOKE_RUN=0
    -lidbfs.js "--pre-js=${PROJECT_SOURCE_DIR}/web/pre.js"
    "--preload-file=${WEB_ASSETS}@/data")
configure_file(web/index.html "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.html" COPYONLY)
