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

#include "states_screens/dialogs/custom_camera_settings.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

CustomCameraSettingsDialog::CustomCameraSettingsDialog(const float w, const float h) :
        ModalDialog(w, h)
{
    m_self_destroy = false;
    loadFromFile("custom_camera_settings.stkgui");
}

// -----------------------------------------------------------------------------

CustomCameraSettingsDialog::~CustomCameraSettingsDialog()
{
}


// -----------------------------------------------------------------------------

void CustomCameraSettingsDialog::beforeAddingWidgets()
{
#ifndef SERVER_ONLY
    getWidget<SpinnerWidget>("fov")->setRange(75, 115);
    getWidget<SpinnerWidget>("camera_distance")->setRange(0.05f, 20, 0.05f);
    getWidget<SpinnerWidget>("camera_angle")->setRange(0, 80);
    getWidget<SpinnerWidget>("backward_camera_distance")->setRange(0.05f, 20, 0.05f);
    getWidget<SpinnerWidget>("backward_camera_angle")->setRange(0, 80);
    if (UserConfigParams::m_camera_present == 1) // Standard camera
    {
        getWidget<LabelWidget>("camera_name")->setText(_("Standard"), false);
        getWidget<SpinnerWidget>("fov")->setValue(UserConfigParams::m_standard_camera_fov);
        getWidget<SpinnerWidget>("camera_distance")->setFloatValue(UserConfigParams::m_standard_camera_distance);
        getWidget<SpinnerWidget>("camera_angle")->setValue(UserConfigParams::m_standard_camera_forward_up_angle);
        getWidget<SpinnerWidget>("backward_camera_distance")->setFloatValue(UserConfigParams::m_standard_camera_backward_distance);
        getWidget<SpinnerWidget>("backward_camera_angle")->setValue(UserConfigParams::m_standard_camera_backward_up_angle);
        getWidget<CheckBoxWidget>("camera_smoothing")->setState(UserConfigParams::m_standard_camera_forward_smoothing);
        getWidget<CheckBoxWidget>("use_soccer_camera")->setState(UserConfigParams::m_standard_reverse_look_use_soccer_cam);
        // Not allowed to change fov, distance, and angles. Only allow to change smoothing and follow soccer
        getWidget<SpinnerWidget>("fov")->setActive(false);
        getWidget<SpinnerWidget>("camera_distance")->setActive(false);
        getWidget<SpinnerWidget>("camera_angle")->setActive(false);
        getWidget<SpinnerWidget>("backward_camera_distance")->setActive(false);
        getWidget<SpinnerWidget>("backward_camera_angle")->setActive(false);
    }
    else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
    {
        getWidget<LabelWidget>("camera_name")->setText(_("Drone chase"), false);
        getWidget<SpinnerWidget>("fov")->setValue(UserConfigParams::m_drone_camera_fov);
        getWidget<SpinnerWidget>("camera_distance")->setFloatValue(UserConfigParams::m_drone_camera_distance);
        getWidget<SpinnerWidget>("camera_angle")->setValue(UserConfigParams::m_drone_camera_forward_up_angle);
        getWidget<SpinnerWidget>("backward_camera_distance")->setFloatValue(UserConfigParams::m_drone_camera_backward_distance);
        getWidget<SpinnerWidget>("backward_camera_angle")->setValue(UserConfigParams::m_drone_camera_backward_up_angle);
        getWidget<CheckBoxWidget>("camera_smoothing")->setState(UserConfigParams::m_drone_camera_forward_smoothing);
        getWidget<CheckBoxWidget>("use_soccer_camera")->setState(UserConfigParams::m_drone_reverse_look_use_soccer_cam);
        // Not allowed to change fov, distance, and angles. Only allow to change smoothing and follow soccer
        getWidget<SpinnerWidget>("fov")->setActive(false);
        getWidget<SpinnerWidget>("camera_distance")->setActive(false);
        getWidget<SpinnerWidget>("camera_angle")->setActive(false);
        getWidget<SpinnerWidget>("backward_camera_distance")->setActive(false);
        getWidget<SpinnerWidget>("backward_camera_angle")->setActive(false);
    }
    else // Custom camera
    {
        getWidget<LabelWidget>("camera_name")->setText(_("Custom"), false);
        getWidget<SpinnerWidget>("fov")->setValue(UserConfigParams::m_saved_camera_fov);
        getWidget<SpinnerWidget>("camera_distance")->setFloatValue(UserConfigParams::m_saved_camera_distance);
        getWidget<SpinnerWidget>("camera_angle")->setValue(UserConfigParams::m_saved_camera_forward_up_angle);
        getWidget<CheckBoxWidget>("camera_smoothing")->setState(UserConfigParams::m_saved_camera_forward_smoothing);
        getWidget<SpinnerWidget>("backward_camera_distance")->setFloatValue(UserConfigParams::m_saved_camera_backward_distance);
        getWidget<SpinnerWidget>("backward_camera_angle")->setValue(UserConfigParams::m_saved_camera_backward_up_angle);
        getWidget<CheckBoxWidget>("use_soccer_camera")->setState(UserConfigParams::m_saved_reverse_look_use_soccer_cam);
    }
#endif
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomCameraSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY
    if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            UserConfigParams::m_camera_forward_smoothing = getWidget<CheckBoxWidget>("camera_smoothing")->getState();
            UserConfigParams::m_reverse_look_use_soccer_cam = getWidget<CheckBoxWidget>("use_soccer_camera")->getState();
            if (UserConfigParams::m_camera_present == 1) // Standard camera, only smoothing and follow soccer is customizable
            {
                UserConfigParams::m_saved_camera_forward_smoothing = UserConfigParams::m_camera_forward_smoothing;
                UserConfigParams::m_standard_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;

            }
            else if (UserConfigParams::m_camera_present == 2) // Drone chase camera, only smoothing and follow soccer is customizable
            {
                UserConfigParams::m_drone_camera_forward_smoothing = UserConfigParams::m_camera_forward_smoothing;
                UserConfigParams::m_drone_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
            }
            else // Custom camera, everything is customizable
            {
                UserConfigParams::m_camera_fov = getWidget<SpinnerWidget>("fov")->getValue();
                UserConfigParams::m_camera_distance = getWidget<SpinnerWidget>("camera_distance")->getFloatValue();
                UserConfigParams::m_camera_forward_up_angle = getWidget<SpinnerWidget>("camera_angle")->getValue();
                UserConfigParams::m_camera_backward_distance = getWidget<SpinnerWidget>("backward_camera_distance")->getFloatValue();
                UserConfigParams::m_camera_backward_up_angle = getWidget<SpinnerWidget>("backward_camera_angle")->getValue();
                UserConfigParams::m_saved_camera_fov = UserConfigParams::m_camera_fov;
                UserConfigParams::m_saved_camera_distance = UserConfigParams::m_camera_distance;
                UserConfigParams::m_saved_camera_forward_up_angle = UserConfigParams::m_camera_forward_up_angle;
                UserConfigParams::m_saved_camera_forward_smoothing = UserConfigParams::m_camera_forward_smoothing;
                UserConfigParams::m_saved_camera_backward_distance = UserConfigParams::m_camera_backward_distance;
                UserConfigParams::m_saved_camera_backward_up_angle = UserConfigParams::m_camera_backward_up_angle;
                UserConfigParams::m_saved_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
            }
            OptionsScreenUI::getInstance()->updateCameraPresetSpinner();
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "reset") // discard all the changes
        {
            if (UserConfigParams::m_camera_present == 1) // Standard camera
            {
                getWidget<CheckBoxWidget>("camera_smoothing")->setState(UserConfigParams::m_standard_camera_forward_smoothing);
                getWidget<CheckBoxWidget>("use_soccer_camera")->setState(UserConfigParams::m_standard_reverse_look_use_soccer_cam);
            }
            else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
            {
                getWidget<CheckBoxWidget>("camera_smoothing")->setState(UserConfigParams::m_drone_camera_forward_smoothing);
                getWidget<CheckBoxWidget>("use_soccer_camera")->setState(UserConfigParams::m_drone_reverse_look_use_soccer_cam);
            }
            else // Custom camera
            {
                getWidget<SpinnerWidget>("fov")->setValue(UserConfigParams::m_saved_camera_fov);
                getWidget<SpinnerWidget>("camera_distance")->setFloatValue(UserConfigParams::m_saved_camera_distance);
                getWidget<SpinnerWidget>("camera_angle")->setValue(UserConfigParams::m_saved_camera_forward_up_angle);
                getWidget<CheckBoxWidget>("camera_smoothing")->setState(UserConfigParams::m_saved_camera_forward_smoothing);
                getWidget<SpinnerWidget>("backward_camera_distance")->setFloatValue(UserConfigParams::m_saved_camera_backward_distance);
                getWidget<SpinnerWidget>("backward_camera_angle")->setValue(UserConfigParams::m_saved_camera_backward_up_angle);
                getWidget<CheckBoxWidget>("use_soccer_camera")->setState(UserConfigParams::m_saved_reverse_look_use_soccer_cam);
            }
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
#endif
    return GUIEngine::EVENT_LET;
}   // processEvent
