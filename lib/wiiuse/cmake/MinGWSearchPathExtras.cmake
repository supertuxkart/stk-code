# - Additional help finding search paths on MinGW distributions, including MSYS2.
#
# Much of this is really more in the purview of CMake or the packages of CMake for
# those distributions, but if I can centralize/simplify the pain here, it's worth doing.
#
# Variables: (all are internal cache variables)
#  MINGWSEARCH_INCLUDE_DIRS - use under PATHS in your find_path() commands
#  MINGWSEARCH_LIBRARY_DIRS - use under PATHS in your find_library() commands
#  MINGWSEARCH_PREFIXES - suitable for temporary use in CMAKE_FIND_ROOT_PATH or CMAKE_PREFIX_PATH.
#  MINGWSEARCH_TARGET_TRIPLE - something like x86_64-w64-mingw32 or i686-w64-mingw32, use as you see fit.
#
# Original Author:
# 2016 Ryan Pavlik <ryan@sensics.com> <ryan.pavlik@gmail.com>
#
# Copyright Sensics, Inc. 2016.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(MINGW AND NOT MINGWSEARCH_COMPLETED)
    ###
    # Helper function
    ###
    function(_mingwsearch_conditional_add _var _path)
        #message(STATUS "conditional add to ${_var}: ${_path}")
        if(("${_path}" MATCHES "registry") OR (NOT IS_DIRECTORY "${_path}"))
            # Path invalid - do not add
            return()
        endif()
        list(FIND ${_var} "${_path}" _idx)
        if(_idx GREATER -1)
            # Path already in list - do not add
            return()
        endif()
        # Not yet in the list, so we'll add it
        list(APPEND ${_var} "${_path}")
        set(${_var} ${${_var}} PARENT_SCOPE)
    endfunction()

    # Clear the working variables.
    set(MINGWSEARCH_INCLUDE_DIRS_WORK)
    set(MINGWSEARCH_LIBRARY_DIRS_WORK)
    set(MINGWSEARCH_PREFIXES_WORK)
    set(_mingw_target_triple)

    # Try to find the string like x86_64-w64-mingw32 by parsing the implicit link directories...
    # TODO this is a hack that either should be resolved in CMake or in MSYS2's package of CMake.
    foreach(_link_dir ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
        _mingwsearch_conditional_add(MINGWSEARCH_LIBRARY_DIRS_WORK "${_link_dir}")
        if("${_link_dir}" MATCHES "/([^/]*-mingw32)/lib")
            set(_mingw_target_triple ${CMAKE_MATCH_1})
            get_filename_component(_mingw_internal_basedir "${_link_dir}" PATH)
            # Try adding the parallel include dir
            if(IS_DIRECTORY "${_mingw_internal_basedir}/include")
                _mingwsearch_conditional_add(MINGWSEARCH_INCLUDE_DIRS_WORK "${_mingw_internal_basedir}/include")
                _mingwsearch_conditional_add(MINGWSEARCH_PREFIXES_WORK "${_mingw_internal_basedir}")
            endif()
            if(NOT CMAKE_CROSSCOMPILING)
                # Try going up a level, since the directory with the target is usually a sibling to the main prefix.
                get_filename_component(_mingw_main_basedir_candidate "${_mingw_internal_basedir}/.." ABSOLUTE)
                if(IS_DIRECTORY "${_mingw_main_basedir_candidate}/include" AND NOT ("${_mingw_main_basedir_candidate}" STREQUAL "${_mingw_internal_basedir}"))
                    # If we could go up a level, add that include dir too.
                    _mingwsearch_conditional_add(MINGWSEARCH_INCLUDE_DIRS_WORK "${_mingw_main_basedir_candidate}/include")
                    _mingwsearch_conditional_add(MINGWSEARCH_PREFIXES_WORK "${_mingw_main_basedir_candidate}")
                endif()
            endif()
        endif()
    endforeach()

    ###
    # Output results.
    ###
    if(MINGWSEARCH_INCLUDE_DIRS_WORK)
        set(MINGWSEARCH_INCLUDE_DIRS "${MINGWSEARCH_INCLUDE_DIRS_WORK}" CACHE INTERNAL "" FORCE)
        #message(STATUS "MINGWSEARCH_INCLUDE_DIRS ${MINGWSEARCH_INCLUDE_DIRS}")
    endif()

    if(MINGWSEARCH_LIBRARY_DIRS_WORK)
        set(MINGWSEARCH_LIBRARY_DIRS "${MINGWSEARCH_LIBRARY_DIRS_WORK}" CACHE INTERNAL "" FORCE)
        #message(STATUS "MINGWSEARCH_LIBRARY_DIRS ${MINGWSEARCH_LIBRARY_DIRS}")
    endif()

    if(MINGWSEARCH_PREFIXES_WORK)
        set(MINGWSEARCH_PREFIXES "${MINGWSEARCH_PREFIXES_WORK}" CACHE INTERNAL "" FORCE)
        #message(STATUS "MINGWSEARCH_PREFIXES ${MINGWSEARCH_PREFIXES}")
    endif()

    if(_mingw_target_triple)
        set(MINGWSEARCH_TARGET_TRIPLE ${_mingw_target_triple} CACHE INTERNAL "" FORCE)
        #message(STATUS "MINGWSEARCH_TARGET_TRIPLE ${MINGWSEARCH_TARGET_TRIPLE}")
    endif()

    set(MINGWSEARCH_COMPLETED TRUE CACHE INTERNAL "" FORCE)
endif()
