# - Contains a function to sensibly and easily enable the "USE_FOLDERS" global property
# without burning people using old MSVC Express Editions.
#
#  use_folders([option_name]) - Creates an option (default name if you don't pass
#    one: BUILD_WITH_PROJECT_FOLDERS) that controls the USE_FOLDERS global property.
#    It has intelligently-set defaults that err on the side of caution (disabling)
#    on old MSVC versions, since solutions generated with USE_FOLDERS set to ON
#    cannot be used in some older MSVC Express Editions, so it's explicit opt-in there.
#
# Original Author:
# 2015 Ryan Pavlik <ryan@sensics.com> <abiryan@ryand.net>
# http://academic.cleardefinition.com
#
# Copyright Sensics, Inc. 2015.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

function(use_folders)
    set(_option_name BUILD_WITH_PROJECT_FOLDERS)
    if(ARGV0)
        set(_option_name ${ARGV0})
    endif()
    # Nitpicky TODO: This unnecessarily defaults to project folders off when using
    # an older toolset in a newer IDE...
    if(MSVC_IDE AND MSVC_VERSION LESS 1600)
        # VS 2012 Express and newer has folder support...
        option(${_option_name} "Enable project folders in the IDE. May only work in non-Express Editions!" OFF)
    else()
        option(${_option_name} "Enable project folders in the IDE." ON)
    endif()
    set_property(GLOBAL PROPERTY
        USE_FOLDERS ${${_option_name}})
endfunction()