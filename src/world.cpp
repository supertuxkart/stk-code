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

#include <assert.h>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <ctime>

#include "world.hpp"
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
#include "sound_manager.hpp"
#include "translation.hpp"
#include "highscore_manager.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "robots/default_robot.hpp"
#include "unlock_manager.hpp"
#ifdef HAVE_GHOST_REPLAY
#  include "replay_player.hpp"
#endif


#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif

World* world = 0;

World::World()
#ifdef HAVE_GHOST_REPLAY
, m_p_replay_player(NULL)
#endif
{
    delete world;
    world          = this;
    m_phase        = SETUP_PHASE;
    m_track        = NULL;

    m_clock        = 0.0f;
    m_fastest_lap  = 9999999.9f;
    m_fastest_kart = 0;


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
    m_physics = new Physics(m_track->getGravity());

    assert(race_manager->getNumKarts() > 0);

    // Load the track models - this must be done before the karts so that the
    // karts can be positioned properly on (and not in) the tracks.
    loadTrack() ;

    int playerIndex = 0;
    for(unsigned int position=0; position<race_manager->getNumKarts(); position++)
    {
        sgCoord init_pos;
        m_track->getStartCoords(position, &init_pos);
        Kart* newkart;
        const std::string& kart_name=race_manager->getKartName(position);
        if(user_config->m_profile)
        {
            // In profile mode, load only the old kart
            newkart = new DefaultRobot(kart_name, position, init_pos);
    	    // Create a camera for the last kart (since this way more of the 
	        // karts can be seen.
            if(position==race_manager->getNumKarts()-1) 
            {
                scene->createCamera(race_manager->getNumPlayers(), playerIndex);
            }
        }
        else
        {
            if (race_manager->isPlayer(position))
            {
                Camera *cam = scene->createCamera(race_manager->getNumPlayers(), playerIndex);
                // the given position belongs to a player
                newkart = new PlayerKart(kart_name, position,
                                         &(user_config->m_player[playerIndex]),
                                         init_pos, cam);
                m_player_karts.push_back((PlayerKart*)newkart);
                playerIndex++;
            }
            else
            {
                newkart = loadRobot(kart_name, position, init_pos);
            }
        }   // if !user_config->m_profile
        if(user_config->m_replay_history)
        {
            history->LoadKartData(newkart, position);
        }
        newkart -> getModelTransform() -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

        scene->add ( newkart -> getModelTransform() ) ;
        m_kart.push_back(newkart);
    }  // for i

    resetAllKarts();

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
    //ssgSetBackFaceCollisions ( !not defined! race_manager->mirror ) ;
#endif

    Highscores::HighscoreType hst = (race_manager->getRaceMode()==RaceManager::RM_TIME_TRIAL) 
                                  ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                  : Highscores::HST_RACE_OVERALL_TIME;

    m_highscores   = highscore_manager->getHighscores(hst);

    callback_manager->initAll();
    menu_manager->switchToRace();

    m_track->playMusic();

    m_phase = user_config->m_profile ? RACE_PHASE : SETUP_PHASE;

#ifdef HAVE_GHOST_REPLAY
    m_replay_recorder.initRecorder( race_manager->getNumKarts() );

    m_p_replay_player = new ReplayPlayer;
    if( !loadReplayHumanReadable( "test1" ) ) 
    {
        delete m_p_replay_player;
        m_p_replay_player = NULL;
    }
    if( m_p_replay_player ) m_p_replay_player->showReplayAt( 0.0 );
#endif
}   // World

