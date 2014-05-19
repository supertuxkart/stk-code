find_path(XRANDR_INCLUDE_DIR NAMES X11/extensions/Xrandr.h
          PATH_SUFFIXES X11/extensions
          DOC "The XRANDR include directory"
)

find_library(XRANDR_LIBRARY NAMES Xrandr
          DOC "The XRANDR library"
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XRANDR DEFAULT_MSG XRANDR_LIBRARY XRANDR_INCLUDE_DIR)

if(XRANDR_FOUND)
  set( XRANDR_LIBRARIES ${XRANDR_LIBRARY} )
  set( XRANDR_INCLUDE_DIRS ${XRANDR_INCLUDE_DIR} )
endif()

mark_as_advanced(XRANDR_INCLUDE_DIR XRANDR_LIBRARY)
