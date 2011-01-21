//  $Id$
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

#include "network/message.hpp"

#include <string>
#include <math.h>
#include <stdexcept>
#include <assert.h>

/** Creates a message to be sent.
 *  This only initialised the data structures, it does not reserve any memory.
 *  A call to allocate() is therefore necessary.
 *  \param type  The type of the message
 */
Message::Message(MessageType type)
{
    assert(sizeof(int)==4);
    m_type          = type;
    m_pos           = 0;
    m_data_size     = -1;
    m_data          = NULL;
    m_needs_destroy = 0;    // enet destroys message after send
}   // Message

// ----------------------------------------------------------------------------
/** Handles a received message.
 *  The message in pkt is received, and the message type is checked.
 *  \param pkt The ENetPacket
 *  \param m   The type of the message. The type is checked via an assert!
 */

Message::Message(ENetPacket* pkt, MessageType m)
{
    receive(pkt, m);
}

// ----------------------------------------------------------------------------
/** Loads the message in pkt, and checks for the correct message type.
 *  Paramaters:
 *  \param pkt The ENetPacket
 *  \param m   The type of the message. The type is checked via an assert!
 */
void Message::receive(ENetPacket* pkt, MessageType m)
{
    assert(sizeof(int)==4);
    m_pkt           = pkt;
    m_data_size     = pkt->dataLength;
    m_data          = (char*)pkt->data;
    m_type          = (MessageType)m_data[0];
    if(m_type!=m) 
        printf("type %d %d\n",m_type,m);
    assert(m_type==m);
    m_pos           = 1; 
    m_needs_destroy = true;
}  // Message

// ----------------------------------------------------------------------------
/** Frees the memory allocated for this message. */
Message::~Message()
{
    clear();
}   // ~Message

// ----------------------------------------------------------------------------
/** Frees the memory for a received message.
 *  Calls enet_packet_destroy if necessary (i.e. if the message was received).
 *  The memory for a message created to be sent does not need to be freed, it 
 *  is handled by enet. */
void Message::clear()
{
    if(m_needs_destroy)
        enet_packet_destroy(m_pkt);
}   // clear

// ----------------------------------------------------------------------------
/** Reserves the memory for a message. 
 *  \param size Number of bytes to reserve.
 */
void Message::allocate(int size)
{
    m_data_size = size+1;
    m_pkt       = enet_packet_create (NULL, m_data_size, ENET_PACKET_FLAG_RELIABLE);
    m_data      = (char*)m_pkt->data;
    m_data[0]   = m_type;
    m_pos       = 1;
}   // allocate

// ----------------------------------------------------------------------------
/** Adds an integer value to the message.
 *  \param data The integer value to add.
 */
void Message::addInt(int data)
{
    assert((int)(m_pos + sizeof(int)) <= m_data_size);
    int l=htonl(data);
    memcpy(m_data+m_pos, &l, sizeof(int)); 
    m_pos+=sizeof(int);
}   // addInt

// ----------------------------------------------------------------------------
/** Extracts an integer value from a message.
 *  \return The extracted integer.
 */
int Message::getInt()
{
    m_pos+=sizeof(int);
    return ntohl(*(int*)(&m_data[m_pos-sizeof(int)]));
}   // getInt

// ----------------------------------------------------------------------------
/** Adds a short value to the message.
 *  \param data The integer value to add.
 */
void Message::addShort(short data)
{
    assert((int)(m_pos + sizeof(short)) <= m_data_size);
    int l=htons(data);
    memcpy(m_data+m_pos, &l, sizeof(short)); 
    m_pos+=sizeof(short);
}   // addShort

// ----------------------------------------------------------------------------
/** Extracts a short value from a message.
 *  \return The short value.
 */
short Message::getShort()
{
    m_pos+=sizeof(short);
    return ntohs(*(short*)(&m_data[m_pos-sizeof(short)]));
}   // getShort

// ----------------------------------------------------------------------------
/** Adds a floating point value to the message.
 *  \param data Floating point value to add.
 */
void Message::addFloat(const float data)
{
    // The simple approach of using  addInt(*(int*)&data)
    // does not work (at least with optimisation on certain g++ versions,
    // see getFloat for more details)
    int n;
    memcpy(&n, &data, sizeof(float));
    addInt(n);
}   // addFloat
// ----------------------------------------------------------------------------
float Message::getFloat()
{ 
    int i    = getInt();
    float f;
    memcpy(&f, &i, sizeof(int));
    return f;
    // The 'obvious' way:
    // float *f = (float*) &i;
    // return *f;
    // does NOT work, see http://www.velocityreviews.com/forums/showthread.php?t=537336
    // for details
}   // getFloat

// ----------------------------------------------------------------------------
void Message::addString(const std::string &data)
{ 
    int len = data.size()+1;  // copy 0 end byte
    assert((int)(m_pos+len) <=m_data_size);
    memcpy (&(m_data[m_pos]), data.c_str(), len);
    m_pos += len;
}   // addString

// ----------------------------------------------------------------------------
std::string Message::getString()
{
    char *str = (char*) &(m_data[m_pos]);
    int len = strlen(str)+1;
    m_pos += len;
    return std::string(str);
}   // getString

// ----------------------------------------------------------------------------
/** Returns the number of bytes necessary to store a string vector.
 *  \param vs std::vector<std::string>
 */
int Message::getStringVectorLength(const std::vector<std::string>& vs)
{
    int len=getShortLength();
    for(unsigned int i=0; i<vs.size(); i++)
        len += getStringLength(vs[i]);
    return len;
}   // getStringVectorLength

// ----------------------------------------------------------------------------
/** Adds a std::vector<std::string> to the message.
 */
void Message::addStringVector(const std::vector<std::string>& vs)
{
    assert(vs.size()<32767);
    addShort(vs.size());
    for(unsigned short i=0; i<vs.size(); i++)
        addString(vs[i]);
}   // addStringVector

// ----------------------------------------------------------------------------
std::vector<std::string> Message::getStringVector()
{
    std::vector<std::string> vs;
    vs.resize(getShort());
    for(unsigned int i=0; i<vs.size(); i++)
        vs[i]=getString();
    return vs;
}   // getStringVector
// ----------------------------------------------------------------------------
