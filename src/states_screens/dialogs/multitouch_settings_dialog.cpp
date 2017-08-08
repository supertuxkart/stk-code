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
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/multitouch_device.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

MultitouchSettingsDialog::MultitouchSettingsDialog(const float w, const float h)
        : ModalDialog(w, h)
{
    loadFromFile("multitouch_settings.stkgui");
}

// -----------------------------------------------------------------------------

MultitouchSettingsDialog::~MultitouchSettingsDialog()
{
}

// -----------------------------------------------------------------------------

void MultitouchSettingsDialog::beforeAddingWidgets()
{
    SpinnerWidget* accelerometer = getWidget<SpinnerWidget>("accelerometer");
    assert(accelerometer != NULL);

    accelerometer->m_properties[PROP_WRAP_AROUND] = "true";
    accelerometer->clearLabels();
    accelerometer->addLabel(_("Disabled"));
    accelerometer->addLabel(_("Tablet"));
    accelerometer->addLabel(_("Phone"));
    accelerometer->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    accelerometer->m_properties[GUIEngine::PROP_MAX_VALUE] = "2";

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

        SpinnerWidget* deadzone_edge = getWidget<SpinnerWidget>("deadzone_edge");
        assert(deadzone_edge != NULL);
        UserConfigParams::m_multitouch_deadzone_edge =
                                    (float)deadzone_edge->getValue() / 100.0f;

        SpinnerWidget* deadzone_center = getWidget<SpinnerWidget>("deadzone_center");
        assert(deadzone_center != NULL);
        UserConfigParams::m_multitouch_deadzone_center =
                                    (float)deadzone_center->getValue() / 100.0f;

        CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("buttons_enabled");
        assert(buttons_en != NULL);
        UserConfigParams::m_multitouch_mode = buttons_en->getState() ? 1 : 0;
        
        CheckBoxWidget* buttons_inv = getWidget<CheckBoxWidget>("buttons_inverted");
        assert(buttons_inv != NULL);
        UserConfigParams::m_multitouch_inverted = buttons_inv->getState();

        SpinnerWidget* accelerometer = getWidget<SpinnerWidget>("accelerometer");
        assert(accelerometer != NULL);

        UserConfigParams::m_multitouch_accelerometer = accelerometer->getValue();

        MultitouchDevice* touch_device = input_manager->getDeviceManager()->
                                                        getMultitouchDevice();

        if (touch_device != NULL)
        {
            touch_device->updateConfigParams();
        }

        user_config->saveConfig();

        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "restore")
    {
        UserConfigParams::m_multitouch_scale.revertToDefaults();
        UserConfigParams::m_multitouch_deadzone_edge.revertToDefaults();
        UserConfigParams::m_multitouch_deadzone_center.revertToDefaults();
        UserConfigParams::m_multitouch_mode.revertToDefaults();
        UserConfigParams::m_multitouch_accelerometer.revertToDefaults();

        updateValues();

        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------

void MultitouchSettingsDialog::updateValues()
{
    SpinnerWidget* scale = getWidget<SpinnerWidget>("scale");
    assert(scale != NULL);
    scale->setValue((int)(UserConfigParams::m_multitouch_scale * 100.0f));

    SpinnerWidget* deadzone_edge = getWidget<SpinnerWidget>("deadzone_edge");
    assert(deadzone_edge != NULL);
    deadzone_edge->setValue(
                (int)(UserConfigParams::m_multitouch_deadzone_edge * 100.0f));

    SpinnerWidget* deadzone_center = getWidget<SpinnerWidget>("deadzone_center");
    assert(deadzone_center != NULL);
    deadzone_center->setValue(
                (int)(UserConfigParams::m_multitouch_deadzone_center * 100.0f));

    CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("buttons_enabled");
    assert(buttons_en != NULL);
    buttons_en->setState(UserConfigParams::m_multitouch_mode != 0);
    
    CheckBoxWidget* buttons_inv = getWidget<CheckBoxWidget>("buttons_inverted");
    assert(buttons_inv != NULL);
    buttons_inv->setState(UserConfigParams::m_multitouch_inverted);

    SpinnerWidget* accelerometer = getWidget<SpinnerWidget>("accelerometer");
    assert(accelerometer != NULL);
    accelerometer->setValue(UserConfigParams::m_multitouch_accelerometer);
}

// -----------------------------------------------------------------------------
