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

#include "states_screens/options/options_screen_ui.hpp"

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
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace Online;

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
        skinSelector->setActive(false);
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


    // Setup the minimap options spinner
    GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
    assert( minimap_options != NULL );

    minimap_options->m_properties[PROP_WRAP_AROUND] = "true";
    minimap_options->clearLabels();
    //I18N: In the UI options, minimap position in the race UI 
    minimap_options->addLabel( core::stringw(_("In the bottom-left")));
    //I18N: In the UI options, minimap position in the race UI 
    minimap_options->addLabel( core::stringw(_("On the right side")));
    //I18N: In the UI options, minimap position in the race UI 
    minimap_options->addLabel( core::stringw(_("Hidden")));
    minimap_options->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 && 
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;

    if (multitouch_enabled && UserConfigParams::m_multitouch_draw_gui)
    {
        minimap_options->m_properties[GUIEngine::PROP_MIN_VALUE] = "1";
    }
    minimap_options->m_properties[GUIEngine::PROP_MAX_VALUE] = "2";
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenUI::init()
{
    Screen::init();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_ui", PLAYER_ID_GAME_MASTER );

    GUIEngine::SpinnerWidget* skinSelector = getWidget<GUIEngine::SpinnerWidget>("skinchoice");
    assert( skinSelector != NULL );

    GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
    assert( minimap_options != NULL );

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 && 
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;

    if (multitouch_enabled && UserConfigParams::m_multitouch_draw_gui &&
        UserConfigParams::m_minimap_display == 0)
    {
        UserConfigParams::m_minimap_display = 1;
    }
    minimap_options->setValue(UserConfigParams::m_minimap_display);
    
    GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
    assert( font_size != NULL );
    
    font_size->setValue((int)roundf(UserConfigParams::m_fonts_size));
    m_prev_font_size = UserConfigParams::m_fonts_size;

    // ---- video modes
    CheckBoxWidget* splitscreen_method = getWidget<CheckBoxWidget>("split_screen_horizontally");
    assert(splitscreen_method != NULL);
    splitscreen_method->setState(UserConfigParams::split_screen_horizontally);

    //Forbid changing this setting in game
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    splitscreen_method->setActive(!in_game);

    CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
    assert( fps != NULL );
    fps->setState( UserConfigParams::m_display_fps );

    // --- select the right skin in the spinner
    bool currSkinFound = false;
    const int skinCount = (int) m_skins.size();
    std::string user_skin = StringUtils::toLowerCase(UserConfigParams::m_skin_file.c_str());
    for (int n=0; n<skinCount; n++)
    {
        const std::string skinFileName = StringUtils::toLowerCase(StringUtils::getBasename(m_skins[n]));

        if (user_skin == skinFileName)
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
        irr_driver->unsetMaxTextureSize();
        GUIEngine::reloadSkin();
        irr_driver->setMaxTextureSize();
    }
}   // init

// -----------------------------------------------------------------------------

void OptionsScreenUI::eventCallback(Widget* widget, const std::string& name, const int playerID)
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
        //else if (selection == "tab_ui")
        //    screen = OptionsScreenUI::getInstance();
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
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
        irr_driver->unsetMaxTextureSize();
        GUIEngine::reloadSkin();
        irr_driver->setMaxTextureSize();
    }
    else if (name == "minimap")
    {
        GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
        assert( minimap_options != NULL );
        UserConfigParams::m_minimap_display = minimap_options->getValue();
    }
    else if (name == "font_size")
    {
        GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
        assert( font_size != NULL );
        UserConfigParams::m_fonts_size = font_size->getValue();
    }
    else if (name == "split_screen_horizontally")
    {
        CheckBoxWidget* split_screen_horizontally = getWidget<CheckBoxWidget>("split_screen_horizontally");
        assert(split_screen_horizontally != NULL);
        UserConfigParams::split_screen_horizontally = split_screen_horizontally->getState();

    }
    else if (name == "showfps")
    {
        CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
        assert( fps != NULL );
        UserConfigParams::m_display_fps = fps->getState();
    }
#endif
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenUI::tearDown()
{
    if (m_prev_font_size != UserConfigParams::m_fonts_size)
    {
        irr_driver->sameRestart();
    }
    
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
