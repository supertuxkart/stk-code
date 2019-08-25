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

#import "../events.h"

#import <IOBluetooth/IOBluetoothUtilities.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothHostController.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#import <IOBluetooth/objc/IOBluetoothL2CAPChannel.h>


@implementation WiiuseWiimote

#pragma mark init, dealloc

- (id) initWithPtr: (wiimote*) wm_ device:(IOBluetoothDevice *)device_ {
	self = [super init];
	if(self) {
		wm = wm_;
		
		device = [device_ retain];
		controlChannel = nil;
		interruptChannel = nil;
		
		disconnectNotification = nil;
		
		receivedData = [[NSMutableArray alloc] initWithCapacity: 2];
		receivedDataLock = [[NSLock alloc] init];
	}
	return self;
}

- (void) dealloc {
	wm = NULL;
	
	[interruptChannel release];
	[controlChannel release];
	[device release];
	
	[disconnectNotification unregister];
	[disconnectNotification release];
	
	[receivedData release];
	
	[super dealloc];
}

#pragma mark connect, disconnect

- (BOOL) connectChannel: (IOBluetoothL2CAPChannel**) pChannel PSM: (BluetoothL2CAPPSM) psm {
	if ([device openL2CAPChannelSync:pChannel withPSM:psm delegate:self] != kIOReturnSuccess) {
		WIIUSE_ERROR("Unable to open L2CAP channel [id %i].", wm->unid);
		*pChannel = nil;
		return NO;
	} else {
		[*pChannel retain];
		return YES;
	}
}

- (IOReturn) connect {
	if(!device) {
		WIIUSE_ERROR("Missing device.");
		return kIOReturnBadArgument;
	}
	
	// open channels
	if(![self connectChannel:&controlChannel PSM:kBluetoothL2CAPPSMHIDControl]) {
		[self disconnect];
		return kIOReturnNotOpen;
	} else if(![self connectChannel:&interruptChannel PSM:kBluetoothL2CAPPSMHIDInterrupt]) {
		[self disconnect];
		return kIOReturnNotOpen;
	}
	
	// register for device disconnection
	disconnectNotification = [device registerForDisconnectNotification:self selector:@selector(disconnected:fromDevice:)];
	if(!disconnectNotification) {
		WIIUSE_ERROR("Unable to register disconnection handler [id %i].", wm->unid);
		[self disconnect];
		return kIOReturnNotOpen;
	}
	
	return kIOReturnSuccess;
}

- (void) disconnectChannel: (IOBluetoothL2CAPChannel**) pChannel {
	if(!pChannel) return;
	
	if([*pChannel closeChannel] != kIOReturnSuccess)
		WIIUSE_ERROR("Unable to close channel [id %i].", wm ? wm->unid : -1);
	[*pChannel release];
	*pChannel = nil;
}

- (void) disconnect {
	// channels
	[self disconnectChannel:&interruptChannel];
	[self disconnectChannel:&controlChannel];
	
	// device
	if([device closeConnection] != kIOReturnSuccess)
		WIIUSE_ERROR("Unable to close the device connection [id %i].", wm ? wm->unid : -1);
	[device release];
	device = nil;
}

- (void) disconnected:(IOBluetoothUserNotification*) notification fromDevice:(IOBluetoothDevice*) device {
	
	WiiuseDisconnectionMessage* message = [[WiiuseDisconnectionMessage alloc] init];
	[receivedDataLock lock];
	[receivedData addObject:message];
	[receivedDataLock unlock];
	[message release];
}

#pragma mark read, write

// <0: nothing received, else: length of data received (can be 0 in case of disconnection message)
- (int) checkForAvailableDataForBuffer: (byte*) buffer length: (NSUInteger) bufferLength {
	int result = -1;
	
	[receivedDataLock lock];
	if([receivedData count]) {
		// look at first item in queue
		NSObject<WiiuseReceivedMessage>* firstMessage = [receivedData objectAtIndex:0];
		result = [firstMessage applyToStruct:wm buffer: buffer length: bufferLength];
		if(result >= 0)
			[receivedData removeObjectAtIndex:0];
	}
	[receivedDataLock unlock];
	
	return result;
}

- (void) waitForIncomingData: (NSTimeInterval) duration {
	NSDate* timeoutDate = [NSDate dateWithTimeIntervalSinceNow: duration];
	NSRunLoop *theRL = [NSRunLoop currentRunLoop];
	while (true) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; // This is used for fast release of NSDate, otherwise it leaks
		if(![theRL runMode:NSDefaultRunLoopMode beforeDate:timeoutDate]) {
			WIIUSE_ERROR("Could not start run loop while waiting for read [id %i].", wm->unid);
			break;
		}
		[pool drain];
		
		[receivedDataLock lock];
		NSUInteger count = [receivedData count];
		[receivedDataLock unlock];
		if(count) {
			// received some data, stop waiting
			break;
		}
		
		if([timeoutDate isLessThanOrEqualTo:[NSDate date]]) {
			// timeout
			break;
		}
	}
}

