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
    m_state = network_manager->getMode()==NetworkManager::NW_NONE ? SRF_LOADING : SRF_NETWORK;

    //Add some feedback so people know they are going to start the race
    widget_manager->reset();
    if(m_state==SRF_NETWORK)
        widget_manager->addTextWgt( WTOK_MSG, 60, 7, _("Synchronising network...") );
    else
        widget_manager->addTextWgt( WTOK_MSG, 60, 7, _("Loading race...") );

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
StartRaceFeedback::~StartRaceFeedback()
{
    widget_manager->reset();
}


//-----------------------------------------------------------------------------
void StartRaceFeedback::update(float DELTA)
{
    widget_manager->update(0.0f);

    // We need one call to update to display the current text. So we use a
    // simple finite state machine to take care of one additional call:
    switch(m_state)
    {
        case SRF_LOADING_DISPLAY: 
            m_state=SRF_LOADING;
            break;
        case SRF_NETWORK_DISPLAY: 
            m_state = SRF_NETWORK; 
            break;
        case SRF_NETWORK:
            if(network_manager->getMode()==NetworkManager::NW_SERVER)
                network_manager->sendRaceInformationToClients();
            else
                network_manager->waitForRaceInformation();
            m_state = SRF_LOADING_DISPLAY;
            widget_manager->setWgtText(WTOK_MSG, _("Loading race...") );
            break;
        case SRF_LOADING:
            race_manager->startNew();
            break;
    }   // switch m_state

}   // update