//-----------------------------------------------------------------------------
World::~World()
{
#ifdef HAVE_GHOST_REPLAY
    saveReplayHumanReadable( "test" );
#endif
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

#ifdef HAVE_GHOST_REPLAY
    m_replay_recorder.destroy();
    if( m_p_replay_player )
    {
        m_p_replay_player->destroy();
        delete m_p_replay_player;
        m_p_replay_player = NULL;
    }
#endif
}   // ~World

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
    for(int i=0; i<100; i++) m_physics->update(1.f/60.f);
    while(!all_finished)
    {
        m_physics->update(1.f/60.f);
        all_finished=true;
        for ( Karts::iterator i=m_kart.begin(); i!=m_kart.end(); i++)
        {
            if(!(*i)->isInRest()) 
            {
                all_finished=false;
                break;
            }
        }
    }   // while

}   // resetAllKarts

//-----------------------------------------------------------------------------
void World::update(float dt)
{
    if(user_config->m_replay_history) dt=history->GetNextDelta();
    updateRaceStatus(dt);

    if( getPhase() == FINISH_PHASE )
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

        // Don't record the time for the last kart, since it didn't finish
        // the race - unless it's timetrial (then there is only one kart)
        unsigned int karts_to_enter = (race_manager->getRaceMode()==RaceManager::RM_TIME_TRIAL) 
                                    ? (unsigned int)m_kart.size() : (unsigned int)m_kart.size()-1;
        for(unsigned int pos=0; pos<karts_to_enter; pos++)
        {
            // Only record times for player karts
            if(!m_kart[index[pos]]->isPlayerKart()) continue;

            PlayerKart *k = (PlayerKart*)m_kart[index[pos]];
            
            Highscores::HighscoreType hst = (race_manager->getRaceMode()==
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
        pause();
        menu_manager->pushMenu(MENUID_RACERESULT);
    }
    if(!user_config->m_replay_history) history->StoreDelta(dt);
    m_physics->update(dt);
    for ( Karts::size_type i = 0 ; i < m_kart.size(); ++i)
    {
        m_kart[i]->update(dt) ;
    }

    projectile_manager->update(dt);
    herring_manager->update(dt);

    for ( Karts::size_type i = 0 ; i < m_kart.size(); ++i)
    {
        if(!m_kart[i]->raceIsFinished()) updateRacePosition((int)i);
        m_kart[i]->addMessages();
    }

    /* Routine stuff we do even when paused */
    callback_manager->update(dt);

#ifdef HAVE_GHOST_REPLAY
    // we start recording after START_PHASE, since during start-phase m_clock is incremented
    // normally, but after switching to RACE_PHASE m_clock is set back to 0.0
    if( m_phase == GO_PHASE ) 
    {
        m_replay_recorder.pushFrame();
        if( m_p_replay_player ) m_p_replay_player->showReplayAt( m_clock );
    }
#endif
}

#ifdef HAVE_GHOST_REPLAY
//-----------------------------------------------------------------------------
bool World::saveReplayHumanReadable( std::string const &filename ) const
{
    std::string path;
    path = file_manager->getReplayFile(filename+"."+ReplayBase::REPLAY_FILE_EXTENSION_HUMAN_READABLE);

    FILE *fd = fopen( path.c_str(), "w" );
    if( !fd ) 
    {
        fprintf(stderr, _("Error while opening replay file for writing '%s'\n"), path.c_str());
        return false;
    }
    int  nKarts = world->getNumKarts();
    const char *version = "unknown";
#ifdef VERSION
    version = VERSION;
#endif
    fprintf(fd, "Version:  %s\n",   version);
    fprintf(fd, "numkarts: %d\n",   m_kart.size());
    fprintf(fd, "numplayers: %d\n", race_manager->getNumPlayers());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      m_track->getIdent().c_str());

    for(int i=0; i<race_manager->getNumKarts(); i++)
    {
        fprintf(fd, "model %d: %s\n", i, race_manager->getKartName(i).c_str());
    }
    if( !m_replay_recorder.saveReplayHumanReadable( fd ) )
    {
        fclose( fd ); fd = NULL;
        return false;
    }

    fclose( fd ); fd = NULL;

    return true;
}  // saveReplayHumanReadable
#endif  // HAVE_GHOST_REPLAY

