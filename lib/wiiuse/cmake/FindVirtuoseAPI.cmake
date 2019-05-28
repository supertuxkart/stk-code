# - try to find Haption VirtuoseAPI library and include files
#
#  VIRTUOSEAPI_INCLUDE_DIRS, where to find headers
#  VIRTUOSEAPI_LIBRARIES, the libraries to link against
#  VIRTUOSEAPI_FOUND, If false, do not try to use this library
#  VIRTUOSEAPI_RUNTIME_LIBRARY_DIRS, path to DLL/SO for runtime use.
#  VIRTUOSEAPI_RUNTIME_LIBRARIES, runtime libraries you might want to install

set(VIRTUOSEAPI_ROOT_DIR
	"${VIRTUOSEAPI_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for VirtuoseAPI")

set(_dirs)
if(WIN32)
	include(ProgramFilesGlob)
	program_files_fallback_glob(_dirs "/VirtuoseAPI_v*/")
endif()

find_path(VIRTUOSEAPI_INCLUDE_DIR
	virtuoseAPI.h
	VirtuoseAPI.h
	PATHS
	${_dirs}
	HINTS
	"${VIRTUOSEAPI_ROOT_DIR}")

set(_suffixes)
if(WIN32)
	set(_lib_name virtuoseDLL)
	set(_runtime_name virtuoseAPI.dll)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_suffixes win64)
	else()
		set(_suffixes win32)
	endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(_lib_name virtuose)
	set(_runtime_name virtuoseAPI.so)

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(_suffixes linux-64b)
	else()
		set(_suffixes linux linux-2.6)
	endif()
endif()

if(_suffixes)
	find_library(VIRTUOSEAPI_LIBRARY
		NAMES
		${_lib_name}
		PATHS
		${_dirs}
		HINTS
		"${VIRTUOSEAPI_ROOT_DIR}"
		PATH_SUFFIXES
		${_suffixes})
	find_file(VIRTUOSEAPI_RUNTIME_LIBRARY
		NAMES
		${_runtime_name}
		PATHS
		${_dirs}
		HINTS
		"${VIRTUOSEAPI_ROOT_DIR}"
		PATH_SUFFIXES
		${_suffixes})
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VirtuoseAPI
	DEFAULT_MSG
	VIRTUOSEAPI_LIBRARY
	VIRTUOSEAPI_RUNTIME_LIBRARY
	VIRTUOSEAPI_INCLUDE_DIR)

if(VIRTUOSEAPI_FOUND)
	set(VIRTUOSEAPI_LIBRARIES "${VIRTUOSEAPI_LIBRARY}")
	set(VIRTUOSEAPI_RUNTIME_LIBRARIES "${VIRTUOSEAPI_RUNTIME_LIBRARY}")
	set(VIRTUOSEAPI_INCLUDE_DIRS "${VIRTUOSEAPI_INCLUDE_DIR}")
	get_filename_component(VIRTUOSEAPI_RUNTIME_LIBRARY_DIRS
		"${VIRTUOSEAPI_RUNTIME_LIBRARY}"
		PATH)

	mark_as_advanced(VIRTUOSEAPI_ROOT_DIR)
endif()

mark_as_advanced(VIRTUOSEAPI_LIBRARY
	VIRTUOSEAPI_RUNTIME_LIBRARY
	VIRTUOSEAPI_INCLUDE_DIR)
