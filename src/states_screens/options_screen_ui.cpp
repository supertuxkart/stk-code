//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "states_screens/options_screen_ui.hpp"

#include "addons/news_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/hardware_stats.hpp"
#include "config/user_config.hpp"
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
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OptionsScreenUI );

// -----------------------------------------------------------------------------

OptionsScreenUI::OptionsScreenUI() : Screen("options_ui.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// -----------------------------------------------------------------------------

void OptionsScreenUI::loadedFromFile()
{
    m_inited = false;

    GUIEngine::SpinnerWidget* skinSelector = getWidget<GUIEngine::SpinnerWidget>("skinchoice");
    assert( skinSelector != NULL );

    skinSelector->m_properties[PROP_WRAP_AROUND] = "true";

    m_skins.clear();
    skinSelector->clearLabels();

    std::set<std::string> skinFiles;
    file_manager->listFiles(skinFiles /* out */, file_manager->getAsset(FileManager::SKIN,""),
                            true /* make full path */ );

    for (std::set<std::string>::iterator it = skinFiles.begin(); it != skinFiles.end(); it++)
    {
        if(StringUtils::getExtension(*it)=="stkskin")
        {
            m_skins.push_back( *it );
        }
    }

    if (m_skins.size() == 0)
    {
        Log::warn("OptionsScreenUI", "Could not find a single skin, make sure that "
                                     "the data files are correctly installed");
        skinSelector->setDeactivated();
        return;
    }

    const int skin_count = (int)m_skins.size();
    for (int n=0; n<skin_count; n++)
    {
        const std::string skinFileName = StringUtils::getBasename(m_skins[n]);
        const std::string skinName = StringUtils::removeExtension( skinFileName );
        skinSelector->addLabel( core::stringw(skinName.c_str()) );
    }
    skinSelector->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    skinSelector->m_properties[GUIEngine::PROP_MAX_VALUE] = StringUtils::toString(skin_count-1);


}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenUI::init()
{
    Screen::init();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "tab_ui", PLAYER_ID_GAME_MASTER );

    ribbon->getRibbonChildren()[0].setTooltip( _("Graphics") );
    ribbon->getRibbonChildren()[1].setTooltip( _("Audio") );
    ribbon->getRibbonChildren()[3].setTooltip( _("Players") );
    ribbon->getRibbonChildren()[4].setTooltip( _("Controls") );

    GUIEngine::SpinnerWidget* skinSelector = getWidget<GUIEngine::SpinnerWidget>("skinchoice");
    assert( skinSelector != NULL );

    // ---- video modes

    CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
    assert( fps != NULL );
    fps->setState( UserConfigParams::m_display_fps );
    CheckBoxWidget* news = getWidget<CheckBoxWidget>("enable-internet");
    assert( news != NULL );
    news->setState( UserConfigParams::m_internet_status
                                     ==RequestManager::IPERM_ALLOWED );
    CheckBoxWidget* stats = getWidget<CheckBoxWidget>("enable-hw-report");
    assert( stats != NULL );
    LabelWidget *stats_label = getWidget<LabelWidget>("label-hw-report");
    assert( stats_label );
            stats->setState(UserConfigParams::m_hw_report_enable);

    if(news->getState())
    {
        stats_label->setVisible(true);
        stats->setVisible(true);
        stats->setState(UserConfigParams::m_hw_report_enable);
    }
    else
    {
        stats_label->setVisible(false);
        stats->setVisible(false);
    }

    CheckBoxWidget* show_login = getWidget<CheckBoxWidget>("show-login");
    assert( show_login!= NULL );
    show_login->setState( UserConfigParams::m_always_show_login_screen);

    // --- select the right skin in the spinner
    bool currSkinFound = false;
    const int skinCount = (int) m_skins.size();
    for (int n=0; n<skinCount; n++)
    {
        const std::string skinFileName = StringUtils::getBasename(m_skins[n]);

        if (UserConfigParams::m_skin_file.c_str() == skinFileName)
        {
            skinSelector->setValue(n);
            currSkinFound = true;
            break;
        }
    }
    if (!currSkinFound)
    {
        Log::warn("OptionsScreenUI",
                  "Couldn't find current skin in the list of skins!");
        skinSelector->setValue(0);
        GUIEngine::reloadSkin();
    }

    // --- language
    ListWidget* list_widget = getWidget<ListWidget>("language");

    // I18N: in the language choice, to select the same language as the OS
    list_widget->addItem("system", _("System Language"));

    const std::vector<std::string>* lang_list = translations->getLanguageList();
    const int amount = (int)lang_list->size();

    // The names need to be sorted alphabetically. Store the 2-letter
    // language names in a mapping, to be able to get them from the
    // user visible full name.
    std::vector<std::string> nice_lang_list;
    std::map<std::string, std::string> nice_name_2_id;
    for (int n=0; n<amount; n++)
    {
        std::string code_name = (*lang_list)[n];
        std::string nice_name = tinygettext::Language::from_name(code_name).get_name();
        nice_lang_list.push_back(nice_name);
        nice_name_2_id[nice_name] = code_name;
    }
    std::sort(nice_lang_list.begin(), nice_lang_list.end());
    for(unsigned int i=0; i<nice_lang_list.size(); i++)
    {
        list_widget->addItem(nice_name_2_id[nice_lang_list[i]],
                              nice_lang_list[i].c_str());
    }

    list_widget->setSelectionID( list_widget->getItemID(UserConfigParams::m_language) );

    // Forbid changing language while in-game, since this crashes (changing the language involves
    // tearing down and rebuilding the menu stack. not good when in-game)
    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        list_widget->setDeactivated();
    }
    else
    {
        list_widget->setActivated();
    }

}   // init

