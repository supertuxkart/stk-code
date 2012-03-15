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

# Find include directory and library
find_path(IRRLICHT_INCLUDE_DIR NAMES irrlicht.h
    PATHS ${IRRLICHT_DIR}
        /Library/Frameworks/IrrFramework.framework/Versions/A/Headers/
        ${PROJECT_SOURCE_DIR}/dependencies/include/irrlicht
    PATH_SUFFIXES include irrlicht)

if(APPLE)
    find_library(IRRLICHT_LIBRARY NAMES IrrFramework PATHS /Library/Frameworks/IrrFramework.framework)
else()
    find_library(IRRLICHT_LIBRARY NAMES Irrlicht libIrrlicht
        PATHS ${IRRLICHT_DIR}/lib/Linux ${PROJECT_SOURCE_DIR}/dependencies/lib ${PROJECT_SOURCE_DIR})
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

# STK uses Irrlicht/OpenGL on all platforms
find_package(OpenGL REQUIRED)

# Publish variables
set(IRRLICHT_INCLUDE_DIRS ${IRRLICHT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
set(IRRLICHT_LIBRARIES ${IRRLICHT_LIBRARY} ${OPENGL_LIBRARIES})
mark_as_advanced(IRRLICHT_INCLUDE_DIR IRRLICHT_LIBRARY)

# Check if Xxf86vm is required when building for platforms using X11
if(UNIX AND NOT APPLE AND NOT CYGWIN)
    find_library(IRRLICHT_XF86VM_LIBRARY Xxf86vm)
    mark_as_advanced(IRRLICHT_XF86VM_LIBRARY)

    set(IRRLICHT_SNIPPET "#include <irrlicht.h>
        int main() { irr::createDevice(irr::video::EDT_NULL)\; return 0\; }")

    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_INCLUDES ${IRRLICHT_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${IRRLICHT_LIBRARIES})
    check_cxx_source_compiles(${IRRLICHT_SNIPPET} IRRLICHT_WITHOUT_XF86VM)

    # If it does not work without Xxf86vm library...
    if(NOT IRRLICHT_WITHOUT_XF86VM)
        set(CMAKE_REQUIRED_LIBRARIES ${IRRLICHT_LIBRARIES} ${IRRLICHT_XF86VM_LIBRARY})
        check_cxx_source_compiles(${IRRLICHT_SNIPPET} IRRLICHT_WITH_XF86VM)

        # ... but with Xxf86vm, then add this library to Irrlicht dependencies
        if(IRRLICHT_WITH_XF86VM)
            set(IRRLICHT_LIBRARIES ${IRRLICHT_LIBRARIES} ${IRRLICHT_XF86VM_LIBRARY})
        else()
            message(WARNING "Irrlicht does not compile with and without Xxf86vm")
        endif()
    endif()
endif()
