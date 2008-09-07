//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <SDL/SDL.h>

#include "start_race_feedback.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "translation.hpp"
#include "network/network_manager.hpp"

enum WidgetTokens
{
    WTOK_MSG
};

StartRaceFeedback::StartRaceFeedback()
{
    //Add some feedback so people know they are going to start the race
    widget_manager->reset();
    widget_manager->addTextWgt( WTOK_MSG, 60, 7, "" );
    m_loading_text       = _("Loading race...");
    m_synchronising_text = _("Synchronising network...");
    if(network_manager->getMode()==NetworkManager::NW_NONE)
        widget_manager->setWgtText(WTOK_MSG, m_loading_text);
    else  // networking
        // the state and mode are checked in update()
        widget_manager->setWgtText(WTOK_MSG, m_synchronising_text);

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
StartRaceFeedback::~StartRaceFeedback()
{
    widget_manager->reset();
}


//-----------------------------------------------------------------------------
void StartRaceFeedback::update(float delta)
{

    // If the server hasn't received all client information, keep on waiting
    if(network_manager->getMode()==NetworkManager::NW_SERVER &&
       network_manager->getState()!=NetworkManager::NS_ALL_REMOTE_CHARACTERS_DONE)
    {
        widget_manager->update(delta);
        return;
    }
    // If the client hasn't received the race data yet, keep on waiting
    if(network_manager->getMode()==NetworkManager::NW_CLIENT &&
        network_manager->getState()==NetworkManager::NS_WAIT_FOR_RACE_DATA)
    {
        widget_manager->update(delta);
        return;
    }

    if(network_manager->getMode()==NetworkManager::NW_NONE)
    {
        // This copies the loca lplayer information to the global
        // player information in the race manager
        network_manager->setupPlayerKartInfo();
    }
    else if(network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        network_manager->sendRaceInformationToClients();
        widget_manager->setWgtText(WTOK_MSG, m_loading_text);
    } 
    else if(network_manager->getMode()==NetworkManager::NW_CLIENT)
    {
        // Client received race information
        widget_manager->setWgtText(WTOK_MSG, m_loading_text);
    }

    widget_manager->update(delta);

    // Pops this menu
    race_manager->startNew();

}   // update

