//  $Id: RaceManager.cxx,v 1.4 2004/08/24 21:01:44 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
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
#include "Loader.h"
#include "TrackManager.h"
#include "RaceSetup.h"
#include "ScreenManager.h"
#include "StartScreen.h"
#include "WorldScreen.h"
#include "RaceManager.h"

RaceManager* RaceManager::instance_ = 0;

GrandPrixMode::GrandPrixMode(const CupData& cup_,
                             RaceDifficulty difficulty_)
  : cup(cup_), difficulty(difficulty_)
{}

void
GrandPrixMode::start_race(int n)
{
  RaceSetup raceSetup;

  raceSetup.mode       = RaceSetup::RM_GRAND_PRIX;
  raceSetup.difficulty = difficulty;
  raceSetup.numLaps    = 3; 
  raceSetup.track      = cup.tracks[n];

  raceSetup.karts.push_back("tuxkart");
  raceSetup.karts.push_back("pennykart");
  raceSetup.karts.push_back("dinokart");
  raceSetup.karts.push_back("eviltux");

  raceSetup.players.push_back(0);

  ScreenManager::current()->set_screen(new WorldScreen(raceSetup)); 
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

  if (stat.race < int(cup.tracks.size()))
    {
      start_race(stat.race);
    }
  else
    {
      // FIXME: Insert credits/extro stuff here
      ScreenManager::current()->set_screen(new StartScreen()); 
    }
}


QuickRaceMode::QuickRaceMode(const TrackData& data_, RaceDifficulty difficulty_)
  : data(data_), difficulty(difficulty_)
{}

void
QuickRaceMode::start()
{
  RaceSetup raceSetup;

  raceSetup.track = data.ident;

  raceSetup.karts.push_back("tuxkart");
  raceSetup.karts.push_back("pennykart");
  raceSetup.karts.push_back("dinokart");
  raceSetup.karts.push_back("eviltux");

  raceSetup.players.push_back(0);

  ScreenManager::current()->set_screen(new WorldScreen(raceSetup));
}

void
QuickRaceMode::next()
{
  ScreenManager::current()->set_screen(new StartScreen());
}

TimeTrialMode::TimeTrialMode(const TrackData& data_)
  : data(data_)
{}

void
TimeTrialMode::start()
{
  RaceSetup raceSetup;

  raceSetup.track = data.ident;

  raceSetup.karts.push_back("tuxkart");
  raceSetup.karts.push_back("pennykart");
  raceSetup.karts.push_back("dinokart");
  raceSetup.karts.push_back("eviltux");

  raceSetup.players.push_back(0);

  ScreenManager::current()->set_screen(new WorldScreen(raceSetup));
}

void
TimeTrialMode::next()
{
  ScreenManager::current()->set_screen(new StartScreen());
}

RaceManager::RaceManager()
{ 
  mode = 0;
  difficulty = RD_MEDIUM;
  track = "race";
  players.push_back("tuxkart");
}
/*
void
RaceManager::set_grandprix(const std::string& cup, RaceDifficulty difficulty_)
{
  delete mode;
  mode = new GrandPrixMode(loader->getPath("data/" + cup + ".cup"), difficulty_);
}

void
RaceManager::set_quickrace(const std::string& track, RaceDifficulty difficulty_)
{
  delete mode;
 
  mode = new QuickRaceMode(track_manager.getTrack(track), difficulty_);
}

void
RaceManager::set_timetrial(const std::string& track)
{
  delete mode;
  mode = new TimeTrialMode(track_manager.getTrack(track));
}
*/
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
  if (player >= 0 && player < int(players.size()))
    players[player] = kart;
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
RaceManager::start()
{
  delete mode;

  switch(race_mode)
    {
    case RaceSetup::RM_GRAND_PRIX:
      mode = new GrandPrixMode(CupData(loader->getPath("data/herring.cup")), difficulty);
      break;
    case RaceSetup::RM_TIME_TRIAL:
      mode = new TimeTrialMode(track_manager.getTrack(track));
      break;
    case RaceSetup::RM_QUICK_RACE:
      mode = new QuickRaceMode(track_manager.getTrack(track), difficulty);
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

/* EOF */
