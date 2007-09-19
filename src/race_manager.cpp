//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <iostream>

#include "loader.hpp"
#include "track_manager.hpp"
#include "race_setup.hpp"
#include "game_manager.hpp"
#include "kart_properties_manager.hpp"
#include "race_manager.hpp"
#include "gui/menu_manager.hpp"
#include "world.hpp"
#include "scene.hpp"
#include "user_config.hpp"

RaceManager* race_manager= NULL;


void
RaceMode::next()
{
    exit_race();
}

//-----------------------------------------------------------------------------
void
RaceMode::exit_race()
{
    menu_manager->switchToMainMenu();

    delete world;
    world = 0;

    scene->clear();

    race_manager->m_active_race = false;

}

//=============================================================================
GrandPrixMode::GrandPrixMode(const std::vector<std::string>& players_,
                             const CupData& cup_,
                             RaceDifficulty difficulty_,
                             int numKarts_)
: m_difficulty(difficulty_), m_num_karts(numKarts_), m_players(players_),
    m_cup(cup_), m_track(0)
{
    const int NUM_PLAYERS = m_players.size();

    std::vector<std::string> kart_names;
    
    // make sure we have a valid number of karts
    if ((m_num_karts < 0) || (m_num_karts > NUM_PLAYERS + int(kart_properties_manager->getNumberOfKarts()))) {
    	m_num_karts = NUM_PLAYERS + kart_properties_manager->getNumberOfKarts();
    }

    kart_names.resize(m_num_karts);

    for(int i = 0; i < NUM_PLAYERS; ++i)
    {
        /*Players position is behind the AI in the first race*/
        kart_names[m_num_karts-1 - i] = m_players[NUM_PLAYERS - 1 - i];
    }

    kart_properties_manager->fillWithRandomKarts(kart_names);

    const int NUM_AI_KARTS = m_num_karts - NUM_PLAYERS;
    //Add the AI karts
    for(int i = 0; i < NUM_AI_KARTS; ++i)
        m_karts.push_back(KartStatus(kart_names[i], 0, i, NUM_PLAYERS));
    //Add the player karts
    for(int i = 0; i < NUM_PLAYERS; ++i)
        m_karts.push_back(KartStatus(kart_names[i+NUM_AI_KARTS], 0, i+NUM_AI_KARTS, i));
}

//-----------------------------------------------------------------------------
void
GrandPrixMode::start_race(int n)
{
    RaceSetup raceSetup;
    raceSetup.m_mode       = RaceSetup::RM_GRAND_PRIX;
    raceSetup.m_difficulty = m_difficulty;
    raceSetup.m_num_laps    = 2;
    raceSetup.m_track      = m_cup.getTrack(n);
    raceSetup.m_karts.resize(m_karts.size());
    raceSetup.m_players.resize(m_players.size());
    raceSetup.setHerringStyle(m_cup.getHerringStyle());


    for(int i = 0; i < int(m_karts.size()); ++i)
    {
        raceSetup.m_karts[m_karts[i].prev_finish_pos] = m_karts[i].ident;
        if (m_karts[i].player < int(m_players.size()))
        {
            raceSetup.m_players[m_karts[i].player] = i;
        }
    }

    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    new World(raceSetup);
}

//-----------------------------------------------------------------------------
void
GrandPrixMode::start()
{
    start_race(m_track);
}

//-----------------------------------------------------------------------------
void
GrandPrixMode::next()
{
    m_track += 1;
    if (m_track < int(m_cup.getTrackCount()))
    {
        scene->clear();
        start_race(m_track);

    }
    else
    {
        exit_race();
    }
}

//-----------------------------------------------------------------------------
void
GrandPrixMode::exit_race()
{
    if (m_track < int(m_cup.getTrackCount()))
    {
        RaceMode::exit_race();
    }
    else
    {
        menu_manager->switchToGrandPrixEnding();
    }
}

