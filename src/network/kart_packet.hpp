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

#ifndef HEADER_KART_PACKET_H
#define HEADER_KART_PACKET_H

#include <string>

#ifdef HAVE_ENET
#  include "enet/enet.h"
#else
#  error "need enet for this implementation!"
#endif

// sjl: when a message is received, need to work out what kind of message it 
// is and therefore what to do with it

class KartPacket
{ // collects and serialises/deserialises kart info to send
public:
                               KartPacket(int data_size);    // create from scratch (to send)
                               KartPacket(ENetPacket* pkt);  // create from (received) ENetacket 
                              ~KartPacket();

    enum                       PacketType { LOCAL_KART_INFO_PACKET, CLIENT_HOST_ID_PACKET };
    PacketType                 getType();
    bool                       send(ENetPeer& peer);

protected:
    bool                       pushInt(int &data);
    bool                       pushFloat(float &data);
    bool                       pushString(std::string &data); 
    int                        pullInt(); 
    float                      pullFloat();
    std::string                pullString();

    PacketType                 m_type;
    int                       *m_data;

    //virtual ENetPacket*        serialise();                  // convert KartPacket into ENetPacket
    virtual void               serialise() =0;                 // convert KartPacket into ENetPacket

private:
    ENetPacket                *m_pkt;
    int                        m_data_size;
    int                        m_pos; // simple stack counter for constructing packet data
};

class ClientHostIdPacket : public KartPacket
{ // during init phase, server sends this to client
public:
                               ClientHostIdPacket(int id);           // create one to send
                               ClientHostIdPacket(ENetPacket* pkt);  // wait for one from server

    int                        getId() { return m_id; };

protected:
    void                       serialise();                  // convert KartPacket into ENetPacket

private:
    int                        m_id;

};

class LocalKartInfoPacket : public KartPacket
{ // before a race, each client sends server one of these for each player on that client
public:
                               LocalKartInfoPacket(const std::string& player_name,
                                                   const std::string& kart_name);
                               LocalKartInfoPacket(ENetPacket* pkt);
                              // ~LocalKartInfoPacket();

    std::string                getPlayerName();
    std::string                getKartName();

protected:
    void                       serialise();                  // convert KartPacket into ENetPacket

private:
    std::string                m_player_name;
    std::string                m_kart_name;
};


#endif

