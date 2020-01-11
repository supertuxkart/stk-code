cmake_minimum_required(VERSION 2.8.4)

# root CMakeLists for the SuperTuxKart project
project(SuperTuxKart)
set(PROJECT_VERSION "git")
add_definitions( -DSUPERTUXKART_VERSION="${PROJECT_VERSION}" )

if(NOT (CMAKE_MAJOR_VERSION VERSION_LESS 3))
  cmake_policy(SET CMP0043 OLD)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
include(CMakeDependentOption)

include(BuildTypeSTKRelease)
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to STKRelease")
    set(CMAKE_BUILD_TYPE "STKRelease")
endif()

option(SERVER_ONLY "Create a server only (i.e. no graphics or sound)" OFF)
option(CHECK_ASSETS "Check if assets are installed in ../stk-assets" ON)
option(USE_SYSTEM_ANGELSCRIPT "Use system angelscript instead of built-in angelscript. If you enable this option, make sure to use a compatible version." OFF)
option(USE_SYSTEM_ENET "Use system ENet instead of the built-in version, when available." ON)
option(USE_IPV6 "Allow create or connect to game server with IPv6 address, system enet will not be used." ON)
option(USE_SYSTEM_GLEW "Use system GLEW instead of the built-in version, when available." ON)
option(USE_SYSTEM_WIIUSE "Use system WiiUse instead of the built-in version, when available." OFF)
option(USE_SQLITE3 "Use sqlite to manage server stats and ban list." ON)

option(USE_CRYPTO_OPENSSL "Use OpenSSL instead of Nettle for cryptography in STK." ON)
CMAKE_DEPENDENT_OPTION(BUILD_RECORDER "Build opengl recorder" ON
    "NOT SERVER_ONLY;NOT APPLE" OFF)
CMAKE_DEPENDENT_OPTION(USE_SYSTEM_SQUISH "Use system Squish library instead of the built-in version, when available." ON
    "NOT SERVER_ONLY" OFF)
CMAKE_DEPENDENT_OPTION(USE_WIIUSE "Support for wiimote input devices" ON
    "NOT SERVER_ONLY;NOT CYGWIN" OFF)

if(APPLE AND NOT IOS)
    list(APPEND CMAKE_PREFIX_PATH /usr/local/opt)
    include_directories(/usr/local/opt/fribidi/include/)
    include_directories(/usr/local/opt/openssl@1.1/include/)
    include_directories(/usr/local/opt/openssl@1.1/include/openssl/)
    include_directories(/usr/local/opt/freetype/include/freetype2/)
    include_directories(/usr/local/opt/harfbuzz/include/harfbuzz/)
endif()

