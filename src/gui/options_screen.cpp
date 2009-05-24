#include "gui/options_screen.hpp"
#include "gui/engine.hpp"
#include "gui/widget.hpp"
#include "gui/screen.hpp"
#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "gui/state_manager.hpp"
#include <iostream>

using namespace GUIEngine;

/**
 * Callback handling events from the options menus
 */
namespace StateManager
{
    void eventInput(Widget* widget, const std::string& name);
    
    
    // -----------------------------------------------------------------------------    
    void initAudioVideo(Widget* widget, const std::string& name)
    {
        // ---- sfx volume
        SpinnerWidget* gauge = getCurrentScreen()->getWidget<SpinnerWidget>("sfx_volume");
        assert(gauge != NULL);
        
        gauge->setValue( (int)(sfx_manager->getMasterSFXVolume()*10.0f) );
        
        
        gauge = getCurrentScreen()->getWidget<SpinnerWidget>("music_volume");
        assert(gauge != NULL);
        gauge->setValue( (int)(sound_manager->getMasterMusicVolume()*10.f) );
        
        // ---- music volume
        CheckBoxWidget* sfx = getCurrentScreen()->getWidget<CheckBoxWidget>("sfx_enabled");
        
        CheckBoxWidget* music = getCurrentScreen()->getWidget<CheckBoxWidget>("music_enabled");
        
        // ---- audio enables/disables
        sfx->setState( user_config->doSFX() );
        music->setState( user_config->doMusic() );
        
        // ---- video modes
        {
            RibbonGridWidget* res = getCurrentScreen()->getWidget<RibbonGridWidget>("resolutions");
            assert( res != NULL );
            
            
            CheckBoxWidget* full = getCurrentScreen()->getWidget<CheckBoxWidget>("fullscreen");
            assert( full != NULL );
            full->setState( user_config->m_fullscreen );
            
            // --- get resolution list from irrlicht the first time
            if(!getCurrentScreen()->m_inited)
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
                        res->addItem(name,name,"gui/screen54.png");
                    else if( ABOUT_EQUAL( ratio, (4.0f/3.0f) ) )
                        res->addItem(name,name,"gui/screen43.png");
                    else if( ABOUT_EQUAL( ratio, (16.0f/10.0f) ) )
                        res->addItem(name,name,"gui/screen1610.png");
                    else if( ABOUT_EQUAL( ratio, (5.0f/3.0f) ) )
                        res->addItem(name,name,"gui/screen53.png");
                    else if( ABOUT_EQUAL( ratio, (3.0f/2.0f) ) )
                        res->addItem(name,name,"gui/screen32.png");
                    else
                    {
                        std::cout << "Unknown screen size ratio : " << ratio << std::endl;
                        // FIXME - do something better than showing a random icon
                        res->addItem(name,name,"gui/screen1610.png");
                    }
#undef ABOUT_EQUAL
                } // next resolution
                
            } // end if not inited
            
            res->updateItemDisplay();
            
