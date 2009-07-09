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

#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/player.hpp"
#include "graphics/irr_driver.hpp"
#include "gui/engine.hpp"
#include "gui/modaldialog.hpp"
#include "gui/options_screen.hpp"
#include "gui/screen.hpp"
#include "gui/state_manager.hpp"
#include "gui/widget.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"

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
        sfx->setState( UserConfigParams::m_sfx );
        music->setState( UserConfigParams::m_music );

        // ---- video modes
        {
            RibbonGridWidget* res = getCurrentScreen()->getWidget<RibbonGridWidget>("resolutions");
            assert( res != NULL );


            CheckBoxWidget* full = getCurrentScreen()->getWidget<CheckBoxWidget>("fullscreen");
            assert( full != NULL );
            full->setState( UserConfigParams::m_fullscreen );

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

                if(w == UserConfigParams::m_width && h == UserConfigParams::m_height)
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

            RibbonGridWidget* w1 = getCurrentScreen()->getWidget<RibbonGridWidget>("resolutions");
            assert(w1 != NULL);

            const std::string& res = w1->getSelectionIDString();

            int w = -1, h = -1;
            if( sscanf(res.c_str(), "%ix%i", &w, &h) != 2 || w == -1 || h == -1 )
            {
                std::cerr << "Failed to decode resolution : " << res.c_str() << std::endl;
                return;
            }

            CheckBoxWidget* w2 = getCurrentScreen()->getWidget<CheckBoxWidget>("fullscreen");
            assert(w2 != NULL);

            UserConfigParams::m_width = w;
            UserConfigParams::m_height = h;
            UserConfigParams::m_fullscreen = w2->getState();
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

                getCurrentScreen()->m_inited = true;

            }
            devices->updateItemDisplay();

            // trigger displaying bindings for default selected device
            const std::string name2("devices");
            eventInput(devices, name2);
    }
    
    // -----------------------------------------------------------------------------
    void initPlayers(Widget* widget, const std::string& name)
    {
        ListWidget* players = getCurrentScreen()->getWidget<ListWidget>("players");
        assert(players != NULL);
        
        const int playerAmount = UserConfigParams::m_player.size();
        for(int n=0; n<playerAmount; n++)
        {
            players->addItem( UserConfigParams::m_player[n].getName() );
        }
    }
    
    void eventPlayers(Widget* widget, const std::string& name)
    {
        if(name == "addplayer")
        {
            new EnterPlayerNameDialog(0.5f, 0.4f);
        }
        else if(name == "players")
        {
            std::cout << "Event : players\n";
            ListWidget* players = getCurrentScreen()->getWidget<ListWidget>("players");
            assert(players != NULL);

            std::string selectedPlayer = players->getSelectionName();
            const int playerAmount = UserConfigParams::m_player.size();
            for(int n=0; n<playerAmount; n++)
            {
                if(UserConfigParams::m_player[n].getName() == selectedPlayer)
                {
                    new PlayerInfoDialog( &UserConfigParams::m_player[n], 0.5f, 0.6f );
                    return;
                }
            } // end for
        }
        
    }
    
    // -----------------------------------------------------------------------------
    static PlayerAction binding_to_set;
    static std::string binding_to_set_button;

    void eventInput(Widget* widget, const std::string& name)
    {
        if(name == "devices")
        {
            RibbonGridWidget* devices = getCurrentScreen()->getWidget<RibbonGridWidget>("devices");
            assert(devices != NULL);

            const std::string& selection = devices->getSelectionIDString();
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
        else if(name.find("binding_") != std::string::npos)
        {
            binding_to_set_button = name;

            if(name == "binding_up")
            {
                binding_to_set = PA_ACCEL;
            }
            else if(name == "binding_down")
            {
                binding_to_set = PA_BRAKE;
            }
            else if(name == "binding_left")
            {
                binding_to_set = PA_LEFT;
            }
            else if(name == "binding_right")
            {
                binding_to_set = PA_RIGHT;
            }
            else if(name == "binding_fire")
            {
                binding_to_set = PA_FIRE;
            }
            else if(name == "binding_nitro")
            {
                binding_to_set = PA_NITRO;
            }
            else if(name == "binding_drift")
            {
                binding_to_set = PA_DRIFT;
            }
            else if(name == "binding_rescue")
            {
                binding_to_set = PA_RESCUE;
            }
            else if(name == "binding_look_back")
            {
                binding_to_set = PA_LOOK_BACK;
            }
            else
            {
                std::cerr << "Unknown binding name : " << name.c_str() << std::endl;
                return;
            }

            RibbonGridWidget* devices = getCurrentScreen()->getWidget<RibbonGridWidget>("devices");
            assert( devices != NULL );
            std::cout << "\n% Entering sensing mode for " << devices->getSelectionIDString().c_str() << std::endl;

            new PressAKeyDialog(0.4f, 0.4f);

            if(devices->getSelectionIDString() == "keyboard")
            {
                input_manager->setMode(InputManager::INPUT_SENSE_KEYBOARD);
            }
            else if(devices->getSelectionIDString().find("gamepad") != std::string::npos)
            {
                input_manager->setMode(InputManager::INPUT_SENSE_GAMEPAD);
            }
            else
            {
                std::cerr << "unknown selection device in options : " << devices->getSelectionIDString() << std::endl;
            }

        }
    }

    #define MAX_VALUE 32768

    // -----------------------------------------------------------------------------
    void gotSensedInput(Input* sensedInput)
    {
        RibbonGridWidget* devices = getCurrentScreen()->getWidget<RibbonGridWidget>("devices");
        assert( devices != NULL );

        const bool keyboard = sensedInput->type == Input::IT_KEYBOARD && devices->getSelectionIDString() == "keyboard";
        const bool gamepad =  (sensedInput->type == Input::IT_STICKMOTION ||
                               sensedInput->type == Input::IT_STICKBUTTON) &&
                               devices->getSelectionIDString().find("gamepad") != std::string::npos;

        if(!keyboard && !gamepad) return;
        if(gamepad)
        {
            if(sensedInput->type != Input::IT_STICKMOTION &&
               sensedInput->type != Input::IT_STICKBUTTON)
               return; // that kind of input does not interest us
        }


        if(keyboard)
        {
            std::cout << "% Binding " << KartActionStrings[binding_to_set] << " : setting to keyboard key " << sensedInput->btnID << " \n\n";

            KeyboardDevice* keyboard = input_manager->getDeviceList()->getKeyboard(0);
            keyboard->editBinding(binding_to_set, sensedInput->btnID);

            // refresh display
            initInput(NULL, "init");
        }
        else if(gamepad)
        {
            std::cout << "% Binding " << KartActionStrings[binding_to_set] << " : setting to gamepad #" << sensedInput->deviceID << " : ";
            if(sensedInput->type == Input::IT_STICKMOTION)
            {
                std::cout << "axis " << sensedInput->btnID << " direction " <<
                    (sensedInput->axisDirection == Input::AD_NEGATIVE ? "-" : "+") << "\n\n";
            }
            else if(sensedInput->type == Input::IT_STICKBUTTON)
            {
                std::cout << "button " << sensedInput->btnID << "\n\n";
            }
            else
                std::cout << "Sensed unknown gamepad event type??\n";

            int gamepadID = -1;

            if(sscanf( devices->getSelectionIDString().c_str(), "gamepad%i", &gamepadID ) != 1 ||
               gamepadID >= input_manager->getDeviceList()->getGamePadAmount())
            {
                if(gamepadID >= input_manager->getDeviceList()->getGamePadAmount() || gamepadID == -1 )
                {
                    std::cerr << "gamepad ID does not exist (or failed to read it) : " << gamepadID << "\n";
                    gamepadID = sensedInput->deviceID;
                }

                if(input_manager->getDeviceList()->getGamePad(gamepadID)->m_index != sensedInput->deviceID)
                {
                    // should not happen, but let's try to be bulletproof...
                    std::cerr << "The key that was pressed is not on the gamepad we're trying to configure! ID in list=" << gamepadID <<
                    " which has irrID " << input_manager->getDeviceList()->getGamePad(gamepadID)->m_index <<
                    " and we got input from " << sensedInput->deviceID << "\n";
                }

            }
            GamePadDevice* gamepad =  input_manager->getDeviceList()->getGamePad(gamepadID);
            gamepad->editBinding(binding_to_set, sensedInput->type, sensedInput->btnID,
                                 (Input::AxisDirection)sensedInput->axisDirection);

            // refresh display
            initInput(NULL, "init");
        }
        else
        {
            return;
        }

        ModalDialog::dismiss();
        input_manager->setMode(InputManager::MENU);

        // re-select the previous button
        ButtonWidget* btn = getCurrentScreen()->getWidget<ButtonWidget>(binding_to_set_button.c_str());
        if(btn != NULL) GUIEngine::getGUIEnv()->setFocus( btn->getIrrlichtElement() );

        // save new binding to file
        input_manager->getDeviceList()->serialize();
    }

    /**
      * Adds a new player (if 'player' is NULL) or renames an existing player (if 'player' is not NULL)
      */
    void gotNewPlayerName(const stringw& newName, Player* player)
    {
        stringc newNameC( newName );
        ListWidget* players = getCurrentScreen()->getWidget<ListWidget>("players");
        if(players == NULL) return;
        
        // ---- Add new player
        if(player == NULL)
        {
            UserConfigParams::m_player.push_back( new Player(newNameC.c_str()) );
            
            players->addItem( newNameC.c_str() );
        }
        else // ---- Rename existing player
        {
            player->setName( newNameC.c_str() );
            
            // refresh list display
            players->clear();
            const int playerAmount =  UserConfigParams::m_player.size();
            for(int n=0; n<playerAmount; n++)
            {
                players->addItem(UserConfigParams::m_player[n].getName());
            }

        }
        // TODO : need to re-save user config here?
    }
    
    void deletePlayer(Player* player)
    {
        UserConfigParams::m_player.erase(player);
        
        // refresh list display
        ListWidget* players = getCurrentScreen()->getWidget<ListWidget>("players");
        if(players == NULL) return;
        players->clear();

        const int playerAmount =  UserConfigParams::m_player.size();
        for(int n=0; n<playerAmount; n++)
        {
            players->addItem(UserConfigParams::m_player[n].getName());
        }

        // TODO : need to re-save user config here?
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
            else if(screen_name == "options_players.stkgui") initPlayers(widget, name);
        }
        else if(name == "options_choice")
        {
            std::string selection = ((RibbonWidget*)widget)->getSelectionIDString().c_str();

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
            else if(screen_name == "options_players.stkgui") eventPlayers(widget, name);
        }

    }
}
