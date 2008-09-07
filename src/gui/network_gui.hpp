//  $Id: network_gui.hpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#ifndef HEADER_NETWORK_GUI_H
#define HEADER_NETWORK_GUI_H

#include <string>
#include <SDL/SDL.h>
#include "base_gui.hpp"
#include "player.hpp"

class NetworkGUI: public BaseGUI
{
private:
    unsigned int          m_num_clients;
    std::string           m_server_address;
    enum {NGS_NONE, NGS_CONNECT_DISPLAY, NGS_CONNECT_DOIT} 
                          m_state;

	static const int      SERVER_NAME_MAX;

    void switchToWaitForConnectionMode();
public:
         NetworkGUI();
        ~NetworkGUI();
    void select();
    void update(float dt);
	void handle(GameAction, int);
	void inputKeyboard(SDLKey, int);

};

#endif
