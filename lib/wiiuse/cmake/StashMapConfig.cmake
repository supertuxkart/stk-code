# Manipulate CMAKE_MAP_IMPORTED_CONFIG_ cautiously and reversibly.
#
# In all usage docs, <config> is a configuration name in all caps (RELEASE, DEBUG,
# RELWITHDEBINFO, MINSIZEREL, and NONE are the ones made by default - NONE is how
# targets are exported from single-config generators where CMAKE_BUILD_TYPE isn't set.)
#
#  stash_map_config(<config> <new list of configs for map imported>) and unstash_map_config(<config>)
#
# Saves+changes and restores the value (or unset-ness) of CMAKE_MAP_IMPORTED_CONFIG_${config}.
# Re-entrant calls OK - this does actually "push" and "pop"
#
#  stash_common_map_config() and unstash_common_map_config()
#
# Calls stash_map_config/unstash_map_config for each configuration with sensible
# defaults based on the platform.
#
# Original Author:
# 2015, 2017 Ryan Pavlik <ryan@sensics.com> <abiryan@ryand.net>
# http://ryanpavlik.com
#
# Copyright Sensics, Inc. 2015, 2017.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

macro(stash_map_config config)
    string(TOUPPER "${config}" smc_config)
    string(TOUPPER "${ARGN}" smc_new)
    # Re-entrancy protection - push an entry onto a list
    list(APPEND smc_IN_MAP_CONFIG_STASH_${smc_config} yes)
    list(LENGTH smc_IN_MAP_CONFIG_STASH_${smc_config} smc_IN_MAP_CONFIG_STASH_LEN)

    # Actually perform the saving and replacement of CMAKE_MAP_IMPORTED_CONFIG_${config}
    set(smc_var smc_OLD_CMAKE_MAP_IMPORTED_CONFIG_${smc_config}_${smc_IN_MAP_CONFIG_STASH_LEN})
    message(STATUS "Stashing to ${smc_var}")
    if(DEFINED CMAKE_MAP_IMPORTED_CONFIG_${smc_config})
        set(${smc_var} ${CMAKE_MAP_IMPORTED_CONFIG_${smc_config}})
    else()
        unset(${smc_var})
    endif()
    set(CMAKE_MAP_IMPORTED_CONFIG_${smc_config} ${smc_new})
endmacro()

macro(unstash_map_config config)
    string(TOUPPER "${config}" smc_config)
    if(NOT DEFINED smc_IN_MAP_CONFIG_STASH_${smc_config})
        # Nobody actually called the matching stash...
        return()
    endif()
    # Get stack size so we know which entries to restore.
    list(LENGTH smc_IN_MAP_CONFIG_STASH_${smc_config} smc_IN_MAP_CONFIG_STASH_LEN)
    # Other half of re-entrancy protection - pop an entry off a list
    list(REMOVE_AT smc_IN_MAP_CONFIG_STASH_${smc_config} -1)

    # Restoration of CMAKE_MAP_IMPORTED_CONFIG_${config}
    set(smc_var smc_OLD_CMAKE_MAP_IMPORTED_CONFIG_${smc_config}_${smc_IN_MAP_CONFIG_STASH_LEN})
    if(DEFINED ${smc_var})
        set(CMAKE_MAP_IMPORTED_CONFIG_${smc_config} ${${smc_var}})
        unset(${smc_var})
    else()
        unset(CMAKE_MAP_IMPORTED_CONFIG_${smc_config})
    endif()
endmacro()

macro(stash_common_map_config)
    if(MSVC)
        # Can't do this - different runtimes, incompatible ABI, etc.
        set(smc_DEBUG_FALLBACK)
        stash_map_config(DEBUG DEBUG)
    else()
        set(smc_DEBUG_FALLBACK DEBUG)
        stash_map_config(DEBUG DEBUG RELWITHDEBINFO RELEASE MINSIZEREL NONE)
    endif()
    stash_map_config(RELEASE RELEASE RELWITHDEBINFO MINSIZEREL NONE ${smc_DEBUG_FALLBACK})
    stash_map_config(RELWITHDEBINFO RELWITHDEBINFO RELEASE MINSIZEREL NONE ${smc_DEBUG_FALLBACK})
    stash_map_config(MINSIZEREL MINSIZEREL RELEASE RELWITHDEBINFO NONE ${smc_DEBUG_FALLBACK})
    stash_map_config(NONE NONE RELEASE RELWITHDEBINFO MINSIZEREL ${smc_DEBUG_FALLBACK})
endmacro()

macro(unstash_common_map_config)
    unstash_map_config(DEBUG)
    unstash_map_config(RELEASE)
    unstash_map_config(RELWITHDEBINFO)
    unstash_map_config(MINSIZEREL)
    unstash_map_config(NONE)
endmacro()
