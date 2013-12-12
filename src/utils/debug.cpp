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
#include "config/user_config.hpp"
#include "karts/controller/controller.hpp"
#include "karts/abstract_kart.hpp"
#include "graphics/irr_driver.hpp"
#include "items/powerup_manager.hpp"
#include "modes/world.hpp"
#include "physics/irr_debug_drawer.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "replay/replay_recorder.hpp"
#include "utils/log.hpp"
#include <IGUIEnvironment.h>
#include <IGUIContextMenu.h>
using namespace irr;
using namespace gui;

namespace Debug {

/** This is to let mouse input events go through when the debug menu is visible, otherwise
        GUI events would be blocked while in a race... */
static bool g_debug_menu_visible = false;

// -----------------------------------------------------------------------------
// Commands for the debug menu
enum DebugMenuCommand
{
	//! graphics commands
	DEBUG_GRAPHICS_RELOAD_SHADERS,
    DEBUG_GRAPHICS_RESET,
    DEBUG_GRAPHICS_WIREFRAME,
    DEBUG_GRAPHICS_MIPMAP_VIZ,
    DEBUG_GRAPHICS_NORMALS_VIZ,
    DEBUG_GRAPHICS_SSAO_VIZ,
    DEBUG_GRAPHICS_SHADOW_VIZ,
    DEBUG_GRAPHICS_LIGHT_VIZ,
    DEBUG_GRAPHICS_DISTORT_VIZ,
    DEBUG_GRAPHICS_BULLET_1,
    DEBUG_GRAPHICS_BULLET_2,
    DEBUG_PROFILER,
    DEBUG_FPS,
    DEBUG_SAVE_REPLAY,
    DEBUG_SAVE_HISTORY,
    DEBUG_POWERUP_BOWLING,
    DEBUG_POWERUP_BUBBLEGUM,
    DEBUG_POWERUP_CAKE,
    DEBUG_POWERUP_PLUNGER,
    DEBUG_POWERUP_RUBBERBALL,
    DEBUG_POWERUP_SWATTER,
    DEBUG_POWERUP_SWITCH,
    DEBUG_POWERUP_ZIPPER,
    DEBUG_POWERUP_NITRO,
    DEBUG_TOGGLE_GUI
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
            gui::IGUIEnvironment* guienv = irr_driver->getGUI();
            IGUIContextMenu* mnu = guienv->addContextMenu(
                core::rect<s32>(event.MouseInput.X, event.MouseInput.Y, event.MouseInput.Y+100, event.MouseInput.Y+100),NULL);
            int graphicsMenuIndex = mnu->addItem(L"Graphics >",-1,true,true);
            
            // graphics menu
            IGUIContextMenu* sub = mnu->getSubMenu(graphicsMenuIndex);

            sub->addItem(L"Reload shaders", DEBUG_GRAPHICS_RELOAD_SHADERS );
            sub->addItem(L"Reset debug views", DEBUG_GRAPHICS_RESET );
            sub->addItem(L"Wireframe", DEBUG_GRAPHICS_WIREFRAME );
            sub->addItem(L"Mipmap viz", DEBUG_GRAPHICS_MIPMAP_VIZ );
            sub->addItem(L"Normals viz", DEBUG_GRAPHICS_NORMALS_VIZ );
            sub->addItem(L"SSAO viz", DEBUG_GRAPHICS_SSAO_VIZ );
            sub->addItem(L"Shadow viz", DEBUG_GRAPHICS_SHADOW_VIZ );
            sub->addItem(L"Light viz", DEBUG_GRAPHICS_LIGHT_VIZ );
            sub->addItem(L"Distort viz", DEBUG_GRAPHICS_DISTORT_VIZ );
            sub->addItem(L"Physics debug", DEBUG_GRAPHICS_BULLET_1);
            sub->addItem(L"Physics debug (no kart)", DEBUG_GRAPHICS_BULLET_2);

            int itemsMenuIndex = mnu->addItem(L"Items >",-1,true,true);
            sub = mnu->getSubMenu(1);
            sub->addItem(L"Basketball", DEBUG_POWERUP_RUBBERBALL );
            sub->addItem(L"Bowling", DEBUG_POWERUP_BOWLING );
            sub->addItem(L"Bubblegum", DEBUG_POWERUP_BUBBLEGUM );
            sub->addItem(L"Cake", DEBUG_POWERUP_CAKE );
            sub->addItem(L"Plunger", DEBUG_POWERUP_PLUNGER );
            sub->addItem(L"Swatter", DEBUG_POWERUP_SWATTER );
            sub->addItem(L"Switch", DEBUG_POWERUP_SWITCH );
            sub->addItem(L"Zipper", DEBUG_POWERUP_ZIPPER );
            sub->addItem(L"Nitro", DEBUG_POWERUP_NITRO );
            
