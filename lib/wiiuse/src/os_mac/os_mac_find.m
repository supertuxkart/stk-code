/*
 *  wiiuse
 *
 *  Written By:
 *    Michael Laforest  < para >
 *    Email: < thepara (--AT--) g m a i l [--DOT--] com >
 *
 *  Copyright 2006-2007
 *
 *  This file is part of wiiuse.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  $Header$
 *
 */

/**
 *  @file
 *  @brief Handles device I/O for Mac OS X.
 */

#ifdef __APPLE__

#import "os_mac.h"

#import "../io.h"
#import "../events.h"
#import "../os.h"

#import <IOBluetooth/IOBluetoothUtilities.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothHostController.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#if !WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE
#import <IOBluetooth/IOBluetoothUserLib.h> // IOBluetoothLocalDeviceGetPowerState
#endif


#pragma mark -
#pragma mark WiiuseDeviceInquiry

#if WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE
@interface WiiuseDeviceInquiry : NSObject<IOBluetoothDeviceInquiryDelegate> {
#else
@interface WiiuseDeviceInquiry : NSObject {
#endif
	wiimote** wiimotes;
	NSUInteger maxDevices;
	int timeout;
	
	BOOL _running;
	NSUInteger _foundDevices;
	BOOL _inquiryComplete;
}

- (id) initWithMemory:(wiimote**)wiimotes maxDevices:(int)maxDevices timeout:(int)timeout;
- (int) run;

@end

@implementation WiiuseDeviceInquiry

- (id) initWithMemory:(wiimote**)wiimotes_ maxDevices:(int)maxDevices_ timeout:(int)timeout_ {
	self = [super init];
	if(self) {

		BluetoothHCIPowerState powerState = kBluetoothHCIPowerStateUnintialized;
#if WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE
		if([IOBluetoothHostController defaultController])
			powerState = [IOBluetoothHostController defaultController].powerState;
#else
		// yes it is deprecated. no, there is no alternative (on 10.6).
		IOBluetoothLocalDeviceGetPowerState(&powerState);
#endif
		if (![IOBluetoothHostController defaultController] ||
			powerState != kBluetoothHCIPowerStateON)
		{
			WIIUSE_DEBUG("Bluetooth hardware not available.");
			[self release];
			self = nil;
		} else {
			wiimotes = wiimotes_;
			maxDevices = maxDevices_;
			timeout = timeout_;
			_running = NO;
		}
	}
	return self;
}

// creates and starts inquiry. the returned object is in the current autorelease pool.
- (IOBluetoothDeviceInquiry*) start {
	
	// reset state variables
	_foundDevices = 0;
	_inquiryComplete = NO;
	
	// create inquiry
	IOBluetoothDeviceInquiry* inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate: self];
	
	// refine search & set timeout
	[inquiry setSearchCriteria:kBluetoothServiceClassMajorAny
			  majorDeviceClass:WM_DEV_MAJOR_CLASS
			  minorDeviceClass:kBluetoothDeviceClassMinorAny];
	[inquiry setUpdateNewDeviceNames: NO];
	if(timeout > 0)
		[inquiry setInquiryLength:timeout];
	
	// start inquiry
	IOReturn status = [inquiry start];
	if (status != kIOReturnSuccess) {
		WIIUSE_ERROR("Unable to start bluetooth device inquiry.");
		if(![inquiry stop]) {
			WIIUSE_ERROR("Unable to stop bluetooth device inquiry.");
		} else {
			WIIUSE_DEBUG("Bluetooth device inquiry stopped.");
		}
		return nil;
	}
	
	return inquiry;
}

- (void) wait {
	// wait for the inquiry to complete
	NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
	while(!_inquiryComplete &&
		  [runLoop runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]]) {
		// no-op
	}
}

- (NSUInteger) collectResultsOf: (IOBluetoothDeviceInquiry*) inquiry {
	// stop the inquiry
	if(![inquiry stop])
		WIIUSE_ERROR("Unable to stop bluetooth device inquiry.");
	
	// read found device information
	NSArray* devices = [inquiry foundDevices];
	NSUInteger i;
	for(i = 0; i < [devices count]; i++) {
		IOBluetoothDevice* device = [devices objectAtIndex:i];
		
		// save the device in the wiimote structure
		wiimotes[i]->objc_wm = (void*) [[WiiuseWiimote alloc] initWithPtr:wiimotes[i] device: device];
		
		// mark as found
		WIIMOTE_ENABLE_STATE(wiimotes[i], WIIMOTE_STATE_DEV_FOUND);
		NSString* address = IOBluetoothNSStringFromDeviceAddress([device getAddress]);
		const char* address_str = [address cStringUsingEncoding:NSMacOSRomanStringEncoding];
		WIIUSE_INFO("Found Wiimote (%s) [id %i]", address_str, wiimotes[i]->unid);
	}
	
	return [devices count];
}

- (int) run {
	int result = -1;
	
	if(maxDevices == 0) {
		result = 0;
	} else if(!_running) {
		_running = YES;
		
		if (![IOBluetoothHostController defaultController]) {
			WIIUSE_ERROR("Unable to find any bluetooth receiver on your host.");
		} else {
			IOBluetoothDeviceInquiry* inquiry = [self start];
			if(inquiry) {
				[self wait];
				result = [self collectResultsOf: inquiry];
				WIIUSE_INFO("Found %i Wiimote device(s).", result);
			}
		}
		
		_running = NO;
	} else { // if(_running)
		WIIUSE_ERROR("Device inquiry already running - won't start it again.");
	}
	
	return result;
}

#pragma mark IOBluetoothDeviceInquiryDelegate

- (void) deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *) inquiry device:(IOBluetoothDevice *) device {
	
	WIIUSE_DEBUG("Found a wiimote");
	
	_foundDevices++;
	if(_foundDevices >= maxDevices) {
		// reached maximum number of devices
		_inquiryComplete = YES;
	}
}

- (void) deviceInquiryComplete:(IOBluetoothDeviceInquiry*) inquiry error:(IOReturn) error aborted:(BOOL) aborted
{
	WIIUSE_DEBUG("Inquiry complete, error=%i, aborted=%s", error, aborted ? "YES" : "NO");
	
	// mark completion
	_inquiryComplete = YES;
	
	// print error message if we stop due to an error
	if ((error != kIOReturnSuccess) && !aborted) {
		WIIUSE_ERROR("Bluetooth device inquiry not completed due to unexpected errors. Try increasing the timeout.");
	}
}

@end

#pragma mark -
#pragma mark public interface

int wiiuse_os_find(struct wiimote_t** wm, int max_wiimotes, int timeout) {
	int result;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	WiiuseDeviceInquiry* inquiry = [[WiiuseDeviceInquiry alloc] initWithMemory:wm maxDevices:max_wiimotes timeout:timeout];
	result = [inquiry run];
	[inquiry release];
	
	[pool drain];
	return result;
}

#endif // __APPLE__
