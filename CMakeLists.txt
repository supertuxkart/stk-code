# root CMakeLists for the SuperTuxKart project
project(SuperTuxKart)
set(PROJECT_VERSION "0.7.3")

cmake_minimum_required(VERSION 2.8.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(BuildTypeSTKRelease)
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to STKRelease")
    set(CMAKE_BUILD_TYPE "STKRelease")
endif()

option(USE_WIIUSE "Support for wiimote input devices" OFF)
option(USE_FRIBIDI "Support for right-to-left languages" ON)

set(STK_SOURCE_DIR "src")
set(STK_DATA_DIR "${PROJECT_SOURCE_DIR}/data")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# Define install paths
set(STK_INSTALL_BINARY_DIR "bin" CACHE
    STRING "Install executable to this directory, absolute or relative to CMAKE_INSTALL_PREFIX")
set(STK_INSTALL_DATA_DIR "share/supertuxkart" CACHE
    STRING "Install data folder to this directory, absolute or relative to CMAKE_INSTALL_PREFIX")

# Build the Bullet physics library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/bullet")
include_directories("${PROJECT_SOURCE_DIR}/lib/bullet/src")

# Build the ENet UDP network library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/enet")
include_directories("${PROJECT_SOURCE_DIR}/lib/enet/include")

# Build the Wiiuse library
if(USE_WIIUSE)
    add_subdirectory("${PROJECT_SOURCE_DIR}/lib/wiiuse")
    include_directories("${PROJECT_SOURCE_DIR}/lib/wiiuse")
endif()

# Set include paths
include_directories(${STK_SOURCE_DIR})

# These variables enable MSVC to find libraries located in "dependencies"
if(MSVC)
    set(ENV{PATH} ${PROJECT_SOURCE_DIR}/dependencies/include)
    set(ENV{LIB} ${PROJECT_SOURCE_DIR}/dependencies/lib)
    set(ENV{OPENALDIR} ${PROJECT_SOURCE_DIR}/dependencies)
endif()

# OpenAL
find_package(OpenAL REQUIRED)
include_directories(${OPENAL_INCLUDE_DIR})

# OggVorbis
find_package(OggVorbis REQUIRED)
include_directories(${OGGVORBIS_INCLUDE_DIRS})

# CURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# Fribidi
if(USE_FRIBIDI)
    find_package(Fribidi)
    if(FRIBIDI_FOUND)
        include_directories(${FRIBIDI_INCLUDE_DIRS})
    else()
        message(FATAL_ERROR "Fribidi not found. "
            "Either install fribidi or disable bidi support with -DUSE_FRIBIDI=0 "
            "(if you don't use a right-to-left language then you don't need this).")
    endif()
endif()


# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

if(UNIX AND NOT APPLE)
    find_library(IRRLICHT_XF86VM_LIBRARY Xxf86vm)
    mark_as_advanced(IRRLICHT_XF86VM_LIBRARY)
else()
    set(IRRLICHT_XF86VM_LIBRARY "")
endif()


# Irrlicht
if (APPLE)
    set(IRRLICHT_LIBRARY "/Library/Frameworks/IrrFramework.framework")
else()
    set(IRRLICHT_LIBRARY ${PROJECT_SOURCE_DIR}/lib/irrlicht/lib/Linux/libIrrlicht.a)
endif()

    
# Set some compiler options
if(UNIX)
    add_definitions(-Wall)
endif()

if(WIN32)
    # By default windows.h has macros defined for min and max that screw up everything
    add_definitions(-DNOMINMAX)

    # And shut up about unsafe stuff
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# TODO: remove this switch
add_definitions(-DHAVE_OGGVORBIS)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()


# Provides list of source and header files (STK_SOURCES and STK_HEADERS)
include(sources.cmake)

# Generate source groups useful for MSVC project explorer
include(cmake/SourceGroupFunctions.cmake)
source_group_hierarchy(STK_SOURCES STK_HEADERS)


if(APPLE)
    # icon files to copy in the bundle
    set(OSX_ICON_FILES ${PROJECT_SOURCE_DIR}/src/ide/Xcode/stk.icns)
    set_source_files_properties(${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(STK_SOURCES ${STK_SOURCES} ${OSX_ICON_FILES})

    # build the executable and create an app bundle
    add_executable(supertuxkart MACOSX_BUNDLE ${STK_SOURCES})

    find_library(IOKIT_LIBRARY IOKit)
    find_library(QUICKTIME_LIBRARY QuickTime)
    find_library(CARBON_LIBRARY Carbon)
    find_library(AUDIOUNIT_LIBRARY AudioUnit)
    find_library(COCOA_LIBRARY Cocoa)

    target_link_libraries(supertuxkart
         ${IOKIT_LIBRARY}
         ${QUICKTIME_LIBRARY}
         ${CARBON_LIBRARY}
         ${AUDIOUNIT_LIBRARY}
         ${COCOA_LIBRARY})

    # configure CMake to use a custom Info.plist
    set_target_properties(supertuxkart PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/ide/Xcode/SuperTuxKart-Info.plist)

    add_custom_command(TARGET supertuxkart PRE_BUILD
        COMMAND ln -s ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/supertuxkart.app/Contents/Resources)
else()
    find_library(PTHREAD_LIBRARY NAMES pthread pthreadVC2 PATHS ${PROJECT_SOURCE_DIR}/dependencies/lib)
    mark_as_advanced(PTHREAD_LIBRARY)

    # Set data dir (absolute or relative to CMAKE_INSTALL_PREFIX)
    get_filename_component(STK_INSTALL_DATA_DIR_ABSOLUTE ${STK_INSTALL_DATA_DIR} ABSOLUTE)
    if(${STK_INSTALL_DATA_DIR_ABSOLUTE} STREQUAL ${STK_INSTALL_DATA_DIR})
        add_definitions(-DSUPERTUXKART_DATADIR=\"${STK_INSTALL_DATA_DIR_ABSOLUTE}\")
    else()
        add_definitions(-DSUPERTUXKART_DATADIR=\"${CMAKE_INSTALL_PREFIX}/${STK_INSTALL_DATA_DIR}\")
    endif()

    # Build the final executable
    add_executable(supertuxkart ${STK_SOURCES} ${STK_HEADERS})
    target_link_libraries(supertuxkart ${PTHREAD_LIBRARY})
endif()


# To invoke irrlicht makefile. Just a few things must be checked before it can be uncommented :
# 1) make it pass NDEBUG=1
# 2) make the stdout output visible

#add_custom_target(
#   stkirrlicht
#   COMMAND make
#   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/irrlicht/source/Irrlicht
#)
#add_dependencies(supertuxkart stkirrlicht)

include_directories(${PROJECT_SOURCE_DIR}/lib/irrlicht/include)


# Common library dependencies
target_link_libraries(supertuxkart
    bulletdynamics
    bulletcollision
    bulletmath
    enet
    ${CURL_LIBRARIES}
    ${IRRLICHT_LIBRARIES}
    ${OGGVORBIS_LIBRARIES}
    ${IRRLICHT_LIBRARY}
    ${IRRLICHT_XF86VM_LIBRARY}
    ${OPENAL_LIBRARY}
    ${OPENGL_LIBRARIES})

if(USE_FRIBIDI)
    target_link_libraries(supertuxkart ${FRIBIDI_LIBRARIES})
    add_definitions(-DENABLE_BIDI)
endif()

if(USE_WIIUSE)
    target_link_libraries(supertuxkart wiiuse bluetooth)
    add_definitions(-DENABLE_WIIUSE)
endif()


# Optional tools
add_subdirectory(tools/font_tool)


# ==== Make dist target ====
add_custom_target(dist
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION} && rm -f ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}.tar.bz2
            && echo "Exporting..."
            && svn export ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}
            && echo "Compressing..."
            && cd ${CMAKE_BINARY_DIR}
            && tar -cjf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}.tar.bz2 ./SuperTuxKart-${PROJECT_VERSION}
            && echo "Done, cleaning up"
            && rm -rf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# ==== Install target ====
install(TARGETS supertuxkart RUNTIME DESTINATION ${STK_INSTALL_BINARY_DIR} BUNDLE DESTINATION .)
install(DIRECTORY ${STK_DATA_DIR} DESTINATION ${STK_INSTALL_DATA_DIR} PATTERN ".svn" EXCLUDE)
install(FILES ${PROJECT_BINARY_DIR}/supertuxkart.desktop DESTINATION share/applications)
install(FILES data/supertuxkart_32.xpm data/supertuxkart_64.xpm DESTINATION share/pixmaps)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
configure_file(data/supertuxkart_desktop.template supertuxkart.desktop)
add_dependencies(supertuxkart supertuxkart.desktop)