//  $Id$
//
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

#include <assert.h>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <ctime>

#include "modes/world.hpp"
#include "herring_manager.hpp"
#include "projectile_manager.hpp"
#include "gui/menu_manager.hpp"
#include "file_manager.hpp"
#include "player_kart.hpp"
#include "auto_kart.hpp"
#include "track.hpp"
#include "kart_properties_manager.hpp"
#include "track_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "callback_manager.hpp"
#include "history.hpp"
#include "constants.hpp"
#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "translation.hpp"
#include "highscore_manager.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "robots/default_robot.hpp"
#include "unlock_manager.hpp"
#include "network/network_manager.hpp"
#include "network/race_state.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif

//-----------------------------------------------------------------------------
World::World()
{
    RaceManager::setWorld(this);
    race_state            = new RaceState();
    m_track               = NULL;
    m_faster_music_active = false;
    m_fastest_lap         = 9999999.9f;
    m_fastest_kart        = 0;
    m_eliminated_karts    = 0;
    m_eliminated_players  = 0;

    m_clock.setMode( CHRONO );
    
    // Grab the track file
    try
    {
        m_track = track_manager->getTrack(race_manager->getTrackName());
    }
    catch(std::runtime_error)
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), 
                 "Track '%s' not found.\n",race_manager->getTrackName().c_str());
        throw std::runtime_error(msg);
    }

    // Create the physics
    m_physics = new Physics();

    assert(race_manager->getNumKarts() > 0);

    // Load the track models - this must be done before the karts so that the
    // karts can be positioned properly on (and not in) the tracks.
    loadTrack() ;

    m_player_karts.resize(race_manager->getNumPlayers());
    m_network_karts.resize(race_manager->getNumPlayers());
    m_local_player_karts.resize(race_manager->getNumLocalPlayers());

    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        int position = i+1;   // position start with 1
        btTransform init_pos=m_track->getStartTransform(position);
        Kart* newkart;
        const std::string& kart_name = race_manager->getKartName(i);
        int local_player_id          = race_manager->getKartLocalPlayerId(i);
        int global_player_id         = race_manager->getKartGlobalPlayerId(i);
        if(user_config->m_profile)
        {
            // In profile mode, load only the old kart
            newkart = new DefaultRobot(kart_name, position, init_pos);
    	    // Create a camera for the last kart (since this way more of the 
	        // karts can be seen.
            if(i==race_manager->getNumKarts()-1) 
            {
                scene->createCamera(local_player_id, newkart);
            }
        }
        else
        {
            switch(race_manager->getKartType(i))
            {
            case RaceManager::KT_PLAYER:
                newkart = new PlayerKart(kart_name, position,
                                         &(user_config->m_player[local_player_id]),
                                         init_pos, local_player_id);
                m_player_karts[global_player_id] = (PlayerKart*)newkart;
                m_local_player_karts[local_player_id] = static_cast<PlayerKart*>(newkart);
                break;
            case RaceManager::KT_NETWORK_PLAYER:
                newkart = new NetworkKart(kart_name, position, init_pos,
                                          global_player_id);
                m_network_karts[global_player_id] = static_cast<NetworkKart*>(newkart);
                m_player_karts[global_player_id] = (PlayerKart*)newkart;
                break;
            case RaceManager::KT_AI:
                newkart = loadRobot(kart_name, position, init_pos);
                break;
            case RaceManager::KT_GHOST:
                break;
            case RaceManager::KT_LEADER: 
                break;
            }
        }   // if !user_config->m_profile
        if(user_config->m_replay_history)
        {
            history->LoadKartData(newkart, i);
        }
        newkart -> getModelTransform() -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

        scene->add ( newkart -> getModelTransform() ) ;
        m_kart.push_back(newkart);
        newkart->setWorldKartId(m_kart.size()-1);
    }  // for i

    resetAllKarts();

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
    //ssgSetBackFaceCollisions ( !not defined! race_manager->mirror ) ;
#endif

    Highscores::HighscoreType hst = (race_manager->getMinorMode()==RaceManager::RM_TIME_TRIAL) 
                                  ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                  : Highscores::HST_RACE_OVERALL_TIME;

    m_highscores   = highscore_manager->getHighscores(hst);

    callback_manager->initAll();
    menu_manager->switchToRace();

    m_track->startMusic();

    network_manager->worldLoaded();
}   // World

