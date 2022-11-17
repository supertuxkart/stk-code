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
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/multitouch_device.hpp"
#include "modes/world.hpp"
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
        CheckBoxWidget* accelerometer = getWidget<CheckBoxWidget>("accelerometer");
        assert(accelerometer != NULL);
        accelerometer->setActive(false);
    }

    if (!gyroscope_available)
    {
        CheckBoxWidget* gyroscope = getWidget<CheckBoxWidget>("gyroscope");
        assert(gyroscope != NULL);
        gyroscope->setActive(false);
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
    if (eventSource == "close")
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

        CheckBoxWidget* accelerometer = getWidget<CheckBoxWidget>("accelerometer");
        assert(accelerometer != NULL);

        CheckBoxWidget* gyroscope = getWidget<CheckBoxWidget>("gyroscope");
        assert(gyroscope != NULL);

        UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;

        if (accelerometer->getState())
        {
            UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_ACCELEROMETER;
        }

        if (gyroscope->getState())
        {
            UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_GYROSCOPE;
        }

        if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_STEERING_WHEEL)
            UserConfigParams::m_multitouch_auto_acceleration = getWidget<CheckBoxWidget>("auto_acceleration")->getState();

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
    else if (eventSource == "restore")
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
    else if (eventSource == "accelerometer")
    {
        CheckBoxWidget* gyroscope = getWidget<CheckBoxWidget>("gyroscope");
        assert(gyroscope != NULL);
        gyroscope->setState(false);
        getWidget<CheckBoxWidget>("auto_acceleration")->setState(false);
    }
    else if (eventSource == "gyroscope")
    {
        CheckBoxWidget* accelerometer = getWidget<CheckBoxWidget>("accelerometer");
        assert(accelerometer != NULL);
        accelerometer->setState(false);
        getWidget<CheckBoxWidget>("auto_acceleration")->setState(false);
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

    CheckBoxWidget* accelerometer = getWidget<CheckBoxWidget>("accelerometer");
    assert(accelerometer != NULL);
    accelerometer->setState(UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_ACCELEROMETER);

    CheckBoxWidget* gyroscope = getWidget<CheckBoxWidget>("gyroscope");
    assert(gyroscope != NULL);
    gyroscope->setState(UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE);

    if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_STEERING_WHEEL)
        getWidget<CheckBoxWidget>("auto_acceleration")->setState(UserConfigParams::m_multitouch_auto_acceleration);
    else
        getWidget<CheckBoxWidget>("auto_acceleration")->setState(false);
}

// -----------------------------------------------------------------------------
