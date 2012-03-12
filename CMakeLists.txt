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
if(MSVC)
  find_library(OPENAL_LIBRARY OpenAL32 REQUIRED PATHS "${PROJECT_SOURCE_DIR}/dependencies/lib")
else()
  find_package(OpenAL REQUIRED)
endif()
include_directories(${OPENAL_INCLUDE_DIR})

# OggVorbis
find_package(OggVorbis REQUIRED)
include_directories(${OGGVORBIS_INCLUDE_DIRS})


# CURL
if(CYGWIN)
  find_package(CURL REQUIRED)
elseif(WIN32)
  find_library(CURL_LIBRARIES curl libcurl_imp REQUIRED PATHS ${PROJECT_SOURCE_DIR}/dependencies/lib)
else()
  find_package(CURL REQUIRED)
endif()
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
	if(CYGWIN)
	    target_link_libraries(supertuxkart pthread)
    elseif(UNIX)
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

if(WIN32)
    if(CYGWIN)
		find_library(PTHREAD_LIBRARY NAMES pthread PATHS "${PROJECT_SOURCE_DIR}/dependencies/lib")
		target_link_libraries(supertuxkart ${PTHREAD_LIBRARY})
	else()
		find_library(PTHREAD_LIBRARY NAMES pthreadVC2 PATHS "${PROJECT_SOURCE_DIR}/dependencies/lib")
		target_link_libraries(supertuxkart ${PTHREAD_LIBRARY})
	endif()
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
install(TARGETS supertuxkart RUNTIME DESTINATION games BUNDLE DESTINATION .)
install(DIRECTORY "${STK_DATA_DIR}/" DESTINATION "share/games/supertuxkart/data" PATTERN ".svn" EXCLUDE)
install(FILES ${PROJECT_BINARY_DIR}/supertuxkart.desktop DESTINATION share/applications)
install(FILES data/supertuxkart_32.xpm data/supertuxkart_64.xpm DESTINATION share/pixmaps)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
configure_file(data/supertuxkart_desktop.template supertuxkart.desktop)
add_dependencies(supertuxkart supertuxkart.desktop)
