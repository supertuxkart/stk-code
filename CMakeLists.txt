# root CMakeLists for the SuperTuxKart project
project(SuperTuxKart)
set(PROJECT_VERSION "0.8.1")

cmake_minimum_required(VERSION 2.8.1)
if(NOT (CMAKE_MAJOR_VERSION VERSION_LESS 3))
  cmake_policy(SET CMP0043 OLD)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

include(BuildTypeSTKRelease)
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to STKRelease")
    set(CMAKE_BUILD_TYPE "STKRelease")
endif()

option(USE_WIIUSE "Support for wiimote input devices" ON)
option(USE_FRIBIDI "Support for right-to-left languages" ON)
option(CHECK_ASSETS "Check if assets are installed in ../stk-assets" ON)

if(UNIX OR MINGW)
    option(USE_CPP2011 "Activate C++ 2011 mode (GCC only)" OFF)
endif()
if(MSVC OR MINGW)
    # Normally hide the option to build wiiuse on VS, since it depends
    # on the installation of the Windows DDK (Driver Developer Kit),
    # which also needs an absolute path :(
    option(WIIUSE_BUILD "Build wiiuse lib (only for developers)" OFF)
    mark_as_advanced(WIIUSE_BUILD)
else()
    set(WIIUSE_BUILD ON)
endif()

if(MINGW)
    set(USE_WIIUSE OFF)
endif()

if(UNIX AND NOT APPLE)
    option(USE_XRANDR "Use xrandr instead of vidmode" ON)
    option(USE_ASAN   "Build with Leak/Address sanitizer" OFF)
endif()

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

# Build glew library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/glew")
include_directories("${PROJECT_SOURCE_DIR}/lib/glew/include")

# Build the irrlicht library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/irrlicht")
include_directories("${PROJECT_SOURCE_DIR}/lib/irrlicht/include")

# Build the Wiiuse library
# Note: wiiuse MUST be declared after irrlicht, since otherwise
# (at least on VS) irrlicht will find wiiuse io.h file because
# of the added include directory.
if(USE_WIIUSE)
    if(WIIUSE_BUILD)
        add_subdirectory("${PROJECT_SOURCE_DIR}/lib/wiiuse")
    endif()
    include_directories("${PROJECT_SOURCE_DIR}/lib/wiiuse")
endif()

# Build the angelscript library
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/angelscript/projects/cmake")
include_directories("${PROJECT_SOURCE_DIR}/lib/angelscript/include")

# Set include paths
include_directories(${STK_SOURCE_DIR})

# These variables enable MSVC to find libraries located in "dependencies"
if(WIN32)
    set(ENV{PATH} "$ENV{PATH};${PROJECT_SOURCE_DIR}/dependencies/include")
    set(ENV{LIB} ${PROJECT_SOURCE_DIR}/dependencies/lib)
    set(ENV{OPENALDIR} ${PROJECT_SOURCE_DIR}/dependencies)
    add_definitions(-D_IRR_STATIC_LIB_)
endif()


if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -F/Library/Frameworks")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")   # Enable multi-processor compilation (faster)
endif()

# OpenAL
if(APPLE)
    # In theory it would be cleaner to let CMake detect the right dependencies. In practice, this means that if a OSX user has
    # unix-style installs of Vorbis/Ogg/OpenAL/etc. they will be picked up over our frameworks. This is blocking when I make releases :
    # the mac I use to make STK releases does have other installs of vorbis/ogg/etc. which aren't compatible with STK, so letting
    # CMake pick the library it wants essentially means I can't build.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenAL")
    set(OPENAL_LIBRARY)
else()
    find_package(OpenAL REQUIRED)
    include_directories(${OPENAL_INCLUDE_DIR})
endif()

# OggVorbis
if(APPLE)
    # In theory it would be cleaner to let CMake detect the right dependencies. In practice, this means that if a OSX user has
    # unix-style installs of Vorbis/Ogg/OpenAL/etc. they will be picked up over our frameworks. This is blocking when I make releases :
    # the mac I use to make STK releases does have other installs of vorbis/ogg/etc. which aren't compatible with STK, so letting
    # CMake pick the library it wants essentially means I can't build.
    set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -I/Library/Frameworks/Ogg.framework/Versions/A/Headers -I/Library/Frameworks/Vorbis.framework/Versions/A/Headers")
else()
    find_package(OggVorbis REQUIRED)
    include_directories(${OGGVORBIS_INCLUDE_DIRS})
endif()

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

# OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


if(UNIX OR MINGW)
#    if(USE_CPP2011)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
#    endif()
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

if(UNIX AND NOT APPLE)
    if(USE_XRANDR)
        find_package(Xrandr REQUIRED)
        if(NOT XRANDR_FOUND)
            message(FATAL_ERROR "XRANDR not found.")
        endif()
    else()
        find_library(IRRLICHT_XF86VM_LIBRARY Xxf86vm)
        mark_as_advanced(IRRLICHT_XF86VM_LIBRARY)
    endif()
endif()


