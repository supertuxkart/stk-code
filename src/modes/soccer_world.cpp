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

#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
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
#include "tracks/check_goal.hpp"
#include "tracks/check_manager.hpp"
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
    if(race_manager->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, race_manager->getTimeTarget());
        m_count_down_reached_zero = false;
    }
    else
    {
        WorldStatus::setClockMode(CLOCK_CHRONO);
    }

    m_use_highscores = false;
}   // SoccerWorld

//-----------------------------------------------------------------------------
/** The destructor frees al data structures.
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
    WorldWithRank::init();
    m_display_rank = false;
    m_goal_timer = 0.0f;
    m_ball_hitter = -1;
    m_goal_target = race_manager->getMaxGoal();
    m_goal_sound = SFXManager::get()->createSoundSource("goal_scored");

}   // init

//-----------------------------------------------------------------------------
/** Called when a soccer game is restarted.
 */
void SoccerWorld::reset()
{
    WorldWithRank::reset();
    if(race_manager->hasTimeTarget())
    {
        WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, race_manager->getTimeTarget());
        m_count_down_reached_zero = false;
    }
    else WorldStatus::setClockMode(CLOCK_CHRONO);

    m_animation_timer = 0.0f;
    m_animation_showing_kart = -1;
    m_can_score_points = true;
    m_red_goal = 0;
    m_blue_goal = 0;

    // Reset original positions for the soccer balls
    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
    assert(tom);
    m_red_scorers.clear();
    m_red_score_times.clear();
    m_blue_scorers.clear();
    m_blue_score_times.clear();
    m_ball_hitter = -1;
    PtrVector<TrackObject>& objects = tom->getObjects();
    for(unsigned int i=0; i<objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if(!obj->isSoccerBall())
            continue;

        obj->reset();
        obj->getPhysicalObject()->reset();
    }

    if (m_goal_sound != NULL &&
        m_goal_sound->getStatus() == SFXBase::SFX_PLAYING)
    {
        m_goal_sound->stop();
    }

    initKartList();
    resetAllNodes();
    initGoalNodes();

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
    World *world = World::getWorld();

    WorldWithRank::update(dt);
    WorldWithRank::updateTrack(dt);

    updateBallPosition();
    if (m_track->hasNavMesh())
        updateKartNodes();

    if (world->getPhase() == World::GOAL_PHASE)
    {
        m_goal_timer += dt;

        if (m_goal_timer > 3.0f)
        {
            world->setPhase(WorldStatus::RACE_PHASE);
            m_goal_timer = 0.0f;
        }
    }

    if (!(isRaceOver() || isStartPhase()) && m_animation_showing_kart != -1)
    {
        m_animation_timer += dt;
        if (m_animation_timer > 6.0f)
        {
            m_karts[m_animation_showing_kart]
                ->getKartModel()->setAnimation(KartModel::AF_BEGIN);
            m_animation_timer = 0.0f;
            m_animation_showing_kart = -1;
        }
    }
}   // update