// result = length of data copied to event buffer
- (int) readBuffer:(byte *)buffer length:(NSUInteger)bufferLength {
	// is there already some data to read?
	int result = [self checkForAvailableDataForBuffer: buffer length: bufferLength];
	if(result < 0) {
		// wait a short amount of time, until data becomes available or a timeout is reached
		[self waitForIncomingData:1];
		
		// check again
		result = [self checkForAvailableDataForBuffer: buffer length: bufferLength];
	}
	
	return result >= 0 ? result : 0;
}

- (int) writeReport: (byte) report_type buffer: (byte*) buffer length: (NSUInteger) length {
	if(interruptChannel == nil) {
		WIIUSE_ERROR("Attempted to write to nil interrupt channel [id %i].", wm->unid);
		return 0;
	}
	
	byte write_buffer[MAX_PAYLOAD];
	write_buffer[0] = WM_SET_DATA | WM_BT_OUTPUT;
	write_buffer[1] = report_type;
	memcpy(write_buffer+2, buffer, length);

	IOReturn error = [interruptChannel writeSync:write_buffer length:length+2];
	if (error != kIOReturnSuccess) {
		WIIUSE_ERROR("Error writing to interrupt channel [id %i].", wm->unid);
		
		WIIUSE_DEBUG("Attempting to reopen the interrupt channel [id %i].", wm->unid);
		[self disconnectChannel:&interruptChannel];
		[self connectChannel:&interruptChannel PSM:kBluetoothL2CAPPSMHIDInterrupt];
		if(!interruptChannel) {
			WIIUSE_ERROR("Error reopening the interrupt channel [id %i].", wm->unid);
			[self disconnect];
		} else {
			WIIUSE_DEBUG("Attempting to write again to the interrupt channel [id %i].", wm->unid);
			error = [interruptChannel writeSync:write_buffer length:length+2];
			if (error != kIOReturnSuccess)
				WIIUSE_ERROR("Unable to write again to the interrupt channel [id %i].", wm->unid);
		}
	}
	
	return (error == kIOReturnSuccess) ? length : 0;
}

#pragma mark IOBluetoothL2CAPChannelDelegate

- (void) l2capChannelData:(IOBluetoothL2CAPChannel*)channel data:(void*)data_ length:(NSUInteger)length {
	
	byte* data = (byte*) data_;
	
	// This is done in case the control channel woke up this handler
#if WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE
	BluetoothL2CAPPSM psm = channel.PSM;
#else
	BluetoothL2CAPPSM psm = [channel getPSM];
#endif
	if(!data || (psm == kBluetoothL2CAPPSMHIDControl)) {
		return;
	}
	
	// copy the data into the buffer
	// on Mac, we ignore the first byte
	WiiuseReceivedData* newData = [[WiiuseReceivedData alloc] initWithBytes: data+1 length: length-1];
	[receivedDataLock lock];
	[receivedData addObject: newData];
	[receivedDataLock unlock];
	[newData release];
}

#if !WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE
// the following delegate methods were required on 10.6. They are here to get rid of 10.6 compiler warnings.
- (void)l2capChannelOpenComplete:(IOBluetoothL2CAPChannel*)l2capChannel status:(IOReturn)error {
	/* no-op */
}
- (void)l2capChannelClosed:(IOBluetoothL2CAPChannel*)l2capChannel {
	/* no-op */
}
- (void)l2capChannelReconfigured:(IOBluetoothL2CAPChannel*)l2capChannel {
	/* no-op */
}
- (void)l2capChannelWriteComplete:(IOBluetoothL2CAPChannel*)l2capChannel refcon:(void*)refcon status:(IOReturn)error {
	/* no-op */
}
- (void)l2capChannelQueueSpaceAvailable:(IOBluetoothL2CAPChannel*)l2capChannel {
	/* no-op */
}
#endif

@end

#pragma mark -
#pragma mark WiiuseReceivedMessage

@implementation WiiuseReceivedData

- (id) initWithData:(NSData *)data_ {
	self = [super init];
	if (self) {
		data = [data_ retain];
	}
	return self;
}
- (id) initWithBytes: (void*) bytes length: (NSUInteger) length {
	NSData* data_ = [[NSData alloc] initWithBytes:bytes length:length];
	id result = [self initWithData: data_];
	[data_ release];
	return result;
}

- (void) dealloc {
	[data release];
	[super dealloc];
}

- (int) applyToStruct:(wiimote *)wm buffer:(byte *)buffer length:(NSUInteger)bufferLength {
	byte* bytes = (byte*) [data bytes];
	NSUInteger length = [data length];
	if(length > bufferLength) {
		WIIUSE_WARNING("Received data was longer than event buffer. Dropping excess bytes.");
		length = bufferLength;
	}
	
	// log the received data
#ifdef WITH_WIIUSE_DEBUG
	{
		printf("[DEBUG] (id %i) RECV: (%.2x) ", wm->unid, bytes[0]);
		int x;
		for (x = 1; x < length; ++x)
			printf("%.2x ", bytes[x]);
		printf("\n");
	}
#endif
	
	// copy to struct
	memcpy(buffer, bytes, length);
	
	return length;
}

@end

@implementation WiiuseDisconnectionMessage

- (int) applyToStruct:(wiimote *)wm buffer:(byte *)buffer length:(NSUInteger)bufferLength {
	wiiuse_disconnected(wm);
	return 0;
}

@end


#endif // __APPLE__
