/** 
 @file  protocol.c
 @brief ENet protocol functions
*/
#include <stdio.h>
#include <string.h>
#define ENET_BUILDING_LIB 1
#include "enet/utility.h"
#include "enet/time.h"
#include "enet/enet.h"

static size_t commandSizes [ENET_PROTOCOL_COMMAND_COUNT] =
{
    0,
    sizeof (ENetProtocolAcknowledge),
    sizeof (ENetProtocolConnect),
    sizeof (ENetProtocolVerifyConnect),
    sizeof (ENetProtocolDisconnect),
    sizeof (ENetProtocolPing),
    sizeof (ENetProtocolSendReliable),
    sizeof (ENetProtocolSendUnreliable),
    sizeof (ENetProtocolSendFragment),
    sizeof (ENetProtocolSendUnsequenced),
    sizeof (ENetProtocolBandwidthLimit),
    sizeof (ENetProtocolThrottleConfigure),
};

size_t
enet_protocol_command_size (enet_uint8 commandNumber)
{
    return commandSizes [commandNumber & ENET_PROTOCOL_COMMAND_MASK];
}

static int
enet_protocol_dispatch_incoming_commands (ENetHost * host, ENetEvent * event)
{
    ENetPeer * currentPeer = host -> lastServicedPeer;
    ENetChannel * channel;

    do
    {
       ++ currentPeer;
       
       if (currentPeer >= & host -> peers [host -> peerCount])
         currentPeer = host -> peers;

       switch (currentPeer -> state)
       {
       case ENET_PEER_STATE_CONNECTION_PENDING:
       case ENET_PEER_STATE_CONNECTION_SUCCEEDED:
           currentPeer -> state = ENET_PEER_STATE_CONNECTED;

           event -> type = ENET_EVENT_TYPE_CONNECT;
           event -> peer = currentPeer;

           return 1;
           
       case ENET_PEER_STATE_ZOMBIE:
           host -> recalculateBandwidthLimits = 1;

           event -> type = ENET_EVENT_TYPE_DISCONNECT;
           event -> peer = currentPeer;
           event -> data = currentPeer -> disconnectData;

           enet_peer_reset (currentPeer);

           host -> lastServicedPeer = currentPeer;

           return 1;
       }

       if (currentPeer -> state != ENET_PEER_STATE_CONNECTED)
         continue;

       for (channel = currentPeer -> channels;
            channel < & currentPeer -> channels [currentPeer -> channelCount];
            ++ channel)
       {
           if (enet_list_empty (& channel -> incomingReliableCommands) &&
               enet_list_empty (& channel -> incomingUnreliableCommands))
             continue;

           event -> packet = enet_peer_receive (currentPeer, channel - currentPeer -> channels);
           if (event -> packet == NULL)
             continue;
             
           event -> type = ENET_EVENT_TYPE_RECEIVE;
           event -> peer = currentPeer;
           event -> channelID = (enet_uint8) (channel - currentPeer -> channels);

           host -> lastServicedPeer = currentPeer;

           return 1;
       }
    } while (currentPeer != host -> lastServicedPeer);

    return 0;
}

static void
enet_protocol_notify_connect (ENetHost * host, ENetPeer * peer, ENetEvent * event)
{
    host -> recalculateBandwidthLimits = 1;

    if (event == NULL)
       peer -> state = (peer -> state == ENET_PEER_STATE_CONNECTING ? ENET_PEER_STATE_CONNECTION_SUCCEEDED : ENET_PEER_STATE_CONNECTION_PENDING);
    else
    {
       peer -> state = ENET_PEER_STATE_CONNECTED;

       event -> type = ENET_EVENT_TYPE_CONNECT;
       event -> peer = peer;
    }
}

static void
enet_protocol_notify_disconnect (ENetHost * host, ENetPeer * peer, ENetEvent * event)
{
    if (peer -> state >= ENET_PEER_STATE_CONNECTION_PENDING)
        host -> recalculateBandwidthLimits = 1;

    if (peer -> state != ENET_PEER_STATE_CONNECTING && peer -> state < ENET_PEER_STATE_CONNECTION_SUCCEEDED)
        enet_peer_reset (peer);
    else
    if (event == NULL)
        peer -> state = ENET_PEER_STATE_ZOMBIE;
    else
    {
        event -> type = ENET_EVENT_TYPE_DISCONNECT;
        event -> peer = peer;
        event -> data = 0;

        enet_peer_reset (peer);
    }
}

static void
enet_protocol_remove_sent_unreliable_commands (ENetPeer * peer)
{
    ENetOutgoingCommand * outgoingCommand;

    while (! enet_list_empty (& peer -> sentUnreliableCommands))
    {
        outgoingCommand = (ENetOutgoingCommand *) enet_list_front (& peer -> sentUnreliableCommands);
        
        enet_list_remove (& outgoingCommand -> outgoingCommandList);

        if (outgoingCommand -> packet != NULL)
        {
           -- outgoingCommand -> packet -> referenceCount;

           if (outgoingCommand -> packet -> referenceCount == 0)
             enet_packet_destroy (outgoingCommand -> packet);
        }

        enet_free (outgoingCommand);
    }
}

static ENetProtocolCommand
enet_protocol_remove_sent_reliable_command (ENetPeer * peer, enet_uint16 reliableSequenceNumber, enet_uint8 channelID)
{
    ENetOutgoingCommand * outgoingCommand;
    ENetListIterator currentCommand;
    ENetProtocolCommand commandNumber;

    for (currentCommand = enet_list_begin (& peer -> sentReliableCommands);
         currentCommand != enet_list_end (& peer -> sentReliableCommands);
         currentCommand = enet_list_next (currentCommand))
    {
       outgoingCommand = (ENetOutgoingCommand *) currentCommand;
        
       if (outgoingCommand -> reliableSequenceNumber == reliableSequenceNumber &&
           outgoingCommand -> command.header.channelID == channelID)
         break;
    }

    if (currentCommand == enet_list_end (& peer -> sentReliableCommands))
    {
       for (currentCommand = enet_list_begin (& peer -> outgoingReliableCommands);
            currentCommand != enet_list_end (& peer -> outgoingReliableCommands);
            currentCommand = enet_list_next (currentCommand))
       {
          outgoingCommand = (ENetOutgoingCommand *) currentCommand;

          if (outgoingCommand -> sendAttempts < 1) return ENET_PROTOCOL_COMMAND_NONE;

          if (outgoingCommand -> reliableSequenceNumber == reliableSequenceNumber &&
              outgoingCommand -> command.header.channelID == channelID)
            break;
       }

       if (currentCommand == enet_list_end (& peer -> outgoingReliableCommands))
         return ENET_PROTOCOL_COMMAND_NONE;
    }

    if (channelID < peer -> channelCount)
    {
       ENetChannel * channel = & peer -> channels [channelID];
       enet_uint16 reliableWindow = reliableSequenceNumber / ENET_PEER_RELIABLE_WINDOW_SIZE;
       if (channel -> reliableWindows [reliableWindow] > 0)
       {
          -- channel -> reliableWindows [reliableWindow];
          if (! channel -> reliableWindows [reliableWindow])
            channel -> usedReliableWindows &= ~ (1 << reliableWindow);
       }
    }

    commandNumber = (ENetProtocolCommand) (outgoingCommand -> command.header.command & ENET_PROTOCOL_COMMAND_MASK);
    
    enet_list_remove (& outgoingCommand -> outgoingCommandList);

    if (outgoingCommand -> packet != NULL)
    {
       peer -> reliableDataInTransit -= outgoingCommand -> fragmentLength;

       -- outgoingCommand -> packet -> referenceCount;

       if (outgoingCommand -> packet -> referenceCount == 0)
         enet_packet_destroy (outgoingCommand -> packet);
    }

    enet_free (outgoingCommand);

    if (enet_list_empty (& peer -> sentReliableCommands))
      return commandNumber;
    
    outgoingCommand = (ENetOutgoingCommand *) enet_list_front (& peer -> sentReliableCommands);
    
    peer -> nextTimeout = outgoingCommand -> sentTime + outgoingCommand -> roundTripTimeout;

    return commandNumber;
} 

