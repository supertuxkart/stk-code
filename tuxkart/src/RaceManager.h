//  $Id: RaceManager.h,v 1.4 2004/08/24 21:01:44 grumbel Exp $
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

#ifndef HEADER_RACEMANAGER_H
#define HEADER_RACEMANAGER_H

#include "GrandPrixSetup.h"
#include "TrackData.h"
#include "RaceSetup.h"

class RaceMode
{
public:
  /** Start the mode and go into the first race */
  virtual void start() =0;

  /** Do the 'next thing' after the race is finished, ie. return to
      the start screen or start a new race */
  virtual void next() =0;
};

class GrandPrixMode : public RaceMode
{
private:
  void start_race(int n);
public:
  CupData cup;
  RaceDifficulty difficulty;
  GrandPrixSetup stat;

  GrandPrixMode(const CupData& cup_,
                RaceDifficulty difficulty_);
  virtual ~GrandPrixMode() {}

  void start();
  void next();
};

class QuickRaceMode : public RaceMode
{
public:
  TrackData data;
  RaceDifficulty difficulty;

  QuickRaceMode(const TrackData& data_, RaceDifficulty difficulty_);
  virtual ~QuickRaceMode() {}

  void start();
  void next();
};

class TimeTrialMode : public RaceMode
{
public:
  TrackData data;

  TimeTrialMode(const TrackData& data_);
  virtual ~TimeTrialMode() {}
  
  void start();
  void next();
};

/** RaceManager keeps track of the game mode, number of players and
    such, the GUI calls the RaceManager to setup and start a race,
    grandprix or similar stuff */
class RaceManager
{
private:
  static RaceManager* instance_;
public:
  static RaceManager* instance() { return instance_ ? instance_ : (instance_ = new RaceManager()); }

private:
  RaceMode*       mode;

  RaceDifficulty difficulty;
  RaceSetup::RaceMode race_mode;
  typedef std::vector<std::string> Players;
  Players players;
  std::string track;
public:
  RaceManager();
  /*  
  void set_grandprix(const std::string& cup, RaceDifficulty difficulty_);
  void set_quickrace(const std::string& track, RaceDifficulty difficulty_);
  void set_timetrial(const std::string& track);
  */
  RaceSetup::RaceMode getRaceMode() const;
  int getNumPlayers() const;

  void setTrack(const std::string& track);
  void setRaceMode(RaceSetup::RaceMode mode);
  void setDifficulty(RaceDifficulty difficulty_);
  void setPlayerKart(int player, const std::string& kart);
  void setNumPlayers(int num);
  
  void start();

  /** Start the next race or go back to the start screen, depending on
      the currently set game mode */
  void next();
};

#endif

/* EOF */
