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

#ifndef SERVER_ONLY
    getWidget<SpinnerWidget>("fov")->setRange(75, 115);
    getWidget<SpinnerWidget>("camera_distance")->setRange(0.05f, 20, 0.05f);
    getWidget<SpinnerWidget>("camera_angle")->setRange(0, 80);
    getWidget<SpinnerWidget>("backward_camera_distance")->setRange(0.05f, 20, 0.05f);
    getWidget<SpinnerWidget>("backward_camera_angle")->setRange(0, 80);
#endif
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

#ifndef SERVER_ONLY
    getWidget<SpinnerWidget>("fov")->setRange(75, 115);
    getWidget<SpinnerWidget>("camera_distance")->setRange(0.05f, 20, 0.05f);
    getWidget<SpinnerWidget>("camera_angle")->setRange(0, 80);
    getWidget<SpinnerWidget>("backward_camera_distance")->setRange(0.05f, 20, 0.05f);
    getWidget<SpinnerWidget>("backward_camera_angle")->setRange(0, 80);
    if (UserConfigParams::m_camera_present == 1) // Standard camera
    {
        setCameraParameters (UserConfigParams::m_standard_camera_fov, UserConfigParams::m_standard_camera_distance,
            UserConfigParams::m_standard_camera_forward_up_angle, UserConfigParams::m_standard_camera_backward_distance,
            UserConfigParams::m_standard_camera_backward_up_angle, UserConfigParams::m_standard_camera_forward_smoothing,
            UserConfigParams::m_standard_reverse_look_use_soccer_cam);
        // Not allowed to change fov, distance, and angles. Only allow to change smoothing and follow soccer
        setSpinnersActive(false);
    }
    else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
    {
        setCameraParameters (UserConfigParams::m_drone_camera_fov, UserConfigParams::m_drone_camera_distance,
            UserConfigParams::m_drone_camera_forward_up_angle, UserConfigParams::m_drone_camera_backward_distance,
            UserConfigParams::m_drone_camera_backward_up_angle, UserConfigParams::m_drone_camera_forward_smoothing,
            UserConfigParams::m_drone_reverse_look_use_soccer_cam);
        // Not allowed to change fov, distance, and angles. Only allow to change smoothing and follow soccer
        setSpinnersActive(false);
    }
    else // Custom camera
    {
        setCameraParameters (UserConfigParams::m_saved_camera_fov, UserConfigParams::m_saved_camera_distance,
            UserConfigParams::m_saved_camera_forward_up_angle, UserConfigParams::m_saved_camera_backward_distance,
            UserConfigParams::m_saved_camera_backward_up_angle, UserConfigParams::m_saved_camera_forward_smoothing,
            UserConfigParams::m_saved_reverse_look_use_soccer_cam);
        setSpinnersActive(true);
    }
#endif
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
void OptionsScreenCamera::setSpinnersActive(bool spinner_status)
{
    getWidget<SpinnerWidget>("fov")->setActive(spinner_status);
    getWidget<SpinnerWidget>("camera_distance")->setActive(spinner_status);
    getWidget<SpinnerWidget>("camera_angle")->setActive(spinner_status);
    getWidget<SpinnerWidget>("backward_camera_distance")->setActive(spinner_status);
    getWidget<SpinnerWidget>("backward_camera_angle")->setActive(spinner_status);
} // setSpinnersActive

