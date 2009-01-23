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

#include "start_race_feedback.hpp"

#include "race_manager.hpp"
#include "gui/widget_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/translation.hpp"

enum WidgetTokens
{
    WTOK_MSG
};

/** Constructor for feedback screen. */
StartRaceFeedback::StartRaceFeedback()
{
    m_is_first_frame = true;
    //Add some feedback so people know they are going to start the race
    widget_manager->reset();
    widget_manager->addTextWgt( WTOK_MSG, 60, 7, "" );

    widget_manager->setWgtText(WTOK_MSG, _("Synchronising network..."));

    if(network_manager->getMode()==NetworkManager::NW_NONE)
    {
        // This copies the local player information to the global
        // player information in the race manager.
        network_manager->setupPlayerKartInfo();
    }

    widget_manager->layout(WGT_AREA_ALL);
}   // StartRaceFeedback

//-----------------------------------------------------------------------------
/** Destructor for feedback screen.
 */
StartRaceFeedback::~StartRaceFeedback()
{
    widget_manager->reset();
}   // ~StartRaceFeedback

//-----------------------------------------------------------------------------
/** Updates the feedback screen. Depending on the state of the network manager
 *  it will change the displayed text.
 *  \param delta Time step size.
 */
void StartRaceFeedback::update(float delta)
{
    // First test if we are still waiting
    // ===================================

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

    // Waiting is finished, switch to loading
    // ======================================
    widget_manager->setWgtText(WTOK_MSG, _("Loading race..."));
    widget_manager->update(delta);

    // We can't do the actual loading etc. in the first call here, since then
    // the text 'loading' would not be displayed. So a simple state variable
    // 'is_first_frame' is used to make sure that the text is displayed 
    // before initiating the loading of the race.
    if(!m_is_first_frame)
    {
        if(network_manager->getMode()==NetworkManager::NW_SERVER)
        {
            network_manager->sendRaceInformationToClients();
        } 
        race_manager->startNew();
    }
    m_is_first_frame = false;

}   // update

