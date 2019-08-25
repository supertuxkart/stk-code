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


#define BLUETOOTH_VERSION_USE_CURRENT

#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothL2CAPChannel.h>

#import "../wiiuse_internal.h"


#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
#define WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE 1
#else
#define WIIUSE_MAC_OS_X_VERSION_10_7_OR_ABOVE 0
#endif


@interface WiiuseWiimote : NSObject<IOBluetoothL2CAPChannelDelegate> {
	wiimote* wm; // reference to the C wiimote struct
	
	IOBluetoothDevice* device;
	IOBluetoothL2CAPChannel* controlChannel;
	IOBluetoothL2CAPChannel* interruptChannel;
	
	IOBluetoothUserNotification* disconnectNotification;
	
	NSMutableArray* receivedData; // a queue of NSObject<WiiuseReceivedMessage>*
	NSLock* receivedDataLock;
}

- (id) initWithPtr: (wiimote*) wm device: (IOBluetoothDevice*) device;

- (IOReturn) connect;
- (void) disconnect;

- (int) readBuffer: (byte*) buffer length: (NSUInteger) bufferLength;
- (int) writeReport: (byte) report_type buffer: (byte*) buffer length: (NSUInteger) length;

@end


@protocol WiiuseReceivedMessage <NSObject>
- (int) applyToStruct: (wiimote*) wm buffer: (byte*) buffer length: (NSUInteger) bufferLength; // <0: not copied, 0: copied empty, >0: copied
@end

@interface WiiuseReceivedData : NSObject<WiiuseReceivedMessage> {
	NSData* data;
}
- (id) initWithBytes: (void*) bytes length: (NSUInteger) length;
- (id) initWithData: (NSData*) data;
@end

@interface WiiuseDisconnectionMessage : NSObject<WiiuseReceivedMessage> {
}
@end


#endif // __APPLE__
