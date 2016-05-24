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
#include "karts/abstract_kart.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "karts/controller/soccer_ai.hpp"
#include "physics/physics.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"

#include <IMeshSceneNode.h>
#include <string>
//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
SoccerWorld::SoccerWorld() : WorldWithRank()
{
    if (race_manager->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            race_manager->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }

    m_frame_count = 0;
    m_start_time = irr_driver->getRealTime();
    m_use_highscores = false;
    m_red_ai = 0;
    m_blue_ai = 0;
}   // SoccerWorld

//-----------------------------------------------------------------------------
/** The destructor frees all data structures.
 */
SoccerWorld::~SoccerWorld()
{
    m_goal_sound->deleteSFX();
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
    m_goal_timer = 0.0f;
    m_ball_hitter = -1;
    m_ball = NULL;
    m_ball_body = NULL;
    m_goal_target = race_manager->getMaxGoal();
    m_goal_sound = SFXManager::get()->createSoundSource("goal_scored");

    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
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

    m_bgd.init(m_ball->getPhysicalObject()->getRadius());

}   // init

//-----------------------------------------------------------------------------
/** Called when a soccer game is restarted.
 */
void SoccerWorld::reset()
{
    WorldWithRank::reset();
    if (race_manager->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN,
            race_manager->getTimeTarget());
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }

    m_count_down_reached_zero = false;
    m_red_scorers.clear();
    m_red_score_times.clear();
    m_blue_scorers.clear();
    m_blue_score_times.clear();
    m_ball_hitter = -1;
    m_red_kdm.clear();
    m_blue_kdm.clear();
    m_ball_heading = 0.0f;
    m_ball_invalid_timer = 0.0f;

    if (m_goal_sound != NULL &&
        m_goal_sound->getStatus() == SFXBase::SFX_PLAYING)
    {
        m_goal_sound->stop();
    }

    initKartList();
    resetAllPosition();
    m_ball->reset();
    m_bgd.reset();
    // Make the player kart in profiling mode up
    // ie make this kart less likely to affect gaming result
    if (UserConfigParams::m_arena_ai_stats)
        getKart(8)->flyUp();

}   // reset

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& SoccerWorld::getIdent() const
{
    return IDENT_SOCCER;
}   // getIdent

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param dt Time step size.
 */
void SoccerWorld::update(float dt)
{
    updateBallPosition(dt);
    if (m_track->hasNavMesh())
    {
        updateKartNodes();
        updateAIData();
    }

    WorldWithRank::update(dt);
    WorldWithRank::updateTrack(dt);

    if (getPhase() == World::GOAL_PHASE)
    {
        if (m_goal_timer == 0.0f)
        {
            // Stop all karts
            for (unsigned int i = 0; i < m_karts.size(); i++)
                m_karts[i]->setVelocity(btVector3(0, 0, 0));
        }
        m_goal_timer += dt;

        if (m_goal_timer > 3.0f)
        {
            setPhase(WorldStatus::RACE_PHASE);
            m_goal_timer = 0.0f;
            if (!isRaceOver())
            {
                // Reset all karts
                for (unsigned int i = 0; i < m_karts.size(); i++)
                    moveKartAfterRescue(m_karts[i]);
                if (UserConfigParams::m_arena_ai_stats)
                    getKart(8)->flyUp();
            }
        }
    }
    if (UserConfigParams::m_arena_ai_stats)
        m_frame_count++;

}   // update

