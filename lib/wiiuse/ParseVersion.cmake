set(_version_file "${CMAKE_CURRENT_SOURCE_DIR}/src/include/wiiuse.h")
if(EXISTS "${_version_file}")
	file(READ "${_version_file}" _version_contents)
endif()

if("${_version_contents}" MATCHES "WIIUSE_MAJOR ([0-9]+)")
	set(CPACK_PACKAGE_VERSION_MAJOR "${CMAKE_MATCH_1}")
else()
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	message("Could not parse major version from wiiuse.h")
endif()

if("${_version_contents}" MATCHES "WIIUSE_MINOR ([0-9]+)")
	set(CPACK_PACKAGE_VERSION_MINOR "${CMAKE_MATCH_1}")
else()
	set(CPACK_PACKAGE_VERSION_MINOR "13")
	message("Could not parse minor version from wiiuse.h")
endif()

if("${_version_contents}" MATCHES "WIIUSE_MICRO ([0-9]+)")
	set(CPACK_PACKAGE_VERSION_MICRO "${CMAKE_MATCH_1}")
else()
	set(CPACK_PACKAGE_VERSION_MICRO "0")
	message("Could not parse micro version from wiiuse.h")
endif()

set(CPACK_PACKAGE_VERSION
	"${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_MICRO}")