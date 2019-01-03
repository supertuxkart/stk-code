# - try to find Haption VirtuoseAPI C++ wrapper include files
#
# Use of this header depends on the VirtuoseAPI, so we search for that too.
#
#  VIRTUOSEVPP_INCLUDE_DIRS, where to find headers
#  VIRTUOSEVPP_LIBRARIES, the libraries to link against
#  VIRTUOSEVPP_FOUND, If false, do not try to use this library
#  VIRTUOSEVPP_RUNTIME_LIBRARY_DIRS, path to DLL/SO for runtime use.
#  VIRTUOSEAPI_RUNTIME_LIBRARIES, runtime libraries you might want to install

set(VIRTUOSEVPP_ROOT_DIR
	"${VIRTUOSEVPP_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for VirtuoseAPI VPP wrapper")

find_package(VirtuoseAPI)

find_path(VIRTUOSEVPP_INCLUDE_DIR
	vpp.h
	PATHS
	${_dirs}
	HINTS
	"${VIRTUOSEVPP_ROOT_DIR}")

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VirtuoseVPP
	DEFAULT_MSG
	VIRTUOSEVPP_INCLUDE_DIR
	VIRTUOSEAPI_LIBRARY
	VIRTUOSEAPI_RUNTIME_LIBRARIES
	VIRTUOSEAPI_RUNTIME_LIBRARY_DIRS
	VIRTUOSEAPI_INCLUDE_DIR)

if(VIRTUOSEVPP_FOUND)
	set(VIRTUOSEVPP_LIBRARIES "${VIRTUOSEAPI_LIBRARY}")
	set(VIRTUOSEVPP_INCLUDE_DIRS
		"${VIRTUOSEAPI_INCLUDE_DIR}"
		"${VIRTUOSEVPP_INCLUDE_DIR}")
	set(VIRTUOSEVPP_RUNTIME_LIBRARIES "${VIRTUOSEAPI_RUNTIME_LIBRARIES}")
	set(VIRTUOSEVPP_RUNTIME_LIBRARY_DIRS
		"${VIRTUOSEAPI_RUNTIME_LIBRARY_DIRS}")

	mark_as_advanced(VIRTUOSEVPP_ROOT_DIR)
endif()

mark_as_advanced(VIRTUOSEVPP_INCLUDE_DIR)
