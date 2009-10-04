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


#include "states_screens/state_manager.hpp"

#include <vector>

#include "main_loop.hpp"
#include "audio/sound_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "input/input_device.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/options_screen.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "states_screens/dialogs/race_paused_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"

// FIXME : remove, temporary test
#include "states_screens/feature_unlocked.hpp"

using namespace GUIEngine;

StateManager* state_manager_singleton = NULL;

StateManager* StateManager::get()
{
    if (state_manager_singleton == NULL) state_manager_singleton = new StateManager();
    return state_manager_singleton;
}

#if 0
#pragma mark -
#pragma mark Player Management
#endif

ptr_vector<ActivePlayer, HOLD>& StateManager::getActivePlayers()
{
    return m_active_players;
}
ActivePlayer* StateManager::getActivePlayer(const int id)
{
    ActivePlayer *returnPlayer = NULL;
    if (id < m_active_players.size() && id >= 0)
    {
        returnPlayer = m_active_players.get(id);
    }
    else
    {
        fprintf(stderr, "getActivePlayer(): id out of bounds\n");
    }
    
    assert( returnPlayer->m_id == id );
    
    return returnPlayer;
}
void StateManager::updateActivePlayerIDs()
{
    const int amount = m_active_players.size();
    for (int n=0; n<amount; n++)
    {
        m_active_players[n].m_id = n;
    }
}

int StateManager::createActivePlayer(PlayerProfile *profile, InputDevice *device)
{
    ActivePlayer *p;
    int i;
    p = new ActivePlayer(profile, device);
    i = m_active_players.size();
    m_active_players.push_back(p);
    
    updateActivePlayerIDs();
    
    return i;
}

void StateManager::removeActivePlayer(int id)
{
    m_active_players.erase(id);
    updateActivePlayerIDs();
}
int StateManager::activePlayerCount()
{
    return m_active_players.size();
}

void StateManager::resetActivePlayers()
{
    const int amount = m_active_players.size();
    for(int i=0; i<amount; i++)
    {
        m_active_players[i].setDevice(NULL);
    }
    m_active_players.clearWithoutDeleting();
}

#if 0
#pragma mark -
#pragma mark Callbacks
#endif

// -------------------------------------------------------------------------
/**
 * Callback handling events from the main menu
 */
void StateManager::menuEventMain(Widget* widget, const std::string& name)
{
    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if(ribbon == NULL) return; // only interesting stuff in main menu is the ribbons
    std::string selection = ribbon->getSelectionIDString(GUI_PLAYER_ID);


    if (selection == "network")
    {
        printf("+++++ FeatureUnlockedCutScene::show() +++++\n");
        // FIXME : remove, temporary test
        FeatureUnlockedCutScene::show();
    }
    
    if (selection == "new")
    {
        pushMenu("karts.stkgui");
    }
    else if (selection == "options")
    {
        StateManager::pushMenu("options_av.stkgui");
    }
    else if (selection == "quit")
    {
        main_loop->abort();
        return;
    }
    else if (selection == "about")
    {
        pushMenu("credits.stkgui");
    }
    else if (selection == "help")
    {
        pushMenu("help1.stkgui");
    }
}

// -------------------------------------------------------------------------
/**
 * Callback handling events from the race setup menu
 */
