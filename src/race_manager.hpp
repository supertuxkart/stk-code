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

#ifndef HEADER_RACEMANAGER_H
#define HEADER_RACEMANAGER_H

#include <vector>
#include <string>

#include "race_setup.hpp"
#include "cup_data.hpp"

class RaceMode
{
public:
    virtual ~RaceMode() {};       // avoid compiler warning
    virtual void start()    = 0;  // Start the mode and go into the first race
    virtual void next ();         // go to the next race if there is any, otherwise main menu
    virtual void exit_race ();    // go directly to main menu

    virtual void addKartScore    (int kart, int pos) {}
    virtual int  getKartScore    (int kart) const {return 0;}
    virtual std::string getKartName(int kart) const { return "";}
    virtual int  getPositionScore(int pos ) const {return 0;}
};

class GrandPrixMode : public RaceMode
{
private:
    void start_race(int n);

    RaceDifficulty m_difficulty;
    int m_num_karts;

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
        player(player_) {}}
    ;

public:
    std::vector<std::string> m_players;
    CupData m_cup;
    std::vector<KartStatus> m_karts;
    int m_track;

    GrandPrixMode(const std::vector<std::string>& players_,
                  const CupData& cup_,
                  RaceDifficulty difficulty_,
                  int numKarts);
    virtual ~GrandPrixMode() {}

    void start();
    void next();
    void exit_race();

    int  getKartScore    (int kart) const    { return m_karts[kart].score;}
    int  getPositionScore(int pos)  const    { return pos>4 ? 0 : 4-pos;}
    std::string getKartName(int kart) const  { return m_karts[kart].ident;}
    void addKartScore    (int kart, int pos)
    {
        m_karts[kart].score +=
            getPositionScore(pos);
    }
};

class QuickRaceMode : public RaceMode
{
public:
    std::string m_track;
    std::vector<std::string> m_players;
    RaceDifficulty m_difficulty;
    int m_num_karts, m_num_laps;

    QuickRaceMode(const std::string& track_,
                  const std::vector<std::string>& players_,
                  RaceDifficulty difficulty_,
                  int numKarts, int numLaps);
    virtual ~QuickRaceMode() {}

    void start();
};

class TimeTrialMode : public RaceMode
{
public:
    std::string m_track;
    std::string m_kart;
    int m_num_laps;

    TimeTrialMode(const std::string& track_, const std::string& kart_,
                  const int& numLaps_);
    virtual ~TimeTrialMode() {}

    void start();
};

/** RaceManager keeps track of the game mode, number of players and
    such, the GUI calls the RaceManager to setup and start a race,
    grandprix or similar stuff */
class RaceManager
{
private:
    RaceMode*                        m_mode;
    RaceDifficulty                   m_difficulty;
    RaceSetup::RaceMode              m_race_mode;
    typedef std::vector<std::string> Players;
    Players                          m_players;
    std::string                      m_track;
    CupData                          m_cup;
    int                              m_num_laps;
    int                              m_num_karts;
    unsigned int                     m_num_finished_karts;
    unsigned int                     m_num_finished_players;
public:
    bool                             m_active_race; //True if there is a race

public:
    RaceManager();
    ~RaceManager();

    RaceSetup::RaceMode getRaceMode() const { return m_race_mode; }

    void setPlayerKart(int player, const std::string& kart);
    void setNumPlayers(int num);
    void reset();
    void setGrandPrix(CupData *cup_)           { m_cup = *cup_;                  }
    void setDifficulty(RaceDifficulty diff_)   { m_difficulty = diff_;           }
    void setNumLaps(int num)                   { m_num_laps = num;                }
    void setTrack(const std::string& track_)   { m_track = track_;               }
    void setRaceMode(RaceSetup::RaceMode mode) { m_race_mode = mode;             }
    void setNumKarts(int num)                  { m_num_karts = num;            }
    int  getNumKarts()              const { return m_num_karts;                }
    int  getNumPlayers()            const { return (int)m_players.size();        }
    int  getNumLaps()               const { return m_num_laps;                    }
    CupData *getGrandPrix()               { return &m_cup;                       }
    unsigned int getFinishedKarts() const { return m_num_finished_karts;           }
    unsigned int getFinishedPlayers() const { return m_num_finished_players;   }
    int  getKartScore(int kart   )  const { return m_mode->getKartScore(kart);   }
    int  getPositionScore(int pos)  const { return m_mode->getPositionScore(pos);}
    std::string getKartName(int kart) const { return m_mode->getKartName(kart);  }
    void addKartScore(int kart, int pos)  { m_mode->addKartScore(kart, pos);     }
    void addFinishedKarts(int num)        { m_num_finished_karts += num;           }
    void PlayerFinishes()                 { m_num_finished_players++;          }
    int  allPlayerFinished()        {return m_num_finished_players==m_players.size();}
    int  raceIsActive()                   {return m_active_race;}

    void setMirror() {/*FIXME*/}
    void setReverse(){/*FIXME*/}

    void start(); // start a new race
    void next(); // start the next race or go back to the start screen
    void exit_race();
};

extern RaceManager *race_manager;
#endif

/* EOF */