            mnu->addItem(L"Profiler",DEBUG_PROFILER);
            mnu->addItem(L"FPS",DEBUG_FPS);
            mnu->addItem(L"Save replay", DEBUG_SAVE_REPLAY);
            mnu->addItem(L"Save history", DEBUG_SAVE_HISTORY);
            mnu->addItem(L"Toggle GUI", DEBUG_TOGGLE_GUI);


            g_debug_menu_visible = true;
            irr_driver->showPointer();
        }

        // Let Irrlicht handle the event while the menu is visible - otherwise in a race the GUI events won't be generated
        if(g_debug_menu_visible)
            return false;
    }

    if (event.EventType == EET_GUI_EVENT)
    {
        if (event.GUIEvent.Caller != NULL && event.GUIEvent.Caller->getType() == EGUIET_CONTEXT_MENU )
        {
            IGUIContextMenu *menu = (IGUIContextMenu*)event.GUIEvent.Caller;
            s32 cmdID = menu->getItemCommandId(menu->getSelectedItem());

            if(event.GUIEvent.EventType == EGET_ELEMENT_CLOSED)
            {
                g_debug_menu_visible = false;
            }

            if (event.GUIEvent.EventType == gui::EGET_MENU_ITEM_SELECTED)
            {
                if(cmdID == DEBUG_GRAPHICS_RELOAD_SHADERS)
                {
                    Log::info("Debug", "Reloading shaders...\n");
                    irr_driver->updateShaders();
                }
                else if (cmdID == DEBUG_GRAPHICS_RESET)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                }
                else if (cmdID == DEBUG_GRAPHICS_WIREFRAME)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleWireframe();
                }
                else if (cmdID == DEBUG_GRAPHICS_MIPMAP_VIZ)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleMipVisualization();
                }
                else if (cmdID == DEBUG_GRAPHICS_NORMALS_VIZ)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleNormals();
                }
                else if (cmdID == DEBUG_GRAPHICS_SSAO_VIZ)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleSSAOViz();
                }
                else if (cmdID == DEBUG_GRAPHICS_SHADOW_VIZ)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleShadowViz();
                }
                else if (cmdID == DEBUG_GRAPHICS_LIGHT_VIZ)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleLightViz();
                }
                else if (cmdID == DEBUG_GRAPHICS_DISTORT_VIZ)
                {
                    World* world = World::getWorld();
                    if (world != NULL) world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NONE);

                    irr_driver->resetDebugModes();
                    irr_driver->toggleDistortViz();
                }
                else if (cmdID == DEBUG_GRAPHICS_BULLET_1)
                {
                    irr_driver->resetDebugModes();

                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_KARTS_PHYSICS);
                }
                else if (cmdID == DEBUG_GRAPHICS_BULLET_2)
                {
                    irr_driver->resetDebugModes();

                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    world->getPhysics()->setDebugMode(IrrDebugDrawer::DM_NO_KARTS_GRAPHICS);
                }
                else if (cmdID == DEBUG_PROFILER)
                {
                    UserConfigParams::m_profiler_enabled =
                                            !UserConfigParams::m_profiler_enabled;
                }
                else if (cmdID == DEBUG_FPS)
                {
                    UserConfigParams::m_display_fps =
                                        !UserConfigParams::m_display_fps;
                }
                else if (cmdID == DEBUG_SAVE_REPLAY)
                {
                    ReplayRecorder::get()->Save();
                }
                else if (cmdID == DEBUG_SAVE_HISTORY)
                {
                    history->Save();
                }
                else if (cmdID == DEBUG_POWERUP_BOWLING)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_BOWLING, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_BUBBLEGUM)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_BUBBLEGUM, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_CAKE)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_CAKE, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_PLUNGER)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_PLUNGER, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_RUBBERBALL)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_RUBBERBALL, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_SWATTER)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_SWATTER, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_SWITCH)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_SWITCH, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_ZIPPER)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setPowerup(PowerupManager::POWERUP_ZIPPER, 10000);
                    }
                }
                else if (cmdID == DEBUG_POWERUP_NITRO)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    for(unsigned int i = 0; i < race_manager->getNumLocalPlayers(); i++)
                    {
                        AbstractKart* kart = world->getLocalPlayerKart(i);
                        kart->setEnergy(100.0f);
                    }
                }
                else if (cmdID == DEBUG_TOGGLE_GUI)
                {
                    World* world = World::getWorld();
                    if (world == NULL) return false;
                    RaceGUIBase* gui = world->getRaceGUI();
                    if (gui != NULL) gui->m_enabled = !gui->m_enabled;

                    const int count = World::getWorld()->getNumKarts();
                    for (int n=0; n<count; n++)
                    {
                        AbstractKart* kart = world->getKart(n);
                        if (kart->getController()->isPlayerController())
                            kart->getNode()->setVisible(gui->m_enabled);
                    }
                }
            }

            return false;   // event has been handled
        }
    }
    return true;    // continue event handling
}

bool isOpen()
{
    return g_debug_menu_visible;
}
}
