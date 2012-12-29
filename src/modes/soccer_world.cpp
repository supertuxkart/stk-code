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
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/rescue_animation.hpp"
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
    
    m_can_score_points = true;
    
    // check for possible problems if AI karts were incorrectly added
    if(getNumKarts() > race_manager->getNumPlayers())
    {
        fprintf(stderr, "No AI exists for this game mode\n");
        exit(1);
    }
 
    const unsigned int kart_amount = m_karts.size();
    
    for(unsigned int n=0; n<kart_amount; n++)
    {
        // create the struct that ill hold each player's number of goals
        SoccerInfo info;
        info.m_goals         = 0;
        m_kart_info.push_back(info);
        
        // no positions in this mode
        m_karts[n]->setPosition(-1);
    }// next kart
    
    
    SoccerEvent evt;
    evt.m_time = 0.0f;
    evt.m_kart_info = m_kart_info;
    m_soccer_events.push_back(evt);    
    
}   // SoccerWorld

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
    WorldWithRank::update(dt);
    WorldWithRank::updateTrack(dt);
    
    // TODO
}   // update

void SoccerWorld::onCheckGoalTriggered(bool first_goal)
{
    // TODO
    if(m_can_score_points)
        printf("*** GOOOOOOOOOAAAAAAALLLLLL!!!! (team: %d) ***\n", first_goal ? 0 : 1);
    
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
    }
    
    //for(int i=0 ; i < getNumKarts() ; i++
    
    /*if(World::getWorld()->getTrack()->isAutoRescueEnabled() &&
        !getKartAnimation() && fabs(getRoll())>60*DEGREE_TO_RAD && 
                              fabs(getSpeed())<3.0f                )
    {
        new RescueAnimation(this, true);
    }*/
    
    // TODO: rescue the karts
    // TODO: score a point
}

//-----------------------------------------------------------------------------
/** Updates the ranking of the karts.
 */
void SoccerWorld::updateKartRanks()
{
    beginSetKartPositions();
    // sort karts by their times then give each one its position.
    // in battle-mode, long time = good (meaning he survived longer)
    
    const unsigned int NUM_KARTS = getNumKarts();
    
    int *karts_list = new int[NUM_KARTS];
    for( unsigned int n = 0; n < NUM_KARTS; ++n ) karts_list[n] = n;
    
    bool sorted=false;
    do
    {
        sorted = true;
        for( unsigned int n = 0; n < NUM_KARTS-1; ++n )
        {
            const int this_karts_time = 
                  m_karts[karts_list[n]]->hasFinishedRace() 
                ? (int)m_karts[karts_list[n]]->getFinishTime()
                : (int)WorldStatus::getTime();
            const int next_karts_time = 
                   m_karts[karts_list[n+1]]->hasFinishedRace()
                ? (int)m_karts[karts_list[n+1]]->getFinishTime()
                : (int)WorldStatus::getTime();
            
            // Swap if next kart survived longer or has more lives
            bool swap = next_karts_time > this_karts_time ||
                        m_kart_info[karts_list[n+1]].m_goals 
                        > m_kart_info[karts_list[n]].m_goals;

            if(swap)
            {
                int tmp = karts_list[n+1];
                karts_list[n+1] = karts_list[n];
                karts_list[n] = tmp;
                sorted = false;
                break;
            } 
        }   // for n = 0; n < NUM_KARTS-1
    } while(!sorted);
    
    for( unsigned int n = 0; n < NUM_KARTS; ++n )
    {
        setKartPosition(karts_list[n], n+1);
    }
    delete [] karts_list;
    endSetKartPositions();
}   // updateKartRank

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
    updateKartRanks();
    WorldWithRank::terminateRace();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Called then a battle is restarted.
 */
void SoccerWorld::restartRace()
{
    WorldWithRank::restartRace();
    
    m_can_score_points = true;
    
    const unsigned int kart_amount = m_karts.size();
    
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_kart_info[n].m_goals         = 0;
        
        // no positions in this mode
        m_karts[n]->setPosition(-1);
    }// next kart
    
    // remove old soccer events
    m_soccer_events.clear();

    // add initial occer event
    SoccerEvent evt;
    evt.m_time = 0.0f;
    evt.m_kart_info = m_kart_info;
    m_soccer_events.push_back(evt);
}   // restartRace

//-----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void SoccerWorld::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        
        // reset color
        rank_info.lap = -1;
        
        rank_info.r = 0.5;
        rank_info.g = 0.5;
        rank_info.b = 0.5;
        
        char goals[4];
        sprintf(goals, "%i", m_kart_info[i].m_goals);
        
        rank_info.m_text = goals;
    }
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
    bool kart_over_ground = m_physics->projectKartDownwards(kart);

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