            // ---- select curernt resolution every time
            const std::vector<VideoMode>& modes = irr_driver->getVideoModes();
            const int amount = modes.size();
            for(int n=0; n<amount; n++)
            {
                const int w = modes[n].width;
                const int h = modes[n].height;
                
                char name[32];
                sprintf( name, "%ix%i", w, h );
                
                if(w == user_config->m_width && h == user_config->m_height)
                {
                    //std::cout << "************* Detected right resolution!!! " << n << "\n";
                    // that's the current one
                    res->setSelection(n);
                    break;
                }
            }  // end for
            
        }
    }
    
    // -----------------------------------------------------------------------------    
    void eventAudioVideo(Widget* widget, const std::string& name)
    {
        if(name == "music_volume")
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
            user_config->m_sfx_volume = w->getValue()/10.0f;
            
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
            
            user_config->setMusic(w->getState() ? UserConfig::UC_ENABLE : UserConfig::UC_DISABLE);
            
            if(w->getState() == false)
                sound_manager->stopMusic();
            else
                sound_manager->startMusic(sound_manager->getCurrentMusic());
        }
        else if(name == "sfx_enabled")
        {
            CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);
            
            user_config->setSFX(w->getState() ? UserConfig::UC_ENABLE : UserConfig::UC_DISABLE);
        }
        else if(name == "apply_resolution")
        {
            using namespace GUIEngine;
            
            user_config->m_prev_width = user_config->m_width;
            user_config->m_prev_height = user_config->m_height;
            
            RibbonGridWidget* w1 = getCurrentScreen()->getWidget<RibbonGridWidget>("resolutions");
            assert(w1 != NULL);
            
            const std::string& res = w1->getSelectionName();
            
            int w = -1, h = -1;
            if( sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1 )
            {
                std::cerr << "Failed to decode resolution : " << res.c_str() << std::endl;
                return;
            }
            
            CheckBoxWidget* w2 = getCurrentScreen()->getWidget<CheckBoxWidget>("fullscreen");
            assert(w2 != NULL);
            
            user_config->m_width = w;
            user_config->m_height = h;
            user_config->m_fullscreen = w2->getState();
            irr_driver->changeResolution();
        }
        
    }
    
    // -----------------------------------------------------------------------------    
    void updateInputButtons(const InputDevice* device)
    {
        
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_up");
            btn->setLabel( device->getBindingAsString(PA_ACCEL).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_down");
            btn->setLabel( device->getBindingAsString(PA_BRAKE).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_left");
            btn->setLabel( device->getBindingAsString(PA_LEFT).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_right");
            btn->setLabel( device->getBindingAsString(PA_RIGHT).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_fire");
            btn->setLabel( device->getBindingAsString(PA_FIRE).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_nitro");
            btn->setLabel( device->getBindingAsString(PA_NITRO).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_drift");
            btn->setLabel( device->getBindingAsString(PA_DRIFT).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_rescue");
            btn->setLabel( device->getBindingAsString(PA_RESCUE).c_str() );
        }
        {
            ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>("binding_look_back");
            btn->setLabel( device->getBindingAsString(PA_LOOK_BACK).c_str() );
        }

    }
    
    // -----------------------------------------------------------------------------    
    void initInput(Widget* widget, const std::string& name)
    {
        {
            RibbonGridWidget* devices = getCurrentScreen()->getWidget<RibbonGridWidget>("devices");
            assert( devices != NULL );
            
            if(!getCurrentScreen()->m_inited)
            {
                devices->addItem("Keyboard","keyboard","gui/keyboard.png");
                
                const int gamepad_count = input_manager->getDeviceList()->getGamePadAmount();
                
                for(int i=0; i<gamepad_count; i++)
                {
                    std::string name = input_manager->getDeviceList()->getGamePad(i)->m_name;
                    char internal_name[32];
                    sprintf(internal_name, "gamepad%i", i);
                    devices->addItem(name,internal_name,"gui/gamepad.png");
                }
                
            }
            devices->updateItemDisplay();
            
            // trigger displaying bindings for default selected device
            const std::string name("devices");
            eventInput(devices, name);
        }
    }
    
    // -----------------------------------------------------------------------------    
    void eventInput(Widget* widget, const std::string& name)
    {
        if(name == "devices")
        {
            RibbonGridWidget* devices = dynamic_cast<RibbonGridWidget*>(widget);

            const std::string& selection = devices->getSelectionName();
            if( selection.find("gamepad") != std::string::npos )
            {
                int i = -1, read = 0;
                read = sscanf( selection.c_str(), "gamepad%i", &i );
                if(read == 1 && i != -1)
                {
                    updateInputButtons( input_manager->getDeviceList()->getGamePad(i) );
                }
                else
                {
                    std::cerr << "Cannot read internal input device ID : " << selection.c_str() << std::endl;
                }
            }
            else if(selection == "keyboard")
            {
                updateInputButtons( input_manager->getDeviceList()->getKeyboard(0) );
            }
            else
            {
                std::cerr << "Cannot read internal input device ID : " << selection.c_str() << std::endl;
            }
        }
    }
    
    // -----------------------------------------------------------------------------
    // main call (from StateManager); dispatches the call to a specialissed function as needed
    void menuEventOptions(Widget* widget, const std::string& name)
    {
        const std::string& screen_name = getCurrentScreen()->getName();
        
        if(name == "init")
        {
            const std::string& screen_name = getCurrentScreen()->getName();
            
            RibbonWidget* ribbon = getCurrentScreen()->getWidget<RibbonWidget>("options_choice");
            if(ribbon != NULL)
            {
                if(screen_name == "options_av.stkgui") ribbon->select( "audio_video" );
                else if(screen_name == "options_players.stkgui") ribbon->select( "players" );
                else if(screen_name == "options_input.stkgui") ribbon->select( "controls" );
            }
            
            if(screen_name == "options_av.stkgui") initAudioVideo(widget, name);
            else if(screen_name == "options_input.stkgui") initInput(widget, name);
            
            //getCurrentScreen()->m_inited;
        }
        else if(name == "options_choice")
        {
            std::string selection = ((RibbonWidget*)widget)->getSelectionName().c_str();
            
            if(selection == "audio_video") StateManager::replaceTopMostMenu("options_av.stkgui");
            else if(selection == "players") StateManager::replaceTopMostMenu("options_players.stkgui");
            else if(selection == "controls") StateManager::replaceTopMostMenu("options_input.stkgui");
        }
        else if(name == "back")
        {
            StateManager::escapePressed();
        }
        else
        {
            if(screen_name == "options_av.stkgui") eventAudioVideo(widget, name);
            else if(screen_name == "options_input.stkgui") eventInput(widget, name);
        }
        
    }
}
