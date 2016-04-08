# - Try to find angelscript
# Once done this will define
#
#  ANGELSCRIPT_FOUND - system has angelscript
#  Angelscript_INCLUDE_DIRS - the angelscript include directory
#  Angelscript_LIBRARIES - the libraries needed to use angelscript
#

FIND_PATH(Angelscript_INCLUDE_DIRS angelscript.h
    PATHS
    /usr/local
    /usr
    PATH_SUFFIXES include
    )

FIND_LIBRARY(Angelscript_LIBRARY
    NAMES angelscript angelscript_s
    PATHS
    /usr/local
    /usr
    PATH_SUFFIXES lib lib64 lib32
    )

# handle the QUIETLY and REQUIRED arguments and set ANGELSCRIPT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Angelscript DEFAULT_MSG Angelscript_LIBRARY Angelscript_INCLUDE_DIRS)

IF (ANGELSCRIPT_FOUND)
    SET(Angelscript_LIBRARIES ${Angelscript_LIBRARY})
ENDIF (ANGELSCRIPT_FOUND)

MARK_AS_ADVANCED(Angelscript_LIBRARY Angelscript_LIBRARIES Angelscript_INCLUDE_DIRS)
