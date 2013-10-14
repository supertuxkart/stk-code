//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <string>
#include <IMeshSceneNode.h>

#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/rescue_animation.hpp"
#include "karts/controller/player_controller.hpp"
#include "physics/physics.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
SoccerWorld::SoccerWorld() : WorldWithRank()
{
    WorldStatus::setClockMode(CLOCK_CHRONO);
    m_use_highscores = false;
}   // SoccerWorld

//-----------------------------------------------------------------------------
/** Initializes the soccer world. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void SoccerWorld::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_goal_timer = 0.f;
    m_lastKartToHitBall = -1;

    // check for possible problems if AI karts were incorrectly added
    if(getNumKarts() > race_manager->getNumPlayers())
    {
        fprintf(stderr, "No AI exists for this game mode\n");
        exit(1);
    }
    m_goal_target = race_manager->getMaxGoal();
    m_goal_sound = sfx_manager->createSoundSource("goal_scored");

}   // init

//-----------------------------------------------------------------------------
/** Called then a battle is restarted.
 */
void SoccerWorld::reset()
{
    WorldWithRank::reset();

    m_can_score_points = true;
    memset(m_team_goals, 0, sizeof(m_team_goals));
    
    // Reset original positions for the soccer balls
    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
    assert(tom);
    m_redScorers.clear();
    m_redScoreTimes.clear();
    m_blueScorers.clear();
    m_blueScoreTimes.clear();
    m_lastKartToHitBall = -1;
    PtrVector<TrackObject>&   objects = tom->getObjects();
    for(int i=0; i<objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if(!obj->isSoccerBall())
            continue;

        obj->reset();
        obj->getPhysics()->reset();
    }

    initKartList();
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

    if (world->getPhase() == World::GOAL_PHASE)
    {
        m_goal_timer += dt;

        if (m_goal_timer > 3.0f)
        {
            world->setPhase(WorldStatus::RACE_PHASE);
            m_goal_timer = 0;
        }
    }

    // TODO
}   // update

void SoccerWorld::onCheckGoalTriggered(bool first_goal)
{
    if (m_can_score_points)
    {
        //I18N: soccer mode
        //~ m_race_gui->addMessage(_("GOAL!"), NULL,
                              //~ /* time */ 5.0f,
                               //~ video::SColor(255,255,255,255),
                               //~ /*important*/ true,
                               //~ /*big font*/  true);
        m_team_goals[first_goal ? 0 : 1]++;
        //printf("Score:\nTeam One %d : %d Team Two\n", m_team_goals[0], m_team_goals[1]);
        World *world = World::getWorld();
        world->setPhase(WorldStatus::GOAL_PHASE);
        m_goal_sound->play();
        if(m_lastKartToHitBall != -1)
        {
            if(first_goal){
                m_redScorers.push_back(m_lastKartToHitBall);
                m_redScoreTimes.push_back(world->getTime());
            }
            else{
                m_blueScorers.push_back(m_lastKartToHitBall);
                m_blueScoreTimes.push_back(world->getTime());
            }
        }
    }

    //m_check_goals_enabled = false;    // TODO: remove?

    // Reset original positions for the soccer balls
    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
    assert(tom);

    PtrVector<TrackObject>&   objects = tom->getObjects();
    for(int i=0; i<objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if(!obj->isSoccerBall())
            continue;

        obj->reset();
        obj->getPhysics()->reset();
    }

    //Resetting the ball triggers the goal check line one more time.
    //This ensures that only one goal is counted, and the second is ignored.
    m_can_score_points = !m_can_score_points;

    //for(int i=0 ; i < getNumKarts() ; i++

    /*if(World::getWorld()->getTrack()->isAutoRescueEnabled() &&
        !getKartAnimation() && fabs(getRoll())>60*DEGREE_TO_RAD &&
                              fabs(getSpeed())<3.0f                )
    {
        new RescueAnimation(this, true);
    }*/

    // TODO: rescue the karts
}   // onCheckGoalTriggered

//-----------------------------------------------------------------------------
/** Sets the last kart that hit the ball, to be able to
* identify the scorer later.
*/
void SoccerWorld::setLastKartTohitBall(unsigned int kartId){
    m_lastKartToHitBall = kartId;
}
//-----------------------------------------------------------------------------
/** The battle is over if only one kart is left, or no player kart.
 */
