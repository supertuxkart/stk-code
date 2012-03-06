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
option(FONT_TOOL "Compile font tool" OFF)

set(STK_SOURCE_DIR "src")
set(STK_DATA_DIR "${PROJECT_SOURCE_DIR}/data")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")


# Build the Bullet physics library
add_subdirectory("${STK_SOURCE_DIR}/bullet")
include_directories("${STK_SOURCE_DIR}/bullet/src")

# Build the ENet UDP network library
add_subdirectory("${STK_SOURCE_DIR}/enet")
include_directories("${STK_SOURCE_DIR}/enet/include")

# Build the Wiiuse library
if(USE_WIIUSE)
	add_subdirectory("${STK_SOURCE_DIR}/wiiuse")
	include_directories("${STK_SOURCE_DIR}/wiiuse")
endif()

# Set include paths
include_directories(${STK_SOURCE_DIR})

# Irrlicht
find_package(Irrlicht 1.8 REQUIRED)
include_directories(${IRRLICHT_INCLUDE_DIRS})

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

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


# Set some compiler options
if(UNIX)
	add_definitions(-Wall)
endif()

# TODO: remove this switch
add_definitions(-DHAVE_OGGVORBIS)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DDEBUG)
else()
	add_definitions(-DNDEBUG)
endif()

find_package(Freetype)

if(FONT_TOOL)
    if(FREETYPE_FOUND)
	    message(STATUS "Freetype found")
	    find_package(X11)
	    include_directories(${FREETYPE_INCLUDE_DIRS})
	    add_executable(font_tool
		    tools/font_tool/CFontTool.cpp
		    tools/font_tool/CFontTool.h
		    tools/font_tool/CVectorFontTool.h
		    tools/font_tool/main.cpp
	    )
	    target_link_libraries(font_tool ${FREETYPE_LIBRARIES})
	    target_link_libraries(font_tool ${X11_Xft_LIB})
	    target_link_libraries(font_tool ${OPENGL_LIBRARIES})
	    target_link_libraries(font_tool ${IRRLICHT_LIBRARIES})
    else()
	    message(STATUS "Freetype was not found, the font tool won't be built (only useful for developers)")
    endif()
else()
    message(STATUS "Font tool deactivated, the font tool won't be built (only useful for developers)")
endif()


# Provides list of source files in STK_SOURCES
include(sources.cmake)

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
    add_definitions(-DSUPERTUXKART_DATADIR=\"${CMAKE_INSTALL_PREFIX}/share/games/supertuxkart\")

    # Build the final executable
    add_executable(supertuxkart ${STK_SOURCES})
    if(UNIX)
        target_link_libraries(supertuxkart pthread Xxf86vm)
    endif()
endif()

# Common library dependencies
target_link_libraries(supertuxkart
    bulletdynamics
    bulletcollision
    bulletmath
    enet
    ${CURL_LIBRARIES}
    ${IRRLICHT_LIBRARIES}
    ${OGGVORBIS_LIBRARIES}
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

if(UNIX AND NOT APPLE)
	target_link_libraries(supertuxkart Xxf86vm)
elseif(APPLE)
	# We could use smarter detection of the frameworks but supporting a single official setup makes things easy...
	target_link_libraries(supertuxkart /Library/Frameworks/Ogg.framework)
	target_link_libraries(supertuxkart /Library/Frameworks/OpenAL.framework)
	target_link_libraries(supertuxkart /Library/Frameworks/Vorbis.framework)

	target_link_libraries(supertuxkart /System/Library/Frameworks/IOKit.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/QuickTime.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/Carbon.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/AudioUnit.framework)
	target_link_libraries(supertuxkart /System/Library/Frameworks/Cocoa.framework)
endif()

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
install(TARGETS supertuxkart RUNTIME DESTINATION games BUNDLE DESTINATION .)
install(DIRECTORY "${STK_DATA_DIR}/" DESTINATION "share/games/supertuxkart/data" PATTERN ".svn" EXCLUDE)
install(FILES ${PROJECT_BINARY_DIR}/supertuxkart.desktop DESTINATION share/applications)
install(FILES data/supertuxkart_32.xpm data/supertuxkart_64.xpm DESTINATION share/pixmaps)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
configure_file(data/supertuxkart_desktop.template supertuxkart.desktop)
add_dependencies(supertuxkart supertuxkart.desktop)
