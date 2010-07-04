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

#include "states_screens/dialogs/race_paused_dialog.hpp"

#include "guiengine/engine.hpp"
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

#include <string>
using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

// ------------------------------------------------------------------------------------------------------

RacePausedDialog::RacePausedDialog(const float percentWidth, const float percentHeight) :
    ModalDialog(percentWidth, percentHeight)
{
    World::getWorld()->pause(WorldStatus::IN_GAME_MENU_PHASE);
    
    IGUIFont* font = GUIEngine::getTitleFont();
    const int text_height = GUIEngine::getFontHeight();
    
    IGUIFont* titlefont = GUIEngine::getTitleFont();
    const int title_height = font->getDimension(L"X").Height;
    
    
    const int icon_size = (m_area.getHeight() - text_height - 150) / 2;
    
    // ---- Caption
    core::rect< s32 > area(0, 0, m_area.getWidth(), title_height);
    IGUIStaticText* caption = GUIEngine::getGUIEnv()->addStaticText( _("Paused"),
                                                                    area, false, false, // border, word warp
                                                                    m_irrlicht_window);
    caption->setTabStop(false);
    caption->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    caption->setOverrideFont(titlefont);
    caption->setOverrideColor(video::SColor(255,255,255,255));

    // ---- Back button
    IconButtonWidget* back_btn = new IconButtonWidget();
    back_btn->m_properties[PROP_ID] = "backbtn";
    back_btn->m_properties[PROP_ICON] = "gui/back.png";
    //I18N: In the 'paused' screen
    back_btn->m_text = _("Back to Race");
    back_btn->m_x = m_area.getWidth() / 2 - icon_size;
    back_btn->m_y = text_height*2;
    back_btn->m_w = icon_size*2; // width larger to leave room for text
    back_btn->m_h = icon_size;
    back_btn->setParent(m_irrlicht_window);
    m_children.push_back(back_btn);
    back_btn->add();
    
    back_btn->setFocusForPlayer( PLAYER_ID_GAME_MASTER );
    
    // ---- Choice ribbon
    m_choice_ribbon = new RibbonWidget(RIBBON_TOOLBAR);
    m_choice_ribbon->m_properties[PROP_ID] = "choiceribbon";
    
    m_choice_ribbon->m_x = 0;
    m_choice_ribbon->m_y = text_height*2 + icon_size + 50;
    m_choice_ribbon->m_w = m_area.getWidth();
    m_choice_ribbon->m_h = icon_size + text_height;
    m_choice_ribbon->setParent(m_irrlicht_window);
    
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_SINGLE)
    {
        //I18N: In the 'paused' screen
        m_choice_ribbon->addIconChild(_("Setup New Race"), "newrace", 128, 128, "gui/main_race.png");
    }
    
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_SINGLE)
    {
        //I18N: In the 'paused' screen
        m_choice_ribbon->addIconChild(_("Restart Race"), "restart", 128, 128, "gui/restart.png");
    }

    //I18N: In the 'paused' screen
    m_choice_ribbon->addIconChild(_("Options"), "options", 128, 128, "gui/main_options.png");

    //I18N: In the 'paused' screen
    m_choice_ribbon->addIconChild(_("Help"), "help", 128, 128, "gui/main_help.png");

    //I18N: In the 'paused' screen
    m_choice_ribbon->addIconChild(_("Exit Race"), "exit", 128, 128, "gui/main_quit.png");

    m_children.push_back(m_choice_ribbon);
    m_choice_ribbon->add();   
}
// ------------------------------------------------------------------------------------------------------

void RacePausedDialog::onEnterPressedInternal()
{
}

// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation RacePausedDialog::processEvent(const std::string& eventSource)
{
    if(UserConfigParams::m_verbosity>=5)
       std::cout << "RacePausedDialog::processEvent(" 
                 << eventSource.c_str() << ")\n";
    
    if (eventSource == "backbtn")
    {
        // unpausing is done in the destructor so nothing more to do here
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "choiceribbon")
    {
        const std::string& selection = m_choice_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
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
            /*
            ModalDialog::dismiss();
            World::getWorld()->unpause();
            race_manager->exitRace();
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            StateManager::get()->pushScreen(KartSelectionScreen::getInstance());
            return GUIEngine::EVENT_BLOCK;
             */
            ModalDialog::dismiss();
            World::getWorld()->unpause();
            race_manager->exitRace();
            Screen* newStack[] = {MainMenuScreen::getInstance(), RaceSetupScreen::getInstance(), NULL};
            StateManager::get()->resetAndSetStack( newStack );
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// ------------------------------------------------------------------------------------------------------

RacePausedDialog::~RacePausedDialog()
{
    World::getWorld()->unpause();
}

// ------------------------------------------------------------------------------------------------------


