//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
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

#include "network/remote_kart_info.hpp"
#include "network/network_player_profile.hpp"

// ----------------------------------------------------------------------------
void RemoteKartInfo::copyFrom(std::shared_ptr<NetworkPlayerProfile> p,
                              unsigned local_id)
{
    m_kart_name          = p->getKartName();
    m_user_name          = p->getName();
    m_local_player_id    = local_id;
    m_host_id            = p->getHostId();
    m_handicap           = p->getHandicap();
    m_default_kart_color = p->getDefaultKartColor();
    m_online_id          = p->getOnlineId();
    m_country_code       = p->getCountryCode();
    m_kart_data          = p->getKartData();
    m_profile            = p;
}   // copyFrom
