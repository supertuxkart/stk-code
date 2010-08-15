//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#include "states_screens/dialogs/race_paused_dialog.hpp"

#include <string>

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/help_screen_1.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

// ----------------------------------------------------------------------------

RacePausedDialog::RacePausedDialog(const float percentWidth, 
                                   const float percentHeight) :
    ModalDialog(percentWidth, percentHeight)
{
    loadFromFile("race_paused_dialog.stkgui");
    
    World::getWorld()->pause(WorldStatus::IN_GAME_MENU_PHASE);

    ButtonWidget* back_btn = (ButtonWidget*)Screen::getWidget("backbtn", &m_children);
    back_btn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
}   // RacePausedDialog

// ----------------------------------------------------------------------------
RacePausedDialog::~RacePausedDialog()
{
    World::getWorld()->unpause();
}   // ~RacePausedDialog

// ----------------------------------------------------------------------------

void RacePausedDialog::loadedFromFile()
{
    printf("==== RacePausedDialog::loadedFromFile() ====\n");

    // disable the "restart" button in GPs
    if (race_manager->getMajorMode() != RaceManager::MAJOR_MODE_SINGLE)
    {
        printf("==== REMOVING restart button ====\n");
        GUIEngine::RibbonWidget* choice_ribbon = (GUIEngine::RibbonWidget*)
                Screen::getWidget("choiceribbon", &m_children);
        
        
        const bool success = choice_ribbon->deleteChild("restart");
        assert(success);
    }
}

// ----------------------------------------------------------------------------

void RacePausedDialog::onEnterPressedInternal()
{
}   // onEnterPressedInternal

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation 
           RacePausedDialog::processEvent(const std::string& eventSource)
{
    GUIEngine::RibbonWidget* chocie_ribbon = (GUIEngine::RibbonWidget*)
            Screen::getWidget("choiceribbon", &m_children);
    
    if (UserConfigParams::m_verbosity>=5)
    {
       std::cout << "RacePausedDialog::processEvent(" 
                 << eventSource.c_str() << ")\n";
    }
    
    if (eventSource == "backbtn")
    {
        // unpausing is done in the destructor so nothing more to do here
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "choiceribbon")
    {
        const std::string& selection = 
            chocie_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        if(UserConfigParams::m_verbosity>=5)
            std::cout << "RacePausedDialog::processEvent(" 
                      << eventSource.c_str() << " : " << selection << ")\n";

        if (selection == "exit")
        {
            ModalDialog::dismiss();
            race_manager->exitRace();
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "help")
        {
            dismiss();
            StateManager::get()->pushScreen(HelpScreen1::getInstance());
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "options")
        {
            dismiss();
            StateManager::get()->pushScreen(OptionsScreenVideo::getInstance());
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "restart")
        {
            ModalDialog::dismiss();
            network_manager->setState(NetworkManager::NS_MAIN_MENU);
            World::getWorld()->unpause();
            race_manager->rerunRace();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "newrace")
        {
            ModalDialog::dismiss();
            World::getWorld()->unpause();
            race_manager->exitRace();
            Screen* newStack[] = {MainMenuScreen::getInstance(), 
                                  RaceSetupScreen::getInstance(), NULL};
            StateManager::get()->resetAndSetStack( newStack );
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------


