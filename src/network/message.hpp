//  $Id: network_manager.hpp 2128 2008-06-13 00:53:52Z cosmosninja $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs, Stephen Leak
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_MESSAGE_H
#define HEADER_MESSAGE_H

#include <string>
#include <assert.h>

#ifdef HAVE_ENET
#  include "enet/enet.h"
#endif

// sjl: when a message is received, need to work out what kind of message it 
// is and therefore what to do with it

// Collects and serialises/deserialises kart info to send
class Message
{ 
public:
    enum MessageType {MT_CONNECT=1, MT_CHARACTER_INFO, 
                      MT_RACE_INFO, MT_RACE_START, MT_WORLD_LOADED};
private:
    ENetPacket  *m_pkt;
    char        *m_data;
    MessageType  m_type;
    int          m_data_size;
    int          m_pos; // simple stack counter for constructing packet data
    bool         m_needs_destroy;  // only received messages need to be destroyed

protected:
    bool         add(int data);
    bool         add(size_t data)       { return add((int)data); }
    bool         add(unsigned int data) { return add((int)data); }
    bool         add(float data);
    bool         add(const std::string &data); 
    int          getInt(); 
    float        getFloat();
    std::string  getString();
    int          getLength(const std::string& s) { return s.size()+1;    }
    int          getLength(int n)                { return sizeof(int);   }
    int          getLength(unsigned int n)       { return sizeof(int);   }
    int          getLength(size_t n)             { return sizeof(int);   }
    int          getLength(float f)              { return sizeof(float); }

public:
                 Message(MessageType m);    // create from scratch (to send)
                 Message(ENetPacket *pkt, MessageType m);  // create from (received) packet 
                ~Message();
    void         allocate(int size);
    MessageType  getType() const   { return m_type; }
    ENetPacket*  getPacket() const { assert(m_data_size>-1); return m_pkt;   }
    // Return the type of a message without unserialising the message
    static MessageType peekType(ENetPacket *pkt) 
                                   { return (MessageType)pkt->data[0];}

};   // Message


#endif

