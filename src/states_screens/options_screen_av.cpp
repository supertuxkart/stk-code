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

#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_av.hpp"
#include "states_screens/options_screen_players.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"


#include <iostream>
#include <sstream>

using namespace GUIEngine;

OptionsScreenAV::OptionsScreenAV() : Screen("options_av.stkgui")
{
}
// -----------------------------------------------------------------------------
void OptionsScreenAV::init()
{
    RibbonWidget* ribbon = this->getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "audio_video", GUI_PLAYER_ID );
    
    // ---- sfx volume
    SpinnerWidget* gauge = this->getWidget<SpinnerWidget>("sfx_volume");
    assert(gauge != NULL);
    
    gauge->setValue( (int)(sfx_manager->getMasterSFXVolume()*10.0f) );
    
    
    gauge = this->getWidget<SpinnerWidget>("music_volume");
    assert(gauge != NULL);
    gauge->setValue( (int)(sound_manager->getMasterMusicVolume()*10.f) );
    
    // ---- music volume
    CheckBoxWidget* sfx = this->getWidget<CheckBoxWidget>("sfx_enabled");
    
    CheckBoxWidget* music = this->getWidget<CheckBoxWidget>("music_enabled");
    
    // ---- audio enables/disables
    sfx->setState( UserConfigParams::m_sfx );
    music->setState( UserConfigParams::m_music );
    
    // ---- video modes
    {
        DynamicRibbonWidget* res = this->getWidget<DynamicRibbonWidget>("resolutions");
        assert( res != NULL );
        
        
        CheckBoxWidget* full = this->getWidget<CheckBoxWidget>("fullscreen");
        assert( full != NULL );
        full->setState( UserConfigParams::m_fullscreen );
        
        // --- get resolution list from irrlicht the first time
        if(!this->m_inited)
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
                
                if( ABOUT_EQUAL( ratio, (5.0f/4.0f) ) )
                    res->addItem(name, name, file_manager->getDataDir() + "/gui/screen54.png");
                else if( ABOUT_EQUAL( ratio, (4.0f/3.0f) ) )
                    res->addItem(name, name, file_manager->getDataDir() + "/gui/screen43.png");
                else if( ABOUT_EQUAL( ratio, (16.0f/10.0f) ) )
                    res->addItem(name, name, file_manager->getDataDir() + "/gui/screen1610.png");
                else if( ABOUT_EQUAL( ratio, (5.0f/3.0f) ) )
                    res->addItem(name, name, file_manager->getDataDir() + "/gui/screen53.png");
                else if( ABOUT_EQUAL( ratio, (3.0f/2.0f) ) )
                    res->addItem(name, name, file_manager->getDataDir() + "/gui/screen32.png");
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
                const int playerID = 0; // FIXME: don't hardcode player 0 ?
                res->setSelection(n, playerID, false);
                break;
            }
        }  // end for
        
    }
}

// -----------------------------------------------------------------------------
void OptionsScreenAV::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(GUI_PLAYER_ID).c_str();
        
        if (selection == "audio_video") StateManager::get()->replaceTopMostScreen(OptionsScreenAV::getInstance());
        else if (selection == "players") StateManager::get()->replaceTopMostScreen(OptionsScreenPlayers::getInstance());
        else if (selection == "controls") StateManager::get()->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "music_volume")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        assert(w != NULL);
        
        sound_manager->setMasterMusicVolume( w->getValue()/10.0f );
    }
    else if(name == "sfx_volume")
    {
        static SFXBase* sample_sound = NULL;
        
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        assert(w != NULL);
        
        if(sample_sound == NULL)
            sample_sound = sfx_manager->newSFX( SFXManager::SOUND_SKID );
        sample_sound->volume(1);
        
        sfx_manager->setMasterSFXVolume( w->getValue()/10.0f );
        UserConfigParams::m_sfx_volume = w->getValue()/10.0f;
        
        // play a sample sound to show the user what this volume is like
        sample_sound->position ( Vec3(0,0,0) );
        
        if(sample_sound->getStatus() != SFXManager::SFX_PLAYING)
        {
            sample_sound->play();
        }
        
    }
    else if(name == "music_enabled")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);
        
        UserConfigParams::m_music = w->getState();
        std::cout << "music state is now " << (bool)UserConfigParams::m_music << std::endl;
        
        if(w->getState() == false)
            sound_manager->stopMusic();
        else
            sound_manager->startMusic(sound_manager->getCurrentMusic());
    }
    else if(name == "sfx_enabled")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);
        
        UserConfigParams::m_sfx = w->getState();
    }
    else if(name == "apply_resolution")
    {
        using namespace GUIEngine;
        
        UserConfigParams::m_prev_width = UserConfigParams::m_width;
        UserConfigParams::m_prev_height = UserConfigParams::m_height;
        
        DynamicRibbonWidget* w1 = this->getWidget<DynamicRibbonWidget>("resolutions");
        assert(w1 != NULL);
        
        const std::string& res = w1->getSelectionIDString(GUI_PLAYER_ID);
        
        int w = -1, h = -1;
        if( sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1 )
        {
            std::cerr << "Failed to decode resolution : " << res.c_str() << std::endl;
            return;
        }
        
        CheckBoxWidget* w2 = this->getWidget<CheckBoxWidget>("fullscreen");
        assert(w2 != NULL);
        
        UserConfigParams::m_width = w;
        UserConfigParams::m_height = h;
        UserConfigParams::m_fullscreen = w2->getState();
        irr_driver->changeResolution();
    }
    
}

// -----------------------------------------------------------------------------
void OptionsScreenAV::tearDown()
{
}