static ENetPeer *
enet_protocol_handle_connect (ENetHost * host, ENetProtocolHeader * header, ENetProtocol * command)
{
    enet_uint16 mtu;
    enet_uint32 windowSize;
    ENetChannel * channel;
    size_t channelCount;
    ENetPeer * currentPeer;
    ENetProtocol verifyCommand;

#ifdef USE_CRC32
    {
        enet_uint32 crc = header -> checksum;
        ENetBuffer buffer;

        command -> header.reliableSequenceNumber = ENET_HOST_TO_NET_16 (command -> header.reliableSequenceNumber);

        header -> checksum = command -> connect.sessionID;

        buffer.data = host -> receivedData;
        buffer.dataLength = host -> receivedDataLength;

        if (enet_crc32 (& buffer, 1) != crc)
          return NULL;

        command -> header.reliableSequenceNumber = ENET_NET_TO_HOST_16 (command -> header.reliableSequenceNumber);
    }
#endif
 
    channelCount = ENET_NET_TO_HOST_32 (command -> connect.channelCount);

    if (channelCount < ENET_PROTOCOL_MINIMUM_CHANNEL_COUNT ||
        channelCount > ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT)
      return NULL;

    for (currentPeer = host -> peers;
         currentPeer < & host -> peers [host -> peerCount];
         ++ currentPeer)
    {
        if (currentPeer -> state != ENET_PEER_STATE_DISCONNECTED &&
            currentPeer -> address.host == host -> receivedAddress.host &&
            currentPeer -> address.port == host -> receivedAddress.port &&
            currentPeer -> sessionID == command -> connect.sessionID)
          return NULL;
    }

    for (currentPeer = host -> peers;
         currentPeer < & host -> peers [host -> peerCount];
         ++ currentPeer)
    {
        if (currentPeer -> state == ENET_PEER_STATE_DISCONNECTED)
          break;
    }

    if (currentPeer >= & host -> peers [host -> peerCount])
      return NULL;

    currentPeer -> state = ENET_PEER_STATE_ACKNOWLEDGING_CONNECT;
    currentPeer -> sessionID = command -> connect.sessionID;
    currentPeer -> address = host -> receivedAddress;
    currentPeer -> outgoingPeerID = ENET_NET_TO_HOST_16 (command -> connect.outgoingPeerID);
    currentPeer -> incomingBandwidth = ENET_NET_TO_HOST_32 (command -> connect.incomingBandwidth);
    currentPeer -> outgoingBandwidth = ENET_NET_TO_HOST_32 (command -> connect.outgoingBandwidth);
    currentPeer -> packetThrottleInterval = ENET_NET_TO_HOST_32 (command -> connect.packetThrottleInterval);
    currentPeer -> packetThrottleAcceleration = ENET_NET_TO_HOST_32 (command -> connect.packetThrottleAcceleration);
    currentPeer -> packetThrottleDeceleration = ENET_NET_TO_HOST_32 (command -> connect.packetThrottleDeceleration);
    currentPeer -> channels = (ENetChannel *) enet_malloc (channelCount * sizeof (ENetChannel));
    currentPeer -> channelCount = channelCount;

    for (channel = currentPeer -> channels;
         channel < & currentPeer -> channels [channelCount];
         ++ channel)
    {
        channel -> outgoingReliableSequenceNumber = 0;
        channel -> outgoingUnreliableSequenceNumber = 0;
        channel -> incomingReliableSequenceNumber = 0;

        enet_list_clear (& channel -> incomingReliableCommands);
        enet_list_clear (& channel -> incomingUnreliableCommands);

        channel -> usedReliableWindows = 0;
        memset (channel -> reliableWindows, 0, sizeof (channel -> reliableWindows));
    }

    mtu = ENET_NET_TO_HOST_16 (command -> connect.mtu);

    if (mtu < ENET_PROTOCOL_MINIMUM_MTU)
      mtu = ENET_PROTOCOL_MINIMUM_MTU;
    else
    if (mtu > ENET_PROTOCOL_MAXIMUM_MTU)
      mtu = ENET_PROTOCOL_MAXIMUM_MTU;

    currentPeer -> mtu = mtu;

    if (host -> outgoingBandwidth == 0 &&
        currentPeer -> incomingBandwidth == 0)
      currentPeer -> windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
    if (host -> outgoingBandwidth == 0 ||
        currentPeer -> incomingBandwidth == 0)
      currentPeer -> windowSize = (ENET_MAX (host -> outgoingBandwidth, currentPeer -> incomingBandwidth) /
                                    ENET_PEER_WINDOW_SIZE_SCALE) *
                                      ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;
    else
      currentPeer -> windowSize = (ENET_MIN (host -> outgoingBandwidth, currentPeer -> incomingBandwidth) /
                                    ENET_PEER_WINDOW_SIZE_SCALE) * 
                                      ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (currentPeer -> windowSize < ENET_PROTOCOL_MINIMUM_WINDOW_SIZE)
      currentPeer -> windowSize = ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;
    else
    if (currentPeer -> windowSize > ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE)
      currentPeer -> windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;

    if (host -> incomingBandwidth == 0)
      windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
      windowSize = (host -> incomingBandwidth / ENET_PEER_WINDOW_SIZE_SCALE) *
                     ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (windowSize > ENET_NET_TO_HOST_32 (command -> connect.windowSize))
      windowSize = ENET_NET_TO_HOST_32 (command -> connect.windowSize);

    if (windowSize < ENET_PROTOCOL_MINIMUM_WINDOW_SIZE)
      windowSize = ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;
    else
    if (windowSize > ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE)
      windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;

    verifyCommand.header.command = ENET_PROTOCOL_COMMAND_VERIFY_CONNECT | ENET_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
    verifyCommand.header.channelID = 0xFF;
    verifyCommand.verifyConnect.outgoingPeerID = ENET_HOST_TO_NET_16 (currentPeer -> incomingPeerID);
    verifyCommand.verifyConnect.mtu = ENET_HOST_TO_NET_16 (currentPeer -> mtu);
    verifyCommand.verifyConnect.windowSize = ENET_HOST_TO_NET_32 (windowSize);
    verifyCommand.verifyConnect.channelCount = ENET_HOST_TO_NET_32 (channelCount);
    verifyCommand.verifyConnect.incomingBandwidth = ENET_HOST_TO_NET_32 (host -> incomingBandwidth);
    verifyCommand.verifyConnect.outgoingBandwidth = ENET_HOST_TO_NET_32 (host -> outgoingBandwidth);
    verifyCommand.verifyConnect.packetThrottleInterval = ENET_HOST_TO_NET_32 (currentPeer -> packetThrottleInterval);
    verifyCommand.verifyConnect.packetThrottleAcceleration = ENET_HOST_TO_NET_32 (currentPeer -> packetThrottleAcceleration);
    verifyCommand.verifyConnect.packetThrottleDeceleration = ENET_HOST_TO_NET_32 (currentPeer -> packetThrottleDeceleration);

    enet_peer_queue_outgoing_command (currentPeer, & verifyCommand, NULL, 0, 0);

    return currentPeer;
}

