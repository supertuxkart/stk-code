//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "states_screens/dialogs/story_mode_new.hpp"

#include "config/user_config.hpp"
#include "config/player.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

StoryModeNewDialog::StoryModeNewDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("story_mode_new.stkgui");
    
    SpinnerWidget* ident = getWidget<SpinnerWidget>("identity");
    
    const int playerAmount = UserConfigParams::m_all_players.size();
    ident->setMax(playerAmount - 1);
    for(int n=0; n<playerAmount; n++)
    {
        ident->addLabel( translations->fribidize(UserConfigParams::m_all_players[n].getName()) );
    }
    
    RibbonWidget* difficulty = getWidget<RibbonWidget>("difficulty");
    difficulty->setSelection( 1 /* medium */, PLAYER_ID_GAME_MASTER );
}

// -----------------------------------------------------------------------------

StoryModeNewDialog::~StoryModeNewDialog()
{
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation StoryModeNewDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "cancel")
    {
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "startgame")
    {
        // TODO
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------


