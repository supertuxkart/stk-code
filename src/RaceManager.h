//  $Id$
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

  RaceDifficulty difficulty;
  int numKarts;
public:
  std::vector<std::string> players;
  CupData cup;
  GrandPrixSetup stat;

  GrandPrixMode(const std::vector<std::string>& players_, 
                const CupData& cup_,
                RaceDifficulty difficulty_,
                int numKarts);
  virtual ~GrandPrixMode() {}

  void start();
  void next();
};

class QuickRaceMode : public RaceMode
{
public:
  std::string track;
  std::vector<std::string> players;
  RaceDifficulty difficulty;
  int numKarts;

  QuickRaceMode(const std::string& track_, 
                const std::vector<std::string>& players_, 
                RaceDifficulty difficulty_,
                int numKarts);
  virtual ~QuickRaceMode() {}

  void start();
  void next();
};

class TimeTrialMode : public RaceMode
{
public:
  std::string track;
  std::string kart;

  TimeTrialMode(const std::string& track_, const std::string& kart_);
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
  
  int numKarts;
public:
  RaceManager();

  RaceSetup::RaceMode getRaceMode() const;
  int getNumPlayers() const;

  void setNumKarts(int i);
  int  getNumKarts() const { return numKarts; }
  void setTrack(const std::string& track);
  void setRaceMode(RaceSetup::RaceMode mode);
  void setDifficulty(RaceDifficulty difficulty_);
  void setPlayerKart(int player, const std::string& kart);
  void setNumPlayers(int num);

  void setNumLaps(int num);
  void setMirror();
  void setReverse();
  void start();

  /** Start the next race or go back to the start screen, depending on
      the currently set game mode */
  void next();
};

#endif

/* EOF */
