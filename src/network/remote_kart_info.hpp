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

#ifndef HEADER_REMOTE_KART_INFO_HPP
#define HEADER_REMOTE_KART_INFO_HPP

#include <string>
#include <irrString.h>

class RemoteKartInfo
{
        std::string         m_kart_name;
        irr::core::stringw  m_user_name;
        int                 m_local_player_id;
        int                 m_global_player_id;
        int                 m_host_id;

public:
         RemoteKartInfo(int player_id, const std::string& kart_name, 
                        const irr::core::stringw& user_name, int host_id)
                      : m_kart_name(kart_name), m_user_name(user_name), 
                        m_local_player_id(player_id), m_host_id(host_id) 
                                             {};
         RemoteKartInfo(const std::string& kart_name)
                                             {m_kart_name=kart_name; m_user_name="";
                                              m_host_id=-1; m_local_player_id=-1;}
         RemoteKartInfo()                    {m_kart_name=""; m_user_name=""; 
                                              m_host_id=-1; m_local_player_id=-1;}
    void setKartName(const std::string& n)   { m_kart_name = n;              }
    void setPlayerName(const irr::core::stringw& u) { m_user_name = u;              }
    void setHostId(int id)                   { m_host_id = id;               }
    void setLocalPlayerId(int id)            { m_local_player_id = id;       }
    void setGlobalPlayerId(int id)           { m_global_player_id = id;      }
    int  getHostId() const                   { return m_host_id;             }
    int  getLocalPlayerId() const            { return m_local_player_id;     }
    int  getGlobalPlayerId() const           { return m_global_player_id;    }
    const std::string& getKartName() const   { return m_kart_name;           }
    const irr::core::stringw& getPlayerName() const { return m_user_name;           }
    bool operator<(const RemoteKartInfo& other) const
    {
        return ((m_host_id<other.m_host_id) ||
                (m_host_id==other.m_host_id && m_local_player_id<other.m_local_player_id));
    }
};   // RemoteKartInfo

typedef std::vector<RemoteKartInfo> RemoteKartInfoList;

#endif
