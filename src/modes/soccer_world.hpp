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
#include "tracks/check_goal.hpp"
#include "tracks/check_manager.hpp"

#include <IMesh.h>
#include <string>

class AbstractKart;
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

    class BallGoalData
    {
    // These data are used by AI to determine ball aiming angle
    private:
        // Radius of the ball
        float m_radius;

        // Slope of the line from ball to the center point of goals
        float m_red_goal_slope;
        float m_blue_goal_slope;

        // The transform only takes the ball heading into account,
        // ie no hpr of ball which allowing setting aim point easier
        btTransform m_trans;

        // Two goals
        CheckGoal* m_blue_check_goal;
        CheckGoal* m_red_check_goal;

        // Location to red/blue goal points from the ball heading point of view
        Vec3 m_red_goal_1;
        Vec3 m_red_goal_2;
        Vec3 m_red_goal_3;
        Vec3 m_blue_goal_1;
        Vec3 m_blue_goal_2;
        Vec3 m_blue_goal_3;
    public:
        void reset()
        {
            m_red_goal_1 = Vec3(0, 0, 0);
            m_red_goal_2 = Vec3(0, 0, 0);
            m_red_goal_3 = Vec3(0, 0, 0);
            m_blue_goal_1 = Vec3(0, 0, 0);
            m_blue_goal_2 = Vec3(0, 0, 0);
            m_blue_goal_3 = Vec3(0, 0, 0);
            m_red_goal_slope = 1.0f;
            m_blue_goal_slope = 1.0f;
            m_trans = btTransform(btQuaternion(0, 0, 0, 1), Vec3(0, 0, 0));
        }   // reset

        float getDiameter() const
        {
            return m_radius * 2;
        }   // getDiameter

        void init(float ball_radius)
        {
            m_radius = ball_radius;
            assert(m_radius > 0.0f);

            // Save two goals
            unsigned int n = CheckManager::get()->getCheckStructureCount();
            for (unsigned int i = 0; i < n; i++)
            {
                CheckGoal* goal = dynamic_cast<CheckGoal*>
                    (CheckManager::get()->getCheckStructure(i));
                if (goal)
                {
                    if (goal->getTeam())
                        m_blue_check_goal = goal;
                    else
                        m_red_check_goal = goal;
                }
            }
            if (m_blue_check_goal == NULL || m_red_check_goal == NULL)
            {
                Log::error("SoccerWorld", "Goal(s) is missing!");
            }
        }   // init

        void updateBallAndGoal(const Vec3& ball_pos, float heading)
        {
            btQuaternion quat(Vec3(0, 1, 0), -heading);
            m_trans = btTransform(btQuaternion(Vec3(0, 1, 0), heading),
                ball_pos);

            // Red goal
            m_red_goal_1 = quatRotate(quat, m_red_check_goal
                ->getPoint(CheckGoal::POINT_FIRST) - ball_pos);
            m_red_goal_2 = quatRotate(quat, m_red_check_goal
                ->getPoint(CheckGoal::POINT_CENTER) - ball_pos);
            m_red_goal_3 = quatRotate(quat, m_red_check_goal
                ->getPoint(CheckGoal::POINT_LAST) - ball_pos);

            // Blue goal
            m_blue_goal_1 = quatRotate(quat, m_blue_check_goal
                ->getPoint(CheckGoal::POINT_FIRST) - ball_pos);
            m_blue_goal_2 = quatRotate(quat, m_blue_check_goal
                ->getPoint(CheckGoal::POINT_CENTER) - ball_pos);
            m_blue_goal_3 = quatRotate(quat, m_blue_check_goal
                ->getPoint(CheckGoal::POINT_LAST) - ball_pos);

            // Update the slope:
            // Use y = mx + c as an equation from goal center to ball
            // As the line always intercept in (0,0) which is the ball location,
            // so y(z)/x is the slope , it is used for determine aiming position
            // of ball later
            m_red_goal_slope = m_red_goal_2.z() / m_red_goal_2.x();
            m_blue_goal_slope = m_blue_goal_2.z() / m_blue_goal_2.x();
        }   // updateBallAndGoal

        bool isApproachingGoal(KartTeam team) const
        {
            // If the ball lies between the first and last pos, and faces
            // in front of either of them, (inside angular size of goal)
            // than it's likely to goal
            if (team == KART_TEAM_BLUE)
            {
                if ((m_blue_goal_1.z() > 0.0f || m_blue_goal_3.z() > 0.0f) &&
                    ((m_blue_goal_1.x() < 0.0f && m_blue_goal_3.x() > 0.0f) ||
                    (m_blue_goal_3.x() < 0.0f && m_blue_goal_1.x() > 0.0f)))
                    return true;
            }
            else
            {
                if ((m_red_goal_1.z() > 0.0f || m_red_goal_3.z() > 0.0f) &&
                    ((m_red_goal_1.x() < 0.0f && m_red_goal_3.x() > 0.0f) ||
                    (m_red_goal_3.x() < 0.0f && m_red_goal_1.x() > 0.0f)))
                    return true;
            }
            return false;
        }   // isApproachingGoal

        Vec3 getAimPosition(KartTeam team, bool reverse) const
        {
            // If it's likely to goal already, aim the ball straight behind
            // should do the job
            if (isApproachingGoal(team))
                return m_trans(Vec3(0, 0, reverse ? m_radius*2 : -m_radius*2));

            // Otherwise do the below:
            // This is done by using Pythagorean Theorem and solving the
            // equation from ball to goal center (y = (m_***_goal_slope) x)

            // We aim behind the ball from the center of the ball to its
            // diameter, so 2*m_radius = sqrt (x2 + y2),
            // which is next x = sqrt (2*m_radius - y2)
            // And than we have x = y / m(m_***_goal_slope)
            // After put that in the slope equation, we have
            // y = sqrt(2*m_radius*m2 / (1+m2))
            float x = 0.0f;
            float y = 0.0f;
            if (team == KART_TEAM_BLUE)
            {
                y = sqrt((m_blue_goal_slope * m_blue_goal_slope * m_radius*2) /
                    (1 + (m_blue_goal_slope * m_blue_goal_slope)));
                if (m_blue_goal_2.x() == 0.0f ||
                    (m_blue_goal_2.x() > 0.0f && m_blue_goal_2.z() > 0.0f) ||
                    (m_blue_goal_2.x() < 0.0f && m_blue_goal_2.z() > 0.0f))
                {
                    // Determine when y should be negative
                    y = -y;
                }
                x = y / m_blue_goal_slope;
            }
            else
            {
                y = sqrt((m_red_goal_slope * m_red_goal_slope * m_radius*2) /
                    (1 + (m_red_goal_slope * m_red_goal_slope)));
                if (m_red_goal_2.x() == 0.0f ||
                    (m_red_goal_2.x() > 0.0f && m_red_goal_2.z() > 0.0f) ||
                    (m_red_goal_2.x() < 0.0f && m_red_goal_2.z() > 0.0f))
                {
                    y = -y;
                }
                x = y / m_red_goal_slope;
            }
            assert (!std::isnan(x));
            assert (!std::isnan(y));
            // Return the world coordinates
            return (reverse ? m_trans(Vec3(-x, 0, -y)) :
                m_trans(Vec3(x, 0, y)));
        }   // getAimPosition
        void resetCheckGoal(const Track* t)
        {
            m_red_check_goal->reset(*t);
            m_blue_check_goal->reset(*t);
        }
    };   // BallGoalData

    std::vector<KartDistanceMap> m_red_kdm;
    std::vector<KartDistanceMap> m_blue_kdm;
    BallGoalData m_bgd;

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
               std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE {}

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
    float getBallDiameter() const
                                               { return m_bgd.getDiameter(); }
    // ------------------------------------------------------------------------
    bool ballApproachingGoal(KartTeam team) const
                                     { return m_bgd.isApproachingGoal(team); }
    // ------------------------------------------------------------------------
    Vec3 getBallAimPosition(KartTeam team, bool reverse = false) const
                               { return m_bgd.getAimPosition(team, reverse); }
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
        if (race_manager->hasTimeTarget())
        {
            progress.first = (uint32_t)m_time;
        }
        else if (m_red_scorers.size() > m_blue_scorers.size())
        {
            progress.second = (uint32_t)((float)m_red_scorers.size() /
                (float)race_manager->getMaxGoal() * 100.0f);
        }
        else
        {
            progress.second = (uint32_t)((float)m_blue_scorers.size() /
                (float)race_manager->getMaxGoal() * 100.0f);
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
};   // SoccerWorld


#endif
