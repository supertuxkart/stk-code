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
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <string>
using namespace GUIEngine;

RacePausedDialog::RacePausedDialog(const float percentWidth, const float percentHeight) : ModalDialog(percentWidth, percentHeight)
{
    RaceManager::getWorld()->pause();
    
    IGUIFont* font = GUIEngine::getFont();
    const int text_height = font->getDimension(L"X").Height;
    
    const int icon_size = (m_area.getHeight() - text_height - 150) / 2;
    
    // ---- Caption
    core::rect< s32 > area(0, 0, m_area.getWidth(), text_height);
    IGUIStaticText* caption = GUIEngine::getGUIEnv()->addStaticText( _("Paused"),
                                                                    area, false, false, // border, word warp
                                                                    m_irrlicht_window);
    caption->setTabStop(false);
    caption->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
    
    // ---- Back button
    IconButtonWidget* back_btn = new IconButtonWidget();
    back_btn->m_properties[PROP_ID] = "backbtn";
    back_btn->m_properties[PROP_ICON] = "gui/back.png";
    //I18N: In the 'paused' screen
    back_btn->m_text = L"Back to Race";
    back_btn->x = m_area.getWidth() / 2 - icon_size;
    back_btn->y = text_height*2;
    back_btn->w = icon_size*2; // width larger to leave room for text
    back_btn->h = icon_size;
    back_btn->setParent(m_irrlicht_window);
    m_children.push_back(back_btn);
    back_btn->add();
    GUIEngine::getGUIEnv()->setFocus( back_btn->getIrrlichtElement() );
    
    // ---- Choice ribbon
    m_choice_ribbon = new RibbonWidget(RIBBON_TOOLBAR);
    m_choice_ribbon->m_properties[PROP_ID] = "choiceribbon";
    
    m_choice_ribbon->x = 0;
    m_choice_ribbon->y = text_height*2 + icon_size + 50;
    m_choice_ribbon->w = m_area.getWidth();
    m_choice_ribbon->h = icon_size + text_height;
    m_choice_ribbon->setParent(m_irrlicht_window);
    
    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_QUICK_RACE)
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "newrace";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_race.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = L"Setup New Race";
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "restart";
        ribbon_item->m_properties[PROP_ICON] = "gui/restart.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = L"Restart Race";
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "options";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_options.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = L"Options";
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "help";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_help.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = L"Help";
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    {
        IconButtonWidget* ribbon_item = new IconButtonWidget();
        ribbon_item->m_properties[PROP_ID] = "exit";
        ribbon_item->m_properties[PROP_ICON] = "gui/main_quit.png";
        ribbon_item->m_properties[PROP_WIDTH] = "128";
        ribbon_item->m_properties[PROP_HEIGHT] = "128";
        //I18N: In the 'paused' screen
        ribbon_item->m_text = L"Exit Race";
        m_choice_ribbon->m_children.push_back(ribbon_item);
    }
    
    m_children.push_back(m_choice_ribbon);
    m_choice_ribbon->add();   
}

/*
 46    widget_manager->addTitleWgt( WTOK_PAUSE, 50, 7, _("Paused") );
 47 
 48     widget_manager->addTextButtonWgt( WTOK_RETURN_RACE, 50, 7, _("Return To Race"));
 49     widget_manager->addTextButtonWgt( WTOK_OPTIONS, 50, 7, _("Options") );
 50     widget_manager->addTextButtonWgt( WTOK_HELP, 50, 7, _("Help") );
 51     widget_manager->addTextButtonWgt( WTOK_RESTART_RACE, 50, 7, _("Restart Race") );
 52 
 53     if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_QUICK_RACE)
 54     {
 55         widget_manager->addTextButtonWgt( WTOK_SETUP_NEW_RACE, 50, 7,
 56             _("Setup New Race") );
 57     }
 58 
 59     widget_manager->addTextButtonWgt( WTOK_QUIT, 50, 7, _("Exit Race") );
 60 
 */

/*
 77     switch (clicked_token)
 78     {
 79     case WTOK_RETURN_RACE:
 80         RaceManager::getWorld()->unpause();
 81         menu_manager->popMenu();
 82         if(user_config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
 83         break;
 84 
 85     case WTOK_SETUP_NEW_RACE:
 86         RaceManager::getWorld()->unpause();
 87         race_manager->exit_race();
 88         menu_manager->pushMenu(MENUID_CHARSEL_P1);
 89         break;
 90 
 91     case WTOK_RESTART_RACE:
 92         menu_manager->popMenu();
 93         if(user_config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
 94         RaceManager::getWorld()->restartRace();
 95         break;
 96 
 97     case WTOK_OPTIONS:
 98         menu_manager->pushMenu(MENUID_OPTIONS);
 99         break;
 100 
 101     case WTOK_HELP:
 102         menu_manager->pushMenu(MENUID_HELP1);
 103         break;
 104 
 105     case WTOK_QUIT:
 106         RaceManager::getWorld()->unpause();
 107         race_manager->exit_race();
 108         break;
 109 
 110     default:
 111         break;
 112     }
 113 }
 */

/*
 void RaceMenu::handle(GameAction ga, int value)
 117 {
 118     switch ( ga )
 119     {
 120     case GA_LEAVE:
 121         if (value)
 122             break;
 123 
 124         RaceManager::getWorld()->unpause();
 125         menu_manager->popMenu();
 126         break;
 127 
 128     default:
 129         BaseGUI::handle(ga, value);
 130         break;
 131     }
 */

void RacePausedDialog::onEnterPressedInternal()
{
}

bool RacePausedDialog::processEvent(std::string& eventSource)
{
    std::cout << "RacePausedDialog::processEvent(" << eventSource.c_str() << ")\n";
    
    if (eventSource == "backbtn")
    {
        // unpausing is done in the destructor so nothing more to do here
        ModalDialog::dismiss();
        return true;
    }
    else if (eventSource == "choiceribbon")
    {
        // FIXME : don't hardcode player 0
        const int playerId = 0;
        const std::string& selection = m_choice_ribbon->getSelectionIDString(playerId);
        if (selection == "exit")
        {
            ModalDialog::dismiss();
            race_manager->exitRace();
            StateManager::get()->resetAndGoToMenu("main.stkgui");
            input_manager->setMode(InputManager::MENU);
            return true;
        }
        else if (selection == "help")
        {
            dismiss();
            StateManager::get()->pushMenu("help1.stkgui");
            return true;
        }
        else if (selection == "options")
        {
            dismiss();
            StateManager::get()->pushMenu("options_av.stkgui");
            return true;
        }
        else if (selection == "restart")
        {
            ModalDialog::dismiss();
            network_manager->setState(NetworkManager::NS_MAIN_MENU);
            RaceManager::getWorld()->unpause();
            race_manager->rerunRace();
            return true;
        }
        else if (selection == "newrace")
        {
            // TODO
        }
    }
    return false;
}

RacePausedDialog::~RacePausedDialog()
{
    RaceManager::getWorld()->unpause();
}

