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
#include "challenges/story_mode_timer.hpp"
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
#include "states_screens/dialogs/message_dialog.hpp"
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
    //I18N: In the UI options, minimap position in the race UI
    minimap_options->addLabel( core::stringw(_("Centered")));
    minimap_options->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 && 
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;

    if (multitouch_enabled && UserConfigParams::m_multitouch_draw_gui)
    {
        minimap_options->m_properties[GUIEngine::PROP_MIN_VALUE] = "1";
    }
    minimap_options->m_properties[GUIEngine::PROP_MAX_VALUE] = "3";

    GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
    assert( font_size != NULL );

    font_size->clearLabels();
    font_size->addLabel(L"Extremely small");
    //I18N: In the UI options, Very small font size
    font_size->addLabel(_("Very small"));
    //I18N: In the UI options, Small font size
    font_size->addLabel(_("Small"));
    //I18N: In the UI options, Medium font size
    font_size->addLabel(_("Medium"));
    //I18N: In the UI options, Large font size
    font_size->addLabel(_("Large"));
    //I18N: In the UI options, Very large font size
    font_size->addLabel(_("Very large"));
    font_size->addLabel(L"Extremely large");

    if (UserConfigParams::m_artist_debug_mode)
    {
        // Only show extreme size in artist debug mode
        font_size->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
        font_size->m_properties[GUIEngine::PROP_MAX_VALUE] = "6";
    }
    else
    {
        font_size->m_properties[GUIEngine::PROP_MIN_VALUE] = "1";
        font_size->m_properties[GUIEngine::PROP_MAX_VALUE] = "5";
    }
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

    m_skins.clear();
    skinSelector->clearLabels();

    std::set<std::string> skin_files;
    file_manager->listFiles(skin_files /* out */, file_manager->getAsset(FileManager::SKIN,""),
                            true /* make full path */ );
    std::set<std::string> addon_skin_files;
    file_manager->listFiles(addon_skin_files /* out */, file_manager->getAddonsFile("skins/"),
                            true /* make full path */ );

    auto lb = [](const std::set<std::string>& files, bool addon,
                 std::map<core::stringw, std::string>& result)->void
        {
            for (auto& f : files)
            {
                std::string stkskin = f + "/stkskin.xml";
                if (file_manager->fileExists(stkskin))
                {
                    XMLNode* root = file_manager->createXMLTree(stkskin);
                    if (!root)
                        continue;
                    core::stringw skin_name;
                    if (root->get("name", &skin_name))
                    {
                        std::string skin_id = StringUtils::getBasename(f);
                        if (addon)
                            skin_id = std::string("addon_") + skin_id;
                        result[skin_name] = skin_id;
                    }
                    delete root;
                }
            }
        };
    lb(skin_files, false, m_skins);
    lb(addon_skin_files, true, m_skins);

    if (m_skins.size() == 0)
    {
        Log::warn("OptionsScreenUI", "Could not find a single skin, make sure that "
                                     "the data files are correctly installed");
        skinSelector->setActive(false);
        return;
    }

    const int skin_count = (int)m_skins.size();
    for (auto& p : m_skins)
        skinSelector->addLabel(p.first);
    skinSelector->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    skinSelector->m_properties[GUIEngine::PROP_MAX_VALUE] = StringUtils::toString(skin_count-1);

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
    
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    
    GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
    assert( font_size != NULL );

    int size_int = (int)roundf(UserConfigParams::m_font_size);
    if (size_int < 0 || size_int > 6)
        size_int = 3;

    if (!UserConfigParams::m_artist_debug_mode &&
        (size_int < 1 || size_int > 5))
        size_int = 3;

    font_size->setValue(size_int);
    UserConfigParams::m_font_size = font_size->getValue();
    font_size->setActive(!in_game);

    // ---- video modes
    CheckBoxWidget* splitscreen_method = getWidget<CheckBoxWidget>("split_screen_horizontally");
    assert(splitscreen_method != NULL);
    splitscreen_method->setState(UserConfigParams::split_screen_horizontally);

    CheckBoxWidget* karts_powerup_gui = getWidget<CheckBoxWidget>("karts_powerup_gui");
    assert(karts_powerup_gui != NULL);
    karts_powerup_gui->setState(UserConfigParams::m_karts_powerup_gui);

    //Forbid changing this setting in game
    splitscreen_method->setActive(!in_game);

    CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
    assert( fps != NULL );
    fps->setState( UserConfigParams::m_display_fps );

    CheckBoxWidget* story_timer = getWidget<CheckBoxWidget>("story-mode-timer");
    assert( story_timer != NULL );
    story_timer->setState( UserConfigParams::m_display_story_mode_timer );
    CheckBoxWidget* speedrun_timer = getWidget<CheckBoxWidget>("speedrun-timer");
    assert( speedrun_timer != NULL );
    if (story_mode_timer->getStoryModeTime() < 0)
    {
        story_timer->setActive(false);
        speedrun_timer->setActive(false);
    }
    else
    {
        story_timer->setActive(true);

        speedrun_timer->setActive(UserConfigParams::m_display_story_mode_timer);
        getWidget<LabelWidget>("speedrun-timer-text")
            ->setActive(UserConfigParams::m_display_story_mode_timer);
    }
    if (UserConfigParams::m_speedrun_mode)
    {
        if (!story_mode_timer->playerCanRun())
        {
            UserConfigParams::m_speedrun_mode = false;
            new MessageDialog(_("Speedrun mode disabled. It can only be enabled if the game"
                                " has not been closed since the launch of the story mode.\n\n"
                                "Closing the game before the story mode's"
                                " completion invalidates the timer.\n\n"
                                "To use the speedrun mode, please use a new profile."),
                                MessageDialog::MESSAGE_DIALOG_OK,
                                NULL, false, false, 0.6f, 0.7f);
        }
    }
    speedrun_timer->setState( UserConfigParams::m_speedrun_mode );

    // --- select the right skin in the spinner
    bool currSkinFound = false;
    const std::string& user_skin = UserConfigParams::m_skin_file;
    skinSelector->setActive(!in_game);

    for (int n = 0; n <= skinSelector->getMax(); n++)
    {
        auto ret = m_skins.find(skinSelector->getStringValueFromID(n));
        if (ret == m_skins.end())
            continue;
        const std::string skinFileName = ret->second;

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
        bool right = skinSelector->isRightButtonSelected();
        UserConfigParams::m_skin_file = m_skins[selectedSkin];
        irr_driver->unsetMaxTextureSize();
        bool prev_icon_theme = GUIEngine::getSkin()->hasIconTheme();
        bool prev_font = GUIEngine::getSkin()->hasFont();
        GUIEngine::reloadSkin();
        if (GUIEngine::getSkin()->hasIconTheme() != prev_icon_theme ||
            prev_font != GUIEngine::getSkin()->hasFont())
        {
            if (prev_font != GUIEngine::getSkin()->hasFont())
            {
                GUIEngine::clear();
                GUIEngine::cleanUp();
            }

            GUIEngine::clearScreenCache();

            if (prev_font != GUIEngine::getSkin()->hasFont())
            {
                delete font_manager;
                font_manager = new FontManager();
                font_manager->loadFonts();
                GUIEngine::init(irr_driver->getDevice(), irr_driver->getVideoDriver(),
                    StateManager::get(), false/*loading*/);
            }

            Screen* screen_list[] =
                {
                    MainMenuScreen::getInstance(),
                    OptionsScreenUI::getInstance(),
                    nullptr
                };
            GUIEngine::switchToScreen(MainMenuScreen::getInstance());
            StateManager::get()->resetAndSetStack(screen_list);
            // Need to use new widget pointer
            skinSelector =
                OptionsScreenUI::getInstance()->getWidget<GUIEngine::SpinnerWidget>("skinchoice");
            skinSelector->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
            skinSelector->setSelectedButton(right);
        }
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
        bool right = font_size->isRightButtonSelected();
        UserConfigParams::m_font_size = font_size->getValue();
        GUIEngine::clear();
        GUIEngine::cleanUp();
        GUIEngine::clearScreenCache();
        delete font_manager;
        font_manager = new FontManager();
        font_manager->loadFonts();
        GUIEngine::init(irr_driver->getDevice(), irr_driver->getVideoDriver(),
            StateManager::get(), false/*loading*/);
        Screen* screen_list[] =
            {
                MainMenuScreen::getInstance(),
                OptionsScreenUI::getInstance(),
                nullptr
            };
        GUIEngine::switchToScreen(MainMenuScreen::getInstance());
        StateManager::get()->resetAndSetStack(screen_list);
        // Need to use new widget pointer
        font_size =
            OptionsScreenUI::getInstance()->getWidget<GUIEngine::SpinnerWidget>("font_size");
        font_size->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        font_size->setSelectedButton(right);
    }
    else if (name == "split_screen_horizontally")
    {
        CheckBoxWidget* split_screen_horizontally = getWidget<CheckBoxWidget>("split_screen_horizontally");
        assert(split_screen_horizontally != NULL);
        UserConfigParams::split_screen_horizontally = split_screen_horizontally->getState();
    }
    else if (name == "karts_powerup_gui")
    {
        CheckBoxWidget* karts_powerup_gui = getWidget<CheckBoxWidget>("karts_powerup_gui");
        assert(karts_powerup_gui != NULL);
        UserConfigParams::m_karts_powerup_gui = karts_powerup_gui->getState();
    }
    else if (name == "showfps")
    {
        CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
        assert( fps != NULL );
        UserConfigParams::m_display_fps = fps->getState();
    }
    else if (name == "story-mode-timer")
    {
        CheckBoxWidget* story_timer = getWidget<CheckBoxWidget>("story-mode-timer");
        assert( story_timer != NULL );
        UserConfigParams::m_display_story_mode_timer = story_timer->getState();

        CheckBoxWidget* speedrun_timer = getWidget<CheckBoxWidget>("speedrun-timer");
        assert( speedrun_timer != NULL );
        speedrun_timer->setActive( UserConfigParams::m_display_story_mode_timer );
        getWidget<LabelWidget>("speedrun-timer-text")
            ->setActive(UserConfigParams::m_display_story_mode_timer);

        // Disable speedrun mode if the story mode timer is disabled
        if (!UserConfigParams::m_display_story_mode_timer)
        {
            UserConfigParams::m_speedrun_mode = false;
            speedrun_timer->setState(false);
        }

    }
    else if (name == "speedrun-timer")
    {
        CheckBoxWidget* speedrun_timer = getWidget<CheckBoxWidget>("speedrun-timer");
        assert( speedrun_timer != NULL );
        if (speedrun_timer->getState())
        {
            if (!story_mode_timer->playerCanRun())
            {
                speedrun_timer->setState(false);
                new MessageDialog(_("Speedrun mode can only be enabled if the game has not"
                                    " been closed since the launch of the story mode.\n\n"
                                    "Closing the game before the story mode's"
                                    " completion invalidates the timer.\n\n"
                                    "To use the speedrun mode, please use a new profile."),
                                    MessageDialog::MESSAGE_DIALOG_OK,
                                    NULL, false, false, 0.6f, 0.7f);
            }
        }
        UserConfigParams::m_speedrun_mode = speedrun_timer->getState();
    }
#endif
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
