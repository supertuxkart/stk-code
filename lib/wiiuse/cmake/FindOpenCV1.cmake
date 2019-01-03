# - Try to find OpenCV library installation
# See http://sourceforge.net/projects/opencvlibrary/
#
# The following variable is optionally searched for defaults
#  OPENCV_ROOT_DIR:            Base directory of OpenCv tree to use.
#
#  OPENCV_NEW_LIBRARY_NAMES   Set to YES before searching if you want to
# The following are set after configuration is done:
#  OPENCV_FOUND
#  OPENCV_INCLUDE_DIRS
#  OPENCV_LIBRARIES
#
# 2004/05 Jan Woetzel, Friso, Daniel Grest
# 2006/01 complete rewrite by Jan Woetzel
# 2006/09 2nd rewrite introducing ROOT_DIR and PATH_SUFFIXES
#   to handle multiple installed versions gracefully by Jan Woetzel
# 2010/02 Ryan Pavlik (Iowa State University) - partial rewrite to standardize
#
# tested with:
# -OpenCV 0.97 (beta5a):  MSVS 7.1, gcc 3.3, gcc 4.1
# -OpenCV 0.99 (1.0rc1):  MSVS 7.1
#
# www.mip.informatik.uni-kiel.de/~jw
# academic.cleardefinition.com
# --------------------------------

set(OPENCV_ROOT_DIR
	"${OPENCV_ROOT_DIR}"
	CACHE
	PATH
	"Path to search for OpenCV")

find_package(OpenCV QUIET NO_MODULE)
if(OpenCV_LIBS AND NOT OpenCV_LIBRARIES)
	set(OPENCV_LIBRARIES ${OpenCV_LIBS})
	set(OPENCV_FOUND true)
