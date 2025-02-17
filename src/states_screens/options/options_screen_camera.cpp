//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2025 Marianne Gagnon, Alayan et al.
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

// Manages includes common to all options screens
#include "states_screens/options/options_common.hpp"

#include "graphics/camera/camera.hpp"
#include "graphics/camera/camera_normal.hpp"
#include "config/player_manager.hpp"
#include "states_screens/dialogs/custom_camera_settings.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"

#include <IrrlichtDevice.h>

using namespace GUIEngine;
using namespace Online;

// -----------------------------------------------------------------------------

OptionsScreenCamera::OptionsScreenCamera() : Screen("options/options_camera.stkgui")
{
    m_inited = false;
}   // OptionsScreenCamera

// -----------------------------------------------------------------------------

void OptionsScreenCamera::loadedFromFile()
{
    m_inited = false;

    // Setup the camera spinner
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
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenCamera::init()
{
    Screen::init();
    OptionsCommon::setTabStatus();

    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_camera", PLAYER_ID_GAME_MASTER );

    // --- select the right camera in the spinner
    GUIEngine::SpinnerWidget* camera_preset = getWidget<GUIEngine::SpinnerWidget>("camera_preset");
    assert( camera_preset != NULL );

    camera_preset->setValue(UserConfigParams::m_camera_present); // use the saved camera
    updateCameraPresetSpinner();
}   // init

// -----------------------------------------------------------------------------
void OptionsScreenCamera::updateCamera()
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
} // updateCamera

// -----------------------------------------------------------------------------
void OptionsScreenCamera::updateCameraPresetSpinner()
{
    updateCamera();
} // updateCameraPresetSpinner

// -----------------------------------------------------------------------------
void OptionsScreenCamera::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
#ifndef SERVER_ONLY
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_camera")
            OptionsCommon::switchTab(selection);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
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

void OptionsScreenCamera::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenCamera::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------
