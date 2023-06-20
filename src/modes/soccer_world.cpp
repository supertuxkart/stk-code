//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "modes/soccer_world.hpp"

#include "main_loop.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/abstract_kart_animation.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "karts/controller/network_player_controller.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "physics/physics.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/check_goal.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/graph.hpp"
#include "tracks/quad.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "tracks/track_sector.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <IMeshSceneNode.h>
#include <numeric>
#include <string>

//=============================================================================
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
        CheckManager* cm = Track::getCurrentTrack()->getCheckManager();
        unsigned int n = cm->getCheckStructureCount();
        for (unsigned int i = 0; i < n; i++)
        {
            CheckGoal* goal = dynamic_cast<CheckGoal*>
                (cm->getCheckStructure(i));
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

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
SoccerWorld::SoccerWorld() : WorldWithRank()
{
    if (RaceManager::get()->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            RaceManager::get()->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }

    m_frame_count = 0;
    m_use_highscores = false;
    m_red_ai = 0;
    m_blue_ai = 0;
    m_ball_track_sector = NULL;
    m_bgd.reset(new BallGoalData());
}   // SoccerWorld

//-----------------------------------------------------------------------------
/** The destructor frees all data structures.
 */
SoccerWorld::~SoccerWorld()
{
    m_goal_sound->deleteSFX();

    delete m_ball_track_sector;
    m_ball_track_sector = NULL;
}   // ~SoccerWorld

//-----------------------------------------------------------------------------
/** Initializes the soccer world. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void SoccerWorld::init()
{
    m_kart_team_map.clear();
    m_kart_position_map.clear();
    WorldWithRank::init();
    m_display_rank = false;
    m_ball_hitter  = -1;
    m_ball         = NULL;
    m_ball_body    = NULL;
    m_goal_target  = RaceManager::get()->getMaxGoal();
    m_goal_sound   = SFXManager::get()->createSoundSource("goal_scored");

    Track *track = Track::getCurrentTrack();
    if (track->hasNavMesh())
    {
        // Init track sector for ball if navmesh is found
        m_ball_track_sector = new TrackSector();
    }

    TrackObjectManager* tom = track->getTrackObjectManager();
    assert(tom);
    PtrVector<TrackObject>& objects = tom->getObjects();
    for (unsigned int i = 0; i < objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if(!obj->isSoccerBall())
            continue;
        m_ball = obj;
        m_ball_body = m_ball->getPhysicalObject()->getBody();
        // Handle one ball only
        break;
    }
    if (!m_ball)
        Log::fatal("SoccerWorld","Ball is missing in soccer field, abort.");

    float radius = m_ball->getPhysicalObject()->getRadius();
    if (radius <= 0.0f)
    {
        btVector3 min, max;
        m_ball->getPhysicalObject()->getBody()->getAabb(min, max);
        radius = (max.y() - min.y()) / 2.0f;
    }
    m_bgd->init(radius);

}   // init

//-----------------------------------------------------------------------------
/** Called when a soccer game is restarted.
 */
void SoccerWorld::reset(bool restart)
{
    WorldWithRank::reset(restart);
    if (RaceManager::get()->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            RaceManager::get()->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }

    m_count_down_reached_zero = false;
    m_red_scorers.clear();
    m_blue_scorers.clear();
    m_ball_hitter = -1;
    m_red_kdm.clear();
    m_blue_kdm.clear();
    m_ball_heading = 0.0f;
    m_ball_invalid_timer = 0;
    m_goal_transforms.clear();
    m_goal_transforms.resize(m_karts.size());
    if (m_goal_sound != NULL &&
        m_goal_sound->getStatus() == SFXBase::SFX_PLAYING)
    {
        m_goal_sound->stop();
    }

    if (Track::getCurrentTrack()->hasNavMesh())
    {
        m_ball_track_sector->reset();
    }

    m_reset_ball_ticks = -1;
    m_ball->reset();
    m_bgd->reset();
    m_ticks_back_to_own_goal = -1;
    m_ball->setEnabled(false);

    // Make the player kart in profiling mode up
    // ie make this kart less likely to affect gaming result
    if (UserConfigParams::m_arena_ai_stats)
        getKart(8)->flyUp();

    // Team will remain unchanged even with live join
    std::vector<int> red_id, blue_id;
    for (unsigned int i = 0; i < m_karts.size(); i++)
    {
        if (getKartTeam(i) == KART_TEAM_RED)
            red_id.push_back(i);
        else
            blue_id.push_back(i);
    }

    m_team_icon_draw_id.resize(getNumKarts());
    if (!Track::getCurrentTrack()->getMinimapInvert())
        std::swap(red_id, blue_id);
    int pos = 0;
    for (int id : red_id)
        m_team_icon_draw_id[pos++] = id;
    for (int id : blue_id)
        m_team_icon_draw_id[pos++] = id;
}   // reset

//-----------------------------------------------------------------------------
void SoccerWorld::onGo()
{
    m_ball->setEnabled(true);
    m_ball->reset();
    WorldWithRank::onGo();
}   // onGo

//-----------------------------------------------------------------------------
void SoccerWorld::terminateRace()
{
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount ; i++)
    {
        // Soccer mode use goal for race result, and each goal time is
        // handled by handlePlayerGoalFromServer already
        m_karts[i]->finishedRace(0.0f, true/*from_server*/);
    }   // i<kart_amount
    WorldWithRank::terminateRace();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& SoccerWorld::getIdent() const
{
    return IDENT_SOCCER;
}   // getIdent

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param ticks Physics time steps - should be 1.
 */
void SoccerWorld::update(int ticks)
{
    updateBallPosition(ticks);
    if (Track::getCurrentTrack()->hasNavMesh())
    {
        updateSectorForKarts();
        if (!NetworkConfig::get()->isNetworking())
            updateAIData();
    }

    WorldWithRank::update(ticks);
    WorldWithRank::updateTrack(ticks);

    if (isGoalPhase())
    {
        for (unsigned int i = 0; i < m_karts.size(); i++)
        {
            auto& kart = m_karts[i];
            if (kart->isEliminated())
                continue;
            if (kart->getKartAnimation())
            {
                AbstractKartAnimation* ka = kart->getKartAnimation();
                kart->setKartAnimation(NULL);
                delete ka;
            }
            kart->getBody()->setLinearVelocity(Vec3(0.0f));
            kart->getBody()->setAngularVelocity(Vec3(0.0f));
            kart->getBody()->proceedToTransform(m_goal_transforms[i]);
            kart->setTrans(m_goal_transforms[i]);
        }
        if (m_ticks_back_to_own_goal - getTicksSinceStart() == 1 &&
            !isRaceOver())
        {
            // Reset all karts and ball
            resetKartsToSelfGoals();
            if (UserConfigParams::m_arena_ai_stats)
                getKart(8)->flyUp();
        }
    }
    if (UserConfigParams::m_arena_ai_stats)
        m_frame_count++;

    // FIXME before next release always update soccer kart position to fix
    // powerup random number
    /*beginSetKartPositions();
    int pos = 1;
    for (unsigned i = 0; i < getNumKarts(); i++)
    {
        if (getKart(i)->isEliminated())
            continue;
        setKartPosition(i, pos++);
    }
    for (unsigned i = 0; i < getNumKarts(); i++)
    {
        if (!getKart(i)->isEliminated())
            continue;
        setKartPosition(i, pos++);
    }
    endSetKartPositions();*/
}   // update

//-----------------------------------------------------------------------------
void SoccerWorld::onCheckGoalTriggered(bool first_goal)
{
    if (isRaceOver() || isStartPhase() ||
        (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient()))
        return;

    if (getTicksSinceStart() < m_ticks_back_to_own_goal)
        return;
    m_ticks_back_to_own_goal = getTicksSinceStart() +
        stk_config->time2Ticks(3.0f);
    m_goal_sound->play();
    m_ball->reset();
    m_ball->setEnabled(false);
    if (m_ball_hitter != -1)
    {
        if (UserConfigParams::m_arena_ai_stats)
        {
            const int elapsed_frame = m_goal_frame.empty() ? 0 :
                std::accumulate(m_goal_frame.begin(), m_goal_frame.end(), 0);
            m_goal_frame.push_back(m_frame_count - elapsed_frame);
        }

        ScorerData sd = {};
        sd.m_id = m_ball_hitter;
        sd.m_correct_goal = isCorrectGoal(m_ball_hitter, first_goal);
        sd.m_kart = getKart(m_ball_hitter)->getIdent();
        sd.m_player = getKart(m_ball_hitter)->getController()
            ->getName(false/*include_handicap_string*/);
        sd.m_handicap_level = getKart(m_ball_hitter)->getHandicap();
        if (RaceManager::get()->getKartGlobalPlayerId(m_ball_hitter) > -1)
        {
            sd.m_country_code =
                RaceManager::get()->getKartInfo(m_ball_hitter).getCountryCode();
        }
        if (sd.m_correct_goal)
        {
            m_karts[m_ball_hitter]->getKartModel()
                ->setAnimation(KartModel::AF_WIN_START, true/* play_non_loop*/);
        }
        else if (!sd.m_correct_goal)
        {
            m_karts[m_ball_hitter]->getKartModel()
                ->setAnimation(KartModel::AF_LOSE_START, true/* play_non_loop*/);
        }

#ifndef SERVER_ONLY
        // show a message once a goal is made
        core::stringw msg;
        if (sd.m_correct_goal)
            msg = _("%s scored a goal!", sd.m_player);
        else
            msg = _("Oops, %s made an own goal!", sd.m_player);
        if (m_race_gui)
        {
            m_race_gui->addMessage(msg, NULL, 3.0f,
                video::SColor(255, 255, 0, 255), /*important*/true,
                /*big_font*/false, /*outline*/true);
        }
#endif

        if (first_goal)
        {
            if (RaceManager::get()->hasTimeTarget())
            {
                sd.m_time = RaceManager::get()->getTimeTarget() - getTime();
            }
            else
                sd.m_time = getTime();
            // Notice: true first_goal means it's blue goal being shoot,
            // so red team can score
            m_red_scorers.push_back(sd);
        }
        else
        {
            if (RaceManager::get()->hasTimeTarget())
            {
                sd.m_time = RaceManager::get()->getTimeTarget() - getTime();
            }
            else
                sd.m_time = getTime();
            m_blue_scorers.push_back(sd);
        }
        if (NetworkConfig::get()->isNetworking() &&
            NetworkConfig::get()->isServer())
        {
            NetworkString p(PROTOCOL_GAME_EVENTS);
            p.setSynchronous(true);
            p.addUInt8(GameEventsProtocol::GE_PLAYER_GOAL)
                .addUInt8((uint8_t)sd.m_id).addUInt8(sd.m_correct_goal)
                .addUInt8(first_goal).addFloat(sd.m_time)
                .addTime(m_ticks_back_to_own_goal)
                .encodeString(sd.m_kart).encodeString(sd.m_player);
            // Added in 1.1, add missing handicap info and country code
            NetworkString p_1_1 = p;
            p_1_1.encodeString(sd.m_country_code)
                .addUInt8(sd.m_handicap_level);
            auto peers = STKHost::get()->getPeers();
            for (auto& peer : peers)
            {
                if (peer->isValidated() && !peer->isWaitingForGame())
                {
                    if (peer->getClientCapabilities().find("soccer_fixes") !=
                        peer->getClientCapabilities().end())
                    {
                        peer->sendPacket(&p_1_1, true/*reliable*/);
                    }
                    else
                    {
                        peer->sendPacket(&p, true/*reliable*/);
                    }
                }
            }
        }
    }
    for (unsigned i = 0; i < m_karts.size(); i++)
    {
        auto& kart = m_karts[i];
        kart->getBody()->setLinearVelocity(Vec3(0.0f));
        kart->getBody()->setAngularVelocity(Vec3(0.0f));
        m_goal_transforms[i] = kart->getBody()->getWorldTransform();
    }
}   // onCheckGoalTriggered

//-----------------------------------------------------------------------------
void SoccerWorld::handleResetBallFromServer(const NetworkString& ns)
{
    int ticks_now = World::getWorld()->getTicksSinceStart();
    int ticks_back_to_own_goal = ns.getTime();
    if (ticks_now >= ticks_back_to_own_goal)
    {
        Log::warn("SoccerWorld", "Server ticks %d is too close to client ticks "
            "%d when reset player", ticks_back_to_own_goal, ticks_now);
        return;
    }
    m_reset_ball_ticks = ticks_back_to_own_goal;
}   // handleResetBallFromServer

//-----------------------------------------------------------------------------
void SoccerWorld::handlePlayerGoalFromServer(const NetworkString& ns)
{
    ScorerData sd = {};
    sd.m_id = ns.getUInt8();
    sd.m_correct_goal = ns.getUInt8() == 1;
    bool first_goal = ns.getUInt8() == 1;
    sd.m_time = ns.getFloat();
    int ticks_now = World::getWorld()->getTicksSinceStart();
    int ticks_back_to_own_goal = ns.getTime();
    ns.decodeString(&sd.m_kart);
    ns.decodeStringW(&sd.m_player);
    // Added in 1.1, add missing handicap info and country code
    if (NetworkConfig::get()->getServerCapabilities().find("soccer_fixes")
        != NetworkConfig::get()->getServerCapabilities().end())
    {
        ns.decodeString(&sd.m_country_code);
        sd.m_handicap_level = (HandicapLevel)ns.getUInt8();
    }

    if (first_goal)
    {
        m_red_scorers.push_back(sd);
    }
    else
    {
        m_blue_scorers.push_back(sd);
    }

    if (ticks_now >= ticks_back_to_own_goal && !isStartPhase())
    {
        Log::warn("SoccerWorld", "Server ticks %d is too close to client ticks "
            "%d when goal", ticks_back_to_own_goal, ticks_now);
        return;
    }

    // show a message once a goal is made
    core::stringw msg;
    if (sd.m_correct_goal)
        msg = _("%s scored a goal!", sd.m_player);
    else
        msg = _("Oops, %s made an own goal!", sd.m_player);
    float time = stk_config->ticks2Time(ticks_back_to_own_goal - ticks_now);
    // May happen if this message is added when spectate started
    if (time > 3.0f)
        time = 3.0f;
    if (m_race_gui && !isStartPhase())
    {
        m_race_gui->addMessage(msg, NULL, time,
            video::SColor(255, 255, 0, 255), /*important*/true,
            /*big_font*/false, /*outline*/true);
    }

    m_ticks_back_to_own_goal = ticks_back_to_own_goal;
    for (unsigned i = 0; i < m_karts.size(); i++)
    {
        auto& kart = m_karts[i];
        btTransform transform_now = kart->getBody()->getWorldTransform();
        kart->getBody()->setLinearVelocity(Vec3(0.0f));
        kart->getBody()->setAngularVelocity(Vec3(0.0f));
        kart->getBody()->proceedToTransform(transform_now);
        kart->setTrans(transform_now);
        m_goal_transforms[i] = transform_now;
    }
    m_ball->reset();
    m_ball->setEnabled(false);

    // Ignore the rest in live join
    if (isStartPhase())
        return;

    if (sd.m_correct_goal)
    {
        m_karts[sd.m_id]->getKartModel()
            ->setAnimation(KartModel::AF_WIN_START, true/* play_non_loop*/);
    }
    else if (!sd.m_correct_goal)
    {
        m_karts[sd.m_id]->getKartModel()
            ->setAnimation(KartModel::AF_LOSE_START, true/* play_non_loop*/);
    }
    m_goal_sound->play();

}   // handlePlayerGoalFromServer

//-----------------------------------------------------------------------------
void SoccerWorld::resetKartsToSelfGoals()
{
    m_ball->setEnabled(true);
    m_ball->reset();
    m_bgd->resetCheckGoal(Track::getCurrentTrack());
    for (unsigned i = 0; i < m_karts.size(); i++)
    {
        auto& kart = m_karts[i];
        if (kart->isEliminated())
            continue;

        if (kart->getKartAnimation())
        {
            AbstractKartAnimation* ka = kart->getKartAnimation();
            kart->setKartAnimation(NULL);
            delete ka;
        }
        kart->getBody()->setLinearVelocity(Vec3(0.0f));
        kart->getBody()->setAngularVelocity(Vec3(0.0f));
        unsigned index = m_kart_position_map.at(kart->getWorldKartId());
        btTransform t = Track::getCurrentTrack()->getStartTransform(index);
        moveKartTo(kart.get(), t);
    }
}   // resetKartsToSelfGoals

//-----------------------------------------------------------------------------
/** Sets the last kart that hit the ball, to be able to
 * identify the scorer later.
 */
void SoccerWorld::setBallHitter(unsigned int kart_id)
{
    m_ball_hitter = kart_id;
}   // setBallHitter

//-----------------------------------------------------------------------------
/** The soccer game is over if time up or either team wins.
 */
bool SoccerWorld::isRaceOver()
{
    if (m_unfair_team)
        return true;

    if (RaceManager::get()->hasTimeTarget())
    {
        return m_count_down_reached_zero;
    }
    // One team scored the target goals ...
    else
    {
        return (getScore(KART_TEAM_BLUE) >= m_goal_target ||
            getScore(KART_TEAM_RED) >= m_goal_target);
    }

}   // isRaceOver

//-----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void SoccerWorld::countdownReachedZero()
{
    // Prevent negative time in network soccer when finishing
    m_time_ticks = 0;
    m_time = 0.0f;
    m_count_down_reached_zero = true;
}   // countdownReachedZero

//-----------------------------------------------------------------------------
bool SoccerWorld::getKartSoccerResult(unsigned int kart_id) const
{
    if (m_red_scorers.size() == m_blue_scorers.size()) return true;

    bool red_win = m_red_scorers.size() > m_blue_scorers.size();
    KartTeam team = getKartTeam(kart_id);

    if ((red_win && team == KART_TEAM_RED) ||
        (!red_win && team == KART_TEAM_BLUE))
        return true;
    else
        return false;

}   // getKartSoccerResult

//-----------------------------------------------------------------------------
/** Localize the ball on the navigation mesh.
 */
void SoccerWorld::updateBallPosition(int ticks)
{
    if (isRaceOver()) return;

    if (!ballNotMoving())
    {
        // Only update heading if the ball is moving
        m_ball_heading = atan2f(m_ball_body->getLinearVelocity().getX(),
            m_ball_body->getLinearVelocity().getZ());
    }

    if (Track::getCurrentTrack()->hasNavMesh())
    {
        m_ball_track_sector
            ->update(getBallPosition(), true/*ignore_vertical*/);

        bool is_client = NetworkConfig::get()->isNetworking() &&
            NetworkConfig::get()->isClient();
        bool is_server = NetworkConfig::get()->isNetworking() &&
            NetworkConfig::get()->isServer();

        if (!is_client && getTicksSinceStart() > m_reset_ball_ticks &&
            !m_ball_track_sector->isOnRoad())
        {
            m_ball_invalid_timer += ticks;
            // Reset the ball and karts if out of navmesh after 2 seconds
            if (m_ball_invalid_timer >= stk_config->time2Ticks(2.0f))
            {
                if (is_server)
                {
                    // Reset the ball 2 seconds in the future to make sure it's
                    // after all clients time
                    m_reset_ball_ticks = getTicksSinceStart() +
                        stk_config->time2Ticks(2.0f);

                    NetworkString p(PROTOCOL_GAME_EVENTS);
                    p.setSynchronous(true);
                    p.addUInt8(GameEventsProtocol::GE_RESET_BALL)
                        .addTime(m_reset_ball_ticks);
                    STKHost::get()->sendPacketToAllPeers(&p, true);
                }
                else if (!NetworkConfig::get()->isNetworking())
                {
                    m_ball_invalid_timer = 0;
                    resetKartsToSelfGoals();
                    if (UserConfigParams::m_arena_ai_stats)
                        getKart(8)->flyUp();
                }
            }
        }
        else
            m_ball_invalid_timer = 0;
        if (m_reset_ball_ticks == World::getWorld()->getTicksSinceStart())
        {
            resetKartsToSelfGoals();
        }
    }

}   // updateBallPosition

//-----------------------------------------------------------------------------
int SoccerWorld::getBallNode() const
{
    assert(m_ball_track_sector != NULL);
    return m_ball_track_sector->getCurrentGraphNode();
}   // getBallNode

//-----------------------------------------------------------------------------
bool SoccerWorld::isCorrectGoal(unsigned int kart_id, bool first_goal) const
{
    KartTeam team = getKartTeam(kart_id);
    if (first_goal)
    {
        if (team == KART_TEAM_RED)
            return true;
    }
    else if (!first_goal)
    {
        if (team == KART_TEAM_BLUE)
            return true;
    }
    return false;
}   // isCorrectGoal

//-----------------------------------------------------------------------------
void SoccerWorld::updateAIData()
{
    if (isRaceOver()) return;

    // Fill the kart distance map
    m_red_kdm.clear();
    m_blue_kdm.clear();

    for (unsigned int i = 0; i < m_karts.size(); ++i)
    {
        if (UserConfigParams::m_arena_ai_stats &&
            m_karts[i]->getController()->isPlayerController())
            continue;

        if (getKartTeam(m_karts[i]->getWorldKartId()) == KART_TEAM_RED)
        {
            Vec3 rd = m_karts[i]->getXYZ() - getBallPosition();
            m_red_kdm.push_back(KartDistanceMap(i, rd.length_2d()));
        }
        else
        {
            Vec3 bd = m_karts[i]->getXYZ() - getBallPosition();
            m_blue_kdm.push_back(KartDistanceMap(i, bd.length_2d()));
        }
    }
    // Sort the vectors, so first vector will have the min distance
    std::sort(m_red_kdm.begin(), m_red_kdm.end());
    std::sort(m_blue_kdm.begin(), m_blue_kdm.end());

    // Fill Ball and goals data
    m_bgd->updateBallAndGoal(getBallPosition(), getBallHeading());

}   // updateAIData

//-----------------------------------------------------------------------------
int SoccerWorld::getAttacker(KartTeam team) const
{
    if (team == KART_TEAM_BLUE && m_blue_kdm.size() > 1)
    {
        for (unsigned int i = 1; i < m_blue_kdm.size(); i++)
        {
            // Only AI will do the attack job
            if (getKart(m_blue_kdm[i].m_kart_id)
                ->getController()->isPlayerController())
                continue;
            return m_blue_kdm[i].m_kart_id;
        }
    }
    else if (team == KART_TEAM_RED && m_red_kdm.size() > 1)
    {
        for (unsigned int i = 1; i < m_red_kdm.size(); i++)
        {
            if (getKart(m_red_kdm[i].m_kart_id)
                ->getController()->isPlayerController())
                continue;
            return m_red_kdm[i].m_kart_id;
        }
    }

    // No attacker
    return -1;
}   // getAttacker

//-----------------------------------------------------------------------------
unsigned int SoccerWorld::getRescuePositionIndex(AbstractKart *kart)
{
    if (!Track::getCurrentTrack()->hasNavMesh())
        return m_kart_position_map.at(kart->getWorldKartId());

    int last_valid_node =
        getTrackSector(kart->getWorldKartId())->getLastValidGraphNode();
    if (last_valid_node >= 0)
        return last_valid_node;
    Log::warn("SoccerWorld", "Missing last valid node for rescuing");
    return 0;
}   // getRescuePositionIndex

//-----------------------------------------------------------------------------
btTransform SoccerWorld::getRescueTransform(unsigned int rescue_pos) const
{
    if (!Track::getCurrentTrack()->hasNavMesh())
        return WorldWithRank::getRescueTransform(rescue_pos);

    const Vec3 &xyz = Graph::get()->getQuad(rescue_pos)->getCenter();
    const Vec3 &normal = Graph::get()->getQuad(rescue_pos)->getNormal();
    btTransform pos;
    pos.setOrigin(xyz);
    btQuaternion q1 = shortestArcQuat(Vec3(0.0f, 1.0f, 0.0f), normal);
    pos.setRotation(q1);

    // Get the ball initial position (at the moment of hitting the rescue button)
    Vec3 ball_position_0 = getBallPosition();

    // Get the ball Linear Velocity
    Vec3 ball_velocity = m_ball_body->getLinearVelocity();

    // Calculate the new ball position after 1.1 s (after the rescue animation is over) assuming the ball to be moving in URM
    Vec3 ball_position = ball_velocity*1.1 + ball_position_0;

    // Calculate the direction vector from the kart to the ball
    Vec3 direction = (ball_position - xyz).normalized();

    // Calculate the angle between the kart's forward direction and the direction vector
    float angle = atan2f(direction.getX(), direction.getZ());

    // Create a quaternion representing the rotation around the Y axis
    btQuaternion q2(btVector3(0.0f, 1.0f, 0.0f), angle);

    // Combine the two rotations and normalize the result
    btQuaternion combined_rotation = q1 * q2;
    combined_rotation.normalize();
    pos.setRotation(combined_rotation);

    return pos;
}   // getRescueTransform

//-----------------------------------------------------------------------------
void SoccerWorld::enterRaceOverState()
{
    WorldWithRank::enterRaceOverState();

    if (UserConfigParams::m_arena_ai_stats)
    {
        Log::verbose("Soccer AI profiling", "Total frames elapsed for a team"
            " to win with 30 goals: %d", m_frame_count);

        // Goal time statistics
        std::sort(m_goal_frame.begin(), m_goal_frame.end());

        const int mean = std::accumulate(m_goal_frame.begin(),
            m_goal_frame.end(), 0) / (int)m_goal_frame.size();

        // Prevent overflow if there is a large frame in vector
        double squared_sum = 0;
        for (const int &i : m_goal_frame)
            squared_sum = squared_sum + (double(i - mean) * double(i - mean));

        // Use sample st. deviation (n-1) as the profiling can't be run forever
        const int stdev = int(sqrt(squared_sum / (m_goal_frame.size() - 1)));

        int median = 0;
        if (m_goal_frame.size() % 2 == 0)
        {
            median = (m_goal_frame[m_goal_frame.size() / 2 - 1] +
                m_goal_frame[m_goal_frame.size() / 2]) / 2;
        }
        else
        {
            median = m_goal_frame[m_goal_frame.size() / 2];
        }

        Log::verbose("Soccer AI profiling", "Frames elapsed for each goal:"
            " min: %d max: %d mean: %d median: %d standard deviation: %d",
            m_goal_frame.front(), m_goal_frame.back(), mean, median, stdev);

        // Goal calculation
        int red_own_goal = 0;
        int blue_own_goal = 0;
        for (unsigned i = 0; i < m_red_scorers.size(); i++)
        {
            // Notice: if a team has own goal, the score will end up in the
            // opposite team
            if (!m_red_scorers[i].m_correct_goal)
                blue_own_goal++;
        }
        for (unsigned i = 0; i < m_blue_scorers.size(); i++)
        {
            if (!m_blue_scorers[i].m_correct_goal)
                red_own_goal++;
        }

        int red_goal = ((int(m_red_scorers.size()) - blue_own_goal) >= 0 ?
            (int)m_red_scorers.size() - blue_own_goal : 0);
        int blue_goal = ((int(m_blue_scorers.size()) - red_own_goal) >= 0 ?
            (int)m_blue_scorers.size() - red_own_goal : 0);

        Log::verbose("Soccer AI profiling", "Red goal: %d, Red own goal: %d,"
            "Blue goal: %d, Blue own goal: %d", red_goal, red_own_goal,
            blue_goal, blue_own_goal);

        if (getScore(KART_TEAM_BLUE) >= m_goal_target)
            Log::verbose("Soccer AI profiling", "Blue team wins");
        else
            Log::verbose("Soccer AI profiling", "Red team wins");

        delete this;
        main_loop->abort();
    }

}   // enterRaceOverState

// ----------------------------------------------------------------------------
void SoccerWorld::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    const unsigned red_scorers = (unsigned)m_red_scorers.size();
    bns->addUInt32(red_scorers);
    for (unsigned i = 0; i < red_scorers; i++)
    {
        bns->addUInt8((uint8_t)m_red_scorers[i].m_id)
            .addUInt8(m_red_scorers[i].m_correct_goal)
            .addFloat(m_red_scorers[i].m_time)
            .encodeString(m_red_scorers[i].m_kart)
            .encodeString(m_red_scorers[i].m_player);
        if (peer->getClientCapabilities().find("soccer_fixes") !=
            peer->getClientCapabilities().end())
        {
            bns->encodeString(m_red_scorers[i].m_country_code)
                .addUInt8(m_red_scorers[i].m_handicap_level);
        }
    }

    const unsigned blue_scorers = (unsigned)m_blue_scorers.size();
    bns->addUInt32(blue_scorers);
    for (unsigned i = 0; i < blue_scorers; i++)
    {
        bns->addUInt8((uint8_t)m_blue_scorers[i].m_id)
            .addUInt8(m_blue_scorers[i].m_correct_goal)
            .addFloat(m_blue_scorers[i].m_time)
            .encodeString(m_blue_scorers[i].m_kart)
            .encodeString(m_blue_scorers[i].m_player);
        if (peer->getClientCapabilities().find("soccer_fixes") !=
            peer->getClientCapabilities().end())
        {
            bns->encodeString(m_blue_scorers[i].m_country_code)
                .addUInt8(m_blue_scorers[i].m_handicap_level);
        }
    }
    bns->addTime(m_reset_ball_ticks).addTime(m_ticks_back_to_own_goal);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void SoccerWorld::restoreCompleteState(const BareNetworkString& b)
{
    m_red_scorers.clear();
    m_blue_scorers.clear();

    const unsigned red_size = b.getUInt32();
    for (unsigned i = 0; i < red_size; i++)
    {
        ScorerData sd;
        sd.m_id = b.getUInt8();
        sd.m_correct_goal = b.getUInt8() == 1;
        sd.m_time = b.getFloat();
        b.decodeString(&sd.m_kart);
        b.decodeStringW(&sd.m_player);
        if (NetworkConfig::get()->getServerCapabilities().find("soccer_fixes")
            != NetworkConfig::get()->getServerCapabilities().end())
        {
            b.decodeString(&sd.m_country_code);
            sd.m_handicap_level = (HandicapLevel)b.getUInt8();
        }
        m_red_scorers.push_back(sd);
    }

    const unsigned blue_size = b.getUInt32();
    for (unsigned i = 0; i < blue_size; i++)
    {
        ScorerData sd;
        sd.m_id = b.getUInt8();
        sd.m_correct_goal = b.getUInt8() == 1;
        sd.m_time = b.getFloat();
        b.decodeString(&sd.m_kart);
        b.decodeStringW(&sd.m_player);
        if (NetworkConfig::get()->getServerCapabilities().find("soccer_fixes")
            != NetworkConfig::get()->getServerCapabilities().end())
        {
            b.decodeString(&sd.m_country_code);
            sd.m_handicap_level = (HandicapLevel)b.getUInt8();
        }
        m_blue_scorers.push_back(sd);
    }
    m_reset_ball_ticks = b.getTime();
    m_ticks_back_to_own_goal = b.getTime();
}   // restoreCompleteState

// ----------------------------------------------------------------------------
float SoccerWorld::getBallDiameter() const
{
    return m_bgd->getDiameter();
}   // getBallDiameter

// ----------------------------------------------------------------------------
bool SoccerWorld::ballApproachingGoal(KartTeam team) const
{
    return m_bgd->isApproachingGoal(team);
}   // ballApproachingGoal

// ----------------------------------------------------------------------------
Vec3 SoccerWorld::getBallAimPosition(KartTeam team, bool reverse) const
{
    return m_bgd->getAimPosition(team, reverse);
}   // getBallAimPosition

// ----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void SoccerWorld::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    if (!UserConfigParams::m_soccer_player_list)
        return;
    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        rank_info.lap = -1;
        rank_info.m_outlined_font = true;
        rank_info.m_color = getKartTeam(i) == KART_TEAM_RED ?
            video::SColor(255, 255, 0, 0) : video::SColor(255, 0, 0, 255);
        rank_info.m_text = getKart(i)->getController()->getName();
        if (RaceManager::get()->getKartGlobalPlayerId(i) > -1)
        {
            const core::stringw& flag = StringUtils::getCountryFlag(
                RaceManager::get()->getKartInfo(i).getCountryCode());
            if (!flag.empty())
            {
                rank_info.m_text += L" ";
                rank_info.m_text += flag;
            }
        }
    }
}   // getKartsDisplayInfo
