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

#ifndef HEADER_MESSAGE_HPP
#define HEADER_MESSAGE_HPP

#include <cstring>
#include <string>
#include <vector>
#include <assert.h>
#include "btBulletDynamicsCommon.h"

using std::memcpy;

#include "enet/enet.h"

#include "utils/vec3.hpp"

// sjl: when a message is received, need to work out what kind of message it 
// is and therefore what to do with it

/** Base class to serialises/deserialises messages. 
 *  This is the base class for all messages being exchange between client
 *  and server. It handles the interface to enet, and adds a message type
 *  (which is checked via an assert to help finding bugs by receiving a 
 *  message of an incorrect type). It also takes care of endianess (though
 *  floats are converted via a byte swap, too - so it must be guaranteed 
 *  that the float representation between all machines is identical).
 */
class Message
{ 
public:
    /** Contains all tags used in identifying a message. */
    enum MessageType {MT_CONNECT=1, MT_CHARACTER_INFO, MT_CHARACTER_CONFIRM,
                      MT_RACE_INFO, MT_RACE_START, MT_WORLD_LOADED,
                      MT_KART_INFO, MT_KART_CONTROL, MT_RACE_STATE,
                      MT_RACE_RESULT, MT_RACE_RESULT_ACK
                     };
private:
    ENetPacket  *m_pkt;
    char        *m_data;
    MessageType  m_type;
    int          m_data_size;
    unsigned int m_pos; // simple stack counter for constructing packet data
    bool         m_needs_destroy;  // only received messages need to be destroyed

public:
    void         addInt(int data);
    void         addShort(short data);
    void         addString(const std::string &data); 
    void         addStringVector(const std::vector<std::string>& vs);
    void         addUInt(unsigned int data)      { addInt(*(int*)&data);  }
    void         addFloat(const float data);    
    void         addBool(bool data)              { addChar(data?1:0);     }
    void         addChar(char data)              { addCharArray((char*)&data,1);}
    void         addCharArray(char *c, unsigned int n=1) 
                                                 { assert((int)(m_pos+n)<=m_data_size);
                                                   memcpy(m_data+m_pos,c,n);
                                                   m_pos+=n;              }
#ifndef WIN32          // on windows size_t is unsigned int
    void         addSizeT(size_t data)           { addInt((int)data);     }
#endif
    void         addIntArray(int *d, unsigned int n) 
                                                 { for(unsigned int i=0; 
                                                       i<n; i++)
                                                       addInt(d[i]);      }
    void         addVec3(const Vec3& v)          { addFloat(v.getX());
                                                   addFloat(v.getY()); 
                                                   addFloat(v.getZ());    }
    void         addQuaternion(const btQuaternion& q) { addFloat(q.getX()); 
                                                   addFloat(q.getY());
                                                   addFloat(q.getZ()); 
                                                   addFloat(q.getW());    }
    int          getInt(); 
    bool         getBool()                       { return getChar()==1;   }
    short        getShort();
    float        getFloat();
    std::string  getString();
    std::vector<std::string>
                 getStringVector();
    char         getChar()                       {char c;getCharArray(&c,1);
                                                  return c;               }
    void         getCharArray(char *c, int n=1) {memcpy(c,m_data+m_pos,n);
                                                  m_pos+=n;
                                                  return;                 }
    Vec3         getVec3()                       { Vec3 v; 
                                                   v.setX(getFloat()); 
                                                   v.setY(getFloat());
                                                   v.setZ(getFloat()); 
                                                   return v;              }
    btQuaternion getQuaternion()                 { btQuaternion q; 
                                                   q.setX(getFloat());
                                                   q.setY(getFloat());
                                                   q.setZ(getFloat());
                                                   q.setW(getFloat()); 
                                                   return q;               }
    static int   getIntLength()             { return sizeof(int);     }
    static int   getUIntLength()            { return sizeof(int);     }
    static int   getShortLength()           { return sizeof(short);   }
    static int   getCharLength()            { return sizeof(char);    }
    static int   getBoolLength()            { return sizeof(char);    }
    static int   getFloatLength()           { return sizeof(float);   }
    static int   getStringLength(const std::string& s) { return s.size()+1;}
    static int   getVec3Length()            { return 3*sizeof(float); }
    static int   getQuaternionLength()      { return 4*sizeof(float); }
    static int   getStringVectorLength(const std::vector<std::string>& vs);
#ifndef WIN32
    static int   getSizeTLength(size_t n)   { return sizeof(int);     }
#endif

public:
                 Message(MessageType m);
                 Message(ENetPacket *pkt, MessageType m);  
    void         receive(ENetPacket *pkt, MessageType m);  
                ~Message();
    void         clear();
    void         allocate(int size);
    MessageType  getType() const   { return m_type; }
    ENetPacket*  getPacket() const { assert(m_data_size>-1); return m_pkt;   }
    /** Return the type of a message without unserialising the message */
    static MessageType peekType(ENetPacket *pkt) 
                                   { return (MessageType)pkt->data[0];}

};   // Message


#endif

