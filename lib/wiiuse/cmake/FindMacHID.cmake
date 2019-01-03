# - try to find Mac HID frameworks
#
# Cache Variables: (probably not for direct use in your scripts)
#  MACHID_CoreFoundation_LIBRARY
#  MACHID_CoreFoundation_INCLUDE_DIR
#  MACHID_IOKit_LIBRARY
#  MACHID_IOKit_INCLUDE_DIR
#  MACOSX_HID_UINT32T  (from CheckMacHIDAPI)
#
# Non-cache variables you should use in your CMakeLists.txt:
#  MACHID_DEFINITIONS
#  MACHID_LIBRARIES
#  MACHID_INCLUDE_DIRS
#  MACHID_FOUND - if this is not true, do not attempt to use this library
#
# Requires these CMake modules:
#  CheckMacHIDAPI
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
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
	find_library(MACHID_CoreFoundation_LIBRARY CoreFoundation)
	find_path(MACHID_CoreFoundation_INCLUDE_DIR
		CoreFoundation/CoreFoundation.h)

	find_library(MACHID_IOKit_LIBRARY IOKit)
	find_path(MACHID_IOKit_INCLUDE_DIR IOKit/hid/IOHIDLib.h)

	include(CheckMacHIDAPI)
	set(MACHID_DEFINITIONS "-DMACOSX_HID_UINT32T=${MACOSX_HID_UINT32T}")

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(MacHID
		DEFAULT_MSG
		MACHID_CoreFoundation_LIBRARY
		MACHID_CoreFoundation_INCLUDE_DIR
		MACHID_IOKit_LIBRARY
		MACHID_IOKit_INCLUDE_DIR
		MACOSX_HID_UINT32T)

endif()

if(MACHID_FOUND)
	set(MACHID_LIBRARIES
		"${MACHID_CoreFoundation_LIBRARY}"
		"${MACHID_IOKit_LIBRARY}")

	set(MACHID_INCLUDE_DIRS
		"${MACHID_CoreFoundation_INCLUDE_DIR}"
		"${MACHID_IOKit_INCLUDE_DIR}")

	mark_as_advanced(MACHID_CoreFoundation_LIBRARY
		MACHID_CoreFoundation_INCLUDE_DIR
		MACHID_IOKit_LIBRARY
		MACHID_IOKit_INCLUDE_DIR)

endif()
