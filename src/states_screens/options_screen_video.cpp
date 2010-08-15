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

#include "states_screens/options_screen_video.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_players.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenVideo );

// -----------------------------------------------------------------------------

OptionsScreenVideo::OptionsScreenVideo() : Screen("options_video.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// -----------------------------------------------------------------------------

void OptionsScreenVideo::loadedFromFile()
{
    m_inited = false;
    
    GUIEngine::SpinnerWidget* skinSelector = this->getWidget<GUIEngine::SpinnerWidget>("skinchoice");
    assert( skinSelector != NULL );
    
    skinSelector->m_properties[PROP_WARP_AROUND] = "true";
    
    m_skins.clear();
    skinSelector->clearLabels();
    
    std::set<std::string> skinFiles;
    file_manager->listFiles(skinFiles /* out */, file_manager->getGUIDir() + "/skins",
                            true /* is full path */, true /* make full path */ );
    
    for (std::set<std::string>::iterator it = skinFiles.begin(); it != skinFiles.end(); it++)
    {
        if ( (*it).find(".stkskin") != std::string::npos )
        {
            m_skins.push_back( *it );
        }
    }
    
    if (m_skins.size() == 0)
    {
        std::cerr << "WARNING: could not find a single skin, make sure that "
                     "the data files are correctly installed\n";
        skinSelector->setDeactivated();
        return;
    }
        
    const int skinCount = m_skins.size();
    for (int n=0; n<skinCount; n++)
    {
        const std::string skinFileName = StringUtils::getBasename(m_skins[n]);
        const std::string skinName = StringUtils::removeExtension( skinFileName );
        skinSelector->addLabel( core::stringw(skinName.c_str()) );
    }
    skinSelector->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    skinSelector->m_properties[GUIEngine::PROP_MAX_VALUE] = StringUtils::toString(skinCount-1);

}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenVideo::init()
{
    Screen::init();
    RibbonWidget* ribbon = this->getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "tab_video", PLAYER_ID_GAME_MASTER );
    
    GUIEngine::SpinnerWidget* skinSelector = this->getWidget<GUIEngine::SpinnerWidget>("skinchoice");
    assert( skinSelector != NULL );
    
    GUIEngine::ButtonWidget* applyBtn = this->getWidget<GUIEngine::ButtonWidget>("apply_resolution");
    assert( applyBtn != NULL );
    
    // ---- video modes
    DynamicRibbonWidget* res = this->getWidget<DynamicRibbonWidget>("resolutions");
    assert( res != NULL );
    
    
    CheckBoxWidget* full = this->getWidget<CheckBoxWidget>("fullscreen");
    assert( full != NULL );
    full->setState( UserConfigParams::m_fullscreen );
    
    
    // --- get resolution list from irrlicht the first time
    if (!m_inited)
    {
        res->clearItems();
        
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
        
    // forbid changing resolution from in-game
    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        res->setDeactivated();
        full->setDeactivated();
        applyBtn->setDeactivated();
    }
    else
    {
        res->setActivated();
        full->setActivated();
        applyBtn->setActivated();
    }
    
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
    
    // --- select the right skin in the spinner
    
    bool currSkinFound = false;
    const int skinCount = m_skins.size();
    for (int n=0; n<skinCount; n++)
    {
        const std::string skinFileName = StringUtils::getBasename(m_skins[n]);
        
        if (UserConfigParams::m_skin_file.c_str() == skinFileName)
        {
            skinSelector->setValue(n);
            currSkinFound = true;
            break;
        }
    }
    if (!currSkinFound)
    {
        std::cerr << "WARNING: couldn't find current skin in the list of skins!!\n";
        skinSelector->setValue(0);
        GUIEngine::reloadSkin();
    }
    
}   // init

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
    else if (name == "skinchoice")
    {
        GUIEngine::SpinnerWidget* skinSelector = this->getWidget<GUIEngine::SpinnerWidget>("skinchoice");
        assert( skinSelector != NULL );
        
        const core::stringw selectedSkin = skinSelector->getStringValue();
        UserConfigParams::m_skin_file = core::stringc(selectedSkin.c_str()).c_str() + std::string(".stkskin");
        GUIEngine::reloadSkin();
    }
    
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenVideo::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenVideo::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------