# Set some compiler options
if(UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if(WIN32)
    # By default windows.h has macros defined for min and max that screw up everything
    add_definitions(-DNOMINMAX)

    # And shut up about unsafe stuff
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
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

# TODO: remove this switch
add_definitions(-DHAVE_OGGVORBIS)

if(WIN32)
    configure_file("${STK_SOURCE_DIR}/windows_installer/icon_rc.template" "${PROJECT_BINARY_DIR}/tmp/icon.rc")
endif()

# Provides list of source and header files (STK_SOURCES and STK_HEADERS)
include(sources.cmake)

# Generate source groups useful for MSVC project explorer
include(cmake/SourceGroupFunctions.cmake)
source_group_hierarchy(STK_SOURCES STK_HEADERS)


if(APPLE)
    # icon files to copy in the bundle
    set(OSX_ICON_FILES ${PROJECT_SOURCE_DIR}/data/supertuxkart.icns)
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
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/data/SuperTuxKart-Info.plist)

    if(CMAKE_GENERATOR MATCHES "Xcode")
        add_custom_command(TARGET supertuxkart POST_BUILD
            COMMAND ln -f -s ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/\${CONFIGURATION}/supertuxkart.app/Contents/Resources)
    else()
        add_custom_command(TARGET supertuxkart POST_BUILD
            COMMAND ln -f -s ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/supertuxkart.app/Contents/Resources)
    endif()
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
    add_executable(supertuxkart ${STK_SOURCES} ${STK_RESOURCES} ${STK_HEADERS})
    target_link_libraries(supertuxkart ${PTHREAD_LIBRARY})
endif()

# CURL
if(WIN32)
    target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/dependencies/lib/libcurldll.a)
else()
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIRS})
endif()

# Common library dependencies
target_link_libraries(supertuxkart
    bulletdynamics
    bulletcollision
    bulletmath
    enet
    glew
    stkirrlicht
    angelscript
    ${CURL_LIBRARIES}
    ${OGGVORBIS_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${OPENGL_LIBRARIES}
    )

if(UNIX AND NOT APPLE)
    if(USE_XRANDR)
        target_link_libraries(supertuxkart ${XRANDR_LIBRARIES})
    else()
        target_link_libraries(supertuxkart ${IRRLICHT_XF86VM_LIBRARY})
    endif()
    if(USE_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        target_link_libraries(supertuxkart "-fsanitize=address")
    endif()
endif()

if(APPLE)
    # In theory it would be cleaner to let CMake detect the right dependencies. In practice, this means that if a OSX user has
    # unix-style installs of Vorbis/Ogg/OpenAL/etc. they will be picked up over our frameworks. This is blocking when I make releases :
    # the mac I use to make STK releases does have other installs of vorbis/ogg/etc. which aren't compatible with STK, so letting
    # CMake pick the library it wants essentially means I can't build.
    set_target_properties(supertuxkart PROPERTIES LINK_FLAGS "-arch i386 -F/Library/Frameworks -framework OpenAL -framework Ogg -framework Vorbis")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/Library/Frameworks/OpenAL.framework/Versions/A/Headers")
endif()

if(USE_FRIBIDI)
    target_link_libraries(supertuxkart ${FRIBIDI_LIBRARIES})
    add_definitions(-DENABLE_BIDI)
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
            target_link_libraries(supertuxkart ${PROJECT_SOURCE_DIR}/dependencies/lib/wiiuse.lib)
        endif()
    else()
        target_link_libraries(supertuxkart wiiuse bluetooth)
    endif()
    add_definitions(-DENABLE_WIIUSE)

endif()

if(MSVC OR MINGW)
  target_link_libraries(supertuxkart iphlpapi.lib)
  add_custom_command(TARGET supertuxkart POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/dependencies/dll"
        $<TARGET_FILE_DIR:supertuxkart>)
  add_custom_target(stkshaders SOURCES ${STK_SHADERS})
endif()

# Optional tools
add_subdirectory(tools/font_tool)


# ==== Make dist target ====
if(MSVC OR MINGW)
  # Don't create a dist target for VS
else()
  add_custom_target(dist
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION} && rm -f ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}.tar.bz2
            && echo "Exporting..."
            && svn export ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}
            && echo "Compressing..."
            && cd ${CMAKE_BINARY_DIR}
            && tar -cjf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}.tar.bz2 ./SuperTuxKart-${PROJECT_VERSION}
            && echo "Done, cleaning up"
            && rm -rf ${CMAKE_BINARY_DIR}/SuperTuxKart-${PROJECT_VERSION}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )
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
install(FILES data/supertuxkart_32.png DESTINATION share/icons/hicolor/32x32 RENAME supertuxkart.png)
install(FILES data/supertuxkart_128.png DESTINATION share/icons/hicolor/128x128 RENAME supertuxkart.png)
install(FILES data/supertuxkart_32.png data/supertuxkart_128.png DESTINATION share/pixmaps)
install(FILES data/supertuxkart.appdata DESTINATION share/appdata)
