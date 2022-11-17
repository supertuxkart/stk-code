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
#include "graphics/camera.hpp"
#include "graphics/camera_normal.hpp"
#include "challenges/story_mode_timer.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "config/stk_config.hpp"
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
#include "items/powerup_manager.hpp"
#include "modes/world.hpp"
#include "online/request_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/dialogs/custom_camera_settings.hpp"
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

#include <IrrlichtDevice.h>

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

    // Setup splitscreen spinner
    GUIEngine::SpinnerWidget* splitscreen_method = getWidget<GUIEngine::SpinnerWidget>("splitscreen_method");
    splitscreen_method->m_properties[PROP_WRAP_AROUND] = "true";
    splitscreen_method->clearLabels();
    //I18N: In the UI options, splitscreen_method in the race UI
    splitscreen_method->addLabel( core::stringw(_("Vertical")));
    //I18N: In the UI options, splitscreen_method position in the race UI
    splitscreen_method->addLabel( core::stringw(_("Horizontal")));
    splitscreen_method->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    splitscreen_method->m_properties[GUIEngine::PROP_MAX_VALUE] = "1";

    // Setup fontsize spinner
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

    // Setup camera spinner
    GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
    assert( camera_preset != NULL );

    camera_preset->m_properties[PROP_WRAP_AROUND] = "true";
    camera_preset->clearLabels();
    //I18N: In the UI options, Camera setting: Custom
    camera_preset->addLabel( core::stringw(_("Custom")));
    //I18N: In the UI options, Camera setting: Standard
    camera_preset->addLabel( core::stringw(_("Standard")));
    //I18N: In the UI options, Camera setting: Drone chase
    camera_preset->addLabel( core::stringw(_("Drone chase")));
    camera_preset->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    camera_preset->m_properties[GUIEngine::PROP_MAX_VALUE] = "2";
    updateCameraPresetSpinner();

    font_size->setValueUpdatedCallback([this](SpinnerWidget* spinner)
    {
        // Add a special value updated callback so font size is updated when
        // it's pressed instead of release to prevent multiple event
        bool right = spinner->isButtonSelected(true/*right*/);
        UserConfigParams::m_font_size = spinner->getValue();
        m_reload_option = std::unique_ptr<ReloadOption>(new ReloadOption);
        m_reload_option->m_reload_font = true;
        m_reload_option->m_reload_skin = false;
        m_reload_option->m_focus_name = "font_size";
        m_reload_option->m_focus_right = right;
    });

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
    GUIEngine::SpinnerWidget* splitscreen_method = getWidget<GUIEngine::SpinnerWidget>("splitscreen_method");
    assert( splitscreen_method != NULL );
    if (UserConfigParams::split_screen_horizontally) splitscreen_method->setValue(1);
    else splitscreen_method->setValue(0);
    splitscreen_method->setActive(!in_game);

    CheckBoxWidget* karts_powerup_gui = getWidget<CheckBoxWidget>("karts_powerup_gui");
    assert(karts_powerup_gui != NULL);
    karts_powerup_gui->setState(UserConfigParams::m_karts_powerup_gui);

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

    // --- select the right camera in the spinner
    GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
    assert( camera_preset != NULL );

    camera_preset->setValue(UserConfigParams::m_camera_present); // use the saved camera
    updateCameraPresetSpinner();
}   // init

void OptionsScreenUI::updateCamera()
{
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    if (in_game)
    {
        (Camera::getActiveCamera()->getCameraSceneNode())->setFOV(DEGREE_TO_RAD * UserConfigParams::m_camera_fov);
        CameraNormal *camera = dynamic_cast<CameraNormal*>(Camera::getActiveCamera());
        if (camera)
        {
            camera->setDistanceToKart(UserConfigParams::m_camera_distance);
        }
    }
}