static int
enet_protocol_handle_send_reliable (ENetHost * host, ENetPeer * peer, const ENetProtocol * command, enet_uint8 ** currentData)
{
    ENetPacket * packet;
    size_t dataLength;

    if (command -> header.channelID >= peer -> channelCount ||
        (peer -> state != ENET_PEER_STATE_CONNECTED && peer -> state != ENET_PEER_STATE_DISCONNECT_LATER))
      return -1;

    dataLength = ENET_NET_TO_HOST_16 (command -> sendReliable.dataLength);
    * currentData += dataLength;
    if (* currentData > & host -> receivedData [host -> receivedDataLength])
      return -1;

    packet = enet_packet_create ((const enet_uint8 *) command + sizeof (ENetProtocolSendReliable),
                                 dataLength,
                                 ENET_PACKET_FLAG_RELIABLE);
    if (packet == NULL)
      return -1;

    enet_peer_queue_incoming_command (peer, command, packet, 0);
    return 0;
}

static int
enet_protocol_handle_send_unsequenced (ENetHost * host, ENetPeer * peer, const ENetProtocol * command, enet_uint8 ** currentData)
{
    ENetPacket * packet;
    enet_uint32 unsequencedGroup, index;
    size_t dataLength;

    if (command -> header.channelID >= peer -> channelCount ||
        (peer -> state != ENET_PEER_STATE_CONNECTED && peer -> state != ENET_PEER_STATE_DISCONNECT_LATER))
      return -1;

    dataLength = ENET_NET_TO_HOST_16 (command -> sendUnsequenced.dataLength);
    * currentData += dataLength;
    if (* currentData > & host -> receivedData [host -> receivedDataLength])
      return -1; 

    unsequencedGroup = ENET_NET_TO_HOST_16 (command -> sendUnsequenced.unsequencedGroup);
    index = unsequencedGroup % ENET_PEER_UNSEQUENCED_WINDOW_SIZE;
   
    if (unsequencedGroup < peer -> incomingUnsequencedGroup)
      unsequencedGroup += 0x10000;

    if (unsequencedGroup >= (enet_uint32) peer -> incomingUnsequencedGroup + ENET_PEER_FREE_UNSEQUENCED_WINDOWS * ENET_PEER_UNSEQUENCED_WINDOW_SIZE)
      return 0;

    unsequencedGroup &= 0xFFFF;

    if (unsequencedGroup - index != peer -> incomingUnsequencedGroup)
    {
        peer -> incomingUnsequencedGroup = unsequencedGroup - index;

        memset (peer -> unsequencedWindow, 0, sizeof (peer -> unsequencedWindow));
    }
    else
    if (peer -> unsequencedWindow [index / 32] & (1 << (index % 32)))
      return 0;
      
    peer -> unsequencedWindow [index / 32] |= 1 << (index % 32);
    
                        
    packet = enet_packet_create ((const enet_uint8 *) command + sizeof (ENetProtocolSendUnsequenced),
                                 dataLength,
                                 ENET_PACKET_FLAG_UNSEQUENCED);
    if (packet == NULL)
      return -1;
    
    enet_peer_queue_incoming_command (peer, command, packet, 0);
    return 0;
}

static int
enet_protocol_handle_send_unreliable (ENetHost * host, ENetPeer * peer, const ENetProtocol * command, enet_uint8 ** currentData)
{
    ENetPacket * packet;
    size_t dataLength;

    if (command -> header.channelID >= peer -> channelCount ||
        (peer -> state != ENET_PEER_STATE_CONNECTED && peer -> state != ENET_PEER_STATE_DISCONNECT_LATER))
      return -1;

    dataLength = ENET_NET_TO_HOST_16 (command -> sendUnreliable.dataLength);
    * currentData += dataLength;
    if (* currentData > & host -> receivedData [host -> receivedDataLength])
      return -1;

    packet = enet_packet_create ((const enet_uint8 *) command + sizeof (ENetProtocolSendUnreliable),
                                 dataLength,
                                 0);
    if (packet == NULL)
      return -1;

    enet_peer_queue_incoming_command (peer, command, packet, 0);
    return 0;
}

static int
enet_protocol_handle_send_fragment (ENetHost * host, ENetPeer * peer, const ENetProtocol * command, enet_uint8 ** currentData)
{
    enet_uint32 fragmentNumber,
           fragmentCount,
           fragmentOffset,
           fragmentLength,
           startSequenceNumber,
           totalLength;
    ENetChannel * channel;
    enet_uint16 startWindow, currentWindow;
    ENetListIterator currentCommand;
    ENetIncomingCommand * startCommand = NULL;

    if (command -> header.channelID >= peer -> channelCount ||
        (peer -> state != ENET_PEER_STATE_CONNECTED && peer -> state != ENET_PEER_STATE_DISCONNECT_LATER))
      return -1;

    fragmentLength = ENET_NET_TO_HOST_16 (command -> sendFragment.dataLength);
    * currentData += fragmentLength;
    if (* currentData > & host -> receivedData [host -> receivedDataLength])
      return -1;

    channel = & peer -> channels [command -> header.channelID];
    startSequenceNumber = ENET_NET_TO_HOST_16 (command -> sendFragment.startSequenceNumber);
    startWindow = startSequenceNumber / ENET_PEER_RELIABLE_WINDOW_SIZE;
    currentWindow = channel -> incomingReliableSequenceNumber / ENET_PEER_RELIABLE_WINDOW_SIZE;

    if (startSequenceNumber < channel -> incomingReliableSequenceNumber)
      startWindow += ENET_PEER_RELIABLE_WINDOWS;

    if (startWindow < currentWindow || startWindow >= currentWindow + ENET_PEER_FREE_RELIABLE_WINDOWS - 1)
      return 0;

    fragmentNumber = ENET_NET_TO_HOST_32 (command -> sendFragment.fragmentNumber);
    fragmentCount = ENET_NET_TO_HOST_32 (command -> sendFragment.fragmentCount);
    fragmentOffset = ENET_NET_TO_HOST_32 (command -> sendFragment.fragmentOffset);
    totalLength = ENET_NET_TO_HOST_32 (command -> sendFragment.totalLength);
    
    if (fragmentOffset >= totalLength ||
        fragmentOffset + fragmentLength > totalLength ||
        fragmentNumber >= fragmentCount)
      return -1;
 
    for (currentCommand = enet_list_previous (enet_list_end (& channel -> incomingReliableCommands));
         currentCommand != enet_list_end (& channel -> incomingReliableCommands);
         currentCommand = enet_list_previous (currentCommand))
    {
       ENetIncomingCommand * incomingCommand = (ENetIncomingCommand *) currentCommand;

       if (startSequenceNumber >= channel -> incomingReliableSequenceNumber)
       {
          if (incomingCommand -> reliableSequenceNumber < channel -> incomingReliableSequenceNumber)
            continue;
       }
       else
       if (incomingCommand -> reliableSequenceNumber >= channel -> incomingReliableSequenceNumber)
         break;

       if (incomingCommand -> reliableSequenceNumber <= startSequenceNumber)
       {
          if (incomingCommand -> reliableSequenceNumber < startSequenceNumber)
            break;
        
          if ((incomingCommand -> command.header.command & ENET_PROTOCOL_COMMAND_MASK) != ENET_PROTOCOL_COMMAND_SEND_FRAGMENT ||
              totalLength != incomingCommand -> packet -> dataLength ||
              fragmentCount != incomingCommand -> fragmentCount)
            return -1;

          startCommand = incomingCommand;
          break;
       }
    }
 
    if (startCommand == NULL)
    {
       ENetProtocol hostCommand = * command;
       ENetPacket * packet = enet_packet_create (NULL, totalLength, ENET_PACKET_FLAG_RELIABLE);
       if (packet == NULL)
         return -1;

       hostCommand.header.reliableSequenceNumber = startSequenceNumber;
       hostCommand.sendFragment.startSequenceNumber = startSequenceNumber;
       hostCommand.sendFragment.dataLength = fragmentLength;
       hostCommand.sendFragment.fragmentNumber = fragmentNumber;
       hostCommand.sendFragment.fragmentCount = fragmentCount;
       hostCommand.sendFragment.fragmentOffset = fragmentOffset;
       hostCommand.sendFragment.totalLength = totalLength;

       startCommand = enet_peer_queue_incoming_command (peer, & hostCommand, packet, fragmentCount);
       if (startCommand == NULL)
         return -1;
    }
    
    if ((startCommand -> fragments [fragmentNumber / 32] & (1 << (fragmentNumber % 32))) == 0)
    {
       -- startCommand -> fragmentsRemaining;

       startCommand -> fragments [fragmentNumber / 32] |= (1 << (fragmentNumber % 32));

       if (fragmentOffset + fragmentLength > startCommand -> packet -> dataLength)
         fragmentLength = startCommand -> packet -> dataLength - fragmentOffset;

       memcpy (startCommand -> packet -> data + fragmentOffset,
               (enet_uint8 *) command + sizeof (ENetProtocolSendFragment),
               fragmentLength);
    }

    return 0;
}