#ifdef HAVE_GHOST_REPLAY
//-----------------------------------------------------------------------------
bool World::loadReplayHumanReadable( std::string const &filename )
{
    assert( m_p_replay_player );
    m_p_replay_player->destroy();

    std::string path = file_manager->getReplayFile(filename+"."+ 
            ReplayBase::REPLAY_FILE_EXTENSION_HUMAN_READABLE);

    try
    {
        path = file_manager->getPath(path.c_str());
    }
    catch(std::runtime_error& e)
    {
        fprintf( stderr, _("Couldn't find replay-file: '%s'\n"), path.c_str() );
        return false;
    }

    FILE *fd = fopen( path.c_str(), "r" );
    if( !fd ) 
    {
        fprintf(stderr, _("Error while opening replay file for loading '%s'\n"), path.c_str());
        return false;
    }

    bool blnRet = m_p_replay_player->loadReplayHumanReadable( fd );

    fclose( fd ); fd = NULL;

    return blnRet;
}  // loadReplayHumanReadable
#endif  // HAVE_GHOST_REPLAY

//-----------------------------------------------------------------------------

void World::updateRaceStatus(float dt)
{
    switch (m_phase) {
        case SETUP_PHASE:   m_clock=0.0f;  m_phase=READY_PHASE;
                            dt = 0.0f;  // solves the problem of adding track loading time
                            break;                // loading time, don't play sound yet
        case READY_PHASE:   if(m_clock==0.0)      // play sound at beginning of next frame
                                sound_manager->playSfx(SOUND_PRESTART);
                            if(m_clock>1.0)
                            {
                                m_phase=SET_PHASE;   
                                sound_manager->playSfx(SOUND_PRESTART);
                            }
                            break;
        case SET_PHASE  :   if(m_clock>2.0) 
                            {
                                m_phase=GO_PHASE;
                                m_clock=0.0f;
                                sound_manager->playSfx(SOUND_START);
                                // Reset the brakes now that the prestart 
                                // phase is over (braking prevents the karts 
                                // from sliding downhill)
                                for(unsigned int i=0; i<m_kart.size(); i++) 
                                {
                                    m_kart[i]->resetBrakes();
                                }
#ifdef HAVE_GHOST_REPLAY
                                // push positions at time 0.0 to replay-data
                                m_replay_recorder.pushFrame();
#endif
                            }
                            break;
        case GO_PHASE  :    if(m_clock>1.0) m_phase=RACE_PHASE;  break;
        default        :    break;
    }   // switch
    
    // Now handling of normal race
    // ===========================
    m_clock += dt;

    /*if all players have finished, or if only one kart is not finished when
      not in time trial mode, the race is over. Players are the last in the
      vector, so substracting the number of players finds the first player's
      position.*/
    int new_finished_karts   = 0;
    for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
    {
        // FIXME: this part should be done as part of Kart::update
        if ((m_kart[i]->getLap () >= race_manager->getNumLaps()) && !m_kart[i]->raceIsFinished())
        {
            m_kart[i]->setFinishingState(m_clock);

            race_manager->addKartResult((int)i, m_kart[i]->getPosition(), m_clock);

            ++new_finished_karts;
            if(m_kart[i]->isPlayerKart())
            {
                race_manager->PlayerFinishes();
                RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
                if(m)
                {
                    m->addMessage(m_kart[i]->getPosition()==1
                                  ? _("You won") 
                                  : _("You finished") ,
                                  m_kart[i], 2.0f, 60);
                    m->addMessage( _("the race!"), m_kart[i], 2.0f, 60);
                }
            }
        }
    }
    race_manager->addFinishedKarts(new_finished_karts);

    // 1) All karts are finished --> end the race
    // ==========================================
    if(race_manager->getFinishedKarts() >= race_manager->getNumKarts() )
    {
        m_phase = FINISH_PHASE;
	    if(user_config->m_profile<0)  // profiling number of laps -> print stats
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
	    }
    }   // if all karts are finished

    // 2) All player karts are finished --> wait some
    //    time for AI karts to arrive before finishing
    // ===============================================
    else if(race_manager->allPlayerFinished() && m_phase == RACE_PHASE &&
            !user_config->m_profile)
    {
        m_phase = DELAY_FINISH_PHASE;
        m_finish_delay_start_time = m_clock;
    }

    // 3) If the 'wait for AI' time is over, estimate arrival times & finish
    // =====================================================================
    else if(m_phase==DELAY_FINISH_PHASE &&
            m_clock-m_finish_delay_start_time>TIME_DELAY_TILL_FINISH &&
            !user_config->m_profile)
    {
        m_phase = FINISH_PHASE;
        for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
        {
            if(!m_kart[i]->raceIsFinished())
            {
                const float est_finish_time = m_kart[i]->estimateFinishTime();
                m_kart[i]->setFinishingState(est_finish_time);
                race_manager->addKartResult((int)i, m_kart[i]->getPosition(),
                                            est_finish_time);
            }   // if !raceIsFinished
        }   // for i
    }
    if(m_phase==FINISH_PHASE) unlock_manager->raceFinished();
}  // updateRaceStatus

