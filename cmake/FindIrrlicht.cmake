# - Find Irrlicht
# Find the Irrlicht includes and libraries
#
# Following variables are provided:
# IRRLICHT_FOUND
#     True if Irrlicht has been found
# IRRLICHT_DIR
#     Path to Irrlicht
# IRRLICHT_INCLUDE_DIRS
#     The include directories of Irrlicht
# IRRLICHT_LIBRARIES
#     Irrlicht library list

set(IRRLICHT_DIR "" CACHE PATH "Path to Irrlicht")

# Set library directories depending on system
if(APPLE)
	set(IRRLICHT_LIBRARY_DIR "/Library/Frameworks/IrrFramework.framework")
elseif(UNIX)
	set(IRRLICHT_LIBRARY_DIR "${IRRLICHT_DIR}/lib/Linux")
elseif(MSVC)
	set(IRRLICHT_LIBRARY_DIR "${PROJECT_SOURCE_DIR}/dependencies/lib")
else()
	set(IRRLICHT_LIBRARY_DIR "${IRRLICHT_DIR}/lib/Win32-gcc")
endif()

# Find include directory and library
find_path(IRRLICHT_INCLUDE_DIR NAMES irrlicht.h
    PATHS ${IRRLICHT_DIR} /Library/Frameworks/IrrFramework.framework/Versions/A/Headers/ ${PROJECT_SOURCE_DIR}/dependencies/include/irrlicht
    PATH_SUFFIXES include irrlicht)

if(APPLE)
    find_library(IRRLICHT_LIBRARY NAMES IrrFramework PATHS ${IRRLICHT_LIBRARY_DIR})
else()
    find_library(IRRLICHT_LIBRARY NAMES Irrlicht PATHS ${IRRLICHT_LIBRARY_DIR} ${PROJECT_SOURCE_DIR})
endif()

# Determine Irrlicht version
if(EXISTS ${IRRLICHT_INCLUDE_DIR}/IrrCompileConfig.h)
    file(STRINGS ${IRRLICHT_INCLUDE_DIR}/IrrCompileConfig.h IRRLICHT_COMPILE_CONFIG REGEX IRRLICHT_VERSION)
    string(REGEX MATCH "IRRLICHT_VERSION_MAJOR ([0-9]+)" _tmp ${IRRLICHT_COMPILE_CONFIG})
    set(IRRLICHT_VERSION_MAJOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "IRRLICHT_VERSION_MINOR ([0-9]+)" _tmp ${IRRLICHT_COMPILE_CONFIG})
    set(IRRLICHT_VERSION_MINOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "IRRLICHT_VERSION_REVISION ([0-9]+)" _tmp ${IRRLICHT_COMPILE_CONFIG})
    set(IRRLICHT_VERSION_REVISION ${CMAKE_MATCH_1})
    set(IRRLICHT_VERSION "${IRRLICHT_VERSION_MAJOR}.${IRRLICHT_VERSION_MINOR}.${IRRLICHT_VERSION_REVISION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Irrlicht
    REQUIRED_VARS IRRLICHT_LIBRARY IRRLICHT_INCLUDE_DIR
    VERSION_VAR IRRLICHT_VERSION)

# Publish variables
set(IRRLICHT_INCLUDE_DIRS ${IRRLICHT_INCLUDE_DIR})
set(IRRLICHT_LIBRARIES ${IRRLICHT_LIBRARY})
mark_as_advanced(IRRLICHT_INCLUDE_DIR IRRLICHT_LIBRARY)
