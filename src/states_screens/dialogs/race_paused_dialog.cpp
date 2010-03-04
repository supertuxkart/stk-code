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
#include "states_screens/options_screen_av.hpp"
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
    World::getWorld()->pause();
    
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
    back_btn->x = m_area.getWidth() / 2 - icon_size;
    back_btn->y = text_height*2;
    back_btn->w = icon_size*2; // width larger to leave room for text
    back_btn->h = icon_size;
    back_btn->setParent(m_irrlicht_window);
    m_children.push_back(back_btn);
    back_btn->add();
    
    back_btn->setFocusForPlayer( GUI_PLAYER_ID );
    
    // ---- Choice ribbon
    m_choice_ribbon = new RibbonWidget(RIBBON_TOOLBAR);
    m_choice_ribbon->m_properties[PROP_ID] = "choiceribbon";
    
    m_choice_ribbon->x = 0;
    m_choice_ribbon->y = text_height*2 + icon_size + 50;
    m_choice_ribbon->w = m_area.getWidth();
    m_choice_ribbon->h = icon_size + text_height;
    m_choice_ribbon->setParent(m_irrlicht_window);
    
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_SINGLE)
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "newrace";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_race.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = _("Setup New Race");
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_SINGLE)
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "restart";
        ribbon_item->m_properties[PROP_ICON] = "gui/restart.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = _("Restart Race");
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "options";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_options.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = _("Options");
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "help";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_help.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = _("Help");
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "exit";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_quit.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = _("Exit Race");
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    
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
    std::cout << "RacePausedDialog::processEvent(" << eventSource.c_str() << ")\n";
    
    if (eventSource == "backbtn")
    {
        // unpausing is done in the destructor so nothing more to do here
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "choiceribbon")
    {
        const std::string& selection = m_choice_ribbon->getSelectionIDString(GUI_PLAYER_ID);
        
        std::cout << "RacePausedDialog::processEvent(" << eventSource.c_str()
                  << " : " << selection << ")\n";

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
            StateManager::get()->pushScreen(OptionsScreenAV::getInstance());
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


