# - Script to check if the signature for a mac HID callback uses UInt32 or uint32_t
# Requires that the associated CPP file be present: CheckMacHIDAPI.cpp.
#
#	MACOSX_HID_UINT32T, set according to the results of our test.
#
# Use add_definitions(-DMACOSX_HID_UINT32T=${MACOSX_HID_UINT32T}) in your
# listfile and the following prototype for the function you'd like to
# register using setInterruptReportHandlerCallback:
#  void ReaderReportCallback(
#                            void *target,
#                            IOReturn result,
#                            void *refcon,
#                            void *sender,
#                            MACOSX_HID_UINT32T size
#                            )
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


if(APPLE)
	if(NOT MACOSX_HID_UINT32T)
		get_filename_component(_moddir ${CMAKE_CURRENT_LIST_FILE} PATH)

		try_compile(_HID_uint32t
			${CMAKE_BINARY_DIR}
			${_moddir}/CheckMacHIDAPI.cpp
			OUTPUT_VARIABLE
			_HID_uint32t_OUTPUT
			COMPILE_DEFINITIONS
			-DMACOSX_HID_UINT32T=uint32_t)
		message(STATUS
			"Checking uint32_t in HID callback signature... ${_HID_uint32t}")

		try_compile(_HID_UInt32
			${CMAKE_BINARY_DIR}
			${_moddir}/CheckMacHIDAPI.cpp
			OUTPUT_VARIABLE
			_HID_UInt32_OUTPUT
			COMPILE_DEFINITIONS
			-DMACOSX_HID_UINT32T=UInt32)
		message(STATUS
			"Checking UInt32 in HID callback signature... ${_HID_UInt32}")


		if(_HID_uint32t)
			set(MACOSX_HID_UINT32T
				"uint32_t"
				CACHE
				STRING
				"The 32-bit uint type desired in the callback set by setInterruptReportHandlerCallback")
			mark_as_advanced(MACOSX_HID_UINT32T)
		elseif(_HID_UInt32)
			set(MACOSX_HID_UINT32T
				"UInt32"
				CACHE
				STRING
				"The 32-bit uint type desired in the callback set by setInterruptReportHandlerCallback")
			mark_as_advanced(MACOSX_HID_UINT32T)
		else()
			message(SEND_ERROR
				"ERROR: Could not detect appropriate Mac HID uint32 type!")
		endif()

	endif()
endif()
