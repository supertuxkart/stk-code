//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/options/options_screen_language.hpp"

#include "addons/news_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "font/bold_face.hpp"
#include "font/font_manager.hpp"
#include "font/regular_face.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "online/request_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "tips/tips_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <IGUIScrollBar.h>

using namespace GUIEngine;
using namespace Online;

// -----------------------------------------------------------------------------

OptionsScreenLanguage::OptionsScreenLanguage() : Screen("options_language.stkgui")
{
    m_inited = false;
}   // OptionsScreenLanguage

// -----------------------------------------------------------------------------

void OptionsScreenLanguage::loadedFromFile()
{
    m_inited = false;
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenLanguage::init()
{
    Screen::init();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_language", PLAYER_ID_GAME_MASTER );

    // --- language
    ListWidget* list_widget = getWidget<ListWidget>("language");

    // I18N: in the language choice, to select the same language as the OS
    list_widget->addItem("system", _("System Language"));
#ifndef SERVER_ONLY
    const std::vector<std::string>* lang_list = translations->getLanguageList();
    const int amount = (int)lang_list->size();

    // The names need to be sorted alphabetically. Store the 2-letter
    // language names in a mapping, to be able to get them from the
    // user visible full name.
    std::vector<core::stringw> nice_lang_list;
    std::map<core::stringw, std::string> nice_name_2_id;
    for (int n=0; n<amount; n++)
    {
        std::string code_name = (*lang_list)[n];
        std::string s_name = translations->getLocalizedName(code_name) +
         " (" + tinygettext::Language::from_name(code_name).get_language();
        std::string country = tinygettext::Language::from_name(code_name).get_country();
        if (!country.empty())
            s_name += "_" + country;
        s_name += ")";
        core::stringw nice_name = StringUtils::utf8ToWide(s_name);
        nice_lang_list.push_back(nice_name);
        nice_name_2_id[nice_name] = code_name;
    }
    std::sort(nice_lang_list.begin(), nice_lang_list.end());
    for(unsigned int i=0; i<nice_lang_list.size(); i++)
    {
        list_widget->addItem(nice_name_2_id[nice_lang_list[i]],
                              nice_lang_list[i]);
    }
#endif
    list_widget->setSelectionID( list_widget->getItemID(UserConfigParams::m_language) );

    // Forbid changing language while in-game, since this crashes (changing the language involves
    // tearing down and rebuilding the menu stack. not good when in-game)
    list_widget->setActive(StateManager::get()->getGameState() != GUIEngine::INGAME_MENU);
    

}   // init

// -----------------------------------------------------------------------------

void OptionsScreenLanguage::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
#ifndef SERVER_ONLY
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        else if (selection == "tab_controls")
            screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        //else if (selection == "tab_language")
        //    screen = OptionsScreenLanguage::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "language")
    {
        ListWidget* list_widget = getWidget<ListWidget>("language");
        irr::gui::CGUISTKListBox* box =
            list_widget->getIrrlichtElement<irr::gui::CGUISTKListBox>();
        int old_pos = box->getScrollBar()->getPos();
        std::string selection = list_widget->getSelectionInternalName();

        delete translations;

        if (selection == "system")
        {
#ifdef WIN32
            _putenv("LANGUAGE=");
#else
            unsetenv("LANGUAGE");
#endif
        }
        else
        {
#ifdef WIN32
            std::string s=std::string("LANGUAGE=")+selection.c_str();
            _putenv(s.c_str());
#else
            setenv("LANGUAGE", selection.c_str(), 1);
#endif
        }

        translations = new Translations();

        // Reload fonts for new translation
        GUIEngine::getStateManager()->hardResetAndGoToScreen<MainMenuScreen>();

        font_manager->getFont<BoldFace>()->reset();
        font_manager->getFont<RegularFace>()->reset();
        font_manager->clearCachedLayouts();

        UserConfigParams::m_language = selection.c_str();
        user_config->saveConfig();

        OptionsScreenLanguage::getInstance()->push();
        // Menu is deleted so we need a new screen instance
        OptionsScreenLanguage* os = OptionsScreenLanguage::getInstance();
        list_widget = os->getWidget<ListWidget>("language");
        box = list_widget->getIrrlichtElement<irr::gui::CGUISTKListBox>();
        box->getScrollBar()->setPos(old_pos);
        // Update tips for new translation
        TipsManager::destroy();
        TipsManager::create();
    }
#endif
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenLanguage::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenLanguage::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------