static int
enet_protocol_handle_ping (ENetHost * host, ENetPeer * peer, const ENetProtocol * command)
{
    return 0;
}

static int
enet_protocol_handle_bandwidth_limit (ENetHost * host, ENetPeer * peer, const ENetProtocol * command)
{
    peer -> incomingBandwidth = ENET_NET_TO_HOST_32 (command -> bandwidthLimit.incomingBandwidth);
    peer -> outgoingBandwidth = ENET_NET_TO_HOST_32 (command -> bandwidthLimit.outgoingBandwidth);

    if (peer -> incomingBandwidth == 0 && host -> outgoingBandwidth == 0)
      peer -> windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;
    else
      peer -> windowSize = (ENET_MIN (peer -> incomingBandwidth, host -> outgoingBandwidth) /
                             ENET_PEER_WINDOW_SIZE_SCALE) * ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (peer -> windowSize < ENET_PROTOCOL_MINIMUM_WINDOW_SIZE)
      peer -> windowSize = ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;
    else
    if (peer -> windowSize > ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE)
      peer -> windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;

    return 0;
}

static int
enet_protocol_handle_throttle_configure (ENetHost * host, ENetPeer * peer, const ENetProtocol * command)
{
    peer -> packetThrottleInterval = ENET_NET_TO_HOST_32 (command -> throttleConfigure.packetThrottleInterval);
    peer -> packetThrottleAcceleration = ENET_NET_TO_HOST_32 (command -> throttleConfigure.packetThrottleAcceleration);
    peer -> packetThrottleDeceleration = ENET_NET_TO_HOST_32 (command -> throttleConfigure.packetThrottleDeceleration);

    return 0;
}

