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

#include "kart_packet.hpp"
#include <string>
#include <math.h>
#include <stdexcept>


// need a more elegant way of setting the data_size, esp when strings are being used

// also, looking at how the packets are actually used, we can probably serialise as 
// part of the constructor, it seems packets to be sent are always created in a 
// single line

KartPacket::KartPacket(int data_size)
{
  m_data_size = data_size;
  m_pkt = enet_packet_create (NULL, data_size*sizeof(int), ENET_PACKET_FLAG_RELIABLE);
  m_pos = 0;
  m_data = (int*) m_pkt->data;
}

KartPacket::KartPacket(ENetPacket* pkt)
{
  m_pkt = pkt;
  m_data = (int*) pkt->data;
  m_data_size = pkt->dataLength/sizeof(int);
  m_pos = 0; 
}

KartPacket::~KartPacket()
{
  enet_packet_destroy(m_pkt);
}

bool KartPacket::pushInt(int &data)
{
  if (m_pos > m_data_size) 
    return false;
  m_data[m_pos] = htonl(data); 
  ++m_pos;
  return true;
}

int KartPacket::pullInt()
{
  m_pos++;
  return ntohl(m_data[m_pos-1]);
}  

bool KartPacket::pushFloat(float &data)
{
  int *dcast = (int*) &data;
  return pushInt(*dcast);
}

float KartPacket::pullFloat()
{ // ugly...
  int i = pullInt();
  float *f = (float*) &i;
  return *f;
}

bool KartPacket::pushString(std::string &data)
{ // urk, this is a little ugly. want to put the string on a 4-byte boundary 
  int len = (int)ceil((float)data.length() / 4.0f) * 4; // round length up to next 4-char boundary 
  if (m_pos+len > m_data_size)
    return false;   // FIXME: resize the packet so it fits
  memcpy (&(m_data[m_pos]), data.c_str(), data.length()+1);
  m_pos += len;
  return true;
}

std::string KartPacket::pullString()
{
  char *str = (char*) &(m_data[m_pos]);
  int len = strlen(str)+1;
  int len4 = len/sizeof(int) + 1;
  m_pos += len4;  // I think this is correct ..
  return std::string(str);
}

bool KartPacket::send(ENetPeer& peer)
{
  this->serialise();
  enet_peer_send(&peer, 0, m_pkt);
  return true;
}

// ---------------------------------------------------------------------

ClientHostIdPacket::ClientHostIdPacket(int id)
  : KartPacket(2)
{ // create a ClientHostIdPacket with id as data
  m_type = CLIENT_HOST_ID_PACKET;
  m_id = id;
}

ClientHostIdPacket::ClientHostIdPacket(ENetPacket* pkt)
  : KartPacket(pkt)
{ // create a ClientHostIdPacket based on a received packet
  m_type = static_cast<PacketType> (pullInt()); // (ntohl(m_data[0]));
  if (m_type != CLIENT_HOST_ID_PACKET) 
    // FIXME: do something more elegant here
    throw std::runtime_error("Received packet mismatch!");
  m_id = pullInt(); // ntohl(m_data[1]);
}

void ClientHostIdPacket::serialise()
{
  int type = m_type;
  pushInt(type);
  pushInt(m_id);
}

// --------------------------------------------------------------------

LocalKartInfoPacket::LocalKartInfoPacket(const std::string& player_name,
                                         const std::string& kart_name)
  : KartPacket(128) // a bit arbitrary, just to check it works
{
  m_type = LOCAL_KART_INFO_PACKET;
  m_player_name = std::string(player_name); 
  m_kart_name   = std::string(kart_name); 
}

LocalKartInfoPacket::LocalKartInfoPacket(ENetPacket* pkt)
  : KartPacket(pkt)
{ // create a LocalKartInfoPacket based on a received packet
  m_type = static_cast<PacketType> (pullInt()); //   (ntohl(m_data[0]));
  if (m_type != LOCAL_KART_INFO_PACKET)
    // FIXME: do something more elegant here
    throw std::runtime_error("Received packet mismatch!");
  m_player_name = pullString(); // new string((char*) &(m_data[1]));
  m_kart_name   = pullString(); // new string(&(m_data[1]));
}

//LocalKartInfoPacket::~LocalKartInfoPacket()
//{
//  delete m_player_name;
//  delete m_kart_name;
//  // FIXME: poss mem leak here, need to call ~KartPacket
//}

void LocalKartInfoPacket::serialise()
{
  int type = m_type;
  pushInt(type);
  pushString(m_player_name);
  pushString(m_kart_name);
}
