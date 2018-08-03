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

#include "states_screens/dialogs/init_android_dialog.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/multitouch_device.hpp"
#include "utils/translation.hpp"

#ifdef ANDROID
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceAndroid.h"
#endif

#include <IGUIEnvironment.h>


using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;

// -----------------------------------------------------------------------------

InitAndroidDialog::InitAndroidDialog(const float w, const float h)
        : ModalDialog(w, h)
{
}

// -----------------------------------------------------------------------------

InitAndroidDialog::~InitAndroidDialog()
{
}

// -----------------------------------------------------------------------------

void InitAndroidDialog::load()
{
    loadFromFile("android/init_android.stkgui");
}

// -----------------------------------------------------------------------------

void InitAndroidDialog::beforeAddingWidgets()
{
    bool accelerometer_available = false;
    
#ifdef ANDROID
    CIrrDeviceAndroid* android_device = dynamic_cast<CIrrDeviceAndroid*>(
                                                    irr_driver->getDevice());
    assert(android_device != NULL);
    accelerometer_available = android_device->isAccelerometerAvailable();
#endif

    if (!accelerometer_available)
    {
        RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
        assert(control_type != NULL);

        int index = control_type->findItemNamed("accelerometer");
        Widget* accelerometer = &control_type->getChildren()[index];
        accelerometer->setActive(false);
        
        if (UserConfigParams::m_multitouch_controls == 2)
        {
            UserConfigParams::m_multitouch_controls = 1;
        }
    }

    updateValues();
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation InitAndroidDialog::processEvent(
                                                const std::string& eventSource)
{
    if (eventSource == "close")
    {  
        RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
        assert(control_type != NULL);
        
        const std::string& selected = control_type->getSelectionIDString(
                                                        PLAYER_ID_GAME_MASTER);
        int index = control_type->getSelection(PLAYER_ID_GAME_MASTER);
        Widget* selected_widget = &control_type->getChildren()[index];
        
        if (!selected_widget->isActivated())
            return GUIEngine::EVENT_BLOCK;
        
        if (selected == "steering_wheel")
        {
            UserConfigParams::m_multitouch_controls = 1;
        }
        else if (selected == "accelerometer")
        {
            UserConfigParams::m_multitouch_controls = 2;
        }
        
        user_config->saveConfig();
        
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }

    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------

void InitAndroidDialog::updateValues()
{
    RibbonWidget* control_type = getWidget<RibbonWidget>("control_type");
    assert(control_type != NULL);
    
    if (UserConfigParams::m_multitouch_controls == 2)
    {
        int id = control_type->findItemNamed("accelerometer");
        control_type->setSelection(id, PLAYER_ID_GAME_MASTER);
    }
    else
    {
        int id = control_type->findItemNamed("steering_wheel");
        control_type->setSelection(id, PLAYER_ID_GAME_MASTER);
    }
}

// -----------------------------------------------------------------------------

bool InitAndroidDialog::onEscapePressed()
{
    UserConfigParams::m_multitouch_controls = 1;
    user_config->saveConfig();
    ModalDialog::dismiss();
    return true;
}   // onEscapePressed

// -----------------------------------------------------------------------------
