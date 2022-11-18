//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#ifndef SOCCER_WORLD_HPP
#define SOCCER_WORLD_HPP

#include "modes/world_with_rank.hpp"
#include "states_screens/race_gui_base.hpp"
#include "karts/abstract_kart.hpp"

#include <string>

class AbstractKart;
class BallGoalData;
class Controller;
class NetworkString;
class TrackObject;
class TrackSector;

/** \brief An implementation of WorldWithRank, to provide the soccer game mode
 *  Notice: In soccer world, true goal means blue, false means red.
 * \ingroup modes
 */
class SoccerWorld : public WorldWithRank
{
public:
    struct ScorerData
    {
        /** World ID of kart which scores. */
        unsigned int  m_id;
        /** Whether this goal is socred correctly (identify for own goal). */
        bool          m_correct_goal;
        /** Time goal. */
        float         m_time;
        /** Kart ident which scores. */
        std::string   m_kart;
        /** Player name which scores. */
        core::stringw m_player;
        /** Country code of player. */
        std::string m_country_code;
        /** Handicap of player. */
        HandicapLevel m_handicap_level;
    };   // ScorerData

private:
    class KartDistanceMap
    {
    public:
        /** World ID of kart. */
        unsigned int    m_kart_id;
        /** Distance to ball from kart */
        float           m_distance;

        bool operator < (const KartDistanceMap& r) const
        {
            return m_distance < r.m_distance;
        }
        KartDistanceMap(unsigned int kart_id = 0, float distance = 0.0f)
        {
            m_kart_id = kart_id;
            m_distance = distance;
        }
    };   // KartDistanceMap

    std::vector<KartDistanceMap> m_red_kdm;
    std::vector<KartDistanceMap> m_blue_kdm;
    std::unique_ptr<BallGoalData> m_bgd;

    /** Keep a pointer to the track object of soccer ball */
    TrackObject* m_ball;
    btRigidBody* m_ball_body;

    /** Number of goals needed to win */
    int m_goal_target;
    bool m_count_down_reached_zero;

    SFXBase *m_goal_sound;

    /** Counts ticks when the ball is off track, so a reset can be
     *  triggered if the ball is off for more than 2 seconds. */
    int m_ball_invalid_timer;
    int m_ball_hitter;

    /** Goals data of each team scored */
    std::vector<ScorerData> m_red_scorers;
    std::vector<ScorerData> m_blue_scorers;

    /** Data generated from navmesh */
    TrackSector* m_ball_track_sector;

    float m_ball_heading;

    std::vector<int> m_team_icon_draw_id;

    std::vector<btTransform> m_goal_transforms;
    /** Function to update the location the ball on the polygon map */
    void updateBallPosition(int ticks);
    /** Function to update data for AI usage. */
    void updateAIData();
    /** Get number of teammates in a team, used by starting position assign. */
    int getTeamNum(KartTeam team) const;

    /** Profiling usage */
    int m_frame_count;
    std::vector<int> m_goal_frame;

    int m_reset_ball_ticks;
    int m_ticks_back_to_own_goal;

    void resetKartsToSelfGoals();

public:

    SoccerWorld();
    virtual ~SoccerWorld();

    virtual void init() OVERRIDE;
    virtual void onGo() OVERRIDE;

    // clock events
    virtual bool isRaceOver() OVERRIDE;
    virtual void countdownReachedZero() OVERRIDE;
    virtual void terminateRace() OVERRIDE;

