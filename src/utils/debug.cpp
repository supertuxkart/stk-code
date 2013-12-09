//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Lionel Fuentes
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

#include "debug.hpp"
#include "graphics/irr_driver.hpp"
#include "config/user_config.hpp"
#include "utils/log.hpp"
#include <IGUIEnvironment.h>
#include <IGUIContextMenu.h>
using namespace irr;
using namespace gui;

namespace Debug {

/** This is to let mouse input events go through when the debug menu is visible, otherwise
        GUI events would be blocked while in a race... */
static bool g_debug_menu_visible = false;

static bool g_was_pointer_shown = false;

// -----------------------------------------------------------------------------
// Commands for the debug menu
enum DebugMenuCommand
{
	//! graphics commands
	DEBUG_GRAPHICS_RELOAD_SHADERS,
};

// -----------------------------------------------------------------------------
// Debug menu handling
bool onEvent(const SEvent &event)
{
    // Only activated in artist debug mode
    if(!UserConfigParams::m_artist_debug_mode)
        return true;    // keep handling the events

    if(event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        // Create the menu (only one menu at a time)
        if(event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN && !g_debug_menu_visible)
        {
            // root menu
            gui::IGUIEnvironment*   guienv = irr_driver->getGUI();
            IGUIContextMenu* mnu = guienv->addContextMenu(
                core::rect<s32>(event.MouseInput.X, event.MouseInput.Y, event.MouseInput.Y+100, event.MouseInput.Y+100),NULL);
            mnu->addItem(L"Graphics >",-1,true,true);
            
            // graphics menu
            IGUIContextMenu* sub = mnu->getSubMenu(0);

            sub->addItem(L"Reload shaders", DEBUG_GRAPHICS_RELOAD_SHADERS );
            g_debug_menu_visible = true;

            g_was_pointer_shown = irr_driver->isPointerShown();
            irr_driver->showPointer();
        }

        // Let Irrlicht handle the event while the menu is visible - otherwise in a race the GUI events won't be generated
        if(g_debug_menu_visible)
            return false;
    }

    if (event.EventType == EET_GUI_EVENT)
    {
        if (event.GUIEvent.Caller != NULL)
        {
            if(event.GUIEvent.Caller->getType() == EGUIET_CONTEXT_MENU)
            {
                IGUIContextMenu *menu = (IGUIContextMenu*)event.GUIEvent.Caller;
                s32 cmdID = menu->getItemCommandId(menu->getSelectedItem());

                if(event.GUIEvent.EventType == EGET_ELEMENT_CLOSED)
                {
                    g_debug_menu_visible = false;
                    if(g_was_pointer_shown)
                        irr_driver->showPointer();
                    else
                        irr_driver->hidePointer();
                }

                if(cmdID == DEBUG_GRAPHICS_RELOAD_SHADERS)
                {
                    Log::info("Debug", "Reloading shaders...\n");
                    irr_driver->updateShaders();
                }
                return false;   // event has been treated
            }
        }
    }
    return true;    // continue event handling
}

}