//=============================================================================
QuickRaceMode::QuickRaceMode(const std::string& track_,
                             const std::vector<std::string>& players_,
                             RaceDifficulty difficulty_,
                             int numKarts_, int numLaps_)
        : m_track(track_), m_players(players_), m_difficulty(difficulty_),
        m_num_karts(numKarts_), m_num_laps(numLaps_)
{
	if ((m_num_karts<0) || (m_num_karts > int(kart_properties_manager->getNumberOfKarts() + m_players.size()))) {
		m_num_karts = kart_properties_manager->getNumberOfKarts() + m_players.size();
	}
}

//-----------------------------------------------------------------------------
void
QuickRaceMode::start()
{
    RaceSetup raceSetup;

    raceSetup.m_mode       = RaceSetup::RM_QUICK_RACE;
    raceSetup.m_difficulty = m_difficulty;
    raceSetup.m_track      = m_track;
    raceSetup.m_num_laps   = m_num_laps;
    raceSetup.m_karts.resize(m_num_karts);

    const int FIRST_PLAYER = m_num_karts - m_players.size();
    for(int i = 0; i < int(m_players.size()); ++i)
    {
        raceSetup.m_karts[FIRST_PLAYER + i] = m_players[i]; // Players starts last in the race
        raceSetup.m_players.push_back(FIRST_PLAYER + i);
    }

    kart_properties_manager->fillWithRandomKarts(raceSetup.m_karts);

    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    new World(raceSetup);
}

//=============================================================================
TimeTrialMode::TimeTrialMode(const std::string& track_, const std::string& kart_,
                             const int& numLaps_)
        : m_track(track_), m_kart(kart_), m_num_laps(numLaps_)
{}

//-----------------------------------------------------------------------------
void
TimeTrialMode::start()
{
    RaceSetup raceSetup;

    raceSetup.m_mode       = RaceSetup::RM_TIME_TRIAL;
    raceSetup.m_track      = m_track;
    raceSetup.m_num_laps   = m_num_laps;
    raceSetup.m_difficulty = RD_HARD;

    raceSetup.m_karts.push_back(m_kart);
    raceSetup.m_players.push_back(0);

    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    new World(raceSetup);
}

//=============================================================================
RaceManager::RaceManager()
{
    m_mode       = 0;
    m_num_karts  = user_config->m_karts;
    m_difficulty = RD_MEDIUM;
    m_race_mode  = RaceSetup::RM_QUICK_RACE;
    m_track      = "race";
    m_active_race = false;

    m_players.push_back("tuxkart");
}

//-----------------------------------------------------------------------------
RaceManager::~RaceManager()
{
    delete m_mode;
}

//-----------------------------------------------------------------------------
void RaceManager::reset()
{
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
}  // reset

//-----------------------------------------------------------------------------
void
RaceManager::setPlayerKart(int player, const std::string& kart)
{
    if (player >= 0 && player < 4)
    {
        if (player >= getNumPlayers())
            setNumPlayers(player+1);

        m_players[player] = kart;
    }
    else
    {
        fprintf(stderr, "Warning: player '%d' does not exists.\n", player);
    }
}

//-----------------------------------------------------------------------------
void
RaceManager::setNumPlayers(int num)
{
    m_players.resize(num);
    for(Players::iterator i = m_players.begin(); i != m_players.end(); ++i)
    {
        if (i->empty())
        {
            *i = "tuxkart";
        }
    }
}

//-----------------------------------------------------------------------------
void
RaceManager::start()
{
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
    delete m_mode;

    assert(m_players.size() > 0);
    switch(m_race_mode)
    {
    case RaceSetup::RM_GRAND_PRIX:
        m_mode = new GrandPrixMode(m_players, m_cup, m_difficulty, m_num_karts);
        break;
    case RaceSetup::RM_TIME_TRIAL:
        m_mode = new TimeTrialMode(m_track, m_players[0], m_num_laps);
        break;
    case RaceSetup::RM_QUICK_RACE:
        m_mode = new QuickRaceMode(m_track, m_players, m_difficulty, m_num_karts, m_num_laps);
        break;
    default:
        assert(!"Unknown game mode");
    }

    m_mode->start();

    m_active_race = true;
}

//-----------------------------------------------------------------------------
void
RaceManager::next()
{
    assert(m_mode);
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
    m_mode->next();
}

//-----------------------------------------------------------------------------
void
RaceManager::exit_race()
{
    m_mode->exit_race();
}

/* EOF */