//-----------------------------------------------------------------------------
void SoccerWorld::onCheckGoalTriggered(bool first_goal)
{
    if (isRaceOver() || isStartPhase())
        return;

    setPhase(WorldStatus::GOAL_PHASE);
    m_goal_sound->play();
    if (m_ball_hitter != -1)
    {
        ScorerData sd;
        sd.m_id = m_ball_hitter;
        sd.m_correct_goal = isCorrectGoal(m_ball_hitter, first_goal);

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

        if (first_goal)
        {
            // Notice: true first_goal means it's blue goal being shoot,
            // so red team can score
            m_red_scorers.push_back(sd);
            if (race_manager->hasTimeTarget())
            {
                m_red_score_times.push_back(race_manager->getTimeTarget()
                    - getTime());
            }
            else
                m_red_score_times.push_back(getTime());
        }
        else
        {
            m_blue_scorers.push_back(sd);
            if (race_manager->hasTimeTarget())
            {
                m_blue_score_times.push_back(race_manager->getTimeTarget()
                    - getTime());
            }
            else
                m_blue_score_times.push_back(getTime());
        }
    }
    m_ball->reset();

}   // onCheckGoalTriggered

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

    if(race_manager->hasTimeTarget())
    {
        return m_count_down_reached_zero;
    }
    // One team scored the target goals ...
    else
    {
      return (getScore(SOCCER_TEAM_BLUE) >= m_goal_target ||
          getScore(SOCCER_TEAM_RED) >= m_goal_target);
    }

}   // isRaceOver

//-----------------------------------------------------------------------------
/** Called when the match time ends.
 */
void SoccerWorld::countdownReachedZero()
{
    m_count_down_reached_zero = true;
}   // countdownReachedZero

//-----------------------------------------------------------------------------
void SoccerWorld::initKartList()
{
    const unsigned int kart_amount = (unsigned int)m_karts.size();

    //Loading the indicator textures
    irr::video::ITexture *red =
            irr_driver->getTexture(FileManager::GUI, "soccer_player_red.png");
    irr::video::ITexture *blue =
            irr_driver->getTexture(FileManager::GUI, "soccer_player_blue.png");

    //Assigning indicators
    for(unsigned int i=0; i<kart_amount; i++)
    {
        scene::ISceneNode *arrow_node;
        float arrow_pos_height = m_karts[i]->getKartModel()->getHeight()+0.5f;
        SoccerTeam team = getKartTeam(i);

        arrow_node = irr_driver->addBillboard(core::dimension2d<irr::f32>(0.3f,
            0.3f), team == SOCCER_TEAM_BLUE ? blue : red, m_karts[i]
            ->getNode(), true);

        arrow_node->setPosition(core::vector3df(0, arrow_pos_height, 0));
    }

}   // initKartList

//-----------------------------------------------------------------------------
bool SoccerWorld::getKartSoccerResult(unsigned int kart_id) const
{
    if (m_red_scorers.size() == m_blue_scorers.size()) return true;

    bool red_win = m_red_scorers.size() > m_blue_scorers.size();
    SoccerTeam team = getKartTeam(kart_id);

    if ((red_win && team == SOCCER_TEAM_RED) ||
        (!red_win && team == SOCCER_TEAM_BLUE))
        return true;
    else
        return false;

}   // getKartSoccerResult

//-----------------------------------------------------------------------------
AbstractKart *SoccerWorld::createKart(const std::string &kart_ident, int index,
                                int local_player_id, int global_player_id,
                                RaceManager::KartType kart_type,
                                PerPlayerDifficulty difficulty)
{
    int cur_red = getTeamNum(SOCCER_TEAM_RED);
    int cur_blue = getTeamNum(SOCCER_TEAM_BLUE);
    int pos_index = 0;
    int position  = index + 1;
    SoccerTeam team = SOCCER_TEAM_BLUE;

    if (kart_type == RaceManager::KT_AI)
    {
        if (index < m_red_ai)
            team = SOCCER_TEAM_RED;
        else
            team = SOCCER_TEAM_BLUE;
        m_kart_team_map[index] = team;
    }
    else
    {
        int rm_id = index -
            (race_manager->getNumberOfKarts() - race_manager->getNumPlayers());

        assert(rm_id >= 0);
        team = race_manager->getKartInfo(rm_id).getSoccerTeam();
        m_kart_team_map[index] = team;
    }

    // Notice: In blender, please set 1,3,5,7... for blue starting position;
    // 2,4,6,8... for red.
    if (team == SOCCER_TEAM_BLUE)
    {
        pos_index = 1 + 2 * cur_blue;
    }
    else
    {
        pos_index = 2 + 2 * cur_red;
    }

    btTransform init_pos = getStartTransform(pos_index - 1);
    m_kart_position_map[index] = (unsigned)(pos_index - 1);

    AbstractKart *new_kart = new Kart(kart_ident, index, position, init_pos,
              difficulty);
            //difficulty, team == SOCCER_TEAM_BLUE ?
            //video::ERT_BLUE : video::ERT_RED);
    new_kart->init(race_manager->getKartType(index));
    Controller *controller = NULL;

    switch(kart_type)
    {
    case RaceManager::KT_PLAYER:
        controller = new LocalPlayerController(new_kart,
                          StateManager::get()->getActivePlayer(local_player_id));
        m_num_players ++;
        break;
    case RaceManager::KT_NETWORK_PLAYER:
        break;  // Avoid compiler warning about enum not handled.
        //controller = new NetworkController(kart_ident, position, init_pos,
        //                          global_player_id);
        //m_num_players++;
        //break;
    case RaceManager::KT_AI:
        controller = loadAIController(new_kart);
        break;
    case RaceManager::KT_GHOST:
        break;
    case RaceManager::KT_LEADER:
        break;
    }

    new_kart->setController(controller);

    return new_kart;
}   // createKart