bool SoccerWorld::isRaceOver()
{
    // for tests : never over when we have a single player there :)
    if (race_manager->getNumPlayers() < 2)
    {
        return false;
    }
        // One team scored the target goals ...
    else if(getScore(0) >= m_goal_target ||
            getScore(1) >= m_goal_target    )
    {
      return true;
    }
    // TODO
    return getCurrentNumKarts()==1 || getCurrentNumPlayers()==0;
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
/** Returns the data to display in the race gui.
 */
void SoccerWorld::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    // TODO!!
    /*
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];

        // reset color
        rank_info.lap = -1;

        AbstractKart* kart = getKart(i);
        switch(kart->getSoccerTeam())
        {
        case SOCCER_TEAM_BLUE:
            rank_info.r = 0.0f;
            rank_info.g = 0.0f;
            rank_info.b = 0.7f;
            break;
        case SOCCER_TEAM_RED:
            rank_info.r = 0.9f;
            rank_info.g = 0.0f;
            rank_info.b = 0.0f;
            break;
        default:
            assert(false && "Soccer team not set to blue or red");
            rank_info.r = 0.0f;
            rank_info.g = 0.0f;
            rank_info.b = 0.0f;
        }
    }
    */
}   // getKartsDisplayInfo

//-----------------------------------------------------------------------------
/** Moves a kart to its rescue position.
 *  \param kart The kart that was rescued.
 */
void SoccerWorld::moveKartAfterRescue(AbstractKart* kart)
{
    // find closest point to drop kart on
    World *world = World::getWorld();
    const int start_spots_amount = world->getTrack()->getNumberOfStartPositions();
    assert(start_spots_amount > 0);

    float largest_accumulated_distance_found = -1;
    int furthest_id_found = -1;

    const float kart_x = kart->getXYZ().getX();
    const float kart_z = kart->getXYZ().getZ();

    for(int n=0; n<start_spots_amount; n++)
    {
        // no need for the overhead to compute exact distance with sqrt(),
        // so using the 'manhattan' heuristic which will do fine enough.
        const btTransform &s = world->getTrack()->getStartTransform(n);
        const Vec3 &v=s.getOrigin();
        float accumulatedDistance = .0f;
        bool spawnPointClear = true;

        for(unsigned int k=0; k<getCurrentNumKarts(); k++)
        {
            const AbstractKart *currentKart = World::getWorld()->getKart(k);
            const float currentKart_x = currentKart->getXYZ().getX();
            const float currentKartk_z = currentKart->getXYZ().getZ();

            if(kart_x!=currentKart_x && kart_z !=currentKartk_z)
            {
                float absDistance = fabs(currentKart_x - v.getX()) +
                    fabs(currentKartk_z - v.getZ());
                if(absDistance < CLEAR_SPAWN_RANGE)
                {
                    spawnPointClear = false;
                    break;
                }
                accumulatedDistance += absDistance;
            }
        }

        if(largest_accumulated_distance_found < accumulatedDistance && spawnPointClear)
        {
            furthest_id_found = n;
            largest_accumulated_distance_found = accumulatedDistance;
        }
    }

    assert(furthest_id_found != -1);
    const btTransform &s = world->getTrack()->getStartTransform(furthest_id_found);
    const Vec3 &xyz = s.getOrigin();
    kart->setXYZ(xyz);
    kart->setRotation(s.getRotation());

    //position kart from same height as in World::resetAllKarts
    btTransform pos;
    pos.setOrigin(kart->getXYZ()+btVector3(0, 0.5f*kart->getKartHeight(), 0.0f));
    pos.setRotation( btQuaternion(btVector3(0.0f, 1.0f, 0.0f), 0 /* angle */) );

    kart->getBody()->setCenterOfMassTransform(pos);

    //project kart to surface of track
    bool kart_over_ground = m_track->findGround(kart);

    if (kart_over_ground)
    {
        //add vertical offset so that the kart starts off above the track
        float vertical_offset = kart->getKartProperties()->getVertRescueOffset() *
                                kart->getKartHeight();
        kart->getBody()->translate(btVector3(0, vertical_offset, 0));
    }
    else
    {
        fprintf(stderr, "WARNING: invalid position after rescue for kart %s on track %s.\n",
                (kart->getIdent().c_str()), m_track->getIdent().c_str());
    }
}   // moveKartAfterRescue

