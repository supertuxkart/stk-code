//  $Id$
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

#include <assert.h>
#include <iostream>
#include <algorithm>
#include "tuxkart.h"
#include "Loader.h"
#include "TrackManager.h"
#include "RaceSetup.h"
#include "ScreenManager.h"
#include "StartScreen.h"
#include "WorldScreen.h"
#include "KartManager.h"
#include "RaceManager.h"

RaceManager* RaceManager::instance_ = 0;

GrandPrixMode::GrandPrixMode(const std::vector<std::string>& players_, 
                             const CupData& cup_,
                             RaceDifficulty difficulty_, 
                             int numKarts_)
  : difficulty(difficulty_), numKarts(numKarts_), players(players_), cup(cup_)
{
  // Decide which karts should be used in the GrandPrix
  std::vector<std::string> karts;
  karts.resize(6);
  for(int i = 0; i < int(players.size()); ++i)
    karts[5-i] = players[i]; // Players starts last in the first race
  kart_manager.fillWithRandomKarts(karts);

  stat.race = 0;
  for(int i = 0; i < int(karts.size()); ++i)
    {
      stat.karts.push_back(GrandPrixSetup::Stat(karts[i], 0, i));
    }
}

void
GrandPrixMode::start_race(int n)
{
  RaceSetup raceSetup;

  raceSetup.mode       = RaceSetup::RM_GRAND_PRIX;
  raceSetup.difficulty = difficulty;
  raceSetup.numLaps    = 3; 
  raceSetup.track      = cup.tracks[n];

  raceSetup.karts.resize(stat.karts.size());
  raceSetup.players.resize(players.size());
  // FIXME: This still isn't really good enough, can't handle multiple of the same kart and stuff
  for(int i = 0; i < int(stat.karts.size()); ++i)
    {
      raceSetup.karts[stat.karts[i].position] = stat.karts[i].ident;
      std::vector<std::string>::iterator j = std::find(players.begin(), players.end(), stat.karts[i].ident);
      if (j != players.end())
        {
          raceSetup.players[j - players.begin()] = i;
        }
    }

  screenManager->setScreen(new WorldScreen(raceSetup)); 
}

void
GrandPrixMode::start()
{
  stat.race = 0;
  start_race(stat.race);
}

void
GrandPrixMode::next()
{
  stat.race += 1;

  if (guiStack.back() == GUIS_NEXTRACE)
      if (stat.race	< int(cup.tracks.size()))
	    {
	      start_race(stat.race);
	    }
      else
	    {
	  // FIXME:	Insert credits/extro stuff here
	      startScreen =	new	StartScreen();
	      screenManager->setScreen(startScreen); 
	    }
  else
    {
      startScreen =	new	StartScreen();
	  screenManager->setScreen(startScreen); 
    }
  

}


QuickRaceMode::QuickRaceMode(const std::string& track_, 
                             const std::vector<std::string>& players_, 
                             RaceDifficulty difficulty_, 
                             int numKarts_)
  : track(track_), players(players_), difficulty(difficulty_), numKarts(numKarts_)
{}

void
QuickRaceMode::start()
{
  RaceSetup raceSetup;


  raceSetup.mode       = RaceSetup::RM_QUICK_RACE;
  raceSetup.difficulty = difficulty;
  raceSetup.track = track;
  raceSetup.karts.resize(numKarts);

  for(int i = 0; i < int(players.size()); ++i)
    {
      raceSetup.karts[numKarts-i-1] = players[i]; // Players starts last in the first race
      raceSetup.players.push_back(numKarts-i-1);
    }
  
  kart_manager.fillWithRandomKarts(raceSetup.karts);
  
  screenManager->setScreen(new WorldScreen(raceSetup));
}

void
QuickRaceMode::next()
{
  startScreen = new StartScreen();
  screenManager->setScreen(startScreen);
}

TimeTrialMode::TimeTrialMode(const std::string& track_, const std::string& kart_)
  : track(track_), kart(kart_)
{}

void
TimeTrialMode::start()
{
  RaceSetup raceSetup;

  raceSetup.track = track;
  raceSetup.karts.push_back(kart);
  raceSetup.players.push_back(0);

  screenManager->setScreen(new WorldScreen(raceSetup));
}

void
TimeTrialMode::next()
{
  startScreen = new StartScreen();
  screenManager->setScreen(startScreen);
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
RaceManager::setDifficulty(RaceDifficulty difficulty_)
{
  difficulty = difficulty_;
}

RaceSetup::RaceMode
RaceManager::getRaceMode() const
{
  return race_mode;
}

void
RaceManager::setRaceMode(RaceSetup::RaceMode mode)
{
  race_mode = mode;
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
    std::cout << "Warning: player " << player << " is out of range" << std::endl;
}

void
RaceManager::setTrack(const std::string& track_)
{
  track = track_;
}

int
RaceManager::getNumPlayers() const
{
  return players.size();
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
RaceManager::setNumKarts(int i)
{
  numKarts = i;
}

void 
RaceManager::start()
{
  delete mode;
  
  assert(players.size() > 0);
  switch(race_mode)
    {
    case RaceSetup::RM_GRAND_PRIX:
      mode = new GrandPrixMode(players, CupData(loader->getPath("data/herring.cup")), difficulty, numKarts);
      break;
    case RaceSetup::RM_TIME_TRIAL:
      mode = new TimeTrialMode(track, players[0]);
      break;
    case RaceSetup::RM_QUICK_RACE:
      mode = new QuickRaceMode(track, players, difficulty, numKarts);
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
  mode->next();
}

void
RaceManager::setNumLaps(int num)
{
  (void)num;
}

void
RaceManager::setMirror()
{
  // FIXME
}

void
RaceManager::setReverse()
{
  // FIXME
}

/* EOF */
