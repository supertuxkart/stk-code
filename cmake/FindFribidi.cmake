# - Find Fribidi
# Find the Fribidi includes and libraries
#
# Following variables are provided:
# FRIBIDI_FOUND
#     True if Fribidi has been found
# FRIBIDI_INCLUDE_DIRS
#     The include directories of Fribidi
# FRIBIDI_LIBRARIES
#     Fribidi library list

if(APPLE)
    set(FRIBIDI_INCLUDE_DIR NAMES fribidi/fribidi.h PATHS /Library/Frameworks/fribidi.framework/Headers)
    find_library(FRIBIDI_LIBRARY NAMES fribidi PATHS /Library/Frameworks/fribidi.framework)
    set(FRIBIDI_LIBRARIES ${FRIBIDI_LIBRARY})
    include_directories(/Library/Frameworks/fribidi.framework/Headers)
    #add_definitions(-framework fribidi)
    set(FRIBIDI_FOUND 1)
elseif(UNIX)
    include(FindPkgConfig)
    pkg_check_modules(FRIBIDI fribidi)
else()
    set(FRIBIDI_FOUND 0)
endif()

if(NOT FRIBIDI_FOUND)
    find_path(FRIBIDI_INCLUDE_DIR NAMES fribidi/fribidi.h PATHS /Library/Frameworks/fribidi.framework/Headers "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/include")
    find_library(FRIBIDI_LIBRARY NAMES fribidi PATHS /Library/Frameworks/fribidi.framework "${PROJECT_SOURCE_DIR}/${DEPENDENCIES}/lib")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Fribidi DEFAULT_MSG FRIBIDI_INCLUDE_DIR FRIBIDI_LIBRARY)

    if(APPLE)
            set(FRIBIDI_INCLUDE_DIR "/Library/Frameworks/fribidi.framework/Headers")
    endif()

    # Publish variables
    set(FRIBIDI_INCLUDE_DIRS ${FRIBIDI_INCLUDE_DIR})
    set(FRIBIDI_LIBRARIES ${FRIBIDI_LIBRARY})
endif()

mark_as_advanced(FRIBIDI_INCLUDE_DIR FRIBIDI_LIBRARY)