    // overriding World methods
    virtual void reset(bool restart=false) OVERRIDE;

    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) OVERRIDE;
    virtual btTransform getRescueTransform(unsigned int rescue_pos) const
        OVERRIDE;
    virtual bool useFastMusicNearEnd() const OVERRIDE { return false; }
    virtual void getKartsDisplayInfo(
               std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE;

    virtual bool raceHasLaps() OVERRIDE { return false; }

    virtual void enterRaceOverState() OVERRIDE;

    virtual const std::string& getIdent() const OVERRIDE;

    virtual void update(int ticks) OVERRIDE;

    bool shouldDrawTimer() const OVERRIDE { return !isStartPhase(); }
    // ------------------------------------------------------------------------
    void onCheckGoalTriggered(bool first_goal);
    // ------------------------------------------------------------------------
    void setBallHitter(unsigned int kart_id);
    // ------------------------------------------------------------------------
    /** Get the soccer result of kart in soccer world (including AIs) */
    bool getKartSoccerResult(unsigned int kart_id) const;
    // ------------------------------------------------------------------------
    int getScore(KartTeam team) const
    {
        return (int)(team == KART_TEAM_BLUE ? m_blue_scorers.size()
                                              : m_red_scorers.size());
    }
    // ------------------------------------------------------------------------
    const std::vector<ScorerData>& getScorers(KartTeam team) const
       { return (team == KART_TEAM_BLUE ? m_blue_scorers : m_red_scorers); }
    // ------------------------------------------------------------------------
    int getBallNode() const;
    // ------------------------------------------------------------------------
    const Vec3& getBallPosition() const
        { return (Vec3&)m_ball_body->getCenterOfMassTransform().getOrigin(); }
    // ------------------------------------------------------------------------
    bool ballNotMoving() const
    {
        return (m_ball_body->getLinearVelocity().x() == 0.0f ||
            m_ball_body->getLinearVelocity().z() == 0.0f);
    }
    // ------------------------------------------------------------------------
    float getBallHeading() const
                                                    { return m_ball_heading; }
    // ------------------------------------------------------------------------
    float getBallDiameter() const;
    // ------------------------------------------------------------------------
    bool ballApproachingGoal(KartTeam team) const;
    // ------------------------------------------------------------------------
    Vec3 getBallAimPosition(KartTeam team, bool reverse = false) const;
    // ------------------------------------------------------------------------
    bool isCorrectGoal(unsigned int kart_id, bool first_goal) const;
    // ------------------------------------------------------------------------
    int getBallChaser(KartTeam team) const
    {
        // Only AI call this function, so each team should have at least a kart
        assert(m_blue_kdm.size() > 0 && m_red_kdm.size() > 0);
        return (team == KART_TEAM_BLUE ? m_blue_kdm[0].m_kart_id :
            m_red_kdm[0].m_kart_id);
    }
    // ------------------------------------------------------------------------
    /** Get the AI who will attack the other team ball chaser. */
    int getAttacker(KartTeam team) const;
    // ------------------------------------------------------------------------
    void handlePlayerGoalFromServer(const NetworkString& ns);
    // ------------------------------------------------------------------------
    void handleResetBallFromServer(const NetworkString& ns);
    // ------------------------------------------------------------------------
    virtual bool hasTeam() const OVERRIDE                      { return true; }
    // ------------------------------------------------------------------------
    virtual std::pair<uint32_t, uint32_t> getGameStartedProgress() const
        OVERRIDE
    {
        std::pair<uint32_t, uint32_t> progress(
            std::numeric_limits<uint32_t>::max(),
            std::numeric_limits<uint32_t>::max());
        if (RaceManager::get()->hasTimeTarget())
        {
            progress.first = (uint32_t)m_time;
        }
        else if (m_red_scorers.size() > m_blue_scorers.size())
        {
            progress.second = (uint32_t)((float)m_red_scorers.size() /
                (float)RaceManager::get()->getMaxGoal() * 100.0f);
        }
        else
        {
            progress.second = (uint32_t)((float)m_blue_scorers.size() /
                (float)RaceManager::get()->getMaxGoal() * 100.0f);
        }
        return progress;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns,
                                   STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool isGoalPhase() const OVERRIDE
    {
        int diff = m_ticks_back_to_own_goal - getTicksSinceStart();
        return diff > 0 && diff < stk_config->time2Ticks(3.0f);
    }
    // ------------------------------------------------------------------------
    AbstractKart* getKartAtDrawingPosition(unsigned int p) const OVERRIDE
                                { return getKart(m_team_icon_draw_id[p - 1]); }
    // ------------------------------------------------------------------------
    TrackObject* getBall() const { return m_ball; }
};   // SoccerWorld


#endif
