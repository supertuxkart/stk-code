//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#include "states_screens/race_setup_screen.hpp"

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/arenas_screen.hpp"
#include "states_screens/easter_egg_screen.hpp"
#include "states_screens/ghost_replay_selection.hpp"
#include "states_screens/soccer_setup_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_and_gp_screen.hpp"
#include "utils/translation.hpp"

const int CONFIG_CODE_NORMAL    = 0;
const int CONFIG_CODE_TIMETRIAL = 1;
const int CONFIG_CODE_FTL       = 2;
const int CONFIG_CODE_3STRIKES  = 3;
const int CONFIG_CODE_EASTER    = 4;
const int CONFIG_CODE_SOCCER    = 5;
const int CONFIG_CODE_GHOST     = 6;

using namespace GUIEngine;
DEFINE_SCREEN_SINGLETON( RaceSetupScreen );

// -----------------------------------------------------------------------------

RaceSetupScreen::RaceSetupScreen() : Screen("race_setup.stkgui")
{
}   // RaceSetupScreen

// -----------------------------------------------------------------------------

void RaceSetupScreen::loadedFromFile()
{
}   // loadedFromFile

// -----------------------------------------------------------------------------

void RaceSetupScreen::init()
{
    Screen::init();
    input_manager->setMasterPlayerOnly(true);
    RibbonWidget* w = getWidget<RibbonWidget>("difficulty");
    assert( w != NULL );

    if (UserConfigParams::m_difficulty == RaceManager::DIFFICULTY_BEST &&
        PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
    {
        w->setSelection(RaceManager::DIFFICULTY_HARD, PLAYER_ID_GAME_MASTER);
    }
    else
    {
        w->setSelection( UserConfigParams::m_difficulty, PLAYER_ID_GAME_MASTER );
    }

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

    if (PlayerManager::getCurrentPlayer()->isLocked(IDENT_FTL))
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
        w2->addItem(name3, IDENT_FTL, RaceManager::getIconOf(RaceManager::MINOR_MODE_FOLLOW_LEADER), false);
    }

    irr::core::stringw name4 = irr::core::stringw(
        RaceManager::getNameOf(RaceManager::MINOR_MODE_3_STRIKES)) + L"\n";
    //FIXME: avoid duplicating descriptions from the help menu!
    name4 += _("Hit others with weapons until they lose all their lives.");
    w2->addItem( name4, IDENT_STRIKES, RaceManager::getIconOf(RaceManager::MINOR_MODE_3_STRIKES));

    irr::core::stringw name5 = irr::core::stringw(
        RaceManager::getNameOf(RaceManager::MINOR_MODE_SOCCER)) + L"\n";
    name5 += _("Push the ball into the opposite cage to score goals.");
    w2->addItem( name5, IDENT_SOCCER, RaceManager::getIconOf(RaceManager::MINOR_MODE_SOCCER));

#define ENABLE_EASTER_EGG_MODE
#ifdef ENABLE_EASTER_EGG_MODE
    if(race_manager->getNumLocalPlayers() == 1)
    {
        irr::core::stringw name1 = irr::core::stringw(
            RaceManager::getNameOf(RaceManager::MINOR_MODE_EASTER_EGG)) + L"\n";
        //FIXME: avoid duplicating descriptions from the help menu!
        name1 +=  _("Explore tracks to find all hidden eggs");

        w2->addItem( name1, IDENT_EASTER,
            RaceManager::getIconOf(RaceManager::MINOR_MODE_EASTER_EGG));
    }
#endif

    irr::core::stringw name6 = irr::core::stringw( _("Ghost replay race")) + L"\n";
    name6 += _("Race against ghost karts and try to beat them!");
    w2->addItem( name6, IDENT_GHOST, "/gui/mode_ghost.png");

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
        w2->setSelection(IDENT_FTL, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_3STRIKES :
        w2->setSelection(IDENT_STRIKES, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_EASTER :
        w2->setSelection(IDENT_EASTER, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_SOCCER :
        w2->setSelection(IDENT_SOCCER, PLAYER_ID_GAME_MASTER, true);
        break;
    case CONFIG_CODE_GHOST :
        w2->setSelection(IDENT_GHOST, PLAYER_ID_GAME_MASTER, true);
        break;
    }

    {
        RibbonWidget* w = getWidget<RibbonWidget>("difficulty");
        assert(w != NULL);

        int index = w->findItemNamed("best");
        Widget* hardestWidget = &w->getChildren()[index];

        if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
        {
            hardestWidget->setBadge(LOCKED_BADGE);
            hardestWidget->setActive(false);
        }
        else
        {
            hardestWidget->unsetBadge(LOCKED_BADGE);
            hardestWidget->setActive(true);
        }
    }
}   // init

// -----------------------------------------------------------------------------
void RaceSetupScreen::eventCallback(Widget* widget, const std::string& name,
                                    const int playerID)
{
    if (name == "difficulty")
    {
        assignDifficulty();
    }
    else if (name == "gamemode")
    {
        assignDifficulty();

        DynamicRibbonWidget* w = dynamic_cast<DynamicRibbonWidget*>(widget);
        const std::string& selectedMode = w->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selectedMode == IDENT_STD)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
            UserConfigParams::m_game_mode = CONFIG_CODE_NORMAL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_TTRIAL)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            UserConfigParams::m_game_mode = CONFIG_CODE_TIMETRIAL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_FTL)
        {
            // Make sure there are at least three karts, otherwise FTL doesn't
            if(race_manager->getNumberOfKarts()<3)
                race_manager->setNumKarts(3);

            race_manager->setMinorMode(RaceManager::MINOR_MODE_FOLLOW_LEADER);
            UserConfigParams::m_game_mode = CONFIG_CODE_FTL;
            TracksAndGPScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_STRIKES)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_3_STRIKES);
            UserConfigParams::m_game_mode = CONFIG_CODE_3STRIKES;
            ArenasScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_EASTER)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_EASTER_EGG);
            UserConfigParams::m_game_mode = CONFIG_CODE_EASTER;
            race_manager->setNumKarts( race_manager->getNumLocalPlayers() ); // no AI karts;
            EasterEggScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_SOCCER)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_SOCCER);
            UserConfigParams::m_game_mode = CONFIG_CODE_SOCCER;
            SoccerSetupScreen::getInstance()->push();
        }
        else if (selectedMode == IDENT_GHOST)
        {
            race_manager->setMinorMode(RaceManager::MINOR_MODE_TIME_TRIAL);
            UserConfigParams::m_game_mode = CONFIG_CODE_GHOST;
            GhostReplaySelection::getInstance()->push();
        }
        else if (selectedMode == "locked")
        {
            unlock_manager->playLockSound();
        }
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

}   // eventCallback

// -----------------------------------------------------------------------------
/** Converts the difficulty string into a RaceManager::Difficulty value
 *  and sets this difficulty in the user config and in the race manager.
 */
void RaceSetupScreen::assignDifficulty()
{
    RibbonWidget* difficulty_widget = getWidget<RibbonWidget>("difficulty");
    assert(difficulty_widget != NULL);
    const std::string& difficulty = 
        difficulty_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    
    RaceManager::Difficulty diff = RaceManager::convertDifficulty(difficulty);
    UserConfigParams::m_difficulty = diff;
    race_manager->setDifficulty(diff);
}   // assignDifficulty

// -----------------------------------------------------------------------------