//-----------------------------------------------------------------------------
World::~World()
{
    delete race_state;
    m_track->cleanup();
    // Clear all callbacks
    callback_manager->clear(CB_TRACK);

    for ( unsigned int i = 0 ; i < m_kart.size() ; i++ )
        delete m_kart[i];

    m_kart.clear();
    projectile_manager->cleanup();
    delete m_physics;

    sound_manager -> stopMusic();

    sgVec3 sun_pos;
    sgVec4 ambient_col, specular_col, diffuse_col;
    sgSetVec3 ( sun_pos, 0.0f, 0.0f, 1.0f );
    sgSetVec4 ( ambient_col , 0.2f, 0.2f, 0.2f, 1.0f );
    sgSetVec4 ( specular_col, 1.0f, 1.0f, 1.0f, 1.0f );
    sgSetVec4 ( diffuse_col , 1.0f, 1.0f, 1.0f, 1.0f );

    ssgGetLight ( 0 ) -> setPosition ( sun_pos ) ;
    ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT , ambient_col  ) ;
    ssgGetLight ( 0 ) -> setColour ( GL_DIFFUSE , diffuse_col ) ;
    ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, specular_col ) ;
}   // ~World
//-----------------------------------------------------------------------------
void World::terminateRace()
{
    m_clock.pause();
    menu_manager->pushMenu(MENUID_RACERESULT);
    estimateFinishTimes();
    unlock_manager->raceFinished();
}
//-----------------------------------------------------------------------------
/** Waits till each kart is resting on the ground
 *
 * Does simulation steps still all karts reach the ground, i.e. are not
 * moving anymore
 */
void World::resetAllKarts()
{
    bool all_finished=false;
    // kart->isInRest() is not fully correct, since it only takes the
    // velocity in count, which might be close to zero when the kart
    // is just hitting the floor, before being pushed up again by
    // the suspension. So we just do a longer initial simulation,
    // which should be long enough for all karts to be firmly on ground.
    for(int i=0; i<200; i++) m_physics->update(1.f/60.f);

    // Stil wait will all karts are in rest (and handle the case that a kart
    // fell through the ground, which can happen if a kart falls for a long
    // time, therefore having a high speed when hitting the ground.
    while(!all_finished)
    {
        m_physics->update(1.f/60.f);
        all_finished=true;
        for ( Karts::iterator i=m_kart.begin(); i!=m_kart.end(); i++)
        {
            if(!(*i)->isInRest()) 
            {
                float           hot;
                Vec3            normal;
                const Material *material;
                // We can't use (*i)->getXYZ(), since this is only defined 
                // after update() was called. Instead we have to get the
                // real position of the rigid body.
                btTransform     t;
                (*i)->getBody()->getMotionState()->getWorldTransform(t);
                // This test can not be done only once before the loop, since 
                // it can happen that the kart falls through the track later!
                m_track->getTerrainInfo(t.getOrigin(), &hot, &normal, &material);
                if(!material)
                {
                    fprintf(stderr, "ERROR: no valid starting position for kart %d on track %s.\n",
			    (int)(i-m_kart.begin()), m_track->getIdent().c_str());
                    exit(-1);
                }
                all_finished=false;
                break;
            }
        }
    }   // while

}   // resetAllKarts

