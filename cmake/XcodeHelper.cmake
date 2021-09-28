# Collect all currently added targets in all subdirectories
#
# Parameters:
# - _result the list containing all found targets
# - _dir root directory to start looking from
function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()

    get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

# set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS doesn't work in ios toolchain, below fixed it
macro(configure_xcode_defaults _EXE_NAME)

if (APPLE)
    macro(set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
        set_property(TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
    endmacro()
endif()

if (APPLE)
    set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] YES)
    set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=MinSizeRel] YES)
    set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=RelWithDebInfo] YES)
    set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Release] YES)
endif()
endmacro()
