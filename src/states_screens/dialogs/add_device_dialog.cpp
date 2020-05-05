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

#include "states_screens/dialogs/add_device_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/wiimote_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/cpp2011.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIStaticText.h>
#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr::gui;
using namespace irr::core;

// ----------------------------------------------------------------------------

AddDeviceDialog::AddDeviceDialog() : ModalDialog(0.90f, 0.80f)
{
    doInit();

    ScalableFont* font = GUIEngine::getFont();
    const int textHeight = GUIEngine::getFontHeight();
    const int buttonHeight = textHeight + 10;

#ifdef ENABLE_WIIUSE
    const int nbButtons = 3;
#else
    const int nbButtons = 2;
#endif

    const int y_bottom = m_area.getHeight() - nbButtons*(buttonHeight + 10) - 10;
    const int y_stride = buttonHeight+10;
    int cur_y = y_bottom;

    core::rect<s32> text_area( 15, 15, m_area.getWidth()-15, y_bottom-15 );

    core::stringw msg =
        _("New gamepads and joysticks will automatically appear in the list "
          "when you connect them to this device.\n\nTo add a "
          "keyboard config, you can use the button below, HOWEVER please "
          "note that most keyboards only support a limited amount of "
          "simultaneous keypresses and are thus inappropriate for multiplayer "
          "gameplay. (You can, however, connect multiple keyboards to this "
          "device. Remember that everyone still needs different keybindings "
          "in this case.)");
    IGUIStaticText* b =
        GUIEngine::getGUIEnv()->addStaticText(msg,
                                              text_area,
                                              /*border*/false ,
                                              /*word wrap*/true,
                                              m_irrlicht_window);
    b->setTabStop(false);
    b->setText(msg);

#ifdef ENABLE_WIIUSE
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "addwiimote";

        //I18N: In the 'add new input device' dialog
        widget->setText( _("Add Wiimote") );

        const int textWidth =
            font->getDimension( widget->getText().c_str() ).Width + 40;

        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = cur_y;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
        cur_y += y_stride;
    }
#endif  // ENABLE_WIIUSE

    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "addkeyboard";

        //I18N: In the 'add new input device' dialog
        widget->setText( _("Add Keyboard Configuration") );

        const int textWidth =
            font->getDimension( widget->getText().c_str() ).Width + 40;

        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = cur_y;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
        cur_y += y_stride;
    }
    {
        ButtonWidget* widget = new ButtonWidget();
        widget->m_properties[PROP_ID] = "cancel";
        widget->setText( _("Cancel") );

        const int textWidth =
            font->getDimension( widget->getText().c_str() ).Width + 40;

        widget->m_x = m_area.getWidth()/2 - textWidth/2;
        widget->m_y = cur_y;
        widget->m_w = textWidth;
        widget->m_h = buttonHeight;
        widget->setParent(m_irrlicht_window);
        m_widgets.push_back(widget);
        widget->add();
        cur_y += y_stride;

        widget->setFocusForPlayer( PLAYER_ID_GAME_MASTER );

    }

}   // AddDeviceDialog

// ----------------------------------------------------------------------------

void AddDeviceDialog::onEnterPressedInternal()
{
}   // onEnterPressedInternal

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation AddDeviceDialog::processEvent
                                               (const std::string& eventSource)
{

    if (eventSource == "cancel")
    {
        ModalDialog::dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if (eventSource == "addkeyboard")
    {
        input_manager->getDeviceManager()->addEmptyKeyboard();
        input_manager->getDeviceManager()->save();
        ModalDialog::dismiss();

        ((OptionsScreenInput*)GUIEngine::getCurrentScreen())->rebuildDeviceList();

        return GUIEngine::EVENT_BLOCK;
    }
#ifdef ENABLE_WIIUSE
    else if (eventSource == "addwiimote")
    {
        // Remove the previous modal dialog to avoid a warning
        GUIEngine::ModalDialog::dismiss();
        if(wiimote_manager->askUserToConnectWiimotes() > 0)
            ((OptionsScreenInput*)GUIEngine::getCurrentScreen())->rebuildDeviceList();

        return GUIEngine::EVENT_BLOCK;
    }
#endif

    return GUIEngine::EVENT_LET;
}   // processEvent