//-----------------------------------------------------------------------------/** Set position and team for the karts */
void SoccerWorld::initKartList()
{
    const unsigned int kart_amount = m_karts.size();

    // Set kart positions, ordering them by team
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_karts[n]->setPosition(-1);
    }
    // TODO: remove

    int team_karts_amount[NB_SOCCER_TEAMS];
    memset(team_karts_amount, 0, sizeof(team_karts_amount));

    {
        // Set the kart teams if they haven't been already set by the setup screen
        // (happens when the setup screen is skipped, with 1 player)
        SoccerTeam    round_robin_team = SOCCER_TEAM_RED;
        for(unsigned int n=0; n<kart_amount; n++)
        {
                        if(race_manager->getLocalKartInfo(n).getSoccerTeam() == SOCCER_TEAM_NONE)
                                race_manager->setLocalKartSoccerTeam(
                                race_manager->getLocalKartInfo(n).getLocalPlayerId(),round_robin_team);

            team_karts_amount[race_manager->getLocalKartInfo(n).getSoccerTeam()]++;

            round_robin_team = (round_robin_team==SOCCER_TEAM_RED ?
                                SOCCER_TEAM_BLUE : SOCCER_TEAM_RED);
        }// next kart
    }

    //Loading the indicator textures
    irr::video::ITexture *redTeamTexture = irr_driver->getTexture(
                                   file_manager->getTextureFile("soccer_player_red.png"));
    irr::video::ITexture *blueTeamTexture = irr_driver->getTexture(
                                   file_manager->getTextureFile("soccer_player_blue.png"));
    //Assigning indicators
    for(unsigned int i=0; i<kart_amount; i++)
    {
        scene::ISceneNode *arrowNode;
        float arrow_pos_height = m_karts[i]->getKartModel()->getHeight()+0.5;

        if(race_manager->getLocalKartInfo(i).getSoccerTeam() == SOCCER_TEAM_RED)
            arrowNode = irr_driver->addBillboard(core::dimension2d<irr::f32>(0.3f,0.3f),
                                               redTeamTexture,m_karts[i]->getNode(), true);
        else
            arrowNode = irr_driver->addBillboard(core::dimension2d<irr::f32>(0.3f,0.3f),
                                               blueTeamTexture,m_karts[i]->getNode(),true);
        arrowNode->setPosition(core::vector3df(0, arrow_pos_height, 0));
    }

    // Compute start positions for each team
    int team_cur_position[NB_SOCCER_TEAMS];
    team_cur_position[0] = 1;
    for(int i=1 ; i < (int)NB_SOCCER_TEAMS ; i++)
        team_cur_position[i] = team_karts_amount[i-1] + team_cur_position[i-1];

    // Set kart positions, ordering them by team
    for(unsigned int n=0; n<kart_amount; n++)
    {
                
        SoccerTeam  team = race_manager->getLocalKartInfo(n).getSoccerTeam();
        m_karts[n]->setPosition(team_cur_position[team]);
        team_cur_position[team]++;
    }   // next kart
}

//-----------------------------------------------------------------------------
int SoccerWorld::getScore(unsigned int i)
{
        return m_team_goals[i];
}
//-----------------------------------------------------------------------------
int SoccerWorld::getTeamLeader(unsigned int team)
{
                for(unsigned int i = 0; i< m_karts.size(); i++){
                        if(race_manager->getLocalKartInfo(i).getSoccerTeam() == (SoccerTeam) team)
                                return i;
                }
                return -1;
        }
//-----------------------------------------------------------------------------
AbstractKart *SoccerWorld::createKart(const std::string &kart_ident, int index,
                                int local_player_id, int global_player_id,
                                RaceManager::KartType kart_type)
{
  int posIndex = index;
  if(race_manager->getLocalKartInfo(index).getSoccerTeam() == SOCCER_TEAM_RED)
  {
      if(index % 2 != 1) posIndex += 1;
  }
  else if(race_manager->getLocalKartInfo(index).getSoccerTeam() == SOCCER_TEAM_BLUE)
  {
      if(index % 2 != 0) posIndex += 1;
  }
  int position           = index+1;
  btTransform init_pos   = m_track->getStartTransform(posIndex);
  AbstractKart *new_kart = new Kart(kart_ident, index, position, init_pos);
  new_kart->init(race_manager->getKartType(index));
  Controller *controller = NULL;
  switch(kart_type)
  {
  case RaceManager::KT_PLAYER:
       controller = new PlayerController(new_kart,
                          StateManager::get()->getActivePlayer(local_player_id),
                                         local_player_id);
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

