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
#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/options_screen.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "utils/translation.hpp"

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
    return m_active_players.get(id);
}
void StateManager::addActivePlayer(ActivePlayer* p)
{
    m_active_players.push_back(p);
}
void StateManager::removeActivePlayer(int id)
{
    m_active_players.erase(id);
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
    std::string selection = ribbon->getSelectionIDString().c_str();


    if(selection == "new")
    {
        InputDevice* device = input_manager->getDeviceList()->getLatestUsedDevice();
        KartSelectionScreen::setPlayer0Device(device);
        pushMenu("karts.stkgui");
    }
    else if(selection == "options")
    {
        StateManager::pushMenu("options_av.stkgui");
    }
    else if(selection == "quit")
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
        w->setSelection(UserConfigParams::m_difficulty);

        race_manager->setDifficulty( (RaceManager::Difficulty)(int)UserConfigParams::m_difficulty );
        
        SpinnerWidget* kartamount = getCurrentScreen()->getWidget<SpinnerWidget>("aikartamount");
        race_manager->setNumKarts( kartamount->getValue() + 1 );
        
        RibbonGridWidget* w2 = getCurrentScreen()->getWidget<RibbonGridWidget>("gamemode");
        assert( w2 != NULL );
        
        if(!getCurrentScreen()->m_inited)
        {
            w2->addItem( _("Snaky Competition\nAll blows allowed, so catch weapons and make clever use of them!"),
                        "normal",
                        "gui/mode_normal.png");
            w2->addItem( _("Time Trial\nContains no powerups, so only your driving skills matter!"),
                        "timetrial",
                        "gui/mode_tt.png");
            w2->addItem( _("Follow the Leader\nrun for second place, as the last kart will be disqualified every time the counter hits zero. Beware : going in front of the leader will get you eliminated too!"),
                        "ftl",
                        "gui/mode_ftl.png");
            w2->addItem( _("3-Strikes Battle\nonly in multiplayer games. Hit others with weapons until they lose all their lives."),
                        "3strikes",
                        "gui/mode_3strikes.png");
            getCurrentScreen()->m_inited = true;
        }
        w2->updateItemDisplay();
        
    }
    else if(name == "difficulty")
    {
        RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
        assert(w != NULL);
        const std::string& selection = w->getSelectionIDString();

        if(selection == "novice")
            race_manager->setDifficulty(RaceManager::RD_EASY);
        else if(selection == "intermediate")
            race_manager->setDifficulty(RaceManager::RD_MEDIUM);
        else if(selection == "expert")
            race_manager->setDifficulty(RaceManager::RD_HARD);
    }
    else if(name == "gamemode")
    {
        // TODO - detect more game modes
        RibbonGridWidget* w = dynamic_cast<RibbonGridWidget*>(widget);
        if(w->getSelectionIDString() == "normal")
        {
            pushMenu("tracks.stkgui");
        }
    }
    else if(name == "aikartamount")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        
        race_manager->setNumKarts( w->getValue() );
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
        RibbonGridWidget* w = getCurrentScreen()->getWidget<RibbonGridWidget>("tracks");
        assert( w != NULL );

        if(!getCurrentScreen()->m_inited)
        {
            w->addItem("Track 1","t1","gui/track1.png");
            w->addItem("Track 2","t2","gui/track2.png");
            w->addItem("Track 3","t3","gui/track3.png");
            w->addItem("Track 4","t4","gui/track4.png");
            w->addItem("Track 5","t5","gui/track5.png");
            w->addItem("Track 6","t6","gui/track6.png");
            w->addItem("Track 7","t7","gui/track7.png");
            w->addItem("Track 8","t8","gui/track8.png");
            getCurrentScreen()->m_inited = true;
        }
        w->updateItemDisplay();

    }
    // -- track seelction screen
    if(name == "tracks")
    {
        RibbonGridWidget* w2 = dynamic_cast<RibbonGridWidget*>(widget);
        if(w2 != NULL)
        {
            std::cout << "Clicked on track " << w2->getSelectionIDString().c_str() << std::endl;
            
            ITexture* screenshot = GUIEngine::getDriver()->getTexture( (file_manager->getDataDir() + "/gui/track1.png").c_str() );
            
            new TrackInfoDialog( w2->getSelectionText().c_str(), screenshot, 0.8f, 0.7f);
        }
    }
    else if(name == "gps")
    {
        RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
        if(w != NULL)
            std::cout << "Clicked on GrandPrix " << w->getSelectionIDString().c_str() << std::endl;
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
            if(screen_name == "help1.stkgui") w->select( "page1" );
            else if(screen_name == "help2.stkgui") w->select( "page2" );
            else if(screen_name == "help3.stkgui") w->select( "page3" );
        }
    }
    // -- options
    else if(name == "category")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString().c_str();

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

    const std::string& screen_name = getCurrentScreen()->getName();

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
    else if(m_game_mode)
    {
        // TODO : show in-game menu
        resetAndGoToMenu("main.stkgui");
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