else()
	include(ProgramFilesGlob)

	# typical root dirs of installations, exactly one of them is used
	program_files_glob(_dirs "/OpenCV*/")

	#
	# select exactly ONE OPENCV base directory/tree
	# to avoid mixing different version headers and libs
	#
	find_path(OPENCV_BASE_DIR
		NAMES
		cv/include/cv.h
		include/opencv/cv.h
		include/cv/cv.h
		include/cv.h
		HINTS
		"${OPENCV_ROOT_DIR}"
		"$ENV{OPENCV_ROOT_DIR}"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Intel(R) Open Source Computer Vision Library_is1;Inno Setup: App Path]"
		${_dirs})




	# header include dir suffixes appended to OPENCV_BASE_DIR
	set(OPENCV_INCDIR_SUFFIXES
		include
		include/cv
		include/opencv
		cv/include
		cxcore/include
		cvaux/include
		otherlibs/cvcam/include
		otherlibs/highgui
		otherlibs/highgui/include
		otherlibs/_graphics/include)

	# library linkdir suffixes appended to OPENCV_BASE_DIR
	set(OPENCV_LIBDIR_SUFFIXES
		lib
		lib64
		OpenCV/lib
		otherlibs/_graphics/lib)


	#
	# find incdir for each lib
	#
	find_path(OPENCV_cv_INCLUDE_DIR
		NAMES
		cv.h
		HINTS
		"${OPENCV_BASE_DIR}"
		"${OPENCV_ROOT_DIR}"
		PATH_SUFFIXES
		${OPENCV_INCDIR_SUFFIXES})
	find_path(OPENCV_cxcore_INCLUDE_DIR
		NAMES
		cxcore.h
		HINTS
		"${OPENCV_BASE_DIR}"
		"${OPENCV_ROOT_DIR}"
		PATH_SUFFIXES
		${OPENCV_INCDIR_SUFFIXES})
	find_path(OPENCV_cxcore_INCLUDE_DIR
		NAMES
		cvaux.h
		HINTS
		"${OPENCV_BASE_DIR}"
		"${OPENCV_ROOT_DIR}"
		PATH_SUFFIXES
		${OPENCV_INCDIR_SUFFIXES})
	find_path(OPENCV_highgui_INCLUDE_DIR
		NAMES
		highgui.h
		HINTS
		"${OPENCV_BASE_DIR}"
		"${OPENCV_ROOT_DIR}"
		PATH_SUFFIXES
		${OPENCV_INCDIR_SUFFIXES})
	find_path(OPENCV_cvcam_INCLUDE_DIR
		NAMES
		cvcam.h
		HINTS
		"${OPENCV_BASE_DIR}"
		"${OPENCV_ROOT_DIR}"
		PATH_SUFFIXES
		${OPENCV_INCDIR_SUFFIXES})

	#
	# find sbsolute path to all libraries
	# some are optionally, some may not exist on Linux
	#
	find_library(OPENCV_legacy_LIBRARY
		NAMES
		opencv_legacy
		HINTS
		"${OPENCV_BASE_DIR}"
		"${OPENCV_ROOT_DIR}"
		PATH_SUFFIXES
		${OPENCV_LIBDIR_SUFFIXES})

	set(OPENCV_NEW_COMPONENTS calib3d contrib core features2d highgui imgproc legacy ml objdetect video)
	set(OPENCV_OLD_COMPONENTS cv cvaux cvcam cvhaartraining cxcore cxts highgui ml trs)
	set(opencv_components)
	if(OPENCV_NEW_LIBRARY_NAMES OR OPENCV_legacy_LIBRARY)

		# New-style library names
		foreach(component ${OPENCV_NEW_COMPONENTS})
			find_library(OPENCV_${component}_LIBRARY
			NAMES
			opencv_${component}
			HINTS
			${OPENCV_BASE_DIR}
			PATH_SUFFIXES
			${OPENCV_LIBDIR_SUFFIXES})
		endforeach()

		# cv components with header and library if COMPONENTS unspecified
		if(NOT OpenCV_FIND_COMPONENTS)
			# default
			set(opencv_components core legacy imgproc highgui)
			if(WIN32)
				list(APPEND opencv_components video)	# WIN32 only actually
			endif()
		else()
			# TODO: clean up/convert to new components
			string(TOLOWER "${OpenCV_FIND_COMPONENTS}" opencv_components)
		endif()

	else()
		# Old-style lib names
		if(NOT OpenCV_FIND_COMPONENTS)
			# default
			set(opencv_components cv cxcore cvaux highgui)
			if(WIN32)
				list(APPEND opencv_components cvcam)	# WIN32 only actually
			endif()
		else()
			string(TOLOWER "${OpenCV_FIND_COMPONENTS}" opencv_components)
		endif()

		foreach(component ${OPENCV_OLD_COMPONENTS})
			find_library(OPENCV_${component}_LIBRARY
				NAMES
				${component}
				HINTS
				${OPENCV_BASE_DIR}
				PATH_SUFFIXES
				${OPENCV_LIBDIR_SUFFIXES})
		endforeach()
	endif()

	#
	# Logic selecting required libs and headers
	#

	set(_req_check)
	set(_req_libs)
	set(_req_includes)
	foreach(component ${opencv_components})
		#message(STATUS "Component requested: ${component}")

		# only good if header and library both found
		list(APPEND
			_req_check
			OPENCV_${component}_LIBRARY)
		list(APPEND _req_libs "${OPENCV_${component}_LIBRARY}")
		if(DEFINED OPENCV_${component}_INCLUDE_DIR)
			list(APPEND
				_req_check
				OPENCV_${component}_INCLUDE_DIR)
			list(APPEND _req_includes "${OPENCV_${component}_INCLUDE_DIR}")
		endif()


	endforeach()

	# handle the QUIETLY and REQUIRED arguments and set xxx_FOUND to TRUE if
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(OpenCV
		DEFAULT_MSG
		OPENCV_cv_INCLUDE_DIR
		${_req_check})

	if(OPENCV_FOUND)
		set(OPENCV_LIBRARY_DIRS)
		foreach(lib ${_req_libs})
			get_filename_component(dir "${lib}" PATH)
			list(APPEND OPENCV_LIBRARY_DIRS "${dir}")
		endforeach()
		list(REVERSE OPENCV_LIBRARY_DIRS)
		list(REMOVE_DUPLICATES OPENCV_LIBRARY_DIRS)
		list(REVERSE OPENCV_LIBRARY_DIRS)

		set(OPENCV_INCLUDE_DIRS ${_req_includes})
		set(OPENCV_LIBRARIES ${_req_libs})
		mark_as_advanced(OPENCV_ROOT_DIR OpenCV_DIR)
	endif()

	mark_as_advanced(OPENCV_BASE_DIR)
	foreach(component ${OPENCV_NEW_COMPONENTS} ${OPENCV_OLD_COMPONENTS})
		mark_as_advanced(OPENCV_${component}_LIBRARY OPENCV_${component}_INCLUDE_DIR)
	endforeach()
endif()


