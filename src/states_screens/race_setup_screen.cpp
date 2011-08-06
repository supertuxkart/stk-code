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

#include "challenges/unlock_manager.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "utils/translation.hpp"

#include "states_screens/race_setup_screen.hpp"

const int CONFIG_CODE_NORMAL = 0;
const int CONFIG_CODE_TIMETRIAL = 1;
const int CONFIG_CODE_FTL = 2;
const int CONFIG_CODE_3STRIKES = 3;

using namespace GUIEngine;
DEFINE_SCREEN_SINGLETON( RaceSetupScreen );

class GameModeRibbonListener : public DynamicRibbonHoverListener
{
    RaceSetupScreen* m_parent;
public:

    GameModeRibbonListener(RaceSetupScreen* parent)
    {
        m_parent = parent;
    }

    virtual void onSelectionChanged(DynamicRibbonWidget* theWidget, const std::string& selectionID, 
                                    const irr::core::stringw& selectionText, const int playerID)
    {
        // game mode changed!!
        m_parent->onGameModeChanged();
    }
};

// -----------------------------------------------------------------------------

RaceSetupScreen::RaceSetupScreen() : Screen("racesetup.stkgui")
{
}

// -----------------------------------------------------------------------------

void RaceSetupScreen::loadedFromFile()
{
}

// -----------------------------------------------------------------------------

void RaceSetupScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "difficulty")
    {
        RibbonWidget* w = dynamic_cast<RibbonWidget*>(widget);
        assert(w != NULL);
        const std::string& selection = w->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        if (selection == "novice")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_EASY;
            race_manager->setDifficulty(RaceManager::RD_EASY);
        }
        else if (selection == "intermediate")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_MEDIUM;
            race_manager->setDifficulty(RaceManager::RD_MEDIUM);
        }
        else if (selection == "expert")
        {
            UserConfigParams::m_difficulty = RaceManager::RD_HARD;
            race_manager->setDifficulty(RaceManager::RD_HARD);
        }
    }
    else if (name == "gamemode")
    {
        DynamicRibbonWidget* w = dynamic_cast<DynamicRibbonWidget*>(widget);
        const std::string selectedMode = w->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        if (selectedMode == IDENT_STD)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
            UserConfigParams::m_game_mode = CONFIG_CODE_NORMAL;
            StateManager::get()->pushScreen( TracksScreen::getInstance() );
        }
        else if (selectedMode == IDENT_TTRIAL)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            UserConfigParams::m_game_mode = CONFIG_CODE_TIMETRIAL;
            StateManager::get()->pushScreen( TracksScreen::getInstance() );
        }
        else if (selectedMode == FTL_IDENT)
        {
            // Make sure there are at least three karts, otherwise FTL doesn't
            if(race_manager->getNumberOfKarts()<3)
                race_manager->setNumKarts(3);

            race_manager->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
            UserConfigParams::m_game_mode = CONFIG_CODE_FTL;
            StateManager::get()->pushScreen( TracksScreen::getInstance() );
        }
        else if (selectedMode == STRIKES_IDENT)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
            UserConfigParams::m_game_mode = CONFIG_CODE_3STRIKES;
            race_manager->setNumKarts( race_manager->getNumLocalPlayers() ); // no AI karts;
            StateManager::get()->pushScreen( ArenasScreen::getInstance() );
        }
        else if (selectedMode == "locked")
        {
            unlock_manager->playLockSound();
        }
    }
    else if (name == "aikartamount")
    {
        SpinnerWidget* w = dynamic_cast<SpinnerWidget*>(widget);
        race_manager->setNumKarts( race_manager->getNumLocalPlayers() + w->getValue() );
        UserConfigParams::m_num_karts = race_manager->getNumLocalPlayers() + w->getValue();
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}

// -----------------------------------------------------------------------------

void RaceSetupScreen::onGameModeChanged()
{
    DynamicRibbonWidget* w2 = getWidget<DynamicRibbonWidget>("gamemode");
    assert( w2 != NULL );
    
    std::string gamemode_str = w2->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    if (gamemode_str == "locked") return;
    
    RaceManager::MinorRaceModeType gamemode = 
        RaceManager::getModeIDFromInternalName(gamemode_str);
    
    // deactivate the AI karts count widget for modes for which we have no AI
    SpinnerWidget* kartamount = getWidget<SpinnerWidget>("aikartamount");
    if (!RaceManager::hasAI(gamemode))
    {
        kartamount->setDeactivated();
    }
    else
    {
        kartamount->setActivated();
    }
}

