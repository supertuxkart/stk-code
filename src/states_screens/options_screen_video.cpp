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

#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_players.hpp"
#include "states_screens/options_screen_video.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"


#include <iostream>
#include <sstream>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenVideo );

// -----------------------------------------------------------------------------

OptionsScreenVideo::OptionsScreenVideo() : Screen("options_video.stkgui")
{
    m_inited = false;
}

// -----------------------------------------------------------------------------

void OptionsScreenVideo::loadedFromFile()
{
    m_inited = false;
}

// -----------------------------------------------------------------------------

void OptionsScreenVideo::init()
{
    RibbonWidget* ribbon = this->getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "tab_video", PLAYER_ID_GAME_MASTER );
    
    // ---- video modes
    DynamicRibbonWidget* res = this->getWidget<DynamicRibbonWidget>("resolutions");
    assert( res != NULL );
    
    
    CheckBoxWidget* full = this->getWidget<CheckBoxWidget>("fullscreen");
    assert( full != NULL );
    full->setState( UserConfigParams::m_fullscreen );
    
    
    // --- get resolution list from irrlicht the first time
    if (!m_inited)
    {
        const std::vector<VideoMode>& modes = irr_driver->getVideoModes();
        const int amount = modes.size();
        for(int n=0; n<amount; n++)
        {
            const int w = modes[n].width;
            const int h = modes[n].height;
            const float ratio = (float)w / h;
            
            char name[32];
            sprintf( name, "%ix%i", w, h );
            
#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)
            
            if      (ABOUT_EQUAL( ratio, (5.0f/4.0f) ))   res->addItem(name, name, "/gui/screen54.png");
            else if (ABOUT_EQUAL( ratio, (4.0f/3.0f) ))   res->addItem(name, name, "/gui/screen43.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/10.0f)))  res->addItem(name, name, "/gui/screen1610.png");
            else if (ABOUT_EQUAL( ratio, (5.0f/3.0f) ))   res->addItem(name, name, "/gui/screen53.png");
            else if (ABOUT_EQUAL( ratio, (3.0f/2.0f) ))   res->addItem(name, name, "/gui/screen32.png");
            else
            {
                std::cout << "Unknown screen size ratio : " << ratio << std::endl;
                // FIXME - do something better than showing a random icon
                res->addItem(name,name, file_manager->getDataDir() + "/gui/screen1610.png");
            }
#undef ABOUT_EQUAL
        } // next resolution
        
    } // end if not inited
    
    res->updateItemDisplay();
    
    // ---- select current resolution every time
    const std::vector<VideoMode>& modes = irr_driver->getVideoModes();
    const int amount = modes.size();
    for(int n=0; n<amount; n++)
    {
        const int w = modes[n].width;
        const int h = modes[n].height;
        
        char name[32];
        sprintf( name, "%ix%i", w, h );
        
        if(w == UserConfigParams::m_width && h == UserConfigParams::m_height)
        {
            //std::cout << "************* Detected right resolution!!! " << n << "\n";
            // that's the current one
            res->setSelection(n, PLAYER_ID_GAME_MASTER, false);
            break;
        }
    }  // end for
    
}

// -----------------------------------------------------------------------------

void OptionsScreenVideo::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        
        if (selection == "tab_audio") StateManager::get()->replaceTopMostScreen(OptionsScreenAudio::getInstance());
        else if (selection == "tab_video") StateManager::get()->replaceTopMostScreen(OptionsScreenVideo::getInstance());
        else if (selection == "tab_players") StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        else if (selection == "tab_controls") StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "apply_resolution")
    {
        using namespace GUIEngine;
        
        DynamicRibbonWidget* w1 = this->getWidget<DynamicRibbonWidget>("resolutions");
        assert(w1 != NULL);
        
        const std::string& res = w1->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        int w = -1, h = -1;
        if (sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1)
        {
            std::cerr << "Failed to decode resolution : " << res.c_str() << std::endl;
            return;
        }
        
        CheckBoxWidget* w2 = this->getWidget<CheckBoxWidget>("fullscreen");
        assert(w2 != NULL);
        

        irr_driver->changeResolution(w, h, w2->getState());
    }
    
}

// -----------------------------------------------------------------------------

void OptionsScreenVideo::tearDown()
{
}

// -----------------------------------------------------------------------------

void OptionsScreenVideo::unloaded()
{
    m_inited = false;
}

// -----------------------------------------------------------------------------

