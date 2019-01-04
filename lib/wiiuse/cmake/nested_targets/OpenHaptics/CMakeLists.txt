# - Build the OpenHaptics utility libraries as a part of your solution.
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file ../../LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(WIN32)
	# Work around bug in library at least <= 3.10
	add_definitions(-DWIN32)
endif()
include_directories("${HDAPI_HDU_INCLUDE_DIR}"
	"${HDAPI_INCLUDE_DIR}"
	"${HLAPI_HLU_INCLUDE_DIR}"
	"${HLAPI_INCLUDE_DIR}")

if(HDAPI_HDU_LIBRARY STREQUAL "openhaptics_hdu_nested_target")
	add_library(openhaptics_hdu_nested_target
		STATIC
		EXCLUDE_FROM_ALL
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduBoundBox.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduError.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduGenericMatrix.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hdu.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduHapticDevice.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduLine.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduLineSegment.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduMath.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduMatrix.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduPlane.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduQuaternion.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduRecord.h"
		"${HDAPI_HDU_INCLUDE_DIR}/HDU/hduVector.h"
		"${HDAPI_HDU_SOURCE_DIR}/hdu.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduAfx.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduAfx.h"
		"${HDAPI_HDU_SOURCE_DIR}/hduDecompose.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduDecompose.h"
		"${HDAPI_HDU_SOURCE_DIR}/hduError.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduHapticDevice.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduLine.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduLineSegment.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduMatrix.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduPlane.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduQuaternion.cpp"
		"${HDAPI_HDU_SOURCE_DIR}/hduRecord.cpp")
	target_link_libraries(openhaptics_hdu_nested_target ${HDAPI_LIBRARY})
	set_property(TARGET
		openhaptics_hdu_nested_target
		PROPERTY
		PROJECT_LABEL
		"OpenHaptics HDU Library")

	if(MSVC)
		# Disable warnings - no sense in warning about Sensable's potential bugs.
		set_property(TARGET
			openhaptics_hdu_nested_target
			PROPERTY
			COMPILE_FLAGS
			"/wd4189 /wd4701")
	endif()
endif()


if(HLAPI_HLU_LIBRARY STREQUAL "openhaptics_hlu_nested_target")
	add_library(openhaptics_hlu_nested_target
		STATIC
		EXCLUDE_FROM_ALL
		"${HLAPI_HLU_INCLUDE_DIR}/HLU/hlu.h"
		"${HLAPI_HLU_SOURCE_DIR}/hlu.cpp"
		"${HLAPI_HLU_SOURCE_DIR}/hluAfx.cpp"
		"${HLAPI_HLU_SOURCE_DIR}/hluAfx.h")
	target_link_libraries(openhaptics_hlu_nested_target ${HLAPI_LIBRARY} ${HDAPI_LIBRARY})

	set_property(TARGET
		openhaptics_hlu_nested_target
		PROPERTY
		PROJECT_LABEL
		"OpenHaptics HLU Library")

	if(MSVC)
		# Disable warnings - no sense in warning about Sensable's potential bugs.
		set_property(TARGET
			openhaptics_hlu_nested_target
			APPEND
			PROPERTY
			COMPILE_FLAGS
			/wd4189)
	endif()
endif()