void OptionsScreenUI::updateCameraPresetSpinner()
{
    updateCamera();
} // updateCameraPresetSpinner

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
        bool right = skinSelector->isButtonSelected(true/*right*/);
        UserConfigParams::m_skin_file = m_skins[selectedSkin];
        bool prev_font = GUIEngine::getSkin()->hasFont();
        irr_driver->unsetMaxTextureSize();
        GUIEngine::reloadSkin();
        // Reload GUIEngine will clear widgets so we don't do that in eventCallback
        m_reload_option = std::unique_ptr<ReloadOption>(new ReloadOption);
        m_reload_option->m_reload_font = prev_font != GUIEngine::getSkin()->hasFont();
        m_reload_option->m_reload_skin = true;
        m_reload_option->m_focus_name = "skinchoice";
        m_reload_option->m_focus_right = right;
    }
    else if (name == "minimap")
    {
        GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
        assert( minimap_options != NULL );
        UserConfigParams::m_minimap_display = minimap_options->getValue();
        if (World::getWorld())
            World::getWorld()->getRaceGUI()->recreateGUI();
    }
    else if (name == "font_size")
    {
        GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
        assert( font_size != NULL );
        bool right = font_size->isButtonSelected(true/*right*/);
        UserConfigParams::m_font_size = font_size->getValue();
        // Reload GUIEngine will clear widgets so we don't do that in eventCallback
        m_reload_option = std::unique_ptr<ReloadOption>(new ReloadOption);
        m_reload_option->m_reload_font = true;
        m_reload_option->m_reload_skin = false;
        m_reload_option->m_focus_name = "font_size";
        m_reload_option->m_focus_right = right;
    }
    else if (name == "splitscreen_method")
    {
        GUIEngine::SpinnerWidget* splitscreen_method = getWidget<GUIEngine::SpinnerWidget>("splitscreen_method");
        assert( splitscreen_method != NULL );
        UserConfigParams::split_screen_horizontally = (splitscreen_method->getValue() == 1);
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
    else if (name == "camera_preset")
    {
        GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
        assert( camera_preset != NULL );
        unsigned int i = camera_preset->getValue();
        UserConfigParams::m_camera_present = i;
        if (i == 1) //Standard
        {
            UserConfigParams::m_camera_fov = UserConfigParams::m_standard_camera_fov;
            UserConfigParams::m_camera_distance = UserConfigParams::m_standard_camera_distance;
            UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_standard_camera_forward_up_angle;
            UserConfigParams::m_camera_forward_smoothing = UserConfigParams::m_standard_camera_forward_smoothing;
            UserConfigParams::m_camera_backward_distance = UserConfigParams::m_standard_camera_backward_distance;
            UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_standard_camera_backward_up_angle;
            UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_standard_reverse_look_use_soccer_cam;
        }
        else if (i == 2) //Drone chase
        {
            UserConfigParams::m_camera_fov = UserConfigParams::m_drone_camera_fov;
            UserConfigParams::m_camera_distance = UserConfigParams::m_drone_camera_distance;
            UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_drone_camera_forward_up_angle;
            UserConfigParams::m_camera_forward_smoothing = UserConfigParams::m_drone_camera_forward_smoothing;
            UserConfigParams::m_camera_backward_distance = UserConfigParams::m_drone_camera_backward_distance;
            UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_drone_camera_backward_up_angle;
            UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_drone_reverse_look_use_soccer_cam;
        }
        else //Custom
        {
            UserConfigParams::m_camera_fov = UserConfigParams::m_saved_camera_fov;
            UserConfigParams::m_camera_distance = UserConfigParams::m_saved_camera_distance;
            UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_saved_camera_forward_up_angle;
            UserConfigParams::m_camera_forward_smoothing = UserConfigParams::m_saved_camera_forward_smoothing;
            UserConfigParams::m_camera_backward_distance = UserConfigParams::m_saved_camera_backward_distance;
            UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_saved_camera_backward_up_angle;
            UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_saved_reverse_look_use_soccer_cam;
        }
        updateCamera();
    }
    else if(name == "custom_camera")
    {
        new CustomCameraSettingsDialog(0.8f, 0.95f);
    }
#endif
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenUI::onUpdate(float delta)
{
    if (m_reload_option)
        reloadGUIEngine();
}   // onUpdate

// -----------------------------------------------------------------------------

void OptionsScreenUI::reloadGUIEngine()
{
    bool reload_font = m_reload_option->m_reload_font;
    bool reload_skin = m_reload_option->m_reload_skin;
    std::string focus_name = m_reload_option->m_focus_name;
    bool focus_right = m_reload_option->m_focus_right;

    if (reload_skin || reload_font)
    {
        if (reload_font)
        {
            GUIEngine::clear();
            GUIEngine::cleanUp();
        }

        GUIEngine::clearScreenCache();

        if (reload_font)
        {
            delete font_manager;
            font_manager = new FontManager(); // Fonts are loaded in GUIEngine::init
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
        GUIEngine::SpinnerWidget* spinner = OptionsScreenUI::getInstance()
            ->getWidget<GUIEngine::SpinnerWidget>(focus_name.c_str());
        spinner->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        spinner->setSelectedButton(focus_right);
    }
    if (reload_skin)
    {
        irr_driver->setMaxTextureSize();
        delete powerup_manager;
        powerup_manager = new PowerupManager();
        powerup_manager->loadPowerupsModels();
    }
    OptionsScreenUI::getInstance()->m_reload_option = nullptr;
}   // reloadGUIEngine

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
