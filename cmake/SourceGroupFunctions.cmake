# Generate source groups which mimic the original folder hierarchy.
# This is mainly useful for MSVC's project explorer
# - SRCS list of source files
# - HDRS list of header files
function(source_group_hierarchy SRCS HDRS)
    if(MSVC)
        # This removes the 'Source Files' folder, which is
        # not really necessary. Also, put header and source
        # files into the same folder
        foreach(source_file ${${SRCS}})
            source_group_file(${source_file} "")
        endforeach()
        foreach(header_file ${${HDRS}})
            source_group_file(${header_file} "")
        endforeach()
    else()
        foreach(source_file ${${SRCS}})
            source_group_file(${source_file} "Source Files\\")
        endforeach()
        foreach(header_file ${${HDRS}})
            source_group_file(${header_file} "Source Files\\")
        endforeach()
    endif()

endfunction()

# Determine source_group depending on file path
# - FILE path to a file (header or source)
# - GROUP_PREFIX prefix for group name
function(source_group_file file group_prefix)
    get_filename_component(file_path ${file} PATH)
    if(${file_path} STREQUAL "src")
        source_group("${group_prefix}" FILES ${file})
    else()
        string(REGEX REPLACE "^src/(.*)$" "\\1" group_name ${file_path})
        string(REPLACE "/" "\\\\" group_name ${group_name})
        source_group("${group_prefix}${group_name}" FILES ${file})
    endif()
endfunction()