void StateManager::menuEventRaceSetup(Widget* widget, const std::string& name)
{
    if(name == "init")
    {
        RibbonWidget* w = getCurrentScreen()->getWidget<RibbonWidget>("difficulty");
        assert( w != NULL );
        w->setSelection( race_manager->getDifficulty(), GUI_PLAYER_ID );
        
        SpinnerWidget* kartamount = getCurrentScreen()->getWidget<SpinnerWidget>("aikartamount");
        kartamount->setValue( race_manager->getNumKarts() - race_manager->getNumPlayers() );
        
        DynamicRibbonWidget* w2 = getCurrentScreen()->getWidget<DynamicRibbonWidget>("gamemode");
        assert( w2 != NULL );
        
        if(!getCurrentScreen()->m_inited)
        {
            w2->addItem( _("Snaky Competition\nAll blows allowed, so catch weapons and make clever use of them!"),
                        "normal",
                        file_manager->getDataDir() + "/gui/mode_normal.png");
            
            w2->addItem( _("Time Trial\nContains no powerups, so only your driving skills matter!"),
                        "timetrial",
                        file_manager->getDataDir() + "/gui/mode_tt.png");
            
            if (unlock_manager->isLocked("followtheleader"))
            {
                w2->addItem( _("Locked!\nFulfill challenges to gain access to locked areas"),
                            "locked",
                            file_manager->getDataDir() + "textures/gui_lock.png");
            }
            else
            {
                w2->addItem( _("Follow the Leader\nrun for second place, as the last kart will be disqualified every time the counter hits zero. Beware : going in front of the leader will get you eliminated too!"),
                            "ftl",
                            file_manager->getDataDir() + "/gui/mode_ftl.png");
            }
            
            if (race_manager->getNumPlayers() > 1)
            {
                w2->addItem( _("3-Strikes Battle\nonly in multiplayer games. Hit others with weapons until they lose all their lives."),
                            "3strikes",
                            file_manager->getDataDir() + "/gui/mode_3strikes.png");
            }
            
            getCurrentScreen()->m_inited = true;
        }
        w2->updateItemDisplay();
        
    }
    else if(name == "difficulty")
    {
        RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
        assert(w != NULL);
        const std::string& selection = w->getSelectionIDString(GUI_PLAYER_ID);

        if(selection == "novice")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_EASY;
            race_manager->setDifficulty(RaceManager::RD_EASY);
        }
        else if(selection == "intermediate")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_MEDIUM;
            race_manager->setDifficulty(RaceManager::RD_MEDIUM);
        }
        else if(selection == "expert")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_HARD;
            race_manager->setDifficulty(RaceManager::RD_HARD);
        }
    }
    else if(name == "gamemode")
    {
        DynamicRibbonWidget* w = dynamic_cast<DynamicRibbonWidget*>(widget);
        const std::string selectedMode = w->getSelectionIDString(GUI_PLAYER_ID);
        
        if (selectedMode == "normal")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_QUICK_RACE);
            pushMenu("tracks.stkgui");
        }
        else if (selectedMode == "timetrial")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            pushMenu("tracks.stkgui");
        }
        else if (selectedMode == "ftl")
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
            pushMenu("tracks.stkgui");
        }
        else if (selectedMode == "3strikes")
        {
            // TODO - 3 strikes battle track selection
            race_manager->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
        }
        else if (selectedMode == "locked")
        {
            std::cout << "Requesting sound to be played\n";
            unlock_manager->playLockSound();
        }
    }
    else if(name == "aikartamount")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        
        race_manager->setNumKarts( race_manager->getNumPlayers() + w->getValue() );
    }
    /*
     289         race_manager->setDifficulty((RaceManager::Difficulty)m_difficulty);
     290         UserConfigParams::setDefaultNumDifficulty(m_difficulty);

     // if there is no AI, there's no point asking the player for the amount of karts.
     299     // It will always be the same as the number of human players
     300     if(RaceManager::isBattleMode( race_manager->getMinorMode() ))
     301     {
     302         race_manager->setNumKarts(race_manager->getNumLocalPlayers());
     303         // Don't change the default number of karts in user_config
     304     }
     305     else
     306     {
     307         race_manager->setNumKarts(m_num_karts);
     308         UserConfigParams::setDefaultNumKarts(race_manager->getNumKarts());
     309     }

     311     if( race_manager->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX    &&
     312         RaceManager::modeHasLaps( race_manager->getMinorMode() )    )
     313     {
     314         race_manager->setNumLaps( m_num_laps );
     315         UserConfigParams::setDefaultNumLaps(m_num_laps);
     316     }
     317     // Might still be set from a previous challenge
     318     race_manager->setCoinTarget(0);

     input_manager->setMode(InputManager::INGAME);

     race_manager->setLocalKartInfo(0, argv[i+1]);

     race_manager->setDifficulty(RaceManager::RD_EASY);
     race_manager->setDifficulty(RaceManager::RD_HARD);
     race_manager->setDifficulty(RaceManager::RD_HARD);

     race_manager->setTrack(argv[i+1]);

     UserConfigParams::setDefaultNumKarts(stk_config->m_max_karts);
     race_manager->setNumKarts(UserConfigParams::getDefaultNumKarts() );

     UserConfigParams::getDefaultNumKarts()

     StateManager::enterGameState();
     race_manager->startNew();
     */

}

// -------------------------------------------------------------------------
/**
 * Callback handling events from the track menu
 */
void StateManager::menuEventTracks(Widget* widget, const std::string& name)
{
    if(name == "init")
    {
        DynamicRibbonWidget* w = getCurrentScreen()->getWidget<DynamicRibbonWidget>("tracks");
        assert( w != NULL );

        if (!getCurrentScreen()->m_inited)
        {
            const int trackAmount = track_manager->getNumberOfTracks();
            bool hasLockedTracks = false;
            for (int n=0; n<trackAmount; n++)
            {
                Track* curr = track_manager->getTrack(n);
                if (unlock_manager->isLocked(curr->getIdent()))
                {
                    hasLockedTracks = true;
                    continue;
                }
                w->addItem(curr->getName(), curr->getIdent(), curr->getScreenshotFile());
            }
            
            if (hasLockedTracks)
            {
                w->addItem(_("Locked Tracks"), "Lock", "textures/gui_lock.png");
            }
            
            getCurrentScreen()->m_inited = true;
        }
        w->updateItemDisplay();

    }
    // -- track seelction screen
    if (name == "tracks")
    {
        DynamicRibbonWidget* w2 = dynamic_cast<DynamicRibbonWidget*>(widget);
        if(w2 != NULL)
        {
            std::cout << "Clicked on track " << w2->getSelectionIDString(GUI_PLAYER_ID).c_str() << std::endl;
            
            Track* clickedTrack = track_manager->getTrack(w2->getSelectionIDString(GUI_PLAYER_ID));
            if (clickedTrack != NULL)
            {
                ITexture* screenshot = GUIEngine::getDriver()->getTexture( clickedTrack->getScreenshotFile().c_str() );
                
                new TrackInfoDialog( clickedTrack->getIdent(), clickedTrack->getName().c_str(), screenshot, 0.8f, 0.7f);
            }
        }
    }
    else if (name == "gps")
    {
        RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
        if(w != NULL)
            std::cout << "Clicked on GrandPrix " << w->getSelectionIDString(GUI_PLAYER_ID).c_str() << std::endl;
    }

}


