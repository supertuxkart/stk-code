# - Try to find LAPACK and BLAS libraries
# Once done, this will define
#  LAPACKLIBS_LIBRARIES, all libraries to link against
#  LAPACKLIBS_FOUND, If false, do not try to use LAPACK library features.
#
# Users may wish to set:
#  LAPACKLIBS_ROOT_DIR, location to start searching for LAPACK libraries
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(_check)

set(LAPACKLIBS_ROOT_DIR
	"${LAPACKLIBS_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search for LAPACK libraries")

if(APPLE)
	find_library(LAPACKLIBS_VECLIB_FRAMEWORK veclib)
	find_library(LAPACKLIBS_ACCELERATE_FRAMEWORK accelerate)
	mark_as_advanced(LAPACKLIBS_VECLIB_FRAMEWORK
		LAPACKLIBS_ACCELERATE_FRAMEWORK)

	set(LAPACKLIBS_LIBRARIES
		"${LAPACKLIBS_VECLIB_FRAMEWORK}"
		"${LAPACKLIBS_ACCELERATE_FRAMEWORK}")
	list(APPEND
		_check
		LAPACKLIBS_VECLIB_FRAMEWORK
		LAPACKLIBS_ACCELERATE_FRAMEWORK)
elseif(WIN32)
	# Tested to work with the files from http://www.fi.muni.cz/~xsvobod2/misc/lapack/
	# You might also see http://icl.cs.utk.edu/lapack-for-windows/clapack/index.html for
	# the libraries and headers.

	# Good luck!

	find_library(LAPACKLIBS_LAPACK_LIBRARY
		NAMES
		lapack_win32_MT
		lapack
		lapackd
		HINTS
		${LAPACKLIBS_ROOT_DIR}
		PATH_SUFFIXES
		lapack-MT-release
		lapack-MT-debug
		lib)
	find_library(LAPACKLIBS_BLAS_LIBRARY
		NAMES
		blas_win32_MT
		blas
		blasd
		HINTS
		${LAPACKLIBS_ROOT_DIR}
		PATH_SUFFIXES
		lapack-MT-release
		lapack-MT-debug
		lib)
	set(LAPACKLIBS_LIBRARIES
		"${LAPACKLIBS_LAPACK_LIBRARY}"
		"${LAPACKLIBS_BLAS_LIBRARY}")
	list(APPEND _check LAPACKLIBS_LAPACK_LIBRARY LAPACKLIBS_BLAS_LIBRARY)
elseif(UNIX)
	# All other Linux/Unix should have lapack without a fuss
	list(APPEND _check LAPACKLIBS_LAPACK_LIBRARY)
	find_library(LAPACKLIBS_LAPACK_LIBRARY lapack)
	set(LAPACKLIBS_LIBRARIES "${LAPACKLIBS_LAPACK_LIBRARY}")
endif()

# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LAPACKLibs
	DEFAULT_MSG
	${_check})

if(LAPACKLIBS_FOUND)
	mark_as_advanced(LAPACKLIBS_ROOT_DIR
		LAPACKLIBS_LAPACK_LIBRARY
		LAPACKLIBS_BLAS_LIBRARY)
endif()
