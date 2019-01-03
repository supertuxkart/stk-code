# Defines a function to install the symbols for a target
#
#  install_debug_symbols(TARGETS <target_name> [...] [DESTINATION <dest>]
#    [CONFIGURATIONS <configname> [...]] [PASSTHRU <arg to pass to INSTALL> [...]])
#
# DESTINATION is only optional if CMAKE_INSTALL_BINDIR and CMAKE_INSTALL_LIBDIR are set,
# then it will use CMAKE_INSTALL_BINDIR for executables (and DLLs) and CMAKE_INSTALL_LIBDIR
# for libraries (static libraries only on DLL platforms).
#
# CONFIGURATIONS are the config names for which symbols (PDB files) are expected -
# the default of RelWithDebInfo Debug is correct unless you've dramatically changed
# the set of configs.
#
# Anything after PASSTHRU is passed directly to the install( command after the arguments
# that the function generates.
#
# Currently a no-op if using CMake pre-3.2 (can't use generator expressions before then
# to get symbol location) or if not using MSVC (MSVC keeps its symbols separate in PDB files
# necessitating this function in the first place).
#
# Original Author:
# 2015, 2017 Ryan Pavlik <ryan@sensics.com> <abiryan@ryand.net>
# http://ryanpavlik.com
#
# Copyright Sensics, Inc. 2015, 2017.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(MSVC AND NOT CMAKE_VERSION VERSION_LESS 3.2)
    include(CMakeParseArguments)
    # debug symbols for MSVC: supported on CMake 3.2 and up where there's a
    # generator expression for a target's PDB file
    function(install_debug_symbols)
        set(options)
        set(oneValueArgs DESTINATION)
        set(multiValueArgs TARGETS CONFIGURATIONS PASSTHRU)
        cmake_parse_arguments(INSTALLSYMS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        if(NOT INSTALLSYMS_DESTINATION AND NOT CMAKE_INSTALL_BINDIR AND NOT CMAKE_INSTALL_LIBDIR)
            message(SEND_ERROR "install_debug_symbols call missing required DESTINATION argument")
            return()
        endif()
        if(NOT INSTALLSYMS_TARGETS)
            message(SEND_ERROR "install_debug_symbols call missing required TARGETS argument")
            return()
        endif()
        if(NOT INSTALLSYMS_CONFIGURATIONS)
            set(INSTALLSYMS_CONFIGURATIONS RelWithDebInfo Debug)
        endif()

        # Wrap each config in a generator expression
        set(HAS_SYMBOLS_CONDITION)
        foreach(_config ${INSTALLSYMS_CONFIGURATIONS})
            list(APPEND HAS_SYMBOLS_CONDITION "$<CONFIG:${_config}>")
        endif()
        # make list comma separated
        string(REPLACE ";" "," HAS_SYMBOLS_CONDITION "${HAS_SYMBOLS_CONDITION}")
        # Wrap in an "OR" generator expression
        set(HAS_SYMBOLS_CONDITION "$<OR:${HAS_SYMBOLS_CONDITION}>")

        set(EXTRA_INSTALL_ARGS ${INSTALLSYMS_PASSTHRU} ${INSTALLSYMS_UNPARSED_ARGUMENTS})
        if(INSTALLSYMS_DESTINATION)
            set(DEST ${INSTALLSYMS_DESTINATION})
        endif()
        foreach(_target ${INSTALLSYMS_TARGETS})
            if(NOT INSTALLSYMS_DESTINATION)
                get_target_property(_target_type ${_target} TYPE)
                if("${_target_type}" STREQUAL "EXECUTABLE" OR "${_target_type}" STREQUAL "SHARED_LIBRARY")
                    # exe or dll: put it alongside the runtime component
                    set(DEST ${CMAKE_INSTALL_BINDIR})
                else()
                    set(DEST ${CMAKE_INSTALL_LIBDIR})
                endif()
            endif()
            install(FILES $<${HAS_SYMBOLS_CONDITION}:$<TARGET_PDB_FILE:${_target}>>
                DESTINATION ${DEST}
                ${EXTRA_INSTALL_ARGS})
        endforeach()
    endfunction()
else()
    function(install_debug_symbols)
        # do nothing if too old of CMake or not MSVC.
    endfunction()
endif()