//-----------------------------------------------------------------------------
void SoccerWorld::onCheckGoalTriggered(bool first_goal)
{
    if (isRaceOver() || isStartPhase())
        return;

    if (m_can_score_points)
    {
        (first_goal ? m_red_goal++ : m_blue_goal++);

        World *world = World::getWorld();
        world->setPhase(WorldStatus::GOAL_PHASE);
        m_goal_sound->play();
        if (m_ball_hitter != -1)
        {
            m_animation_showing_kart = m_ball_hitter;
            ScorerData sd;
            sd.m_id = m_ball_hitter;
            sd.m_correct_goal = isCorrectGoal(m_ball_hitter, first_goal);

            if (sd.m_correct_goal)
            {
                m_karts[m_ball_hitter]->getKartModel()
                    ->setAnimation(KartModel::AF_WIN_START);
            }

            if (first_goal)
            {
                // Notice: true first_goal means it's blue goal being shoot,
                // so red team can score
                m_red_scorers.push_back(sd);
                if(race_manager->hasTimeTarget())
                {
                    m_red_score_times.push_back(race_manager
                        ->getTimeTarget() - world->getTime());
                }
                else
                    m_red_score_times.push_back(world->getTime());
            }
            else
            {
                m_blue_scorers.push_back(sd);
                if (race_manager->hasTimeTarget())
                {
                    m_blue_score_times.push_back(race_manager
                        ->getTimeTarget() - world->getTime());
                }
                else
                    m_blue_score_times.push_back(world->getTime());
            }
        }
    }

    // Reset original positions for the soccer balls
    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
    assert(tom);

    PtrVector<TrackObject>& objects = tom->getObjects();
    for(unsigned int i=0; i<objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if(!obj->isSoccerBall())
            continue;

        obj->reset();
        obj->getPhysicalObject()->reset();
    }

    //Resetting the ball triggers the goal check line one more time.
    //This ensures that only one goal is counted, and the second is ignored.
    m_can_score_points = !m_can_score_points;

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
/** Called when the race finishes, i.e. after playing (if necessary) an
 *  end of race animation. It updates the time for all karts still racing,
 *  and then updates the ranks.
 */
void SoccerWorld::terminateRace()
{
    m_can_score_points = false;
    WorldWithRank::terminateRace();
}   // terminateRace

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
        scene::ISceneNode *arrowNode;
        float arrow_pos_height = m_karts[i]->getKartModel()->getHeight()+0.5f;
        SoccerTeam team = getKartTeam(i);

        arrowNode = irr_driver->addBillboard(core::dimension2d<irr::f32>(0.3f,0.3f),
                    team == SOCCER_TEAM_BLUE ? blue : red, m_karts[i]->getNode(), true);

        arrowNode->setPosition(core::vector3df(0, arrow_pos_height, 0));
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
    int posIndex = index;
    int position = index+1;
    SoccerTeam team = SOCCER_TEAM_BLUE;

    if (kart_type == RaceManager::KT_AI)
    {
        if (race_manager->getNumPlayers() == 1)
        {
            // Make AI even when single player choose a different team
            if (race_manager->getKartInfo(0).getSoccerTeam() == SOCCER_TEAM_RED)
            {
                team = (index % 2 == 0 ? SOCCER_TEAM_BLUE : SOCCER_TEAM_RED);
            }
            else
            {
                team = (index % 2 == 0 ? SOCCER_TEAM_RED : SOCCER_TEAM_BLUE);
            }
        }
        else
        {
            team = (index % 2 == 0 ? SOCCER_TEAM_BLUE : SOCCER_TEAM_RED);
        }
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

    if(!team)
    {
        if(index % 2 != 1) posIndex += 1;
    }
    else
    {
        if(index % 2 != 0) posIndex += 1;
    }

    btTransform init_pos = getStartTransform(posIndex);

    AbstractKart *new_kart = new Kart(kart_ident, index, position, init_pos,
            difficulty);
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
void SoccerWorld::updateBallPosition()
{
    if (isRaceOver()) return;

    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
    assert(tom);

    PtrVector<TrackObject>& objects = tom->getObjects();
    for (unsigned int i = 0; i < objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if (obj->isSoccerBall())
        {
            m_ball_position = obj->getPresentation<TrackObjectPresentationMesh>()
                              ->getNode()->getPosition();
            break;
        }
    }

    if (m_track->hasNavMesh())
    {
        m_ball_on_node  = BattleGraph::get()->pointToNode(m_ball_on_node,
                          m_ball_position, true/*ignore_vertical*/);
    }

}   // updateBallPosition

//-----------------------------------------------------------------------------
/** Localize two goals on the navigation mesh.
 */
void SoccerWorld::initGoalNodes()
{
    if (!m_track->hasNavMesh()) return;

    unsigned int n = CheckManager::get()->getCheckStructureCount();

    for (unsigned int i = 0; i < n; i++)
    {
        CheckGoal* goal =
            dynamic_cast<CheckGoal*>(CheckManager::get()->getCheckStructure(i));
        if (goal)
        {
            if (goal->getTeam())
            {
                m_blue_goal_node = BattleGraph::get()->pointToNode(m_blue_goal_node,
                                   goal->convertTo3DCenter(), true/*ignore_vertical*/);
            }
            else
            {
                m_red_goal_node  = BattleGraph::get()->pointToNode(m_red_goal_node,
                                   goal->convertTo3DCenter(), true/*ignore_vertical*/);
            }
        }
    }
}   // initGoalNodes

//-----------------------------------------------------------------------------
void SoccerWorld::resetAllNodes()
{
    m_kart_on_node.clear();
    m_kart_on_node.resize(m_karts.size());
    for(unsigned int n=0; n<m_karts.size(); n++)
        m_kart_on_node[n] = BattleGraph::UNKNOWN_POLY;
    m_ball_on_node = BattleGraph::UNKNOWN_POLY;
    m_ball_position = Vec3(0, 0, 0);
    m_red_goal_node = BattleGraph::UNKNOWN_POLY;
    m_blue_goal_node = BattleGraph::UNKNOWN_POLY;
}   // resetAllNodes
//-----------------------------------------------------------------------------
SoccerTeam SoccerWorld::getKartTeam(unsigned int kart_id) const
{
    std::map<int, SoccerTeam>::const_iterator n = m_kart_team_map.find(kart_id);
    if (n != m_kart_team_map.end())
    {
        return n->second;
    }

    // Fallback
    Log::warn("SoccerWorld", "Unknown team, using blue default.");
    return SOCCER_TEAM_BLUE;

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
