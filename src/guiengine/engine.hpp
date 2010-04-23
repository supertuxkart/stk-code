//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#ifndef HEADER_ENGINE_HPP
#define HEADER_ENGINE_HPP

#include <irrlicht.h>
#include <string>

#include "guiengine/abstract_state_manager.hpp"
#include "guiengine/widgets.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{    
    class Screen;
    class CutScene;
    class Widget;
    
    /** Returns the widget currently focused by given player, or NULL if none.
        Do NOT use irrLicht's GUI focus facilities; it's too limited for our
        needs, so we use ours. (i.e. always call these functions are never those
        in IGUIEnvironment) */
    Widget* getFocusForPlayer(const int playerID);

    /** Focuses nothing for given player (removes any selection for this player).
     Do NOT use irrLicht's GUI focus facilities; it's too limited for our
     needs, so we use ours. (i.e. always call these functions are never those
     in IGUIEnvironment) */
    void focusNothingForPlayer(const int playerID);

    /** Returns whether given the widget is currently focused by given player.
     Do NOT use irrLicht's GUI focus facilities; it's too limited for our
     needs, so we use ours. (i.e. always call these functions are never those
     in IGUIEnvironment) */
    bool isFocusedForPlayer(const Widget*w, const int playerID);
    
    // In an attempt to make getters as fast as possible by possibly allowing inlining
    // These fields should never be accessed outside of the GUI engine.
    namespace Private
    {
        extern irr::gui::IGUIEnvironment* g_env;
        extern Skin* g_skin;
        extern irr::gui::IGUIFont* g_small_font;
        extern irr::gui::IGUIFont* g_font;
        extern irr::gui::IGUIFont* g_title_font;

        extern IrrlichtDevice* g_device;
        extern irr::video::IVideoDriver* g_driver;
        extern Screen* g_current_screen;
        extern AbstractStateManager* g_state_manager;
        extern Widget* g_focus_for_player[MAX_PLAYER_COUNT];
    }
    
    inline IrrlichtDevice*            getDevice()        { return Private::g_device;         }
    inline irr::gui::IGUIEnvironment* getGUIEnv()        { return Private::g_env;            }
    inline irr::video::IVideoDriver*  getDriver()        { return Private::g_driver;         }
    inline irr::gui::IGUIFont*        getSmallFont()     { return Private::g_small_font;     }
    inline irr::gui::IGUIFont*        getFont()          { return Private::g_font;           }
    inline irr::gui::IGUIFont*        getTitleFont()     { return Private::g_title_font;     }
    inline Screen*                    getCurrentScreen() { return Private::g_current_screen; }
    inline AbstractStateManager*      getStateManager()  { return Private::g_state_manager;  }
    inline Skin*                      getSkin()          { return Private::g_skin;           }

    /** Returns the height of the font in pixels */
    int   getFontHeight();
    
    int   getSmallFontHeight();
    
    float getLatestDt();
    
    /** show a warning message to explain to the player that only the game master cn act at this point */
    void  showMasterOnlyString();
    
    /** Add a cutscene to the list of screens known by the gui engine */
    void  addScreenToList(Screen* screen);
    
    // Widgets that need to be notified at every frame can add themselves there
    extern ptr_vector<Widget, REF> needsUpdate;
    
    void init(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, AbstractStateManager* state_manager);
    void cleanUp();
    
    /** Low-level mean to change current screen. Use a state manager instead to get higher-level functionnality. */
    void switchToScreen(const char* );
    void clear();
    void cleanForGame();
    
    void reshowCurrentScreen();
    
    void render(float dt);
    void renderLoading();
    
    void transmitEvent(Widget* widget, std::string& name, const int playerID);
    
    Widget* getWidget(const char* name);
    Widget* getWidget(const int id);
}

#endif
