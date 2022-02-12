# - Find Freetype
# Find the Freetype includes and libraries
#
# Following variables are provided:
# FREETYPE_FOUND
#     True if Freetype has been found
# FREETYPE_INCLUDE_DIRS
#     The include directories of Freetype
# FREETYPE_LIBRARIES
#     Freetype library list

if(WIN32)
    find_path(FREETYPE_INCLUDE_DIRS NAMES freetype/freetype.h PATHS "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/include/freetype2")
    find_library(FREETYPE_LIBRARY NAMES freetype libfreetype PATHS "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib")
    set(FREETYPE_FOUND 1)
    set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
elseif(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
    find_path(FREETYPE_INCLUDE_DIRS NAMES ft2build.h PATH_SUFFIXES freetype2 include/freetype2 include)
    find_library(FREETYPE_LIBRARY NAMES freetype)
    set(FREETYPE_FOUND 1)
    set(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY})
else()
    pkg_check_modules(FREETYPE freetype2)
endif()