//-----------------------------------------------------------------------------
void World::update(float dt)
{
    // Clear race state so that new information can be stored
    race_state->clear();
    if(user_config->m_replay_history) dt=history->GetNextDelta();

    if(!user_config->m_replay_history) history->StoreDelta(dt);
    if(network_manager->getMode()!=NetworkManager::NW_CLIENT) m_physics->update(dt);
    for (int i = 0 ; i <(int) m_kart.size(); ++i)
    {
        // Update all karts that are not eliminated
        if(!m_kart[i]->isEliminated()) m_kart[i]->update(dt) ;
    }

    projectile_manager->update(dt);
    herring_manager->update(dt);

    for ( Karts::size_type i = 0 ; i < m_kart.size(); ++i)
    {
        if(m_kart[i]->isEliminated()) continue;   // ignore eliminated kart
        if(!m_kart[i]->hasFinishedRace()) updateRacePosition((int)i);
        if(m_kart[i]->isPlayerKart()) m_kart[i]->addMessages();   // add 'wrong direction'
    }

    /* Routine stuff we do even when paused */
    callback_manager->update(dt);
}
// ----------------------------------------------------------------------------
void World::updateHighscores()
{
    // Add times to highscore list. First compute the order of karts,
    // so that the timing of the fastest kart is added first (otherwise
    // someone might get into the highscore list, only to be kicked out
    // again by a faster kart in the same race), which might be confusing
    // if we ever decide to display a message (e.g. during a race)
    unsigned int *index = new unsigned int[m_kart.size()];
    for (unsigned int i=0; i<m_kart.size(); i++ )
    {
        index[m_kart[i]->getPosition()-1] = i;
    }

    for(unsigned int pos=0; pos<m_kart.size(); pos++)
    {
#ifdef DEBUG
        // FIXME begin: triggered if the positions of the karts are incorrect:
        if(index[pos]<0)
        {
            fprintf(stderr, "Error, incorrect kart positions:");
            for (unsigned int i=0; i<m_kart.size(); i++ )
            {
                fprintf(stderr, "i=%d position %d\n",i, m_kart[i]->getPosition());
            }
        }
#endif

        // Only record times for player karts
        if(!m_kart[index[pos]]->isPlayerKart()) continue;

        PlayerKart *k = (PlayerKart*)m_kart[index[pos]];

        Highscores::HighscoreType hst = (race_manager->getMinorMode()==
                                         RaceManager::RM_TIME_TRIAL) 
                                         ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                         : Highscores::HST_RACE_OVERALL_TIME;
        if(m_highscores->addData(hst, k->getName(),
                                 k->getPlayer()->getName(),
                                 k->getFinishTime())>0      )
        {
            highscore_manager->Save();
        }
    }
    delete []index;
}   // updateHighscores


//-----------------------------------------------------------------------------
void World::estimateFinishTimes()
{
    for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
    {
        if(!m_kart[i]->hasFinishedRace())
        {
            const float est_finish_time = m_kart[i]->estimateFinishTime();
            m_kart[i]->raceFinished(est_finish_time);
        }  // if !hasFinishedRace
    }   // for i
}  // estimateFinishTimes

//-----------------------------------------------------------------------------
void World::printProfileResultAndExit()
{
    float min_t=999999.9f, max_t=0.0, av_t=0.0;
    for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
    {
        max_t = std::max(max_t, m_kart[i]->getFinishTime());
        min_t = std::min(min_t, m_kart[i]->getFinishTime());
        av_t += m_kart[i]->getFinishTime();
        printf("%s  start %d  end %d time %f\n",
            m_kart[i]->getName().c_str(),(int)i,
            m_kart[i]->getPosition(),
            m_kart[i]->getFinishTime());
    } 
    printf("min %f  max %f  av %f\n",min_t, max_t, av_t/m_kart.size());
    std::exit(-2);
}   // printProfileResultAndExit

//-----------------------------------------------------------------------------
/** Called in follow-leader-mode to remove the last kart
*/
void World::removeKart(int kart_number)
{
    Kart *kart = m_kart[kart_number];
    // Display a message about the eliminated kart in the race gui 
    RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
    if(m)
    {
        for (std::vector<PlayerKart*>::iterator i  = m_player_karts.begin();
                                                i != m_player_karts.end();  i++ )
        {   
            if(*i==kart) 
            {
                m->addMessage(_("You have been\neliminated!"), *i, 2.0f, 60);
            }
            else
            {
                char s[MAX_MESSAGE_LENGTH];
                snprintf(s, MAX_MESSAGE_LENGTH,_("'%s' has\nbeen eliminated."),
                         kart->getName().c_str());
                m->addMessage( s, *i, 2.0f, 60);
            }
        }   // for i in kart
    }   // if raceMenu exist
    if(kart->isPlayerKart())
    {
        // Change the camera so that it will be attached to the leader 
        // and facing backwards.
        Camera* camera=((PlayerKart*)kart)->getCamera();
        camera->setMode(Camera::CM_LEADER_MODE);
        m_eliminated_players++;
    }
    projectile_manager->newExplosion(kart->getXYZ());
    // The kart can't be really removed from the m_kart array, since otherwise 
    // a race can't be restarted. So it's only marked to be eliminated (and 
    // ignored in all loops). Important:world->getCurrentNumKarts() returns 
    // the number of karts still racing. This value can not be used for loops 
    // over all karts, use race_manager->getNumKarts() instead!
    race_manager->RaceFinished(kart, m_clock.getTime());
    kart->eliminate();
    m_eliminated_karts++;

}   // removeKart