static int
enet_protocol_handle_disconnect (ENetHost * host, ENetPeer * peer, const ENetProtocol * command)
{
    enet_peer_reset_queues (peer);

    if (peer -> state == ENET_PEER_STATE_CONNECTION_SUCCEEDED)
        peer -> state = ENET_PEER_STATE_ZOMBIE;
    else
    if (peer -> state != ENET_PEER_STATE_CONNECTED && peer -> state != ENET_PEER_STATE_DISCONNECT_LATER)
    {
        if (peer -> state == ENET_PEER_STATE_CONNECTION_PENDING) host -> recalculateBandwidthLimits = 1;

        enet_peer_reset (peer);
    }
    else
    if (command -> header.command & ENET_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
      peer -> state = ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT;
    else
      peer -> state = ENET_PEER_STATE_ZOMBIE;

    peer -> disconnectData = ENET_NET_TO_HOST_32 (command -> disconnect.data);
    return 0;
}

static int
enet_protocol_handle_acknowledge (ENetHost * host, ENetEvent * event, ENetPeer * peer, const ENetProtocol * command)
{
    enet_uint32 roundTripTime,
           receivedSentTime,
           receivedReliableSequenceNumber;
    ENetProtocolCommand commandNumber;

    receivedSentTime = ENET_NET_TO_HOST_16 (command -> acknowledge.receivedSentTime);
    receivedSentTime |= host -> serviceTime & 0xFFFF0000;
    if ((receivedSentTime & 0x8000) > (host -> serviceTime & 0x8000))
        receivedSentTime -= 0x10000;

    if (ENET_TIME_LESS (host -> serviceTime, receivedSentTime))
      return 0;

    peer -> lastReceiveTime = host -> serviceTime;
    peer -> earliestTimeout = 0;

    roundTripTime = ENET_TIME_DIFFERENCE (host -> serviceTime, receivedSentTime);

    enet_peer_throttle (peer, roundTripTime);

    peer -> roundTripTimeVariance -= peer -> roundTripTimeVariance / 4;

    if (roundTripTime >= peer -> roundTripTime)
    {
       peer -> roundTripTime += (roundTripTime - peer -> roundTripTime) / 8;
       peer -> roundTripTimeVariance += (roundTripTime - peer -> roundTripTime) / 4;
    }
    else
    {
       peer -> roundTripTime -= (peer -> roundTripTime - roundTripTime) / 8;
       peer -> roundTripTimeVariance += (peer -> roundTripTime - roundTripTime) / 4;
    }

    if (peer -> roundTripTime < peer -> lowestRoundTripTime)
      peer -> lowestRoundTripTime = peer -> roundTripTime;

    if (peer -> roundTripTimeVariance > peer -> highestRoundTripTimeVariance) 
      peer -> highestRoundTripTimeVariance = peer -> roundTripTimeVariance;

    if (peer -> packetThrottleEpoch == 0 ||
        ENET_TIME_DIFFERENCE (host -> serviceTime, peer -> packetThrottleEpoch) >= peer -> packetThrottleInterval)
    {
        peer -> lastRoundTripTime = peer -> lowestRoundTripTime;
        peer -> lastRoundTripTimeVariance = peer -> highestRoundTripTimeVariance;
        peer -> lowestRoundTripTime = peer -> roundTripTime;
        peer -> highestRoundTripTimeVariance = peer -> roundTripTimeVariance;
        peer -> packetThrottleEpoch = host -> serviceTime;
    }

    receivedReliableSequenceNumber = ENET_NET_TO_HOST_16 (command -> acknowledge.receivedReliableSequenceNumber);

    commandNumber = enet_protocol_remove_sent_reliable_command (peer, receivedReliableSequenceNumber, command -> header.channelID);

    switch (peer -> state)
    {
    case ENET_PEER_STATE_ACKNOWLEDGING_CONNECT:
       if (commandNumber != ENET_PROTOCOL_COMMAND_VERIFY_CONNECT)
         return -1;

       enet_protocol_notify_connect (host, peer, event);
       break;

    case ENET_PEER_STATE_DISCONNECTING:
       if (commandNumber != ENET_PROTOCOL_COMMAND_DISCONNECT)
         return -1;

       enet_protocol_notify_disconnect (host, peer, event);
       break;

    case ENET_PEER_STATE_DISCONNECT_LATER:
       if (enet_list_empty (& peer -> outgoingReliableCommands) &&
           enet_list_empty (& peer -> outgoingUnreliableCommands) &&   
           enet_list_empty (& peer -> sentReliableCommands))
         enet_peer_disconnect (peer, peer -> disconnectData);
       break;
    }
   
    return 0;
}

static int
enet_protocol_handle_verify_connect (ENetHost * host, ENetEvent * event, ENetPeer * peer, const ENetProtocol * command)
{
    enet_uint16 mtu;
    enet_uint32 windowSize;

    if (peer -> state != ENET_PEER_STATE_CONNECTING)
      return 0;

    if (ENET_NET_TO_HOST_32 (command -> verifyConnect.channelCount) != peer -> channelCount ||
        ENET_NET_TO_HOST_32 (command -> verifyConnect.packetThrottleInterval) != peer -> packetThrottleInterval ||
        ENET_NET_TO_HOST_32 (command -> verifyConnect.packetThrottleAcceleration) != peer -> packetThrottleAcceleration ||
        ENET_NET_TO_HOST_32 (command -> verifyConnect.packetThrottleDeceleration) != peer -> packetThrottleDeceleration)
    {
        peer -> state = ENET_PEER_STATE_ZOMBIE;

        return -1;
    }

    enet_protocol_remove_sent_reliable_command (peer, 1, 0xFF);
    
    peer -> outgoingPeerID = ENET_NET_TO_HOST_16 (command -> verifyConnect.outgoingPeerID);

    mtu = ENET_NET_TO_HOST_16 (command -> verifyConnect.mtu);

    if (mtu < ENET_PROTOCOL_MINIMUM_MTU)
      mtu = ENET_PROTOCOL_MINIMUM_MTU;
    else 
    if (mtu > ENET_PROTOCOL_MAXIMUM_MTU)
      mtu = ENET_PROTOCOL_MAXIMUM_MTU;

    if (mtu < peer -> mtu)
      peer -> mtu = mtu;

    windowSize = ENET_NET_TO_HOST_32 (command -> verifyConnect.windowSize);

    if (windowSize < ENET_PROTOCOL_MINIMUM_WINDOW_SIZE)
      windowSize = ENET_PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (windowSize > ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE)
      windowSize = ENET_PROTOCOL_MAXIMUM_WINDOW_SIZE;

    if (windowSize < peer -> windowSize)
      peer -> windowSize = windowSize;

    peer -> incomingBandwidth = ENET_NET_TO_HOST_32 (command -> verifyConnect.incomingBandwidth);
    peer -> outgoingBandwidth = ENET_NET_TO_HOST_32 (command -> verifyConnect.outgoingBandwidth);

    enet_protocol_notify_connect (host, peer, event);
    return 0;
}

static int
enet_protocol_handle_incoming_commands (ENetHost * host, ENetEvent * event)
{
    ENetProtocolHeader * header;
    ENetProtocol * command;
    ENetPeer * peer;
    enet_uint8 * currentData;
    size_t headerSize;
    enet_uint16 peerID, flags;

    if (host -> receivedDataLength < sizeof (ENetProtocolHeader))
      return 0;

    header = (ENetProtocolHeader *) host -> receivedData;

    peerID = ENET_NET_TO_HOST_16 (header -> peerID);
    flags = peerID & ENET_PROTOCOL_HEADER_FLAG_MASK;
    peerID &= ~ ENET_PROTOCOL_HEADER_FLAG_MASK;

    if (peerID == ENET_PROTOCOL_MAXIMUM_PEER_ID)
      peer = NULL;
    else
    if (peerID >= host -> peerCount)
      return 0;
    else
    {
       peer = & host -> peers [peerID];

       if (peer -> state == ENET_PEER_STATE_DISCONNECTED ||
           peer -> state == ENET_PEER_STATE_ZOMBIE || 
           (host -> receivedAddress.host != peer -> address.host &&
             peer -> address.host != ENET_HOST_BROADCAST))
         return 0;

#ifdef USE_CRC32
       {
           enet_uint32 crc = header -> checksum;
           ENetBuffer buffer;

           header -> checksum = peer -> sessionID;

           buffer.data = host -> receivedData;
           buffer.dataLength = host -> receivedDataLength;

           if (enet_crc32 (& buffer, 1) != crc)
             return 0;
       }
#else
       if (header -> checksum != peer -> sessionID)
         return 0;
#endif

       peer -> address.host = host -> receivedAddress.host;
       peer -> address.port = host -> receivedAddress.port;
       peer -> incomingDataTotal += host -> receivedDataLength;
    }
    
    headerSize = (flags & ENET_PROTOCOL_HEADER_FLAG_SENT_TIME ? sizeof (ENetProtocolHeader) : (size_t) & ((ENetProtocolHeader *) 0) -> sentTime);
    currentData = host -> receivedData + headerSize;
  
    while (currentData < & host -> receivedData [host -> receivedDataLength])
    {
       enet_uint8 commandNumber;
       size_t commandSize;

       command = (ENetProtocol *) currentData;

       if (currentData + sizeof (ENetProtocolCommandHeader) > & host -> receivedData [host -> receivedDataLength])
         break;

       commandNumber = command -> header.command & ENET_PROTOCOL_COMMAND_MASK;
       if (commandNumber >= ENET_PROTOCOL_COMMAND_COUNT) 
         break;
       
       commandSize = commandSizes [commandNumber];
       if (commandSize == 0 || currentData + commandSize > & host -> receivedData [host -> receivedDataLength])
         break;

       currentData += commandSize;

       if (peer == NULL && commandNumber != ENET_PROTOCOL_COMMAND_CONNECT)
         break;
         
       command -> header.reliableSequenceNumber = ENET_NET_TO_HOST_16 (command -> header.reliableSequenceNumber);

       switch (command -> header.command & ENET_PROTOCOL_COMMAND_MASK)
       {
       case ENET_PROTOCOL_COMMAND_ACKNOWLEDGE:
          if (enet_protocol_handle_acknowledge (host, event, peer, command))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_CONNECT:
          peer = enet_protocol_handle_connect (host, header, command);
          if (peer == NULL)
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_VERIFY_CONNECT:
          if (enet_protocol_handle_verify_connect (host, event, peer, command))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_DISCONNECT:
          if (enet_protocol_handle_disconnect (host, peer, command))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_PING:
          if (enet_protocol_handle_ping (host, peer, command))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_SEND_RELIABLE:
          if (enet_protocol_handle_send_reliable (host, peer, command, & currentData))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_SEND_UNRELIABLE:
          if (enet_protocol_handle_send_unreliable (host, peer, command, & currentData))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_SEND_UNSEQUENCED:
          if (enet_protocol_handle_send_unsequenced (host, peer, command, & currentData))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_SEND_FRAGMENT:
          if (enet_protocol_handle_send_fragment (host, peer, command, & currentData))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_BANDWIDTH_LIMIT:
          if (enet_protocol_handle_bandwidth_limit (host, peer, command))
            goto commandError;
          break;

       case ENET_PROTOCOL_COMMAND_THROTTLE_CONFIGURE:
          if (enet_protocol_handle_throttle_configure (host, peer, command))
            goto commandError;
          break;

       default:
          goto commandError;
       }

       if (peer != NULL &&
           (command -> header.command & ENET_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE) != 0)
       {
           enet_uint16 sentTime;

           if (! (flags & ENET_PROTOCOL_HEADER_FLAG_SENT_TIME))
             break;

           sentTime = ENET_NET_TO_HOST_16 (header -> sentTime);

           switch (peer -> state)
           {
           case ENET_PEER_STATE_DISCONNECTING:
           case ENET_PEER_STATE_ACKNOWLEDGING_CONNECT:
              break;

           case ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT:
              if ((command -> header.command & ENET_PROTOCOL_COMMAND_MASK) == ENET_PROTOCOL_COMMAND_DISCONNECT)
                enet_peer_queue_acknowledgement (peer, command, sentTime);
              break;

           default:   
              enet_peer_queue_acknowledgement (peer, command, sentTime);        
              break;
           }
       }
    }

commandError:
    if (event != NULL && event -> type != ENET_EVENT_TYPE_NONE)
      return 1;

    return 0;
}
 
static int
enet_protocol_receive_incoming_commands (ENetHost * host, ENetEvent * event)
{
    for (;;)
    {
       int receivedLength;
       ENetBuffer buffer;

       buffer.data = host -> receivedData;
       buffer.dataLength = sizeof (host -> receivedData);

       receivedLength = enet_socket_receive (host -> socket,
                                             & host -> receivedAddress,
                                             & buffer,
                                             1);

       if (receivedLength < 0)
         return -1;

       if (receivedLength == 0)
         return 0;

       host -> receivedDataLength = receivedLength;
       
       switch (enet_protocol_handle_incoming_commands (host, event))
       {
       case 1:
          return 1;
       
       case -1:
          return -1;

       default:
          break;
       }
    }

    return -1;
}

static void
enet_protocol_send_acknowledgements (ENetHost * host, ENetPeer * peer)
{
    ENetProtocol * command = & host -> commands [host -> commandCount];
    ENetBuffer * buffer = & host -> buffers [host -> bufferCount];
    ENetAcknowledgement * acknowledgement;
    ENetListIterator currentAcknowledgement;
  
    currentAcknowledgement = enet_list_begin (& peer -> acknowledgements);
         
    while (currentAcknowledgement != enet_list_end (& peer -> acknowledgements))
    {
       if (command >= & host -> commands [sizeof (host -> commands) / sizeof (ENetProtocol)] ||
           buffer >= & host -> buffers [sizeof (host -> buffers) / sizeof (ENetBuffer)] ||
           peer -> mtu - host -> packetSize < sizeof (ENetProtocolAcknowledge))
       {
          host -> continueSending = 1;

          break;
       }

       acknowledgement = (ENetAcknowledgement *) currentAcknowledgement;
 
       currentAcknowledgement = enet_list_next (currentAcknowledgement);

       buffer -> data = command;
       buffer -> dataLength = sizeof (ENetProtocolAcknowledge);

       host -> packetSize += buffer -> dataLength;
 
       command -> header.command = ENET_PROTOCOL_COMMAND_ACKNOWLEDGE;
       command -> header.channelID = acknowledgement -> command.header.channelID;
       command -> acknowledge.receivedReliableSequenceNumber = ENET_HOST_TO_NET_16 (acknowledgement -> command.header.reliableSequenceNumber);
       command -> acknowledge.receivedSentTime = ENET_HOST_TO_NET_16 (acknowledgement -> sentTime);
  
       if ((acknowledgement -> command.header.command & ENET_PROTOCOL_COMMAND_MASK) == ENET_PROTOCOL_COMMAND_DISCONNECT)
         peer -> state = ENET_PEER_STATE_ZOMBIE;

       enet_list_remove (& acknowledgement -> acknowledgementList);
       enet_free (acknowledgement);

       ++ command;
       ++ buffer;
    }

    host -> commandCount = command - host -> commands;
    host -> bufferCount = buffer - host -> buffers;
}

static void
enet_protocol_send_unreliable_outgoing_commands (ENetHost * host, ENetPeer * peer)
{
    ENetProtocol * command = & host -> commands [host -> commandCount];
    ENetBuffer * buffer = & host -> buffers [host -> bufferCount];
    ENetOutgoingCommand * outgoingCommand;
    ENetListIterator currentCommand;

    currentCommand = enet_list_begin (& peer -> outgoingUnreliableCommands);
    
    while (currentCommand != enet_list_end (& peer -> outgoingUnreliableCommands))
    {
       size_t commandSize;

       outgoingCommand = (ENetOutgoingCommand *) currentCommand;
       commandSize = commandSizes [outgoingCommand -> command.header.command & ENET_PROTOCOL_COMMAND_MASK];

       if (command >= & host -> commands [sizeof (host -> commands) / sizeof (ENetProtocol)] ||
           buffer + 1 >= & host -> buffers [sizeof (host -> buffers) / sizeof (ENetBuffer)] ||
           peer -> mtu - host -> packetSize < commandSize ||
           (outgoingCommand -> packet != NULL &&
             peer -> mtu - host -> packetSize < commandSize + outgoingCommand -> packet -> dataLength))
       {
          host -> continueSending = 1;

          break;
       }

       currentCommand = enet_list_next (currentCommand);

       if (outgoingCommand -> packet != NULL)
       {
          peer -> packetThrottleCounter += ENET_PEER_PACKET_THROTTLE_COUNTER;
          peer -> packetThrottleCounter %= ENET_PEER_PACKET_THROTTLE_SCALE;
          
          if (peer -> packetThrottleCounter > peer -> packetThrottle)
          {
             -- outgoingCommand -> packet -> referenceCount;

             if (outgoingCommand -> packet -> referenceCount == 0)
               enet_packet_destroy (outgoingCommand -> packet);
         
             enet_list_remove (& outgoingCommand -> outgoingCommandList);
             enet_free (outgoingCommand);
           
             continue;
          }
       }

       buffer -> data = command;
       buffer -> dataLength = commandSize;
      
       host -> packetSize += buffer -> dataLength;

       * command = outgoingCommand -> command;
       
       enet_list_remove (& outgoingCommand -> outgoingCommandList);

       if (outgoingCommand -> packet != NULL)
       {
          ++ buffer;
          
          buffer -> data = outgoingCommand -> packet -> data;
          buffer -> dataLength = outgoingCommand -> packet -> dataLength;

          host -> packetSize += buffer -> dataLength;

          enet_list_insert (enet_list_end (& peer -> sentUnreliableCommands), outgoingCommand);
       }
       else
         enet_free (outgoingCommand);

       ++ command;
       ++ buffer;
    } 

    host -> commandCount = command - host -> commands;
    host -> bufferCount = buffer - host -> buffers;

    if (peer -> state == ENET_PEER_STATE_DISCONNECT_LATER && 
        enet_list_empty (& peer -> outgoingReliableCommands) &&
        enet_list_empty (& peer -> outgoingUnreliableCommands) && 
        enet_list_empty (& peer -> sentReliableCommands))
      enet_peer_disconnect (peer, peer -> disconnectData);
}

static int
enet_protocol_check_timeouts (ENetHost * host, ENetPeer * peer, ENetEvent * event)
{
    ENetOutgoingCommand * outgoingCommand;
    ENetListIterator currentCommand, insertPosition;

    currentCommand = enet_list_begin (& peer -> sentReliableCommands);
    insertPosition = enet_list_begin (& peer -> outgoingReliableCommands);

    while (currentCommand != enet_list_end (& peer -> sentReliableCommands))
    {
       outgoingCommand = (ENetOutgoingCommand *) currentCommand;

       currentCommand = enet_list_next (currentCommand);

       if (ENET_TIME_DIFFERENCE (host -> serviceTime, outgoingCommand -> sentTime) < outgoingCommand -> roundTripTimeout)
         continue;

       if (peer -> earliestTimeout == 0 ||
           ENET_TIME_LESS (outgoingCommand -> sentTime, peer -> earliestTimeout))
         peer -> earliestTimeout = outgoingCommand -> sentTime;

       if (peer -> earliestTimeout != 0 &&
             (ENET_TIME_DIFFERENCE (host -> serviceTime, peer -> earliestTimeout) >= ENET_PEER_TIMEOUT_MAXIMUM ||
               (outgoingCommand -> roundTripTimeout >= outgoingCommand -> roundTripTimeoutLimit &&
                 ENET_TIME_DIFFERENCE (host -> serviceTime, peer -> earliestTimeout) >= ENET_PEER_TIMEOUT_MINIMUM)))
       {
          enet_protocol_notify_disconnect (host, peer, event);

          return 1;
       }

       if (outgoingCommand -> packet != NULL)
         peer -> reliableDataInTransit -= outgoingCommand -> fragmentLength;
          
       ++ peer -> packetsLost;

       outgoingCommand -> roundTripTimeout *= 2;

       enet_list_insert (insertPosition, enet_list_remove (& outgoingCommand -> outgoingCommandList));

       if (currentCommand == enet_list_begin (& peer -> sentReliableCommands) &&
           ! enet_list_empty (& peer -> sentReliableCommands))
       {
          outgoingCommand = (ENetOutgoingCommand *) currentCommand;

          peer -> nextTimeout = outgoingCommand -> sentTime + outgoingCommand -> roundTripTimeout;
       }
    }
    
    return 0;
}

static void
enet_protocol_send_reliable_outgoing_commands (ENetHost * host, ENetPeer * peer)
{
    ENetProtocol * command = & host -> commands [host -> commandCount];
    ENetBuffer * buffer = & host -> buffers [host -> bufferCount];
    ENetOutgoingCommand * outgoingCommand;
    ENetListIterator currentCommand;
    ENetChannel *channel;
    enet_uint16 reliableWindow;
    size_t commandSize;

    currentCommand = enet_list_begin (& peer -> outgoingReliableCommands);
    
    while (currentCommand != enet_list_end (& peer -> outgoingReliableCommands))
    {
       outgoingCommand = (ENetOutgoingCommand *) currentCommand;

       channel = outgoingCommand -> command.header.channelID < peer -> channelCount ? & peer -> channels [outgoingCommand -> command.header.channelID] : NULL;
       reliableWindow = outgoingCommand -> reliableSequenceNumber / ENET_PEER_RELIABLE_WINDOW_SIZE;
       if (channel != NULL && 
           outgoingCommand -> sendAttempts < 1 && 
           ! (outgoingCommand -> reliableSequenceNumber % ENET_PEER_RELIABLE_WINDOW_SIZE) &&
           (channel -> reliableWindows [(reliableWindow + ENET_PEER_RELIABLE_WINDOWS - 1) % ENET_PEER_RELIABLE_WINDOWS] >= ENET_PEER_RELIABLE_WINDOW_SIZE ||
             channel -> usedReliableWindows & ((((1 << ENET_PEER_FREE_RELIABLE_WINDOWS) - 1) << reliableWindow) | 
               (((1 << ENET_PEER_FREE_RELIABLE_WINDOWS) - 1) >> (ENET_PEER_RELIABLE_WINDOW_SIZE - reliableWindow)))))
         break;
  
       commandSize = commandSizes [outgoingCommand -> command.header.command & ENET_PROTOCOL_COMMAND_MASK];
       if (command >= & host -> commands [sizeof (host -> commands) / sizeof (ENetProtocol)] ||
           buffer + 1 >= & host -> buffers [sizeof (host -> buffers) / sizeof (ENetBuffer)] ||
           peer -> mtu - host -> packetSize < commandSize)
       {
          host -> continueSending = 1;
          
          break;
       }

       if (outgoingCommand -> packet != NULL)
       {
          if (peer -> reliableDataInTransit + outgoingCommand -> fragmentLength > peer -> windowSize)
            break;

          if ((enet_uint16) (peer -> mtu - host -> packetSize) < (enet_uint16) (commandSize + outgoingCommand -> fragmentLength))
          {
             host -> continueSending = 1;

             break;
          }
       }
      
       currentCommand = enet_list_next (currentCommand);

       if (channel != NULL && outgoingCommand -> sendAttempts < 1)
       {
          channel -> usedReliableWindows |= 1 << reliableWindow;
          ++ channel -> reliableWindows [reliableWindow];
       }

       ++ outgoingCommand -> sendAttempts;
 
       if (outgoingCommand -> roundTripTimeout == 0)
       {
          outgoingCommand -> roundTripTimeout = peer -> roundTripTime + 4 * peer -> roundTripTimeVariance;
          outgoingCommand -> roundTripTimeoutLimit = ENET_PEER_TIMEOUT_LIMIT * outgoingCommand -> roundTripTimeout;
       }

       if (enet_list_empty (& peer -> sentReliableCommands))
         peer -> nextTimeout = host -> serviceTime + outgoingCommand -> roundTripTimeout;

       enet_list_insert (enet_list_end (& peer -> sentReliableCommands),
                         enet_list_remove (& outgoingCommand -> outgoingCommandList));

       outgoingCommand -> sentTime = host -> serviceTime;

       buffer -> data = command;
       buffer -> dataLength = commandSize;

       host -> packetSize += buffer -> dataLength;
       host -> headerFlags |= ENET_PROTOCOL_HEADER_FLAG_SENT_TIME;

       * command = outgoingCommand -> command;

       if (outgoingCommand -> packet != NULL)
       {
          ++ buffer;
          
          buffer -> data = outgoingCommand -> packet -> data + outgoingCommand -> fragmentOffset;
          buffer -> dataLength = outgoingCommand -> fragmentLength;

          host -> packetSize += outgoingCommand -> fragmentLength;

          peer -> reliableDataInTransit += outgoingCommand -> fragmentLength;
       }

       ++ peer -> packetsSent;
        
       ++ command;
       ++ buffer;
    }

    host -> commandCount = command - host -> commands;
    host -> bufferCount = buffer - host -> buffers;
}

static int
enet_protocol_send_outgoing_commands (ENetHost * host, ENetEvent * event, int checkForTimeouts)
{
    ENetProtocolHeader header;
    ENetPeer * currentPeer;
    int sentLength;
    
    host -> continueSending = 1;

    while (host -> continueSending)
    for (host -> continueSending = 0,
           currentPeer = host -> peers;
         currentPeer < & host -> peers [host -> peerCount];
         ++ currentPeer)
    {
        if (currentPeer -> state == ENET_PEER_STATE_DISCONNECTED ||
            currentPeer -> state == ENET_PEER_STATE_ZOMBIE)
          continue;

        host -> headerFlags = 0;
        host -> commandCount = 0;
        host -> bufferCount = 1;
        host -> packetSize = sizeof (ENetProtocolHeader);

        if (! enet_list_empty (& currentPeer -> acknowledgements))
          enet_protocol_send_acknowledgements (host, currentPeer);

        if (checkForTimeouts != 0 &&
            ! enet_list_empty (& currentPeer -> sentReliableCommands) &&
            ENET_TIME_GREATER_EQUAL (host -> serviceTime, currentPeer -> nextTimeout) &&
            enet_protocol_check_timeouts (host, currentPeer, event) == 1)
          return 1;

        if (! enet_list_empty (& currentPeer -> outgoingReliableCommands))
          enet_protocol_send_reliable_outgoing_commands (host, currentPeer);
        else
        if (enet_list_empty (& currentPeer -> sentReliableCommands) &&
            ENET_TIME_DIFFERENCE (host -> serviceTime, currentPeer -> lastReceiveTime) >= ENET_PEER_PING_INTERVAL &&
            currentPeer -> mtu - host -> packetSize >= sizeof (ENetProtocolPing))
        { 
            enet_peer_ping (currentPeer);
            enet_protocol_send_reliable_outgoing_commands (host, currentPeer);
        }
                      
        if (! enet_list_empty (& currentPeer -> outgoingUnreliableCommands))
          enet_protocol_send_unreliable_outgoing_commands (host, currentPeer);

        if (host -> commandCount == 0)
          continue;

        if (currentPeer -> packetLossEpoch == 0)
          currentPeer -> packetLossEpoch = host -> serviceTime;
        else
        if (ENET_TIME_DIFFERENCE (host -> serviceTime, currentPeer -> packetLossEpoch) >= ENET_PEER_PACKET_LOSS_INTERVAL &&
            currentPeer -> packetsSent > 0)
        {
           enet_uint32 packetLoss = currentPeer -> packetsLost * ENET_PEER_PACKET_LOSS_SCALE / currentPeer -> packetsSent;

#ifdef ENET_DEBUG
#ifdef WIN32
           printf (
#else
           fprintf (stderr, 
#endif
                    "peer %u: %f%%+-%f%% packet loss, %u+-%u ms round trip time, %f%% throttle, %u/%u outgoing, %u/%u incoming\n", currentPeer -> incomingPeerID, currentPeer -> packetLoss / (float) ENET_PEER_PACKET_LOSS_SCALE, currentPeer -> packetLossVariance / (float) ENET_PEER_PACKET_LOSS_SCALE, currentPeer -> roundTripTime, currentPeer -> roundTripTimeVariance, currentPeer -> packetThrottle / (float) ENET_PEER_PACKET_THROTTLE_SCALE, enet_list_size (& currentPeer -> outgoingReliableCommands), enet_list_size (& currentPeer -> outgoingUnreliableCommands), currentPeer -> channels != NULL ? enet_list_size (& currentPeer -> channels -> incomingReliableCommands) : 0, currentPeer -> channels != NULL ? enet_list_size (& currentPeer -> channels -> incomingUnreliableCommands) : 0);
#endif
          
           currentPeer -> packetLossVariance -= currentPeer -> packetLossVariance / 4;

           if (packetLoss >= currentPeer -> packetLoss)
           {
              currentPeer -> packetLoss += (packetLoss - currentPeer -> packetLoss) / 8;
              currentPeer -> packetLossVariance += (packetLoss - currentPeer -> packetLoss) / 4;
           }
           else
           {
              currentPeer -> packetLoss -= (currentPeer -> packetLoss - packetLoss) / 8;
              currentPeer -> packetLossVariance += (currentPeer -> packetLoss - packetLoss) / 4;
           }

           currentPeer -> packetLossEpoch = host -> serviceTime;
           currentPeer -> packetsSent = 0;
           currentPeer -> packetsLost = 0;
        }

        header.checksum = currentPeer -> sessionID;
        header.peerID = ENET_HOST_TO_NET_16 (currentPeer -> outgoingPeerID | host -> headerFlags);
        
        host -> buffers -> data = & header;
        if (host -> headerFlags & ENET_PROTOCOL_HEADER_FLAG_SENT_TIME)
        {
            header.sentTime = ENET_HOST_TO_NET_16 (host -> serviceTime & 0xFFFF);

            host -> buffers -> dataLength = sizeof (ENetProtocolHeader);
        }
        else
          host -> buffers -> dataLength = (size_t) & ((ENetProtocolHeader *) 0) -> sentTime;
 
#ifdef USE_CRC32
        header.checksum = enet_crc32 (host -> buffers, host -> bufferCount);
#endif

        currentPeer -> lastSendTime = host -> serviceTime;

        sentLength = enet_socket_send (host -> socket, & currentPeer -> address, host -> buffers, host -> bufferCount);

        enet_protocol_remove_sent_unreliable_commands (currentPeer);

        if (sentLength < 0)
          return -1;
    }
   
    return 0;
}

/** Sends any queued packets on the host specified to its designated peers.

    @param host   host to flush
    @remarks this function need only be used in circumstances where one wishes to send queued packets earlier than in a call to enet_host_service().
    @ingroup host
*/
void
enet_host_flush (ENetHost * host)
{
    host -> serviceTime = enet_time_get ();

    enet_protocol_send_outgoing_commands (host, NULL, 0);
}

/** Checks for any queued events on the host and dispatches one if available.

    @param host    host to check for events
    @param event   an event structure where event details will be placed if available
    @retval > 0 if an event was dispatched
    @retval 0 if no events are available
    @retval < 0 on failure
    @ingroup host
*/
int
enet_host_check_events (ENetHost * host, ENetEvent * event)
{
    if (event == NULL) return -1;

    event -> type = ENET_EVENT_TYPE_NONE;
    event -> peer = NULL;
    event -> packet = NULL;

    return enet_protocol_dispatch_incoming_commands (host, event);
}

/** Waits for events on the host specified and shuttles packets between
    the host and its peers.

    @param host    host to service
    @param event   an event structure where event details will be placed if one occurs
                   if event == NULL then no events will be delivered
    @param timeout number of milliseconds that ENet should wait for events
    @retval > 0 if an event occurred within the specified time limit
    @retval 0 if no event occurred
    @retval < 0 on failure
    @remarks enet_host_service should be called fairly regularly for adequate performance
    @ingroup host
*/
int
enet_host_service (ENetHost * host, ENetEvent * event, enet_uint32 timeout)
{
    enet_uint32 waitCondition;

    if (event != NULL)
    {
        event -> type = ENET_EVENT_TYPE_NONE;
        event -> peer = NULL;
        event -> packet = NULL;

        switch (enet_protocol_dispatch_incoming_commands (host, event))
        {
        case 1:
            return 1;

        case -1:
            perror ("Error dispatching incoming packets");

            return -1;

        default:
            break;
        }
    }

    host -> serviceTime = enet_time_get ();
    
    timeout += host -> serviceTime;

    do
    {
       if (ENET_TIME_DIFFERENCE (host -> serviceTime, host -> bandwidthThrottleEpoch) >= ENET_HOST_BANDWIDTH_THROTTLE_INTERVAL)
         enet_host_bandwidth_throttle (host);

       switch (enet_protocol_send_outgoing_commands (host, event, 1))
       {
       case 1:
          return 1;

       case -1:
          perror ("Error sending outgoing packets");

          return -1;

       default:
          break;
       }

       switch (enet_protocol_receive_incoming_commands (host, event))
       {
       case 1:
          return 1;

       case -1:
          perror ("Error receiving incoming packets");

          return -1;

       default:
          break;
       }

       switch (enet_protocol_send_outgoing_commands (host, event, 1))
       {
       case 1:
          return 1;

       case -1:
          perror ("Error sending outgoing packets");

          return -1;

       default:
          break;
       }

       if (event != NULL)
       {
          switch (enet_protocol_dispatch_incoming_commands (host, event))
          {
          case 1:
             return 1;

          case -1:
             perror ("Error dispatching incoming packets");

             return -1;

          default:
             break;
          }
       }

       host -> serviceTime = enet_time_get ();

       if (ENET_TIME_GREATER_EQUAL (host -> serviceTime, timeout))
         return 0;

       waitCondition = ENET_SOCKET_WAIT_RECEIVE;

       if (enet_socket_wait (host -> socket, & waitCondition, ENET_TIME_DIFFERENCE (timeout, host -> serviceTime)) != 0)
         return -1;
       
       host -> serviceTime = enet_time_get ();
    } while (waitCondition == ENET_SOCKET_WAIT_RECEIVE);

    return 0; 
}