// -----------------------------------------------------------------------------

void OptionsScreenUI::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
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
        //else if (selection == "tab_ui")
        //    screen = OptionsScreenUI::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "skinchoice")
    {
        GUIEngine::SpinnerWidget* skinSelector = getWidget<GUIEngine::SpinnerWidget>("skinchoice");
        assert( skinSelector != NULL );

        const core::stringw selectedSkin = skinSelector->getStringValue();
        UserConfigParams::m_skin_file = core::stringc(selectedSkin.c_str()).c_str() + std::string(".stkskin");
        GUIEngine::reloadSkin();
    }
    else if (name == "showfps")
    {
        CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
        assert( fps != NULL );
        UserConfigParams::m_display_fps = fps->getState();
    }
    else if (name=="enable-internet")
    {
        CheckBoxWidget* internet = getWidget<CheckBoxWidget>("enable-internet");
        assert( internet != NULL );
        UserConfigParams::m_internet_status =
            internet->getState() ? RequestManager::IPERM_ALLOWED
                                 : RequestManager::IPERM_NOT_ALLOWED;
        // If internet gets enabled, re-initialise the addon manager (which
        // happens in a separate thread) so that news.xml etc can be
        // downloaded if necessary.
        CheckBoxWidget *stats = getWidget<CheckBoxWidget>("enable-hw-report");
        LabelWidget *stats_label = getWidget<LabelWidget>("label-hw-report");
        if(internet->getState())
        {
            NewsManager::get()->init(false);
            stats->setVisible(true);
            stats_label->setVisible(true);
            stats->setState(UserConfigParams::m_hw_report_enable);
        }
        else
        {
            stats->setVisible(false);
            stats_label->setVisible(false);
        }

    }
    else if (name=="enable-hw-report")
    {
        CheckBoxWidget* stats = getWidget<CheckBoxWidget>("enable-hw-report");
        UserConfigParams::m_hw_report_enable = stats->getState();
        if(stats->getState())
            HardwareStats::reportHardwareStats();
    }
    else if (name=="show-login")
    {
        CheckBoxWidget* show_login = getWidget<CheckBoxWidget>("show-login");
        assert( show_login != NULL );
        UserConfigParams::m_always_show_login_screen = show_login->getState();
    }
    else if (name == "language")
    {
        ListWidget* list_widget = getWidget<ListWidget>("language");
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
        GUIEngine::getStateManager()->hardResetAndGoToScreen<MainMenuScreen>();

        GUIEngine::getFont()->updateRTL();
        GUIEngine::getTitleFont()->updateRTL();
        GUIEngine::getSmallFont()->updateRTL();

        UserConfigParams::m_language = selection.c_str();
        user_config->saveConfig();

        OptionsScreenUI::getInstance()->push();
    }

}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenUI::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenUI::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------

