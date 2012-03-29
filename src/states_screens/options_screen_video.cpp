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
#include "states_screens/dialogs/custom_video_settings.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_players.hpp"
#include "states_screens/options_screen_ui.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( OptionsScreenVideo );

// Look-up table for GFX levels
const bool GFX           [] = {false, true,  true,  true,  true,  true,  true};
const int  GFX_ANIM_KARTS[] = {0,     0,     1,     2,     2,     2,     2};
const bool GFX_WEATHER   [] = {false, false, false, false, true,  true,  true};
const bool GFX_ANTIALIAS [] = {false, false, false, false, false, true,  true};
const bool GFX_POSTPROCESSING[] = 
                              {false, false, false, false, false, false, true};
const bool GFX_PIXEL_SHADERS[] =
                              {false, false, false, false, true,  true,  true};
const int  GFX_LEVEL_AMOUNT = 7;

// ----------------------------------------------------------------------------

OptionsScreenVideo::OptionsScreenVideo() : Screen("options_video.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// ----------------------------------------------------------------------------

void OptionsScreenVideo::loadedFromFile()
{
    m_inited = false;
    

    GUIEngine::SpinnerWidget* gfx = 
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    gfx->m_properties[GUIEngine::PROP_MAX_VALUE] = 
        StringUtils::toString(GFX_LEVEL_AMOUNT);
    
}   // loadedFromFile

// ----------------------------------------------------------------------------

void OptionsScreenVideo::init()
{
    Screen::init();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    if (ribbon != NULL)  ribbon->select( "tab_video", PLAYER_ID_GAME_MASTER );
    
    ribbon->getRibbonChildren()[1].setTooltip( _("Audio") );
    ribbon->getRibbonChildren()[2].setTooltip( _("User Interface") );
    ribbon->getRibbonChildren()[3].setTooltip( _("Players") );
    ribbon->getRibbonChildren()[4].setTooltip( _("Controls") );
    
    GUIEngine::ButtonWidget* applyBtn = 
        getWidget<GUIEngine::ButtonWidget>("apply_resolution");
    assert( applyBtn != NULL );

    GUIEngine::SpinnerWidget* gfx = 
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );
    
    GUIEngine::CheckBoxWidget* vsync =
        getWidget<GUIEngine::CheckBoxWidget>("vsync");
    assert( vsync != NULL );
    vsync->setState( UserConfigParams::m_vsync );
    
    GUIEngine::CheckBoxWidget* fbos = 
        getWidget<GUIEngine::CheckBoxWidget>("fbos");
    assert( fbos != NULL );
    fbos->setState( UserConfigParams::m_fbo );

    
    // ---- video modes
    DynamicRibbonWidget* res = getWidget<DynamicRibbonWidget>("resolutions");
    assert( res != NULL );
    
    
    CheckBoxWidget* full = getWidget<CheckBoxWidget>("fullscreen");
    assert( full != NULL );
    full->setState( UserConfigParams::m_fullscreen );
    
    CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");
    rememberWinpos->setState(UserConfigParams::m_remember_window_location);

    if (UserConfigParams::m_fullscreen) rememberWinpos->setDeactivated();
    else rememberWinpos->setActivated();
    
    // Enable back widgets if they were visited in-game previously
    if (StateManager::get()->getGameState() != GUIEngine::INGAME_MENU)
    {
        res->setActivated();
        full->setActivated();
        applyBtn->setActivated();
        gfx->setActivated();
        getWidget<ButtonWidget>("custom")->setActivated();
    }
    
    // --- get resolution list from irrlicht the first time
    if (!m_inited)
    {
        res->clearItems();
        
        const std::vector<IrrDriver::VideoMode>& modes = 
                                                irr_driver->getVideoModes();
        const int amount = modes.size();
        
        bool found_config_res = false;
        
        // for some odd reason, irrlicht sometimes fails to report the good 
        // old standard resolutions
        // those are always useful for windowed mode
        bool found_800_600 = false;
        bool found_1024_640 = false;
        bool found_1024_768 = false;
        
        for (int n=0; n<amount; n++)
        {
            const int w = modes[n].getWidth();
            const int h = modes[n].getHeight();
            const float ratio = (float)w / h;
            
            if (w == UserConfigParams::m_width && 
                h == UserConfigParams::m_height)
            {
                found_config_res = true;
            }
            
            if (w == 800 && h == 600)
            {
                found_800_600 = true;
            }
            else if (w == 1024 && h == 640)
            {
                found_1024_640 = true;
            }
            else if (w == 1024 && h == 768)
            {
                found_1024_768 = true;
            }
            
            char name[32];
            sprintf( name, "%ix%i", w, h );
            
            core::stringw label;
            label += w;
            label += L"\u00D7";
            label += h;
            
#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)
            
            if      (ABOUT_EQUAL( ratio, (5.0f/4.0f) ))
                res->addItem(label, name, "/gui/screen54.png");
            else if (ABOUT_EQUAL( ratio, (4.0f/3.0f) ))
                res->addItem(label, name, "/gui/screen43.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/10.0f)))
                res->addItem(label, name, "/gui/screen1610.png");
            else if (ABOUT_EQUAL( ratio, (5.0f/3.0f) ))
                res->addItem(label, name, "/gui/screen53.png");
            else if (ABOUT_EQUAL( ratio, (3.0f/2.0f) ))   
                res->addItem(label, name, "/gui/screen32.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/9.0f) ))
                res->addItem(label, name, "/gui/screen169.png");
            else 
                res->addItem(label, name, "/gui/screen_other.png");
#undef ABOUT_EQUAL
        } // next resolution
        
        if (!found_config_res)
        {
            const int w = UserConfigParams::m_width;
            const int h = UserConfigParams::m_height;
            const float ratio = (float)w / h;
            
            if (w == 800 && h == 600)
            {
                found_800_600 = true;
            }
            else if (w == 1024 && h == 640)
            {
                found_1024_640 = true;
            }
            else if (w == 1024 && h == 768)
            {
                found_1024_768 = true;
            }
            
            char name[32];
            sprintf( name, "%ix%i", w, h );
            
            core::stringw label;
            label += w;
            label += L"\u00D7";
            label += h;
            
#define ABOUT_EQUAL(a , b) (fabsf( a - b ) < 0.01)
            
            if      (ABOUT_EQUAL( ratio, (5.0f/4.0f)   ))
                res->addItem(label, name, "/gui/screen54.png");
            else if (ABOUT_EQUAL( ratio, (4.0f/3.0f)   ))
                res->addItem(label, name, "/gui/screen43.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/10.0f) ))
                res->addItem(label, name, "/gui/screen1610.png");
            else if (ABOUT_EQUAL( ratio, (5.0f/3.0f)   ))
                res->addItem(label, name, "/gui/screen53.png");
            else if (ABOUT_EQUAL( ratio, (3.0f/2.0f)   ))
                res->addItem(label, name, "/gui/screen32.png");
            else if (ABOUT_EQUAL( ratio, (16.0f/9.0f)   ))
                res->addItem(label, name, "/gui/screen169.png");
            else
                res->addItem(label, name, "/gui/screen_other.png");
#undef ABOUT_EQUAL
        }
        
        if (!found_800_600)
        {
            res->addItem(L"800\u00D7600", "800x600", "/gui/screen43.png");
        }
        if (!found_1024_640)
        {
            res->addItem(L"1024\u00D7640", "1024x640", "/gui/screen1610.png");
        }
        if (!found_1024_768)
        {
            res->addItem(L"1024\u00D7768", "1024x768", "/gui/screen43.png");
        }
        
    } // end if not inited
    
    res->updateItemDisplay();

    // ---- select current resolution every time
    char searching_for[32];
    snprintf(searching_for, 32, "%ix%i", (int)UserConfigParams::m_width, 
                                         (int)UserConfigParams::m_height);
    

    if (res->setSelection(searching_for, PLAYER_ID_GAME_MASTER, 
                          false /* focus it */, true /* even if deactivated*/))
    {
        // ok found
    }
    else
    {
        std::cerr << "[OptionsScreenVideo] Cannot find resolution '" 
                  << searching_for << "'\n";
    }

    
    // --- set gfx settings values
    updateGfxSlider();
    
    // ---- forbid changing resolution or animation settings from in-game
    // (we need to disable them last because some items can't be edited when 
    // disabled)
    if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
    {
        res->setDeactivated();
        full->setDeactivated();
        applyBtn->setDeactivated();
        gfx->setDeactivated();
        getWidget<ButtonWidget>("custom")->setDeactivated();
    }
    
    updateTooltip();
}   // init

// ----------------------------------------------------------------------------

void OptionsScreenVideo::updateGfxSlider()
{
    GUIEngine::SpinnerWidget* gfx = 
    getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );
    
    bool found = false;
    for (int l=0; l<GFX_LEVEL_AMOUNT; l++)
    {
        if (UserConfigParams::m_show_steering_animations == GFX_ANIM_KARTS[l]&&
            UserConfigParams::m_graphical_effects        == GFX[l] &&
            UserConfigParams::m_weather_effects          == GFX_WEATHER[l] &&
            UserConfigParams::m_fullscreen_antialiasing  == GFX_ANTIALIAS[l] &&
            UserConfigParams::m_postprocess_enabled      == GFX_POSTPROCESSING[l] &&
            UserConfigParams::m_pixel_shaders            == GFX_PIXEL_SHADERS[l])
        {
            gfx->setValue(l+1);
            found = true;
            break;
        }
    }
    
    if (!found)
    {
        //I18N: custom video settings
        gfx->setCustomText( _("Custom") );
    }
    
}

// ----------------------------------------------------------------------------

void OptionsScreenVideo::updateTooltip()
{
    GUIEngine::SpinnerWidget* gfx = 
        getWidget<GUIEngine::SpinnerWidget>("gfx_level");
    assert( gfx != NULL );
    
    core::stringw tooltip;
    
    //I18N: in the graphical options tooltip; 
    // indicates a graphical feature is enabled
    core::stringw enabled = _LTR("Enabled");
    //I18N: in the graphical options tooltip; 
    // indicates a graphical feature is disabled
    core::stringw disabled = _LTR("Disabled");
    //I18N: if all kart animations are enabled
    core::stringw all = _LTR("All");
    //I18N: if some kart animations are enabled
    core::stringw me = _LTR("Me Only");
    //I18N: if no kart animations are enabled
    core::stringw none = _LTR("None");
    
    //I18N: in graphical options
    tooltip = _("Animated Scenery : %s", 
        UserConfigParams::m_graphical_effects ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Weather Effects : %s", 
        UserConfigParams::m_weather_effects ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Animated Characters : %s",
        UserConfigParams::m_show_steering_animations == 2 
        ? all 
        : (UserConfigParams::m_show_steering_animations == 1 ? me : none));
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Anti-aliasing (requires restart) : %s", 
             UserConfigParams::m_fullscreen_antialiasing ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Pixel shaders : %s", 
                                  UserConfigParams::m_pixel_shaders ? enabled : disabled);
    //I18N: in graphical options
    tooltip = tooltip + L"\n" + _("Post-processing (motion blur) : %s", 
        UserConfigParams::m_postprocess_enabled ? enabled : disabled);
    gfx->setTooltip(tooltip);
}   // updateTooltip

// ----------------------------------------------------------------------------

void OptionsScreenVideo::eventCallback(Widget* widget, const std::string& name,
                                       const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = 
            ((RibbonWidget*)widget)
                                 ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        Screen *screen = NULL;
        if (selection == "tab_audio") 
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = OptionsScreenPlayers::getInstance();
        else if (selection == "tab_controls") 
            screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui") 
            screen = OptionsScreenUI::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if(name == "custom")
    {
        new CustomVideoSettingsialog(0.8f, 0.8f);
    }
    else if(name == "apply_resolution")
    {
        using namespace GUIEngine;
        
        DynamicRibbonWidget* w1=getWidget<DynamicRibbonWidget>("resolutions");
        assert(w1 != NULL);
        
        const std::string& res = 
            w1->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        int w = -1, h = -1;
        if (sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1)
        {
            std::cerr << "Failed to decode resolution : " << res.c_str() 
                      << std::endl;
            return;
        }
        
        CheckBoxWidget* w2 = getWidget<CheckBoxWidget>("fullscreen");
        assert(w2 != NULL);
        

        irr_driver->changeResolution(w, h, w2->getState());
    }
    else if (name == "gfx_level")
    {
        GUIEngine::SpinnerWidget* gfx_level = 
            getWidget<GUIEngine::SpinnerWidget>("gfx_level");
        assert( gfx_level != NULL );
        
        const int level = gfx_level->getValue();
        
        UserConfigParams::m_show_steering_animations = GFX_ANIM_KARTS[level-1];
        UserConfigParams::m_graphical_effects        = GFX[level-1];
        UserConfigParams::m_weather_effects          = GFX_WEATHER[level-1];
        UserConfigParams::m_fullscreen_antialiasing  = GFX_ANTIALIAS[level-1];
        UserConfigParams::m_postprocess_enabled      = GFX_POSTPROCESSING[level-1];
        UserConfigParams::m_pixel_shaders            = GFX_PIXEL_SHADERS[level-1];
        
        updateTooltip();
    }
    else if (name == "vsync")
    {
        GUIEngine::CheckBoxWidget* vsync = 
            getWidget<GUIEngine::CheckBoxWidget>("vsync");
        assert( vsync != NULL );
        UserConfigParams::m_vsync = vsync->getState();
    }
    else if (name == "fbos")
    {
        GUIEngine::CheckBoxWidget* fbos = 
            getWidget<GUIEngine::CheckBoxWidget>("fbos");
        assert( fbos != NULL );
        UserConfigParams::m_fbo = fbos->getState();
    }
    else if (name == "rememberWinpos")
    {
        CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");
        UserConfigParams::m_remember_window_location = rememberWinpos->getState();
    }
    else if (name == "fullscreen")
    {
        CheckBoxWidget* fullscreen = getWidget<CheckBoxWidget>("fullscreen");
        CheckBoxWidget* rememberWinpos = getWidget<CheckBoxWidget>("rememberWinpos");
        
        if (fullscreen->getState()) rememberWinpos->setDeactivated();
        else rememberWinpos->setActivated();
    }
}   // eventCallback

// ----------------------------------------------------------------------------

void OptionsScreenVideo::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// ----------------------------------------------------------------------------

void OptionsScreenVideo::unloaded()
{
    m_inited = false;
}   // unloaded

// ----------------------------------------------------------------------------

