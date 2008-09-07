//  $Id: remote_kart_information.hpp 2190 2008-07-29 05:38:30Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008  Joerg Henrichs
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

#ifndef HEADER_REMOTE_KART_INFO_H
#define HEADER_REMOTE_KART_INFO_H

#include <string>

class RemoteKartInfo
{
        std::string m_kart_name;
        std::string m_user_name;
        int         m_client_id;
        int         m_player_id;

public:
    RemoteKartInfo(std::string kart_name, std::string user_name, int id)
                : m_kart_name(kart_name), m_user_name(user_name), m_client_id(id) 
    {};
    RemoteKartInfo() {m_kart_name=""; m_user_name=""; m_client_id=-1; m_player_id=-1;}
    void setKartName(const std::string& n) { m_kart_name = n;  }
    void setUserName(const std::string& u) { m_user_name = u;  }
    void setHostId(int id)                 { m_client_id = id; }
    void setPlayerId(int id)               { m_player_id = id; }
};   // RemoteKartInfo

#endif