if (IOS)
    ADD_DEFINITIONS(-DMOBILE_STK)
    ADD_DEFINITIONS(-DIOS_STK)
    set(CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
    option(USE_GLES2 "Use OpenGL ES2 renderer" ON)
    find_library(RESOLV_LIBRARY NAMES resolv libresolv)
endif()

if((UNIX AND NOT APPLE) AND NOT SERVER_ONLY)
    option(ENABLE_WAYLAND_DEVICE "Enable Wayland device for linux build" ON)
    
    if((${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") OR 
       (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64"))
        option(USE_GLES2 "Use OpenGL ES2 renderer" ON)
    else()
        option(USE_GLES2 "Use OpenGL ES2 renderer" OFF)
    endif()

endif()

if(UNIX OR MINGW)
    option(DEBUG_SYMBOLS "Compile with debug symbols" OFF)
endif()

if(MSVC AND (MSVC_VERSION LESS 1900))
    # Normally hide the option to build wiiuse on VS, since it depends
    # on the installation of the Windows DDK (Driver Developer Kit),
    # which also needs an absolute path :(
    option(WIIUSE_BUILD "Build wiiuse lib (only for developers)" OFF)
    mark_as_advanced(WIIUSE_BUILD)
else()
    set(WIIUSE_BUILD ON)
endif()

if(UNIX AND NOT APPLE)
    option(USE_ASAN   "Build with Leak/Address sanitizer" OFF)
    option(USE_LIBBFD "Use libbfd for crash reporting and leak check" OFF)
endif()

set(STK_SOURCE_DIR "src")
set(STK_DATA_DIR "${PROJECT_SOURCE_DIR}/data")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Define install paths
set(STK_INSTALL_BINARY_DIR "bin" CACHE
    STRING "Install executable to this directory, absolute or relative to CMAKE_INSTALL_PREFIX")
set(STK_INSTALL_DATA_DIR "share/supertuxkart" CACHE
    STRING "Install data folder to this directory, absolute or relative to CMAKE_INSTALL_PREFIX")

# Define dependencies path
if(MSVC)
    set(DEPENDENCIES "dependencies-vs")
elseif(MINGW)
    set(DEPENDENCIES "dependencies-mingw")
else()
    set(DEPENDENCIES "dependencies")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(DEPENDENCIES "${DEPENDENCIES}-64bit")
endif()

if(WIN32)
    ADD_DEFINITIONS(-DUNICODE)
    ADD_DEFINITIONS(-D_UNICODE)
    if(NOT IS_DIRECTORY "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}")
        set(DEPENDENCIES "dependencies")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(DEPENDENCIES "${DEPENDENCIES}-64bit")
        endif()
    endif()

    if(NOT IS_DIRECTORY "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}")
        message(FATAL_ERROR "Dependencies directory not found.")
    endif()
endif()

# These variables enable MSVC to find libraries located in "dependencies"
if(WIN32)
    set(ENV{PATH} "$ENV{PATH};${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/include")
    set(ENV{LIB} ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib)
    set(ENV{OPENALDIR} ${PROJECT_SOURCE_DIR}/${DEPENDENCIES})
    add_definitions(-D_IRR_STATIC_LIB_)
    add_definitions(-DNO_IRR_COMPILE_WITH_X11_)
    include_directories(${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/include)
endif()

if(USE_GLES2)
    add_definitions(-DUSE_GLES2)
    add_definitions(-D_IRR_COMPILE_WITH_OGLES2_ -DNO_IRR_COMPILE_WITH_OPENGL_)
endif()

if(SERVER_ONLY)
    add_definitions(-DSERVER_ONLY)
    add_definitions(-DNO_IRR_COMPILE_WITH_X11_ -DNO_IRR_COMPILE_WITH_OPENGL_)
endif()

if(UNIX OR MINGW)
    if(DEBUG_SYMBOLS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    endif()
endif()

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)

    if(NOT PKGCONFIG_FOUND)
        message(FATAL_ERROR "Pkg-config not found.")
    endif()
endif()

# Build the Bullet physics library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/bullet")
include_directories("${PROJECT_SOURCE_DIR}/lib/bullet/src")

# Find system ENet library or build it if missing
if((UNIX AND NOT APPLE) AND USE_SYSTEM_ENET AND NOT USE_IPV6)
    pkg_check_modules(ENET libenet>=1.3.4)
endif()

if (USE_IPV6)
   add_definitions(-DENABLE_IPV6)
endif()

if(ENET_FOUND AND NOT USE_IPV6)
    include_directories(${ENET_INCLUDE_DIRS})
else()
    # Fallback to built-in version
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/enet")
    include_directories("${PROJECT_SOURCE_DIR}/lib/enet/include")
    set(ENET_LIBRARIES "enet")
endif()

# Find system GLEW library or build it if missing
if (APPLE AND NOT IOS)
    find_library(GLEW_LIBRARY NAMES glew libglew libGLEW)
    find_path(GLEW_INCLUDEDIR NAMES GL/glew.h glew.h PATHS)
    if (NOT GLEW_LIBRARY OR NOT GLEW_INCLUDEDIR)
        add_subdirectory("${PROJECT_SOURCE_DIR}/lib/glew")
        include_directories("${PROJECT_SOURCE_DIR}/lib/glew/include")
        set(GLEW_LIBRARIES "glew")
    else()
        set(GLEW_LIBRARIES ${GLEW_LIBRARY})
        include_directories(${GLEW_INCLUDEDIR})
        MESSAGE(STATUS "Use system GLEW: ${GLEW_LIBRARIES}")
    endif()
elseif(NOT USE_GLES2 AND NOT SERVER_ONLY)
    add_definitions(-DGLEW_NO_GLU)
    if((UNIX AND NOT APPLE) AND USE_SYSTEM_GLEW)
        pkg_check_modules(GLEW glew>=2.1)
    endif()

    if(GLEW_FOUND)
        include_directories(${GLEW_INCLUDE_DIRS})
    else()
        # Fallback to built-in version silently
        add_subdirectory("${PROJECT_SOURCE_DIR}/lib/glew")
        include_directories("${PROJECT_SOURCE_DIR}/lib/glew/include")
        set(GLEW_LIBRARIES "glew")
    endif()
endif()

if (NOT SERVER_ONLY)
    # Add jpeg library
    find_package(JPEG REQUIRED)
    include_directories(${JPEG_INCLUDE_DIR})
endif()

if (BUILD_RECORDER)
    find_library(OPENGLRECORDER_LIBRARY NAMES openglrecorder libopenglrecorder PATHS "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib")
    find_path(OPENGLRECORDER_INCLUDEDIR NAMES openglrecorder.h PATHS "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/include")
    if (NOT OPENGLRECORDER_LIBRARY OR NOT OPENGLRECORDER_INCLUDEDIR)
        if(PROJECT_VERSION STREQUAL "git")
            message(WARNING "libopenglrecorder not found, disabling in-game recorder. "
                "To use recorder, install libopenglrecorder.")
            set(OPENGLRECORDER_LIBRARY CACHE INTERNAL "")
            set(OPENGLRECORDER_INCLUDEDIR CACHE INTERNAL "")
            set(BUILD_RECORDER OFF)
        else()
            message(FATAL_ERROR "libopenglrecorder not found. "
            "Either install libopenglrecorder or disable in-game recorder with -DBUILD_RECORDER=0")
        endif()
    else()
        include_directories(${OPENGLRECORDER_INCLUDEDIR})
        mark_as_advanced(OPENGLRECORDER_LIBRARY OPENGLRECORDER_INCLUDEDIR)
        add_definitions(-DENABLE_RECORDER)
    endif()
endif()

if(NOT SERVER_ONLY)
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/graphics_utils")
    include_directories("${PROJECT_SOURCE_DIR}/lib/graphics_utils")
endif()

# Libmcpp
find_library(MCPP_LIBRARY NAMES mcpp libmcpp)
find_path(MCPP_INCLUDEDIR NAMES mcpp_lib.h PATHS)
if (NOT MCPP_LIBRARY OR NOT MCPP_INCLUDEDIR)
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/mcpp")
    include_directories("${PROJECT_SOURCE_DIR}/lib/mcpp")
    SET(MCPP_LIBRARY mcpp)
    message(STATUS "System libmcpp not found, use the bundled one.")
else()
    include_directories("${MCPP_INCLUDEDIR}")
    MESSAGE(STATUS "Use system libmcpp: ${MCPP_LIBRARY}")
endif()

if(NOT SERVER_ONLY)
    if(USE_SYSTEM_SQUISH)
        find_library(SQUISH_LIBRARY NAMES squish libsquish)
        find_path(SQUISH_INCLUDEDIR NAMES squish.h PATHS)
    endif()
    if (NOT SQUISH_LIBRARY OR NOT SQUISH_INCLUDEDIR)
        add_subdirectory("${PROJECT_SOURCE_DIR}/lib/libsquish")
        include_directories("${PROJECT_SOURCE_DIR}/lib/libsquish")
        SET(SQUISH_LIBRARY squish)
    else()
        include_directories("${SQUISH_INCLUDEDIR}")
        MESSAGE(STATUS "Use system libsquish: ${SQUISH_LIBRARY}")
    endif()
endif()

# Build the irrlicht library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/irrlicht")
include_directories("${PROJECT_SOURCE_DIR}/lib/irrlicht/include")

# Build the Wiiuse library
# Note: wiiuse MUST be declared after irrlicht, since otherwise
# (at least on VS) irrlicht will find wiiuse io.h file because
# of the added include directory.
if(USE_WIIUSE)
    # Find system WiiUse library or build it if missing
    if((UNIX AND NOT APPLE) AND USE_SYSTEM_WIIUSE)
        find_package(WiiUse)
    endif()

    if(WIIUSE_FOUND)
        include_directories(${WIIUSE_INCLUDE_DIR})
    else()
        # Fallback to built-in version
        if(WIIUSE_BUILD)
            add_subdirectory("${PROJECT_SOURCE_DIR}/lib/wiiuse")
        endif()
        include_directories("${PROJECT_SOURCE_DIR}/lib/wiiuse/src")
        set(WIIUSE_LIBRARIES "wiiuse" "bluetooth")
    endif()
endif()

# Set include paths
include_directories(${STK_SOURCE_DIR})

if(APPLE AND NOT IOS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -F/Library/Frameworks")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")   # Enable multi-processor compilation (faster)
endif()


# Build the angelscript library if not in system
if(USE_SYSTEM_ANGELSCRIPT)
    find_package(Angelscript)
    if(ANGELSCRIPT_FOUND)
        include_directories(${Angelscript_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "Angelscript not found. "
            "Either install angelscript or use built-in version using "
            "-DUSE_SYSTEM_ANGELSCRIPT=0")
    endif()
else()
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/angelscript/projects/cmake")
    include_directories("${PROJECT_SOURCE_DIR}/lib/angelscript/include")
    set(Angelscript_LIBRARIES angelscript)
endif()

if(NOT SERVER_ONLY)
    # OpenAL
    find_package(OpenAL REQUIRED)
    include_directories(${OPENAL_INCLUDE_DIR})

    # OggVorbis
    find_package(OggVorbis REQUIRED)
    include_directories(${OGGVORBIS_INCLUDE_DIRS})

    add_definitions(-DENABLE_SOUND)
endif()

# Text handling in STK (We use freetype, harfbuzz, fribidi and libraqm for i18n text handling)
if (NOT SERVER_ONLY)
    # Freetype
    find_package(Freetype)
    if(FREETYPE_FOUND)
        include_directories(${FREETYPE_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "Freetype not found. "
            "Freetype is required to display characters in SuperTuxKart.")
    endif()

    # Harfbuzz
    find_library(HARFBUZZ_LIBRARY NAMES harfbuzz libharfbuzz)
    find_path(HARFBUZZ_INCLUDEDIR NAMES harfbuzz/hb.h hb.h PATHS)
    if (NOT HARFBUZZ_LIBRARY OR NOT HARFBUZZ_INCLUDEDIR)
        message(FATAL_ERROR "Harfbuzz not found. "
            "Harfbuzz is required to display characters in SuperTuxKart.")
    else()
        include_directories("${HARFBUZZ_INCLUDEDIR}")
        MESSAGE(STATUS "Use system harfbuzz: ${HARFBUZZ_LIBRARY}")
    endif()

    # Fribidi
    find_package(Fribidi)
    if(FRIBIDI_FOUND)
        include_directories(${FRIBIDI_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "Fribidi not found. "
            "Fribidi is required to display characters in SuperTuxKart.")
    endif()

    # Libraqm
    find_library(RAQM_LIBRARY NAMES raqm libraqm)
    find_path(RAQM_INCLUDEDIR NAMES raqm.h PATHS)
    if (NOT RAQM_LIBRARY OR NOT RAQM_INCLUDEDIR)
        add_subdirectory("${PROJECT_SOURCE_DIR}/lib/libraqm")
        include_directories("${PROJECT_SOURCE_DIR}/lib/libraqm")
        SET(RAQM_LIBRARY raqm)
        message(STATUS "System libraqm not found, use the bundled one.")
    else()
        include_directories("${RAQM_INCLUDEDIR}")
        MESSAGE(STATUS "Use system libraqm: ${RAQM_LIBRARY}")
    endif()

endif()

# OpenGL
if(NOT USE_GLES2 AND NOT SERVER_ONLY)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

if(USE_LIBBFD)
    find_package(Libbfd)
    if(LIBBFD_FOUND)
        add_definitions(-DENABLE_LIBBFD)
        include_directories(${LIBBFD_INCLUDE_DIRS})
    else()
        set(USE_LIBBFD OFF CACHE BOOL "Use libbfd for crash reporting and leak check" FORCE)
        message(WARNING "Libbfd not found, disable integrated stack trace.")
    endif()
endif()

# SQLITE
if(USE_SQLITE3)
    find_library(SQLITE3_LIBRARY NAMES sqlite3 libsqlite3)
    find_path(SQLITE3_INCLUDEDIR NAMES sqlite3.h PATHS)
    if (NOT SQLITE3_LIBRARY OR NOT SQLITE3_INCLUDEDIR)
#        add_subdirectory("${PROJECT_SOURCE_DIR}/lib/sqlite3")
#        include_directories("${PROJECT_SOURCE_DIR}/lib/sqlite3")
#        SET(SQLITE3_LIBRARY sqlite3)
        set(USE_SQLITE3 OFF CACHE BOOL "Use sqlite to manage server stats and ban list." FORCE)
        message(WARNING "Sqlite3 not found, disable advanced server management.")
    else()
        add_definitions(-DENABLE_SQLITE3)
        include_directories("${SQLITE3_INCLUDEDIR}")
        MESSAGE(STATUS "Use system libsqlite3: ${SQLITE3_LIBRARY}")
    endif()
endif()

# Set some compiler options
if(UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function")
endif()

if(MINGW AND CMAKE_BUILD_TYPE MATCHES Release)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,windows")
endif()

if(WIN32)
    # By default windows.h has macros defined for min and max that screw up everything
    add_definitions(-DNOMINMAX)

    # And shut up about unsafe stuff
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)

    # Avoid timespec structure redeclaration on Visual Studio 2015
    if (NOT (MSVC_VERSION LESS 1900))
        add_definitions(-DHAVE_STRUCT_TIMESPEC)
    endif()
endif()

if(MSVC)
    # VS will automatically add NDEBUG for release mode, but only _DEBUG in debug mode.
    # Since STK uses DEBUG, this is added for debug compilation only:
    set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
else()
    # All non VS generators used create only a single compile mode, so
    # compile flags can be simplye be added
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-DDEBUG)
    else()
        add_definitions(-DNDEBUG)
    endif()
endif()


if(WIN32)
    configure_file("${STK_SOURCE_DIR}/../tools/windows_installer/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")
endif()

# Provides list of source and header files (STK_SOURCES and STK_HEADERS)
include(sources.cmake)

# Generate source groups useful for MSVC project explorer
include(cmake/SourceGroupFunctions.cmake)
source_group_hierarchy(STK_SOURCES STK_HEADERS)


if(APPLE AND NOT IOS)
    # icon files to copy in the bundle
    set(OSX_ICON_FILES ${PROJECT_SOURCE_DIR}/data/supertuxkart.icns)
    set_source_files_properties(${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(STK_SOURCES ${STK_SOURCES} ${OSX_ICON_FILES})

    # build the executable and create an app bundle
    add_executable(supertuxkart MACOSX_BUNDLE ${STK_SOURCES})

    find_library(IOKIT_LIBRARY IOKit)
    find_library(COCOA_LIBRARY Cocoa)

    target_link_libraries(supertuxkart
         ${IOKIT_LIBRARY}
         ${COCOA_LIBRARY})

    # configure CMake to use a custom Info.plist
    set_target_properties(supertuxkart PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/data/SuperTuxKart-Info.plist)

    if(CMAKE_GENERATOR MATCHES "Xcode")
        add_custom_command(TARGET supertuxkart POST_BUILD
            COMMAND ln -f -s ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/\${CONFIGURATION}/supertuxkart.app/Contents/Resources)
    else()
        add_custom_command(TARGET supertuxkart POST_BUILD
            COMMAND ln -f -s ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/supertuxkart.app/Contents/Resources)
    endif()
else()
    if(MSVC)
        set(PTHREAD_NAMES pthreadVC2)
    elseif(MINGW)
        set(PTHREAD_NAMES "winpthread-1" "libwinpthread-1" "pthreadGC2")
    endif()
    find_library(PTHREAD_LIBRARY NAMES pthread ${PTHREAD_NAMES} PATHS ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib)
    mark_as_advanced(PTHREAD_LIBRARY)

    # Set data dir (absolute or relative to CMAKE_INSTALL_PREFIX)
    if(NOT STK_INSTALL_DATA_DIR_ABSOLUTE)
        get_filename_component(STK_INSTALL_DATA_DIR_ABSOLUTE ${STK_INSTALL_DATA_DIR} ABSOLUTE)
        if(${STK_INSTALL_DATA_DIR_ABSOLUTE} STREQUAL ${STK_INSTALL_DATA_DIR})
            add_definitions(-DSUPERTUXKART_DATADIR=\"${STK_INSTALL_DATA_DIR_ABSOLUTE}\")
        else()
            add_definitions(-DSUPERTUXKART_DATADIR=\"${CMAKE_INSTALL_PREFIX}/${STK_INSTALL_DATA_DIR}\")
        endif()
    else()
        add_definitions(-DSUPERTUXKART_DATADIR=\"${STK_INSTALL_DATA_DIR_ABSOLUTE}\")
    endif()

    # Build the final executable
    if (IOS)
        file(GLOB IOS_ASSETS_FILES ${IOS_ASSETS} ${IOS_LAUNCHSCREEN})
        set_source_files_properties(${IOS_ASSETS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        add_executable(supertuxkart ${STK_SOURCES} ${STK_RESOURCES} ${STK_HEADERS} ${IOS_ASSETS_FILES})
        set_target_properties(supertuxkart PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/data/SuperTuxKart-Info-iOS.plist
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
            XCODE_ATTRIBUTE_INSTALL_PATH /Applications
            XCODE_ATTRIBUTE_SKIP_INSTALL No)
    else()
        add_executable(supertuxkart ${STK_SOURCES} ${STK_RESOURCES} ${STK_HEADERS})
    endif()
    target_link_libraries(supertuxkart ${PTHREAD_LIBRARY})
endif()

# check if linking against libatomic is required
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#include <atomic>
#include <cstdint>
int main() {
  std::atomic<uint64_t> x{0};
  x.load();
  return 0;
}
" NO_LIBATOMIC_NEEDED)
if (NOT NO_LIBATOMIC_NEEDED)
    target_link_libraries(supertuxkart atomic)
endif()

# CURL and OpenSSL or Nettle
# 1.0.1d for compatible AES GCM handling
SET(OPENSSL_MINIMUM_VERSION "1.0.1d")
if(MSVC)
    set(USE_CRYPTO_OPENSSL ON)
    target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib/libcurl.lib)
    target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib/libeay32.lib)
elseif(MINGW)
    set(USE_CRYPTO_OPENSSL ON)
    target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib/libcurldll.a)
    target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib/libeay32.dll)
else()
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIRS})

    find_path(NETTLE_INCLUDE_DIRS nettle/version.h)
    find_library(NETTLE_LIBRARY NAMES nettle libnettle)

    if (NOT NETTLE_INCLUDE_DIRS OR NOT NETTLE_LIBRARY OR USE_CRYPTO_OPENSSL)
        set(USE_CRYPTO_OPENSSL ON)
        find_package(OpenSSL REQUIRED)

        if(${OPENSSL_VERSION} VERSION_LESS ${OPENSSL_MINIMUM_VERSION} OR
            (${OPENSSL_VERSION} VERSION_EQUAL ${OPENSSL_MINIMUM_VERSION} AND ${OPENSSL_VERSION} STRLESS ${OPENSSL_MINIMUM_VERSION}))
            message(FATAL_ERROR "OpenSSL version found (${OPENSSL_VERSION}) is less then the minimum required (${OPENSSL_MINIMUM_VERSION}), aborting.")
        endif()
        include_directories(${OpenSSL_INCLUDE_DIRS})
    else()
        set(USE_CRYPTO_OPENSSL OFF)
        include_directories(${NETTLE_INCLUDE_DIRS})
    endif()
endif()

if (USE_CRYPTO_OPENSSL)
    message(STATUS "OpenSSL will be used for cryptography in STK.")
    add_definitions(-DENABLE_CRYPTO_OPENSSL)
else()
    message(STATUS "Nettle will be used for cryptography in STK.")
    add_definitions(-DENABLE_CRYPTO_NETTLE)
endif()

# Common library dependencies
target_link_libraries(supertuxkart
    bulletdynamics
    bulletcollision
    bulletmath
    ${ENET_LIBRARIES}
    stkirrlicht
    ${Angelscript_LIBRARIES}
    ${CURL_LIBRARIES}
    mcpp
    )

if (USE_SQLITE3)
    target_link_libraries(supertuxkart ${SQLITE3_LIBRARY})
endif()

if (USE_CRYPTO_OPENSSL)
    target_link_libraries(supertuxkart ${OPENSSL_CRYPTO_LIBRARY})
else()
    target_link_libraries(supertuxkart ${NETTLE_LIBRARY})
endif()

if(NOT SERVER_ONLY)
    if(NOT USE_GLES2)
        target_link_libraries(supertuxkart ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES})
    elseif (IOS)
        target_link_libraries(supertuxkart "-framework OpenGLES
                                            -framework UIKit
                                            -framework CoreMotion
                                            -framework Foundation
                                            -framework GLKit ${RESOLV_LIBRARY}")
    else()
        target_link_libraries(supertuxkart GLESv2)
    endif()

    target_link_libraries(supertuxkart
        ${SQUISH_LIBRARY}
        ${JPEG_LIBRARIES}
        ${OGGVORBIS_LIBRARIES}
        ${OPENAL_LIBRARY}
        ${RAQM_LIBRARY}
        ${FRIBIDI_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${HARFBUZZ_LIBRARY}
        graphics_utils)
endif()

if(UNIX AND NOT APPLE)
    if(USE_LIBBFD)
        target_link_libraries(supertuxkart ${LIBBFD_LIBRARIES})
        if (CMAKE_SYSTEM_NAME MATCHES "BSD")
            target_link_libraries(supertuxkart  execinfo)
        endif()
    endif()
    if(USE_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        target_link_libraries(supertuxkart "-fsanitize=address")
    endif()
endif()

if(BUILD_RECORDER)
    target_link_libraries(supertuxkart ${OPENGLRECORDER_LIBRARY})
endif()

# FreeBSD does not search in /usr/local/lib, but at least Freetype is installed there :(
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib")
endif()

# Wiiuse
# ------
if(USE_WIIUSE)
    if(APPLE)
        find_library(BLUETOOTH_LIBRARY NAMES IOBluetooth PATHS /Developer/Library/Frameworks/IOBluetooth.framework)
        target_link_libraries(supertuxkart wiiuse ${BLUETOOTH_LIBRARY})
    elseif(WIN32)
        add_definitions(-DWIIUSE_STATIC)
        if(WIIUSE_BUILD)
            target_link_libraries(supertuxkart wiiuse)
        else()
            target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib/wiiuse.lib)
        endif()
    else()
        target_link_libraries(supertuxkart ${WIIUSE_LIBRARIES})
    endif()
    add_definitions(-DENABLE_WIIUSE)

endif()

if(MSVC OR MINGW)
  target_link_libraries(supertuxkart iphlpapi.lib)
  add_custom_command(TARGET supertuxkart POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/dll"
        $<TARGET_FILE_DIR:supertuxkart>)
  add_custom_target(stkshaders SOURCES ${STK_SHADERS})
endif()

if(MINGW)
    target_link_libraries(supertuxkart -ldxguid -ldinput8)
    if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries(supertuxkart -mwindows)
    endif()
    find_library(LIBGCC NAMES "libgcc_s_dw2-1.dll" "libgcc_s_sjlj-1.dll" "libgcc_s_seh-1.dll" PATHS ${CMAKE_FIND_ROOT_PATH})
    if(LIBGCC)
        file(COPY ${LIBGCC} DESTINATION ${CMAKE_BINARY_DIR}/bin/)
    endif()
    find_library(LIBSTDCPP NAMES "libstdc++-6.dll" PATHS ${CMAKE_FIND_ROOT_PATH})
    if(LIBSTDCPP)
        file(COPY ${LIBSTDCPP} DESTINATION ${CMAKE_BINARY_DIR}/bin/)
    endif()
    find_library(LIBPTHREAD NAMES "winpthread-1.dll" "libwinpthread-1.dll" "pthreadGC2.dll" PATHS ${CMAKE_FIND_ROOT_PATH})
    if(LIBPTHREAD)
        file(COPY ${LIBPTHREAD} DESTINATION ${CMAKE_BINARY_DIR}/bin/)
    endif()
endif()


# ==== Checking if data folder exists ====
if(NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data)
  message( FATAL_ERROR "${CMAKE_CURRENT_SOURCE_DIR}/data folder doesn't exist" )
endif()

# ==== Checking if stk-assets folder exists ====
if(CHECK_ASSETS)
  if((IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/karts) AND
       (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/library) AND
       (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/music) AND
       (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/sfx) AND
       (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/textures) AND
       (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/tracks))
    message(STATUS "Assets found in data directory")
  elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../stk-assets)
    set(STK_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../stk-assets/)
    message(STATUS "Assets found")
  elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../supertuxkart-assets)
    set(STK_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../supertuxkart-assets/)
    message(STATUS "Assets found")
  else()
    set (CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(PARENT_DIR ${CUR_DIR} PATH)
    message( FATAL_ERROR "${PARENT_DIR}/stk-assets folder doesn't exist. "
      "Please download the stk-assets, or disable this test with -DCHECK_ASSETS=off." )
  endif()
endif()


# ==== Install target ====
install(TARGETS supertuxkart RUNTIME DESTINATION ${STK_INSTALL_BINARY_DIR} BUNDLE DESTINATION .)
install(DIRECTORY ${STK_DATA_DIR} DESTINATION ${STK_INSTALL_DATA_DIR} PATTERN ".svn" EXCLUDE PATTERN ".git" EXCLUDE)
if(STK_ASSETS_DIR AND CHECK_ASSETS)
  install(DIRECTORY ${STK_ASSETS_DIR} DESTINATION ${STK_INSTALL_DATA_DIR}/data PATTERN ".svn" EXCLUDE PATTERN ".git" EXCLUDE)
endif()
install(FILES ${STK_DATA_DIR}/supertuxkart.desktop DESTINATION share/applications)
install(FILES data/supertuxkart_48.png DESTINATION share/icons/hicolor/48x48/apps RENAME supertuxkart.png)
install(FILES data/supertuxkart_128.png DESTINATION share/icons/hicolor/128x128/apps RENAME supertuxkart.png)
install(FILES data/supertuxkart_48.png DESTINATION share/pixmaps RENAME supertuxkart.png)
install(FILES data/supertuxkart.appdata.xml DESTINATION share/metainfo)

if(MINGW)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/ DESTINATION ${STK_INSTALL_BINARY_DIR}
            FILES_MATCHING PATTERN "*.dll")
endif()