// -----------------------------------------------------------------------------

void RaceSetupScreen::init()
{
    Screen::init();
    RibbonWidget* w = getWidget<RibbonWidget>("difficulty");
    assert( w != NULL );
    w->setSelection( UserConfigParams::m_difficulty, PLAYER_ID_GAME_MASTER );
    
    SpinnerWidget* kartamount = getWidget<SpinnerWidget>("aikartamount");
    kartamount->setActivated();
        
    // Avoid negative numbers (which can happen if e.g. the number of karts
    // in a previous race was lower than the number of players now.
    int num_ai = race_manager->getNumberOfKarts()-race_manager->getNumLocalPlayers();
    if(num_ai<0) num_ai = 0;
    kartamount->setValue(num_ai);
    kartamount->setMax(stk_config->m_max_karts - race_manager->getNumLocalPlayers() );
    
    DynamicRibbonWidget* w2 = getWidget<DynamicRibbonWidget>("gamemode");
    assert( w2 != NULL );
    w2->clearItems();

    // ---- Add game modes
    irr::core::stringw name1 = irr::core::stringw(
        RaceManager::getNameOf(RaceManager::MINOR_MODE_NORMAL_RACE)) + L"\n";
    //FIXME: avoid duplicating descriptions from the help menu!
    name1 +=  _("All blows allowed, so catch weapons and make clever use of them!");
    
    w2->addItem( name1, IDENT_STD, RaceManager::getIconOf(RaceManager::MINOR_MODE_NORMAL_RACE));
    
    irr::core::stringw name2 = irr::core::stringw(
        RaceManager::getNameOf(RaceManager::MINOR_MODE_TIME_TRIAL)) + L"\n";
    //FIXME: avoid duplicating descriptions from the help menu!
    name2 += _("Contains no powerups, so only your driving skills matter!");
    w2->addItem( name2, IDENT_TTRIAL, RaceManager::getIconOf(RaceManager::MINOR_MODE_TIME_TRIAL));
    
    if (unlock_manager->isLocked(FTL_IDENT))
    {
        w2->addItem( _("Locked : solve active challenges to gain access to more!"),
                    "locked", RaceManager::getIconOf(RaceManager::MINOR_MODE_FOLLOW_LEADER), true);
    }
    else
    {
        irr::core::stringw name3 = irr::core::stringw(
            RaceManager::getNameOf(RaceManager::MINOR_MODE_FOLLOW_LEADER)) + L"\n";
        //I18N: short definition for follow-the-leader game mode
        name3 += _("Keep up with the leader kart but don't overtake it!");
        w2->addItem(name3, FTL_IDENT, RaceManager::getIconOf(RaceManager::MINOR_MODE_FOLLOW_LEADER), false);
    }
    
    if (race_manager->getNumLocalPlayers() > 1)
    {
        irr::core::stringw name4 = irr::core::stringw(
            RaceManager::getNameOf(RaceManager::MINOR_MODE_3_STRIKES)) + L"\n";
        //FIXME: avoid duplicating descriptions from the help menu!
        name4 += _("Hit others with weapons until they lose all their lives. (Only in multiplayer games)");
        w2->addItem( name4, STRIKES_IDENT, RaceManager::getIconOf(RaceManager::MINOR_MODE_3_STRIKES));
    }
    

    w2->updateItemDisplay();
    
    // restore saved game mode
    switch (UserConfigParams::m_game_mode)
    {
        case CONFIG_CODE_NORMAL :
            w2->setSelection(IDENT_STD, PLAYER_ID_GAME_MASTER, true);
            break;
        case CONFIG_CODE_TIMETRIAL :
            w2->setSelection(IDENT_TTRIAL, PLAYER_ID_GAME_MASTER, true);
            break;
        case CONFIG_CODE_FTL :
            w2->setSelection(FTL_IDENT, PLAYER_ID_GAME_MASTER, true);
            break;
        case CONFIG_CODE_3STRIKES :
            w2->setSelection(STRIKES_IDENT, PLAYER_ID_GAME_MASTER, true);
            break;
    }
    
    m_mode_listener = new GameModeRibbonListener(this);
    w2->registerHoverListener(m_mode_listener);
}

// -----------------------------------------------------------------------------
