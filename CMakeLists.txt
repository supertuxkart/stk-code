# root CMakeLists for the SuperTuxKart project
project(SuperTuxKart)
set(PROJECT_VERSION "0.7.3")

cmake_minimum_required(VERSION 2.8.1)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()


find_package(Irrlicht 1.8 REQUIRED)
include_directories(${IRRLICHT_INCLUDE_DIRS})
#set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(STK_SOURCE_DIR "src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_CXX_FLAGS_RELEASE -O2)

# Tweakable values
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(DEBUG ON)
else()
	set(DEBUG OFF)
endif()

option(USE_WIIUSE "Support for wiimote input devices" OFF)
option(USE_FRIBIDI "Support for right-to-left languages" ON)
option(FONT_TOOL "Compile font tool" OFF)

# Build the Bullet physics library
add_subdirectory("${STK_SOURCE_DIR}/bullet")

# Build the ENet UDP network library
add_subdirectory("${STK_SOURCE_DIR}/enet")

# Build the Wiiuse library
if(USE_WIIUSE)
	add_subdirectory("${STK_SOURCE_DIR}/wiiuse")
endif()

# Set include paths
include_directories("${STK_SOURCE_DIR}")
include_directories("${STK_SOURCE_DIR}/enet/include")
include_directories("${STK_SOURCE_DIR}/bullet/src")
if(USE_WIIUSE)
	include_directories("${STK_SOURCE_DIR}/wiiuse")
endif()

if(APPLE)
	# We could use smarter detection of the frameworks but supporting a single official setup makes things easy...
	include_directories("/Library/Frameworks/Ogg.framework/Headers")
	include_directories("/Library/Frameworks/Vorbis.framework/Headers")
	include_directories("/usr/local/include")
	include_directories("/usr/include")
	find_library(Ogg /Library/Frameworks/Ogg.framework)
	find_library(Vorbis /Library/Frameworks/Vorbis.framework)
endif()

# Set lib paths
link_directories("${STK_SOURCE_DIR}/bullet")

if(USE_WIIUSE)
	link_directories("${STK_SOURCE_DIR}/wiiuse")
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# OpenAL
if(APPLE)
    find_library(OpenAL /Library/Frameworks/OpenAL.framework)
else()
    find_package(OpenAL)
    
    if(OPENAL_FOUND)
	    message(STATUS "OpenAL found (include dir: ${OPENAL_INCLUDE_DIR})")
	    include_directories(${OPENAL_INCLUDE_DIR})
    else()
	    message(FATAL_ERROR "OpenAL not found.")
    endif()
endif()

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

add_definitions(-DHAVE_OGGVORBIS)	# TODO: remove this switch

if(DEBUG)
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

include(sources.cmake)
set(SRCS ${SRCS} ${STK_SOURCES})

if (APPLE)		
    # icon files to copy in the bundle
    set( OSX_ICON_FILES ${PROJECT_SOURCE_DIR}/src/ide/Xcode/stk.icns)
    set_source_files_properties( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set( SRCS ${SRCS} ${OSX_ICON_FILES} )
    
    # build the executable and create an app bundle
    add_executable(supertuxkart MACOSX_BUNDLE ${SRCS} )
    
    target_link_libraries(supertuxkart
	    bulletdynamics
	    bulletcollision
	    bulletmath
	    enet
	    ${CURL_LIBRARIES}
	    # We could use smarter detection of the frameworks but supporting a single official setup makes things easy...
	    ${IRRLICHT_LIBRARIES}
	    ${OPENGL_LIBRARIES}
	    /Library/Frameworks/Ogg.framework
	    /Library/Frameworks/OpenAL.framework
	    /Library/Frameworks/Vorbis.framework
	    # TODO: allow linking against frameworks from a SDK.
	    /System/Library/Frameworks/IOKit.framework
	    /System/Library/Frameworks/QuickTime.framework
	    /System/Library/Frameworks/Carbon.framework
	    /System/Library/Frameworks/AudioUnit.framework
	    /System/Library/Frameworks/Cocoa.framework)
	    
    # configure CMake to use a custom Info.plist
    set_target_properties(supertuxkart PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/src/ide/Xcode/SuperTuxKart-Info.plist )
  
    add_custom_command(TARGET supertuxkart PRE_BUILD COMMAND ln -s ${PROJECT_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/bin/supertuxkart.app/Contents/Resources)
    
else()

    add_definitions(-DSUPERTUXKART_DATADIR=\"${CMAKE_INSTALL_PREFIX}/share/games/supertuxkart\")

    # Build the final executable
    add_executable(supertuxkart ${SRCS})

    target_link_libraries(supertuxkart
	    ${IRRLICHT_LIBRARIES}
	    bulletdynamics
	    bulletcollision
	    bulletmath
	    enet
	    pthread
	    ${CURL_LIBRARIES}
	    ${OPENGL_LIBRARIES}
	    ${OPENAL_LIBRARY}
	    vorbisfile)
endif()

if(USE_FRIBIDI)
    target_link_libraries(supertuxkart ${FRIBIDI_LIBRARIES})
    add_definitions(-DENABLE_BIDI=1)
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
MACRO(installfilemacro filepath)
    set(source_regex "(.*)/data/(.*)")
    set(svn_regex "(.*).svn(.*)")
    
    string(REGEX MATCH "${svn_regex}" is_svn_dir "${curr}")
    if(is_svn_dir)
        # don't install SVN files
    else()
        string(REGEX MATCH "${source_regex}" source_path "${curr}")
        if(source_path)
            string(REGEX REPLACE "${source_regex}" "\\2" after_source "${source_path}")
            get_filename_component(install_location ${after_source} PATH)
            install(FILES ${filepath} DESTINATION "share/games/supertuxkart/data/${install_location}")
        endif(source_path)
    endif()
ENDMACRO(installfilemacro)

FILE(GLOB_RECURSE datafiles "${CMAKE_CURRENT_SOURCE_DIR}/data/*")
foreach(curr ${datafiles})
    installfilemacro(${curr})
endforeach()


install(TARGETS supertuxkart RUNTIME DESTINATION games BUNDLE DESTINATION .)
install(FILES ${PROJECT_BINARY_DIR}/supertuxkart.desktop DESTINATION share/applications)
install(FILES data/supertuxkart_32.xpm data/supertuxkart_64.xpm DESTINATION share/pixmaps)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
configure_file(data/supertuxkart_desktop.template supertuxkart.desktop)
add_dependencies(supertuxkart supertuxkart.desktop)
