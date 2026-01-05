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
#include "states_screens/options/options_screen_display.hpp"
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
    m_widgets.bind(this);
}

// -----------------------------------------------------------------------------

CustomCameraSettingsDialog::~CustomCameraSettingsDialog()
{
}


// -----------------------------------------------------------------------------

void CustomCameraSettingsDialog::beforeAddingWidgets()
{
#ifndef SERVER_ONLY
    m_widgets.fov->setRange(75, 115);
    m_widgets.camera_distance->setRange(0.05f, 20, 0.05f);
    m_widgets.smooth_position->setRange(0.0f, 0.5f, 0.005f);
    m_widgets.smooth_rotation->setRange(0.0f, 0.5f, 0.005f);
    m_widgets.camera_angle->setRange(0, 80);
    m_widgets.backward_camera_distance->setRange(0.05f, 20, 0.05f);
    m_widgets.backward_camera_angle->setRange(0, 80);
    if (UserConfigParams::m_camera_present == 1) // Standard camera
    {
        m_widgets.camera_name->setText(_("Standard"), false);
        m_widgets.fov->setValue(UserConfigParams::m_standard_camera_fov);
        m_widgets.camera_distance->setFloatValue(UserConfigParams::m_standard_camera_distance);
        m_widgets.camera_angle->setValue(UserConfigParams::m_standard_camera_forward_up_angle);
        m_widgets.backward_camera_distance->setFloatValue(UserConfigParams::m_standard_camera_backward_distance);
        m_widgets.backward_camera_angle->setValue(UserConfigParams::m_standard_camera_backward_up_angle);
        m_widgets.smooth_position->setFloatValue(UserConfigParams::m_standard_camera_forward_smooth_position);
        m_widgets.smooth_rotation->setFloatValue(UserConfigParams::m_standard_camera_forward_smooth_rotation);
        m_widgets.use_soccer_camera->setState(UserConfigParams::m_standard_reverse_look_use_soccer_cam);
    }
    else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
    {
        m_widgets.camera_name->setText(_("Drone chase"), false);
        m_widgets.fov->setValue(UserConfigParams::m_drone_camera_fov);
        m_widgets.camera_distance->setFloatValue(UserConfigParams::m_drone_camera_distance);
        m_widgets.camera_angle->setValue(UserConfigParams::m_drone_camera_forward_up_angle);
        m_widgets.backward_camera_distance->setFloatValue(UserConfigParams::m_drone_camera_backward_distance);
        m_widgets.backward_camera_angle->setValue(UserConfigParams::m_drone_camera_backward_up_angle);
        m_widgets.smooth_position->setFloatValue(UserConfigParams::m_drone_camera_forward_smooth_position);
        m_widgets.smooth_rotation->setFloatValue(UserConfigParams::m_drone_camera_forward_smooth_rotation);
        m_widgets.use_soccer_camera->setState(UserConfigParams::m_drone_reverse_look_use_soccer_cam);
    }
    else // Custom camera
    {
        m_widgets.camera_name->setText(_("Custom"), false);
        m_widgets.fov->setValue(UserConfigParams::m_saved_camera_fov);
        m_widgets.camera_distance->setFloatValue(UserConfigParams::m_saved_camera_distance);
        m_widgets.camera_angle->setValue(UserConfigParams::m_saved_camera_forward_up_angle);
        m_widgets.backward_camera_distance->setFloatValue(UserConfigParams::m_saved_camera_backward_distance);
        m_widgets.backward_camera_angle->setValue(UserConfigParams::m_saved_camera_backward_up_angle);
        m_widgets.smooth_position->setFloatValue(UserConfigParams::m_saved_camera_forward_smooth_position);
        m_widgets.smooth_rotation->setFloatValue(UserConfigParams::m_saved_camera_forward_smooth_rotation);
        m_widgets.use_soccer_camera->setState(UserConfigParams::m_saved_reverse_look_use_soccer_cam);
    }
#endif
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation CustomCameraSettingsDialog::processEvent(const std::string& eventSource)
{
#ifndef SERVER_ONLY
    if (eventSource == "buttons")
    {
        const std::string& selection = m_widgets.buttons->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            UserConfigParams::m_camera_fov = m_widgets.fov->getValue();
            UserConfigParams::m_camera_distance = m_widgets.camera_distance->getFloatValue();
            UserConfigParams::m_camera_forward_up_angle = m_widgets.camera_angle->getValue();
            UserConfigParams::m_camera_backward_distance = m_widgets.backward_camera_distance->getFloatValue();
            UserConfigParams::m_camera_backward_up_angle = m_widgets.backward_camera_angle->getValue();
            UserConfigParams::m_camera_forward_smooth_position = m_widgets.smooth_position->getFloatValue();
            UserConfigParams::m_camera_forward_smooth_rotation = m_widgets.smooth_rotation->getFloatValue();
            UserConfigParams::m_reverse_look_use_soccer_cam = m_widgets.use_soccer_camera->getState();

            if (UserConfigParams::m_camera_present == 1) // Standard camera, only smoothing and follow soccer is customizable
            {
                UserConfigParams::m_standard_camera_fov = UserConfigParams::m_camera_fov;
                UserConfigParams::m_standard_camera_distance = UserConfigParams::m_camera_distance;
                UserConfigParams::m_standard_camera_forward_up_angle = UserConfigParams::m_camera_forward_up_angle;
                UserConfigParams::m_standard_camera_forward_smooth_position = UserConfigParams::m_camera_forward_smooth_position;
                UserConfigParams::m_standard_camera_forward_smooth_rotation = UserConfigParams::m_camera_forward_smooth_rotation;
                UserConfigParams::m_standard_camera_backward_distance = UserConfigParams::m_camera_backward_distance;
                UserConfigParams::m_standard_camera_backward_up_angle = UserConfigParams::m_camera_backward_up_angle;
                UserConfigParams::m_standard_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
            }
            else if (UserConfigParams::m_camera_present == 2) // Drone chase camera, only smoothing and follow soccer is customizable
            {
                UserConfigParams::m_drone_camera_fov = UserConfigParams::m_camera_fov;
                UserConfigParams::m_drone_camera_distance = UserConfigParams::m_camera_distance;
                UserConfigParams::m_drone_camera_forward_up_angle = UserConfigParams::m_camera_forward_up_angle;
                UserConfigParams::m_drone_camera_forward_smooth_position = UserConfigParams::m_camera_forward_smooth_position;
                UserConfigParams::m_drone_camera_forward_smooth_rotation = UserConfigParams::m_camera_forward_smooth_rotation;
                UserConfigParams::m_drone_camera_backward_distance = UserConfigParams::m_camera_backward_distance;
                UserConfigParams::m_drone_camera_backward_up_angle = UserConfigParams::m_camera_backward_up_angle;
                UserConfigParams::m_drone_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
            }
            else // Custom camera, everything is customizable
            {
                UserConfigParams::m_saved_camera_fov = UserConfigParams::m_camera_fov;
                UserConfigParams::m_saved_camera_distance = UserConfigParams::m_camera_distance;
                UserConfigParams::m_saved_camera_forward_up_angle = UserConfigParams::m_camera_forward_up_angle;
                UserConfigParams::m_saved_camera_forward_smooth_position = UserConfigParams::m_camera_forward_smooth_position;
                UserConfigParams::m_saved_camera_forward_smooth_rotation = UserConfigParams::m_camera_forward_smooth_rotation;
                UserConfigParams::m_saved_camera_backward_distance = UserConfigParams::m_camera_backward_distance;
                UserConfigParams::m_saved_camera_backward_up_angle = UserConfigParams::m_camera_backward_up_angle;
                UserConfigParams::m_saved_reverse_look_use_soccer_cam = UserConfigParams::m_reverse_look_use_soccer_cam;
            }
            OptionsScreenDisplay::getInstance()->updateCamera();
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "reset") // discard all the changes
        {
            if (UserConfigParams::m_camera_present == 1) // Standard camera
            {
                UserConfigParams::m_camera_fov = 80;
                UserConfigParams::m_camera_distance = 1.0;
                UserConfigParams::m_camera_forward_up_angle = 0.0;
                UserConfigParams::m_camera_forward_smooth_position = 0.2;
                UserConfigParams::m_camera_forward_smooth_rotation = 0.125;
                UserConfigParams::m_camera_backward_distance = 2.0;
                UserConfigParams::m_camera_backward_up_angle = 10;
                UserConfigParams::m_reverse_look_use_soccer_cam = false;
            }
            else if (UserConfigParams::m_camera_present == 2) // Drone chase camera
            {
                UserConfigParams::m_camera_fov = 100;
                UserConfigParams::m_camera_distance = 2.6;
                UserConfigParams::m_camera_forward_up_angle = 33;
                UserConfigParams::m_camera_forward_smooth_position = 0.0;
                UserConfigParams::m_camera_forward_smooth_rotation = 0.0;
                UserConfigParams::m_camera_backward_distance = 2.0;
                UserConfigParams::m_camera_backward_up_angle = 10;
                UserConfigParams::m_reverse_look_use_soccer_cam = false;
            }
            else // Custom camera
            {
                UserConfigParams::m_camera_fov = UserConfigParams::m_saved_camera_fov;
                UserConfigParams::m_camera_distance = UserConfigParams::m_saved_camera_distance;
                UserConfigParams::m_camera_forward_up_angle = UserConfigParams::m_saved_camera_forward_up_angle;
                UserConfigParams::m_camera_forward_smooth_position = UserConfigParams::m_saved_camera_forward_smooth_position;
                UserConfigParams::m_camera_forward_smooth_rotation = UserConfigParams::m_saved_camera_forward_smooth_rotation;
                UserConfigParams::m_camera_backward_distance = UserConfigParams::m_saved_camera_backward_distance;
                UserConfigParams::m_camera_backward_up_angle = UserConfigParams::m_saved_camera_backward_up_angle;
                UserConfigParams::m_reverse_look_use_soccer_cam = UserConfigParams::m_saved_reverse_look_use_soccer_cam;
            }
            m_widgets.fov->setValue(UserConfigParams::m_camera_fov);
            m_widgets.camera_distance->setFloatValue(UserConfigParams::m_camera_distance);
            m_widgets.camera_angle->setValue(UserConfigParams::m_camera_forward_up_angle);
            m_widgets.smooth_position->setFloatValue(UserConfigParams::m_camera_forward_smooth_position);
            m_widgets.smooth_rotation->setFloatValue(UserConfigParams::m_camera_forward_smooth_rotation);
            m_widgets.backward_camera_distance->setFloatValue(UserConfigParams::m_camera_backward_distance);
            m_widgets.backward_camera_angle->setValue(UserConfigParams::m_camera_backward_up_angle);
            m_widgets.use_soccer_camera->setState(UserConfigParams::m_reverse_look_use_soccer_cam);
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