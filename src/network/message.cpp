//  $Id: message.cpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#include "message.hpp"
#include <string>
#include <math.h>
#include <stdexcept>


// need a more elegant way of setting the data_size, esp when strings are being used

// also, looking at how the packets are actually used, we can probably serialise as 
// part of the constructor, it seems packets to be sent are always created in a 
// single line
#include <assert.h>

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
Message::Message(ENetPacket* pkt, MessageType m)
{
    assert(sizeof(int)==4);
    m_pkt           = pkt;
    m_data_size     = pkt->dataLength;
    m_data          = (char*)pkt->data;
    m_type          = (MessageType)m_data[0];
    assert(m_type==m);
    m_pos           = 1; 
    m_needs_destroy = true;
}  // Message

// ----------------------------------------------------------------------------
Message::~Message()
{
    if(m_needs_destroy)
        enet_packet_destroy(m_pkt);
}   // ~Message

// ----------------------------------------------------------------------------
void Message::allocate(int size)
{
    m_data_size = size+1;
    m_pkt       = enet_packet_create (NULL, m_data_size, ENET_PACKET_FLAG_RELIABLE);
    m_data      = (char*)m_pkt->data;
    m_data[0]   = m_type;
    m_pos       = 1;
}   // allocate

// ----------------------------------------------------------------------------
bool Message::add(int data)
{
    if ((int)(m_pos + sizeof(int)) > m_data_size) 
        return false;
    int l=htonl(data);
    memcpy(m_data+m_pos, &l, sizeof(int)); 
    m_pos+=sizeof(int);
    return true;
}   // add<int>

// ----------------------------------------------------------------------------
int Message::getInt()
{
    m_pos+=sizeof(int);
    return ntohl(*(int*)(&m_data[m_pos-sizeof(int)]));
}   // getInt

// ----------------------------------------------------------------------------
float Message::getFloat()
{ // ugly...
    int i    = getInt();
    float *f = (float*) &i;
    return *f;
}   // getFloat

// ----------------------------------------------------------------------------
bool Message::add(const std::string &data)
{ 
    int len = data.size()+1;  // copy 0 end byte
    assert((int)(m_pos+len) <=m_data_size);
    memcpy (&(m_data[m_pos]), data.c_str(), len);
    m_pos += len;
    return true;
}   // add<string>

// ----------------------------------------------------------------------------
std::string Message::getString()
{
    char *str = (char*) &(m_data[m_pos]);
    int len = strlen(str)+1;
    m_pos += len;
    return std::string(str);
}   // getString

// ----------------------------------------------------------------------------
int Message::getLength(const std::vector<std::string>& vs)
{
    int len=getLength(vs.size());
    for(unsigned int i=0; i<vs.size(); i++)
        len += getLength(vs[i]);
    return len;
}   // getLength(vector<string>)

// ----------------------------------------------------------------------------
bool Message::add(const std::vector<std::string>& vs)
{
    bool result = add(vs.size());
    if(!result) return false;
    for(unsigned int i=0; i<vs.size(); i++)
        if(!add(vs[i])) return false;
    return true;
}   // add(vector<string>)
// ----------------------------------------------------------------------------
std::vector<std::string> Message::getStringVector()
{
    std::vector<std::string> vs;
    vs.resize(getInt());
    for(unsigned int i=0; i<vs.size(); i++)
        vs[i]=getString();
    return vs;
}   // getStringVector
// ----------------------------------------------------------------------------