// -----------------------------------------------------------------------------
/**
 * Callback handling events from the options menus
 */
void StateManager::menuEventHelp(Widget* widget, const std::string& name)
{
    if(name == "init")
    {
        RibbonWidget* w = getCurrentScreen()->getWidget<RibbonWidget>("category");
        
        if(w != NULL)
        {
            const std::string& screen_name = getCurrentScreen()->getName();
            if(screen_name == "help1.stkgui") w->select( "page1", GUI_PLAYER_ID );
            else if(screen_name == "help2.stkgui") w->select( "page2", GUI_PLAYER_ID );
            else if(screen_name == "help3.stkgui") w->select( "page3", GUI_PLAYER_ID );
        }
    }
    // -- options
    else if(name == "category")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(GUI_PLAYER_ID).c_str();

        if(selection == "page1") replaceTopMostMenu("help1.stkgui");
        else if(selection == "page2") replaceTopMostMenu("help2.stkgui");
        else if(selection == "page3") replaceTopMostMenu("help3.stkgui");
    }
    else if(name == "back")
    {
        escapePressed();
    }
}


// -------------------------------------------------------------------------
/**
 * All widget events will be dispatched to this function; arguments are
 * a pointer to the widget from which the event originates, and its internal
 * name. There is one exception : right after showing a new screen, an event with
 * name 'init' and widget set to NULL will be fired, so the screen can be filled
 * with the right values or so.
 */
void StateManager::eventCallback(Widget* widget, const std::string& name)
{
    std::cout << "event!! " << name.c_str() << std::endl;

    if (name == "lock")
    {
        unlock_manager->playLockSound();
    }
    
    Screen* topScreen = getCurrentScreen();
    if (topScreen == NULL) return;
    const std::string& screen_name = topScreen->getName();

    if( screen_name == "main.stkgui" )
        menuEventMain(widget, name);
    else if( screen_name == "karts.stkgui" )
        KartSelectionScreen::menuEventKarts(widget, name);
    else if( screen_name == "racesetup.stkgui" )
        menuEventRaceSetup(widget, name);
    else if( screen_name == "tracks.stkgui" )
        menuEventTracks(widget, name);
    else if( screen_name == "options_av.stkgui" || screen_name == "options_input.stkgui" || screen_name == "options_players.stkgui")
        OptionsScreen::menuEventOptions(widget, name);
    else if( screen_name == "help1.stkgui" || screen_name == "help2.stkgui" || screen_name == "help3.stkgui")
        menuEventHelp(widget, name);
    else if( screen_name == "credits.stkgui" )
    {
        if(name == "init")
        {
            Widget* w = getCurrentScreen()->getWidget<Widget>("animated_area");
            assert(w != NULL);
        
            Credits* credits = Credits::getInstance();
            credits->reset();
            credits->setArea(w->x, w->y, w->w, w->h);
        }
        else if(name == "back")
        {
            StateManager::escapePressed();
        }
    }
    else
        std::cerr << "Warning, unknown menu " << screen_name << " in event callback\n";

}

void StateManager::escapePressed()
{
    // in input sensing mode
    if(input_manager->isInMode(InputManager::INPUT_SENSE_KEYBOARD) ||
       input_manager->isInMode(InputManager::INPUT_SENSE_GAMEPAD) )
    {
        ModalDialog::dismiss();
        input_manager->setMode(InputManager::MENU);
    }
    // when another modal dialog is visible
    else if(ModalDialog::isADialogActive())
    {
        ModalDialog::dismiss();
    }
    // In-game
    else if(m_game_mode == GAME)
    {
        new RacePausedDialog(0.8f, 0.6f);
        //resetAndGoToMenu("main.stkgui");
        //input_manager->setMode(InputManager::MENU);
    }
    // In menus
    else
    {
        popMenu();
    }
}


void StateManager::onUpdate(float elapsed_time)
{
    // FIXME : don't hardcode?
    if (getCurrentScreen()->getName() == "credits.stkgui")
        Credits::getInstance()->render(elapsed_time);
    else if (getCurrentScreen()->getName() == "karts.stkgui")
        KartSelectionScreen::kartSelectionUpdate(elapsed_time);
}

