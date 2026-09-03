//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "states_screens/dialogs/multitouch_settings_dialog.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/multitouch_device.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/custom_gui_settings.hpp"
#include "states_screens/race_gui_multitouch.hpp"
#include "utils/translation.hpp"

#include <IrrlichtDevice.h>
#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

MultitouchSettingsDialog::MultitouchSettingsDialog(const float w, const float h)
        : ModalDialog(w, h)
{
    loadFromFile("android/multitouch_settings.stkgui");
}

// -----------------------------------------------------------------------------

MultitouchSettingsDialog::~MultitouchSettingsDialog()
{
}

// -----------------------------------------------------------------------------

void MultitouchSettingsDialog::beforeAddingWidgets()
{
    bool accelerometer_available = false;
    bool gyroscope_available = false;
    
    IrrlichtDevice* irrlicht_device = irr_driver->getDevice();
    assert(irrlicht_device != NULL);
    accelerometer_available = irrlicht_device->isAccelerometerAvailable();
    gyroscope_available = irrlicht_device->isGyroscopeAvailable() && accelerometer_available;

    if (!accelerometer_available)
    {
        RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
        assert(control_type != NULL);

        int index = control_type->findItemNamed("accelerometer");
        Widget* accelerometer = &control_type->getChildren()[index];
        accelerometer->setActive(false);

        if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_ACCELEROMETER)
            UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
    }

    if (!gyroscope_available)
    {
        RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
        assert(control_type != NULL);

        int index = control_type->findItemNamed("gyroscope");
        Widget* gyroscope = &control_type->getChildren()[index];
        gyroscope->setActive(false);

        if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
            UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
    }
    
    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("buttons_enabled");
        assert(buttons_en != NULL);
        buttons_en->setActive(false);
    }

    updateValues();
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation MultitouchSettingsDialog::processEvent(
                                                const std::string& eventSource)
{
    if (eventSource == "control_type")
    {
        RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
        assert(control_type != NULL);

        const std::string& selected = control_type->getSelectionIDString(
            PLAYER_ID_GAME_MASTER);
        if (selected == "steering_wheel")
        {
            getWidget<CheckBoxWidget>("auto_acceleration")->setActive(true);
        }
        else
        {
            getWidget<CheckBoxWidget>("auto_acceleration")->setState(false);
            getWidget<CheckBoxWidget>("auto_acceleration")->setActive(false);
        }
    }
    else if (eventSource == "buttons")
    {
        const std::string& selection = getWidget<RibbonWidget>("buttons")->
        getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            SpinnerWidget* scale = getWidget<SpinnerWidget>("scale");
            assert(scale != NULL);
            UserConfigParams::m_multitouch_scale = (float)scale->getValue() / 100.0f;

            SpinnerWidget* sensitivity_x = getWidget<SpinnerWidget>("sensitivity_x");
            assert(sensitivity_x != NULL);
            UserConfigParams::m_multitouch_sensitivity_x =
                                        (float)sensitivity_x->getValue() / 100.0f;

            SpinnerWidget* sensitivity_y = getWidget<SpinnerWidget>("sensitivity_y");
            assert(sensitivity_y != NULL);
            UserConfigParams::m_multitouch_sensitivity_y =
                                        (float)sensitivity_y->getValue() / 100.0f;

            SpinnerWidget* deadzone = getWidget<SpinnerWidget>("deadzone");
            assert(deadzone != NULL);
            UserConfigParams::m_multitouch_deadzone =
                                        (float)deadzone->getValue() / 100.0f;

            CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("buttons_enabled");
            assert(buttons_en != NULL);
            UserConfigParams::m_multitouch_draw_gui = buttons_en->getState();

            CheckBoxWidget* buttons_inv = getWidget<CheckBoxWidget>("buttons_inverted");
            assert(buttons_inv != NULL);
            UserConfigParams::m_multitouch_inverted = buttons_inv->getState();

            // Control types selection ribbon.

            RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
            assert(control_type != NULL);

            const std::string& control_selected = control_type->getSelectionIDString(
                PLAYER_ID_GAME_MASTER);

            if (control_selected == "steering_wheel")
            {
                UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
                UserConfigParams::m_multitouch_auto_acceleration = getWidget<CheckBoxWidget>("auto_acceleration")->getState();
            }
            else if (control_selected == "accelerometer")
            {
                UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_ACCELEROMETER;
                UserConfigParams::m_multitouch_auto_acceleration = false;
            }
            else if (control_selected == "gyroscope")
            {
                UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_GYROSCOPE;
                UserConfigParams::m_multitouch_auto_acceleration = false;
            }

            MultitouchDevice* touch_device = input_manager->getDeviceManager()->
                                                            getMultitouchDevice();

            if (touch_device != NULL)
            {
                touch_device->updateConfigParams();
            }

            if (World::getWorld() && World::getWorld()->getRaceGUI())
            {
                World::getWorld()->getRaceGUI()->recreateGUI();
            }

            user_config->saveConfig();

            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "restore")
        {
            UserConfigParams::m_multitouch_sensitivity_y.revertToDefaults();
            UserConfigParams::m_multitouch_deadzone.revertToDefaults();
            UserConfigParams::m_multitouch_inverted.revertToDefaults();
            UserConfigParams::m_multitouch_controls.revertToDefaults();
            UserConfigParams::m_multitouch_scale.revertToDefaults();
            UserConfigParams::m_multitouch_sensitivity_x.revertToDefaults();

            if (StateManager::get()->getGameState() != GUIEngine::INGAME_MENU)
            {
            #ifdef MOBILE_STK
                UserConfigParams::m_multitouch_draw_gui = true;
            #else
                UserConfigParams::m_multitouch_draw_gui.revertToDefaults();
            #endif
            }

            updateValues();

            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "edit")
        {
            new CustomGuiSettingsDialog(0.6f, 0.6f);
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------

void MultitouchSettingsDialog::updateValues()
{
    SpinnerWidget* scale = getWidget<SpinnerWidget>("scale");
    assert(scale != NULL);
    scale->setValue((int)(UserConfigParams::m_multitouch_scale * 100.0f));

    SpinnerWidget* sensitivity_x = getWidget<SpinnerWidget>("sensitivity_x");
    assert(sensitivity_x != NULL);
    sensitivity_x->setValue(
                (int)(UserConfigParams::m_multitouch_sensitivity_x * 100.0f));
                
    SpinnerWidget* sensitivity_y = getWidget<SpinnerWidget>("sensitivity_y");
    assert(sensitivity_y != NULL);
    sensitivity_y->setValue(
                (int)(UserConfigParams::m_multitouch_sensitivity_y * 100.0f));

    SpinnerWidget* deadzone = getWidget<SpinnerWidget>("deadzone");
    assert(deadzone != NULL);
    deadzone->setValue(
                (int)(UserConfigParams::m_multitouch_deadzone * 100.0f));

    CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("buttons_enabled");
    assert(buttons_en != NULL);
    buttons_en->setState(UserConfigParams::m_multitouch_draw_gui);
    
    CheckBoxWidget* buttons_inv = getWidget<CheckBoxWidget>("buttons_inverted");
    assert(buttons_inv != NULL);
    buttons_inv->setState(UserConfigParams::m_multitouch_inverted);

    RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
    assert(control_type != NULL);

    if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_ACCELEROMETER)
    {
        int id = control_type->findItemNamed("accelerometer");
        control_type->setSelection(id, PLAYER_ID_GAME_MASTER);
        getWidget<CheckBoxWidget>("auto_acceleration")->setActive(false);
    }
    else if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
    {
        int id = control_type->findItemNamed("gyroscope");
        control_type->setSelection(id, PLAYER_ID_GAME_MASTER);
        getWidget<CheckBoxWidget>("auto_acceleration")->setActive(false);
    }
    else
    {
        int id = control_type->findItemNamed("steering_wheel");
        control_type->setSelection(id, PLAYER_ID_GAME_MASTER);
    }
}

// -----------------------------------------------------------------------------