//-----------------------------------------------------------------------------
void World::updateRacePosition ( int k )
{
    int p = 1 ;

    /* Find position of kart 'k' */

    for ( Karts::size_type j = 0 ; j < m_kart.size() ; ++j )
    {
        if ( int(j) == k ) continue ;

        // Count karts ahead of the current kart, i.e. kart that are already
        // finished (the current kart k has not yet finished!!), have done more
        // laps, or the same number of laps, but a greater distance.
        if (m_kart[j]->raceIsFinished()                                          ||
            m_kart[j]->getLap() >  m_kart[k]->getLap()                             ||
            (m_kart[j]->getLap() == m_kart[k]->getLap() &&
             m_kart[j]->getDistanceDownTrack() > m_kart[k]->getDistanceDownTrack()) )
            p++ ;
    }

    m_kart [ k ] -> setPosition ( p ) ;
}   // updateRacePosition


//-----------------------------------------------------------------------------
void World::loadTrack()
{
    // remove old herrings (from previous race), and remove old
    // track specific herring models
    herring_manager->cleanup();
    if(race_manager->getRaceMode()== RaceManager::RM_GRAND_PRIX)
    {
        try
        {
            herring_manager->loadHerringStyle(race_manager->getHerringStyle());
        }
        catch(std::runtime_error)
        {
            fprintf(stderr, "The cup '%s' contains an invalid herring style '%s'.\n",
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
    m_clock        = 0.0f;
    m_phase        = SETUP_PHASE;

    for ( Karts::iterator i = m_kart.begin(); i != m_kart.end() ; ++i )
    {
        (*i)->reset();
    }
    resetAllKarts();
    herring_manager->reset();
    projectile_manager->cleanup();
    race_manager->reset();
    callback_manager->reset();

    // Resets the cameras in case that they are pointing too steep up or down
    scene->reset();
#ifdef HAVE_GHOST_REPLAY
    m_replay_recorder.destroy();
    m_replay_recorder.initRecorder( race_manager->getNumKarts() );

    if( m_p_replay_player ) 
    {
        m_p_replay_player->reset();
        m_p_replay_player->showReplayAt( 0.0 );
    }
#endif
}   // restartRace

//-----------------------------------------------------------------------------
Kart* World::loadRobot(const std::string& kart_name, int position,
                       sgCoord init_pos)
{
    Kart* currentRobot;
    
    const int NUM_ROBOTS = 1;

    srand((unsigned)std::time(0));

    switch(rand() % NUM_ROBOTS)
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
    sound_manager -> pauseMusic() ;
    m_phase = LIMBO_PHASE;
}

//-----------------------------------------------------------------------------
void  World::unpause()
{
    sound_manager -> resumeMusic() ;
    m_phase = RACE_PHASE;
}

/* EOF */
