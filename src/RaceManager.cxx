//  $Id: RaceManager.cxx,v 1.6 2005/08/23 19:57:16 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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
#include "Loader.h"
#include "TrackManager.h"
#include "RaceSetup.h"
#include "ScreenManager.h"
#include "StartScreen.h"
#include "WorldScreen.h"
#include "KartManager.h"
#include "RaceManager.h"
#include "gui/BaseGUI.h"

RaceManager* race_manager=0;

GrandPrixMode::GrandPrixMode(const std::vector<std::string>& players_, 
                             const CupData& cup_,
                             RaceDifficulty difficulty_, 
                             int numKarts_)
  : difficulty(difficulty_), numKarts(numKarts_), players(players_),
    cup(cup_), track(0)
{

  std::vector<std::string> kart_names;
  kart_names.resize(numKarts);

  const int numPlayers = players.size();
  for(int i = 0; i < numPlayers; ++i){
    /*Players position is behind the AI in the first race*/
    kart_names[numKarts-1 - i] = players[numPlayers - 1 - i];
  }

  kart_manager->fillWithRandomKarts(kart_names);

  const int numAIKarts = numKarts - numPlayers;
  //Add the AI karts
  for(int i = 0; i < numAIKarts; ++i)
      karts.push_back(KartStatus(kart_names[i], 0, i, numPlayers));
  //Add the player karts
  for(int i = 0; i < numPlayers; ++i)
      karts.push_back(KartStatus(kart_names[i+numAIKarts], 0, i+numAIKarts, i));
}

void
GrandPrixMode::start_race(int n)
{
  RaceSetup raceSetup;

  raceSetup.mode       = RaceSetup::RM_GRAND_PRIX;
  raceSetup.difficulty = difficulty;
  raceSetup.numLaps    = 1;
  raceSetup.track      = cup.tracks[n];

  raceSetup.karts.resize(karts.size());
  raceSetup.players.resize(players.size());

  
  for(int i = 0; i < int(karts.size()); ++i)
    {
      raceSetup.karts[karts[i].prev_finish_pos] = karts[i].ident;
      if (karts[i].player < int(players.size()))
        {
          raceSetup.players[karts[i].player] = i;
        }
    }

  screen_manager->setScreen(new WorldScreen(raceSetup));
}

void
GrandPrixMode::start()
{
  start_race(track);
}

void
GrandPrixMode::next()
{
  track += 1;


 if( guiStack.back() == GUIS_NEXTRACE )
{
   if( track < int ( cup.tracks.size() ) ) start_race(track);
   else
    {
      // FIXME: Insert credits/extra stuff here
      startScreen = new StartScreen();
      screen_manager->setScreen(startScreen); 
    }
}

}


QuickRaceMode::QuickRaceMode(const std::string& track_, 
                             const std::vector<std::string>& players_, 
                             RaceDifficulty difficulty_, 
                             int numKarts_, int numLaps_)
  : track(track_), players(players_), difficulty(difficulty_), 
    numKarts(numKarts_), numLaps(numLaps_)
{}

void
QuickRaceMode::start()
{
  RaceSetup raceSetup;

  raceSetup.mode       = RaceSetup::RM_QUICK_RACE;
  raceSetup.difficulty = difficulty;
  raceSetup.track      = track;
  raceSetup.numLaps    = numLaps;
  raceSetup.karts.resize(numKarts);

  int first_player = numKarts - players.size();
  for(int i = 0; i < int(players.size()); ++i)
    {
      raceSetup.karts[first_player + i] = players[i]; // Players starts last in the first race
      raceSetup.players.push_back(first_player + i);
    }

  kart_manager->fillWithRandomKarts(raceSetup.karts);

  screen_manager->setScreen(new WorldScreen(raceSetup));
}

void
QuickRaceMode::next()
{
  startScreen = new StartScreen();
  screen_manager->setScreen(startScreen);
}

TimeTrialMode::TimeTrialMode(const std::string& track_, const std::string& kart_,
                             const int& numLaps_)
  : track(track_), kart(kart_), numLaps(numLaps_)
{}

void
TimeTrialMode::start()
{
  RaceSetup raceSetup;

  raceSetup.mode       = RaceSetup::RM_TIME_TRIAL;
  raceSetup.track      = track;
  raceSetup.numLaps    = numLaps;
  raceSetup.karts.push_back(kart);
  raceSetup.players.push_back(0);

  screen_manager->setScreen(new WorldScreen(raceSetup));
}

void
TimeTrialMode::next()
{
  startScreen = new StartScreen();
  screen_manager->setScreen(startScreen);
}

RaceManager::RaceManager()
{ 
  mode       = 0;
  numKarts   = 6;
  difficulty = RD_MEDIUM;
  race_mode  = RaceSetup::RM_QUICK_RACE;
  track      = "race";
  players.push_back("tuxkart");
}

void
RaceManager::setPlayerKart(int player, const std::string& kart)
{
  if (player >= 0 && player < 4)
    {
       if (player >= getNumPlayers())
         setNumPlayers(player+1);

      players[player] = kart;
    }
  else
    std::cout << "Warning: player " << player << " doesn't exists" << std::endl;
}

void
RaceManager::setNumPlayers(int num)
{
  players.resize(num);
  for(Players::iterator i = players.begin(); i != players.end(); ++i)
    {
      if (i->empty())
        {
          *i = "tuxkart";
        }
    }
}

void
RaceManager::start()
{
  numFinishedKarts = 0;

  delete mode;

  assert(players.size() > 0);
  switch(race_mode)
    {
    case RaceSetup::RM_GRAND_PRIX:
      mode = new GrandPrixMode(players, 
			       CupData("data/herring.cup"),
			       difficulty, numKarts);
      break;
    case RaceSetup::RM_TIME_TRIAL:
      mode = new TimeTrialMode(track, players[0], numLaps);
      break;
    case RaceSetup::RM_QUICK_RACE:
      mode = new QuickRaceMode(track, players, difficulty, numKarts, numLaps);
      break;
    default:
      assert(!"Unknown game mode");
    }

  mode->start();
}

void
RaceManager::next()
{
  assert(mode);
  numFinishedKarts = 0;
  mode->next();
}

/* EOF */