//-----------------------------------------------------------------------------
/** Updates the m_kart_on_node value of each kart to localize it
 *  on the navigation mesh.
 */
void SoccerWorld::updateKartNodes()
{
    if (isRaceOver()) return;

    const unsigned int n = getNumKarts();
    for (unsigned int i = 0; i < n; i++)
    {
        if (m_karts[i]->isEliminated()) continue;

        m_kart_on_node[i] = BattleGraph::get()->pointToNode(m_kart_on_node[i],
                            m_karts[i]->getXYZ(), false/*ignore_vertical*/);
    }
}   // updateKartNodes

//-----------------------------------------------------------------------------
/** Localize the ball on the navigation mesh.
 */
void SoccerWorld::updateBallPosition(float dt)
{
    if (isRaceOver()) return;

    if (!ballNotMoving())
    {
        // Only update heading if the ball is moving
        m_ball_heading = atan2f(m_ball_body->getLinearVelocity().getX(),
            m_ball_body->getLinearVelocity().getZ());
    }

    if (m_track->hasNavMesh())
    {
        m_ball_on_node  = BattleGraph::get()->pointToNode(m_ball_on_node,
                          getBallPosition(), true/*ignore_vertical*/);

        if (m_ball_on_node == BattleGraph::UNKNOWN_POLY &&
            getPhase() == RACE_PHASE)
        {
            m_ball_invalid_timer += dt;
            // Reset the ball and karts if out of navmesh after 2 seconds
            if (m_ball_invalid_timer >= 2.0f)
            {
                m_ball_invalid_timer = 0.0f;
                m_ball->reset();
                for (unsigned int i = 0; i < m_karts.size(); i++)
                    moveKartAfterRescue(m_karts[i]);
                if (UserConfigParams::m_arena_ai_stats)
                    getKart(8)->flyUp();
            }
        }
        else
            m_ball_invalid_timer = 0.0f;
    }

}   // updateBallPosition

//-----------------------------------------------------------------------------
void SoccerWorld::resetAllPosition()
{
    m_kart_on_node.clear();
    m_kart_on_node.resize(m_karts.size(), BattleGraph::UNKNOWN_POLY);
    m_ball_on_node = BattleGraph::UNKNOWN_POLY;
}   // resetAllPosition
//-----------------------------------------------------------------------------
SoccerTeam SoccerWorld::getKartTeam(unsigned int kart_id) const
{
    std::map<int, SoccerTeam>::const_iterator n =
        m_kart_team_map.find(kart_id);

    assert(n != m_kart_team_map.end());
    return n->second;
}   // getKartTeam

