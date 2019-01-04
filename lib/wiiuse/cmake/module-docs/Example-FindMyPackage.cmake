# - try to find MyPackage library
#
# Example-FindMyPackage.cmake
#
# This example is for a fairly in-depth library that has four
# internal dependencies as well as an external dependency.
# The dependency tree is described below, in graphviz/dot format, and you
# can remove the #'s from the following lines and run it through graphviz,
# with this command: dot dependencies.dot -O -Tpdf
#
# --- start of dependencies.dot ---
# digraph {
#	BLAS;
#	subgraph cluster_mypackage {
#		label = "Components that are part of MyPackage";
#		libmypackagecore -> libmypackagea;
#		libmypackagea -> libmypackageb;
#		libmypackageb -> libmypackagec;
#		libmypackagec -> BLAS;
#	}
# }
# --- end of dependencies.dot ---
#
# Because our imaginary component "c" requires BLAS and BLAS needs some
# linker flags, MYPACKAGE_..._LINKER_FLAGS joins the usual group of
# _LIBRARY/_LIBRARIES and _INCLUDE_DIR/_INCLUDE_DIRS variables.  If
# you don't use a library like that, you don't need to include the
# lines dealing with that group of variables.
#
# Most library aren't nearly this complex - but some are, and many
# have some parts of the complexity handled here.
#
# Start of what would be a minimal module documentation block:
#
# Cache Variables: (not for direct use in CMakeLists.txt)
#  MYPACKAGE_ROOT
#  MYPACKAGE_LIBRARY
#  MYPACKAGE_INCLUDE_DIR
#  MYPACKAGE_a_LIBRARY
#  MYPACKAGE_a_INCLUDE_DIR
#  MYPACKAGE_b_LIBRARY
#  MYPACKAGE_b_INCLUDE_DIR
#  MYPACKAGE_c_LIBRARY
#  MYPACKAGE_c_INCLUDE_DIR
#
# Non-cache variables you might use in your CMakeLists.txt:
#  MYPACKAGE_FOUND
#
#  MYPACKAGE_LIBRARIES
#  MYPACKAGE_INCLUDE_DIRS
#  MYPACKAGE_LINKER_FLAGS
#
#  MYPACKAGE_a_LIBRARIES
#  MYPACKAGE_a_INCLUDE_DIRS
#  MYPACKAGE_a_LINKER_FLAGS
#
#  MYPACKAGE_b_LIBRARIES
#  MYPACKAGE_b_INCLUDE_DIRS
#  MYPACKAGE_b_LINKER_FLAGS
#
#  MYPACKAGE_c_LIBRARIES
#  MYPACKAGE_c_INCLUDE_DIRS
#  MYPACKAGE_c_LINKER_FLAGS
#
# Use this module this way:
#  find_package(MyPackage)
#  include_directories(${MYPACKAGE_INCLUDE_DIRS})
#  add_executable(myapp ${SOURCES})
#  target_link_libraries(myapp ${MYPACKAGE_LIBRARIES})
#  set_property(TARGET myapp PROPERTY LINK_FLAGS ${MYPACKAGE_LINKER_FLAGS})
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (CMake standard module)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC

set(MYPACKAGE_ROOT
	"${MYPACKAGE_ROOT}"
	CACHE
	PATH
	"Root directory to look in")

find_library(MYPACKAGE_LIBRARY
	NAMES
	mypackagecore
	PATHS
	"${MYPACKAGE_ROOT}"
	PATH_SUFFIXES
	lib)

find_path(MYPACKAGE_INCLUDE_DIR
	NAMES
	mypackage/mypackage.h
	PATHS
	"${MYPACKAGE_ROOT}"
	PATH_SUFFIXES
	include)

# Assuming that the components are named libmypackagea, libmypackageb, etc
foreach(lib a b c)
	find_library(MYPACKAGE_${lib}_LIBRARY
		NAMES
		mypackage${lib}
		PATHS
		"${MYPACKAGE_ROOT}"
		PATH_SUFFIXES
		lib)

	find_path(MYPACKAGE_${lib}_INCLUDE_DIR
		NAMES
		mypackage/${lib}/${lib}.h
		PATHS
		"${MYPACKAGE_ROOT}"
		PATH_SUFFIXES
		include)

endforeach()

# Searching for dependencies here - always quiet.
# see /usr/share/cmake-2.x/Modules/FindBLAS.cmake for the variables this will define
if(NOT BLAS_FOUND)
	find_package(BLAS QUIETLY)
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MyPackage
	DEFAULT_MSG
	MYPACKAGE_LIBRARY
	MYPACKAGE_INCLUDE_DIR
	MYPACKAGE_a_LIBRARY
	MYPACKAGE_a_INCLUDE_DIR
	MYPACKAGE_b_LIBRARY
	MYPACKAGE_b_INCLUDE_DIR
	MYPACKAGE_c_LIBRARY
	MYPACKAGE_c_INCLUDE_DIR
	BLAS_FOUND)

if(MYPACKAGE_FOUND)
	# Set variables containing libraries and their dependencies
	# Always use the plural form for the variables defined by other find modules:
	# they might have dependencies too!

	set(MYPACKAGE_c_LIBRARIES ${MYPACKAGE_c_LIBRARY} ${BLAS_LIBRARIES})
	set(MYPACKAGE_c_INCLUDE_DIRS ${MYPACKAGE_c_INCLUDE_DIR})	# No include dir for BLAS?
	set(MYPACKAGE_c_LINKER_FLAGS ${BLAS_LINKER_FLAGS})

	set(MYPACKAGE_b_LIBRARIES
		${MYPACKAGE_b_LIBRARY}
		${MYPACKAGE_c_LIBRARIES})
	set(MYPACKAGE_b_INCLUDE_DIRS
		${MYPACKAGE_b_INCLUDE_DIR}
		${MYPACKAGE_c_INCLUDE_DIRS})
	set(MYPACKAGE_b_LINKER_FLAGS ${MYPACKAGE_c_LINKER_FLAGS})

	set(MYPACKAGE_a_LIBRARIES
		${MYPACKAGE_a_LIBRARY}
		${MYPACKAGE_b_LIBRARIES})
	set(MYPACKAGE_a_INCLUDE_DIRS
		${MYPACKAGE_a_INCLUDE_DIR}
		${MYPACKAGE_b_INCLUDE_DIRS})
	set(MYPACKAGE_a_LINKER_FLAGS ${MYPACKAGE_b_LINKER_FLAGS})

	set(MYPACKAGE_LIBRARIES ${MYPACKAGE_LIBRARY} ${MYPACKAGE_a_LIBRARIES})
	set(MYPACKAGE_INCLUDE_DIRS
		${MYPACKAGE_INCLUDE_DIR}
		${MYPACKAGE_a_INCLUDE_DIRS})
	set(MYPACKAGE_LINKER_FLAGS ${MYPACKAGE_a_LINKER_FLAGS})

endif()

mark_as_advanced(MYPACKAGE_LIBRARY
	MYPACKAGE_INCLUDE_DIR
	MYPACKAGE_a_LIBRARY
	MYPACKAGE_a_INCLUDE_DIR
	MYPACKAGE_b_LIBRARY
	MYPACKAGE_b_INCLUDE_DIR
	MYPACKAGE_c_LIBRARY
	MYPACKAGE_c_INCLUDE_DIR)

if(MYPACKAGE_FOUND)
	mark_as_advanced(MYPACKAGE_ROOT)
endif()

# End of Example-FindMyPackage.cmake