//-----------------------------------------------------------------------------
void World::updateRacePosition ( int k )
{
    int p = 1 ;

    /* Find position of kart 'k' */

    for ( Karts::size_type j = 0 ; j < m_kart.size() ; ++j )
    {
        if(int(j) == k) continue;
        if(m_kart[j]->isEliminated()) continue;   // eliminated karts   

        // Count karts ahead of the current kart, i.e. kart that are already
        // finished (the current kart k has not yet finished!!), have done more
        // laps, or the same number of laps, but a greater distance.
        if (m_kart[j]->hasFinishedRace()                                           ||
            m_kart[j]->getLap() >  m_kart[k]->getLap()                             ||
            (m_kart[j]->getLap() == m_kart[k]->getLap() &&
             m_kart[j]->getDistanceDownTrack() > m_kart[k]->getDistanceDownTrack()) )
            p++ ;
    }

    m_kart[k]->setPosition(p);
    // Switch on faster music (except in follow leader mode) if not already 
    // done so, and the first kart is doing its last lap, and the estimated
    // remaining time is less than 30 seconds.
    if(!m_faster_music_active                                       && 
        m_kart[k]->getLap()==race_manager->getNumLaps()-1           && 
        p==1                                                        &&
        race_manager->getMinorMode()!=RaceManager::RM_FOLLOW_LEADER &&
        m_kart[k]->estimateFinishTime()-getTime()<30.0f                ) 
    {
        sound_manager->switchToFastMusic();
        m_faster_music_active=true;
    }
}   // updateRacePosition

//-----------------------------------------------------------------------------
/** Cleans up old herrings (from a previous race), removes old track specific
 *  herring models, and loads the actual track.
 */
void World::loadTrack()
{
    // remove old herrings (from previous race), and remove old
    // track specific herring models
    herring_manager->cleanup();
    if(race_manager->getMajorMode()== RaceManager::RM_GRAND_PRIX)
    {
        try
        {
            herring_manager->loadHerringStyle(race_manager->getHerringStyle());
        }
        catch(std::runtime_error)
        {
            fprintf(stderr, "The grand prix '%s' contains an invalid herring style '%s'.\n",
                    race_manager->getGrandPrix()->getName().c_str(),
                    race_manager->getHerringStyle().c_str());
            fprintf(stderr, "Please fix the file '%s'.\n",
                    race_manager->getGrandPrix()->getFilename().c_str());
        }
    }
    else
    {
        try
        {
            herring_manager->loadHerringStyle(m_track->getHerringStyle());
        }
        catch(std::runtime_error)
        {
            fprintf(stderr, "The track '%s' contains an invalid herring style '%s'.\n",
                    m_track->getName(), m_track->getHerringStyle().c_str());
            fprintf(stderr, "Please fix the file '%s'.\n",
                    m_track->getFilename().c_str());
        }
    }

    m_track->loadTrackModel();
}   // loadTrack

//-----------------------------------------------------------------------------
void World::restartRace()
{
    m_clock.reset();
    m_faster_music_active = false;
    m_eliminated_karts    = 0;
    m_eliminated_players  = 0;

    for ( Karts::iterator i = m_kart.begin(); i != m_kart.end() ; ++i )
    {
        (*i)->reset();
    }
    
    resetAllKarts();
    
    // Start music from beginning
    sound_manager->stopMusic();
    m_track->startMusic();

    // Enable SFX again
    sfx_manager->resumeAll();

    herring_manager->reset();
    projectile_manager->cleanup();
    race_manager->reset();
    callback_manager->reset();

    // Resets the cameras in case that they are pointing too steep up or down
    scene->reset();
}   // restartRace

//-----------------------------------------------------------------------------
Kart* World::loadRobot(const std::string& kart_name, int position,
                       const btTransform& init_pos)
{
    Kart* currentRobot;
    
    const int NUM_ROBOTS = 1;

    switch(m_random.get(NUM_ROBOTS))
    {
        case 0:
            currentRobot = new DefaultRobot(kart_name, position, init_pos);
            break;
        default:
            std::cerr << "Warning: Unknown robot, using default." << std::endl;
            currentRobot = new DefaultRobot(kart_name, position, init_pos);
            break;
    }
    
    return currentRobot;
}

//-----------------------------------------------------------------------------
void  World::pause()
{
    sound_manager->pauseMusic();
    sfx_manager->pauseAll();
    m_clock.pause();
}

//-----------------------------------------------------------------------------
void  World::unpause()
{
    sound_manager->resumeMusic() ;
    sfx_manager->resumeAll();
    m_clock.unpause();
}

/* EOF */