//-----------------------------------------------------------------------------
bool SoccerWorld::isCorrectGoal(unsigned int kart_id, bool first_goal) const
{
    SoccerTeam team = getKartTeam(kart_id);
    if (first_goal)
    {
        if (team == SOCCER_TEAM_RED)
            return true;
    }
    else if (!first_goal)
    {
        if (team == SOCCER_TEAM_BLUE)
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

        if (getKartTeam(m_karts[i]->getWorldKartId()) == SOCCER_TEAM_RED)
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
    m_bgd.updateBallAndGoal(getBallPosition(), getBallHeading());

}   // updateAIData

//-----------------------------------------------------------------------------
int SoccerWorld::getAttacker(SoccerTeam team) const
{
    if (team == SOCCER_TEAM_BLUE && m_blue_kdm.size() > 1)
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
    else if (team == SOCCER_TEAM_RED && m_red_kdm.size() > 1)
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
int SoccerWorld::getTeamNum(SoccerTeam team) const
{
    int total = 0;
    if (m_kart_team_map.empty()) return total;

    for (unsigned int i = 0; i < (unsigned)m_karts.size(); ++i)
    {
        if (team == getKartTeam(m_karts[i]->getWorldKartId())) total++;
    }

    return total;
}   // getTeamNum

//-----------------------------------------------------------------------------
unsigned int SoccerWorld::getRescuePositionIndex(AbstractKart *kart)
{
    std::map<int, unsigned int>::const_iterator n =
        m_kart_position_map.find(kart->getWorldKartId());

    assert (n != m_kart_position_map.end());
    return n->second;
}   // getRescuePositionIndex

//-----------------------------------------------------------------------------
void SoccerWorld::enterRaceOverState()
{
    if (UserConfigParams::m_arena_ai_stats)
    {
        float runtime = (irr_driver->getRealTime()-m_start_time)*0.001f;
        Log::verbose("Soccer AI profiling", "Number of frames: %d, Average FPS: %f",
            m_frame_count, (float)m_frame_count/runtime);
        Log::verbose("Soccer AI profiling", "Time for a team to have 30 goals: %f",
            runtime);

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

        int red_goal = ((m_red_scorers.size() - blue_own_goal) >= 0 ?
            m_red_scorers.size() - blue_own_goal : 0);
        int blue_goal = ((m_blue_scorers.size() - red_own_goal) >= 0 ?
            m_blue_scorers.size() - red_own_goal : 0);

        Log::verbose("Soccer AI profiling", "Red goal: %d, Red own goal: %d,"
            "Blue goal: %d, Blue own goal: %d", red_goal, red_own_goal,
            blue_goal, blue_own_goal);

        if (getScore(SOCCER_TEAM_BLUE) >= m_goal_target)
            Log::verbose("Soccer AI profiling", "Blue team wins");
        else
            Log::verbose("Soccer AI profiling", "Red team wins");

        delete this;
        main_loop->abort();
    }
    else
        WorldStatus::enterRaceOverState();
}   // enterRaceOverState

//-----------------------------------------------------------------------------
void SoccerWorld::setAITeam()
{
    const int total_player = race_manager->getNumPlayers();
    const int total_karts = race_manager->getNumberOfKarts();

    // No AI
    if ((total_karts - total_player) == 0) return;

    int red_player = 0;
    int blue_player = 0;
    for (int i = 0; i < total_player; i++)
    {
        SoccerTeam team = race_manager->getKartInfo(i).getSoccerTeam();

        // Happen in profiling mode
        if (team == SOCCER_TEAM_NONE)
        {
            race_manager->setKartSoccerTeam(i, SOCCER_TEAM_BLUE);
            team = SOCCER_TEAM_BLUE;
            continue;
        }

        team == SOCCER_TEAM_BLUE ? blue_player++ : red_player++;
    }

    int available_ai = total_karts - red_player - blue_player;
    while (available_ai > 0)
    {
        if ((m_red_ai + red_player) > (m_blue_ai + blue_player))
        {
            m_blue_ai++;
            available_ai--;
        }
        else if ((m_blue_ai + blue_player) > (m_red_ai + red_player))
        {
            m_red_ai++;
            available_ai--;
        }
        else if ((m_blue_ai + blue_player) == (m_red_ai + red_player))
        {
            blue_player > red_player ? m_red_ai++ : m_blue_ai++;
            available_ai--;
        }
    }
    Log::debug("SoccerWorld","blue AI: %d red AI: %d", m_blue_ai, m_red_ai);

}   // setAITeam
