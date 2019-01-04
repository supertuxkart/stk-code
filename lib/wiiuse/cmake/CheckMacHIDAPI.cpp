/**
 * \file CheckMacHIDAPI.cpp
 * \brief C++ source file used by CMake module CheckMacHIDAPI.cmake
 *
 * \author
 * Ryan Pavlik, 2009-2010
 * <rpavlik@iastate.edu>
 * http://academic.cleardefinition.com/
 *
 * \author
 * Based on code extracted from VRPN 07.22 for use as a minimal test case
 *
 * Attempts to compile a difficult bit of code against the Mac
 * HID API, as two different types have been required in the callback
 * function (UInt32 and uint32_t) and testing is the best way to know
 * which one is correct for a given system.
 *
 */


#if defined(__APPLE__)

#include <stdio.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <CoreFoundation/CoreFoundation.h>
void ReaderReportCallback(
						  void *target, IOReturn result, void *refcon, void *sender, MACOSX_HID_UINT32T size
						  )
						  {}
#endif

int main(int argc, char* argv[]) {
#if defined(__APPLE__)
	io_object_t _ioObject;
	IOHIDDeviceInterface122 **_interface;
	unsigned char _buffer[512];
	IOReturn result = (*_interface)->setInterruptReportHandlerCallback(_interface,
															  _buffer, 512,
															  ReaderReportCallback,
																	   NULL, 0);
#endif
	return 0;
}
