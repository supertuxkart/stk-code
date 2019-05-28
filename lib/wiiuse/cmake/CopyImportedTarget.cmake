# - Copy shared libraries from imported targets to the target build directory
# on Windows during post-build. Install them in all cases.
#
#  copy_imported_targets(<target_name> [<imported target name> ...])
#
#  install_imported_target(<imported target name> <arguments to pass to install(FILES))
#
# Likely requires CMake 2.8.12 or newer to work well.
#
# Original Author:
# 2015 Ryan Pavlik <ryan.pavlik@gmail.com> <abiryan@ryand.net>
#
# Copyright Sensics, Inc. 2015.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

function(copy_imported_targets _target)
    foreach(_dep ${ARGN})
        if(WIN32)
            add_custom_command(TARGET ${_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${_dep}> $<TARGET_FILE_DIR:${_target}>
                COMMENT "Copying required DLL for dependency ${_dep}"
                VERBATIM)
        endif()
    endforeach()
endfunction()


function(install_imported_target _dep)
    install(FILES $<TARGET_FILE:${_dep}> ${ARGN})
endfunction()
