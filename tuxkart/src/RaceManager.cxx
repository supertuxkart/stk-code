//  $Id: RaceManager.cxx,v 1.2 2004/08/24 00:07:04 grumbel Exp $
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
  raceSetup.track      = track_manager.getTrackId(cup.tracks[n]);
  raceSetup.numKarts   = 4;

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
}

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

void 
RaceManager::start()
{
  assert(mode);
  mode->start();
}

void
RaceManager::next()
{ 
  assert(mode);
  mode->next();
}

/* EOF */
