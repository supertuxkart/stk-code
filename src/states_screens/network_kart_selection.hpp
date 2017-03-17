//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef NETWORK_KART_SELECTION_HPP
#define NETWORK_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"
#include "guiengine/screen.hpp"

class NetworkKartSelectionScreen : public KartSelectionScreen,
                  public GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>
{
    friend class GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>;
protected:
    //!< map the id of the kart widgets to race ids
    std::vector<uint8_t> m_id_mapping;

    NetworkKartSelectionScreen();
    virtual ~NetworkKartSelectionScreen();

    virtual void playerConfirm(const int playerID) OVERRIDE;
public:
    virtual void init() OVERRIDE;
    virtual bool onEscapePressed() OVERRIDE;
    virtual void playerSelected(uint8_t player_id,
                                const std::string &kart_name);
};

#endif // NETWORK_KART_SELECTION_HPP