// -----------------------------------------------------------------------------
void OptionsScreenCamera::setCameraParameters(int fov, float distance, float angle, float bw_distance,
                                              float bw_angle, bool forward_smoothing, bool soccer_follow)
{
    UserConfigParams::m_camera_fov = fov;
    UserConfigParams::m_camera_distance = distance;
    UserConfigParams::m_camera_forward_up_angle = angle;
    UserConfigParams::m_camera_backward_distance = bw_distance;
    UserConfigParams::m_camera_backward_up_angle = bw_angle;
    UserConfigParams::m_camera_forward_smoothing = forward_smoothing;
    UserConfigParams::m_reverse_look_use_soccer_cam = soccer_follow;

    // Update the widget values alongside
    getWidget<SpinnerWidget>("fov")->setValue(fov);
    getWidget<SpinnerWidget>("camera_distance")->setFloatValue(distance);
    getWidget<SpinnerWidget>("camera_angle")->setValue(angle);
    getWidget<SpinnerWidget>("backward_camera_distance")->setFloatValue(bw_distance);
    getWidget<SpinnerWidget>("backward_camera_angle")->setValue(bw_angle);
    getWidget<CheckBoxWidget>("camera_smoothing")->setState(forward_smoothing);
    getWidget<CheckBoxWidget>("use_soccer_camera")->setState(soccer_follow);
} // setCameraParameters

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
            setCameraParameters (UserConfigParams::m_standard_camera_fov, UserConfigParams::m_standard_camera_distance,
                UserConfigParams::m_standard_camera_forward_up_angle, UserConfigParams::m_standard_camera_backward_distance,
                UserConfigParams::m_standard_camera_backward_up_angle, UserConfigParams::m_standard_camera_forward_smoothing,
                UserConfigParams::m_standard_reverse_look_use_soccer_cam);
            setSpinnersActive(false);
        }
        else if (i == 2) //Drone chase
        {
            setCameraParameters (UserConfigParams::m_drone_camera_fov, UserConfigParams::m_drone_camera_distance,
                UserConfigParams::m_drone_camera_forward_up_angle, UserConfigParams::m_drone_camera_backward_distance,
                UserConfigParams::m_drone_camera_backward_up_angle, UserConfigParams::m_drone_camera_forward_smoothing,
                UserConfigParams::m_drone_reverse_look_use_soccer_cam);
            setSpinnersActive(false);
        }
        else //Custom
        {
            setCameraParameters (UserConfigParams::m_saved_camera_fov, UserConfigParams::m_saved_camera_distance,
                UserConfigParams::m_saved_camera_forward_up_angle, UserConfigParams::m_saved_camera_backward_distance,
                UserConfigParams::m_saved_camera_backward_up_angle, UserConfigParams::m_saved_camera_forward_smoothing,
                UserConfigParams::m_saved_reverse_look_use_soccer_cam);
            setSpinnersActive(true);
        }
        updateCamera();
    }
    else if (name == "camera_smoothing")
    {
        UserConfigParams::m_camera_forward_smoothing = getWidget<CheckBoxWidget>("camera_smoothing")->getState();
        if (UserConfigParams::m_camera_present == 1) // Standard camera
            UserConfigParams::m_standard_camera_forward_smoothing = UserConfigParams::m_camera_forward_smoothing;
        else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
            UserConfigParams::m_drone_camera_forward_smoothing = UserConfigParams::m_camera_forward_smoothing;
        else // Custom camera
            UserConfigParams::m_saved_camera_forward_smoothing = UserConfigParams::m_camera_forward_smoothing;
        updateCamera();
    }
    else if (name == "use_soccer_camera")
    {
        UserConfigParams::m_reverse_look_use_soccer_cam = getWidget<CheckBoxWidget>("use_soccer_camera")->getState();
        if (UserConfigParams::m_camera_present == 1) // Standard camera
            UserConfigParams::m_standard_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
        else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
            UserConfigParams::m_drone_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
        else // Custom camera
            UserConfigParams::m_saved_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
        updateCamera();
    }
    // Only the custom camera can update the following widgets, so we don't test the active camera preset
    else if (name == "fov")
    {
        UserConfigParams::m_camera_fov = getWidget<SpinnerWidget>("fov")->getValue();
        UserConfigParams::m_saved_camera_fov = UserConfigParams::m_camera_fov;
        updateCamera();
    }
    else if (name == "camera_distance")
    {
        UserConfigParams::m_camera_distance = getWidget<SpinnerWidget>("camera_distance")->getFloatValue();
        UserConfigParams::m_saved_camera_distance = UserConfigParams::m_camera_distance;
        updateCamera();
    }
    else if (name == "camera_angle")
    {
        UserConfigParams::m_camera_forward_up_angle = getWidget<SpinnerWidget>("camera_angle")->getValue();
        UserConfigParams::m_saved_camera_forward_up_angle = UserConfigParams::m_camera_forward_up_angle;
        updateCamera();
    }

    else if (name == "backward_camera_distance")
    {
        UserConfigParams::m_camera_backward_distance = getWidget<SpinnerWidget>("backward_camera_distance")->getFloatValue();
        UserConfigParams::m_saved_camera_backward_distance = UserConfigParams::m_camera_backward_distance;
        updateCamera();
    }
    else if (name == "backward_camera_angle")
    {
        UserConfigParams::m_camera_backward_up_angle = getWidget<SpinnerWidget>("backward_camera_angle")->getValue();
        UserConfigParams::m_saved_camera_backward_up_angle = UserConfigParams::m_camera_backward_up_angle;
        updateCamera();
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
