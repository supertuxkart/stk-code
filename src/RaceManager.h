//  $Id: RaceManager.h,v 1.4 2005/08/23 19:56:17 joh Exp $
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

#ifndef HEADER_RACEMANAGER_H
#define HEADER_RACEMANAGER_H

#include <vector>
#include <string>

#include "RaceSetup.h"
#include "CupData.h"

class RaceMode
{
public:
  virtual      ~RaceMode() {};	// avoid compiler warning
  virtual void start()    = 0;  // Start the mode and go into the first race 
  virtual void next ()    = 0;  // Do the 'next thing' after the race is 
                                // finished, ie. return to the start screen or
                                // start a new race
  virtual int  getKartScore(int kart) const {return 0;}
  virtual void setKartScore(int kart, int pos) {}
};

class GrandPrixMode : public RaceMode
{
private:
  void start_race(int n);

  RaceDifficulty difficulty;
  int numKarts;

  struct KartStatus
  {
      std::string ident;//The .tkkf filename without the .tkkf
      int score;
      int prev_finish_pos;
      int player;//Which player controls the kart, for the AI this is
                 //the number of players.

      KartStatus(const std::string& ident_, const int& score_,
                 const int& prev_finish_pos_, const int& player_) :
          ident(ident_), score(score_), prev_finish_pos(prev_finish_pos_),
          player(player_) {}
  };

public:
  std::vector<std::string> players;
  CupData cup;
  std::vector<KartStatus> karts;
  int track;

  GrandPrixMode(const std::vector<std::string>& players_, 
                const CupData& cup_,
                RaceDifficulty difficulty_,
                int numKarts);
  virtual ~GrandPrixMode() {}

  void start();
  void next();

  int  getKartScore(int kart) const { return karts[kart].score; }
  void setKartScore(int kart, int pos)
      { karts[kart].score = pos > 4 ? 0 : 4 - pos; }
};

class QuickRaceMode : public RaceMode
{
public:
  std::string track;
  std::vector<std::string> players;
  RaceDifficulty difficulty;
  int numKarts, numLaps;

  QuickRaceMode(const std::string& track_, 
                const std::vector<std::string>& players_, 
                RaceDifficulty difficulty_,
                int numKarts, int numLaps);
  virtual ~QuickRaceMode() {}

  void start();
  void next();
};

class TimeTrialMode : public RaceMode
{
public:
  std::string track;
  std::string kart;
  int numLaps;

  TimeTrialMode(const std::string& track_, const std::string& kart_,
                const int& numLaps_);
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
  RaceMode*                        mode;
  RaceDifficulty                   difficulty;
  RaceSetup::RaceMode              race_mode;
  typedef std::vector<std::string> Players;
  Players                          players;
  std::string                      track;
  int                              numLaps;
  int                              numKarts;
  int                              numFinishedKarts;

public:
  RaceManager();

  RaceSetup::RaceMode getRaceMode() const { return race_mode; }

  void setNumKarts(int num) { numKarts = num; }
  int  getNumKarts() const   { return numKarts; }
  void setNumPlayers(int num);
  int  getNumPlayers() const { return players.size(); }
  void setNumLaps(int num) { numLaps = num; }
  int  getNumLaps() const    { return numLaps; }
  void setTrack(const std::string& track_) { track = track_; }
  void setRaceMode(RaceSetup::RaceMode mode) { race_mode = mode; }
  void setDifficulty(RaceDifficulty difficulty_) { difficulty = difficulty_; }
  void setPlayerKart(int player, const std::string& kart);
  unsigned int getFinishedKarts() const { return numFinishedKarts; }
  void addFinishedKarts(int num) { numFinishedKarts += num; }
  int  getKartScore(int kart) const { return mode->getKartScore(kart); }
  void setKartScore(int kart, int pos) { mode->setKartScore(kart, pos); }

  void setMirror() {/*FIXME*/}
  void setReverse(){/*FIXME*/}
  void start();

  /** Start the next race or go back to the start screen, depending on
      the currently set game mode */
  void next();
};

extern RaceManager *race_manager;
#endif

/* EOF */
