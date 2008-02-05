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
#include "loader.hpp"
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
#ifdef HAVE_GHOST_REPLAY
#  include "replay_player.hpp"
#endif


#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif

World* world = 0;

World::World(const RaceSetup& raceSetup_) : m_race_setup(raceSetup_)
#ifdef HAVE_GHOST_REPLAY
, m_p_replay_player(NULL)
#endif
{
    delete world;
    world          = this;
    m_phase        = START_PHASE;

    m_track        = NULL;

    m_clock        = 0.0f;
    m_fastest_lap  = 9999999.9f;
    m_fastest_kart = 0;


    // Grab the track file
    try
    {
        m_track = track_manager->getTrack(m_race_setup.m_track) ;
    }
    catch(std::runtime_error)
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), 
                 "Track '%s' not found.\n",m_race_setup.m_track.c_str());
        throw std::runtime_error(msg);
    }

    // Create the physics
    m_physics = new Physics(getGravity());

    assert(m_race_setup.m_karts.size() > 0);

    // Load the track models - this must be done before the karts so that the
    // karts can be positioned properly on (and not in) the tracks.
    loadTrack() ;

    int pos = 0;
    int playerIndex = 0;
    for (RaceSetup::Karts::iterator i = m_race_setup.m_karts.begin() ;
         i != m_race_setup.m_karts.end() ; ++i )
    {
        sgCoord init_pos;
        m_track->getStartCoords(pos, &init_pos);
        Kart* newkart;
        if(user_config->m_profile)
        {
            // In profile mode, load only the old kart
            newkart = new DefaultRobot (kart_properties_manager->getKart("tuxkart"), pos,
                    init_pos);
        }
        else
        {
            if (std::find(m_race_setup.m_players.begin(),
                          m_race_setup.m_players.end(), pos) != m_race_setup.m_players.end())
            {
                
                Camera *cam = scene->createCamera(m_race_setup.getNumPlayers(), playerIndex);
                // the given position belongs to a player
                newkart = new PlayerKart (kart_properties_manager->getKart(*i), pos,
                                          &(user_config->m_player[playerIndex]),
                                          init_pos, cam);
                playerIndex++;
            }
            else
            {
                newkart = loadRobot(kart_properties_manager->getKart(*i), pos,
                    init_pos);
            }
        }   // if !user_config->m_profile
        if(user_config->m_replay_history)
        {
            history->LoadKartData(newkart, pos);
        }
        newkart -> getModelTransform() -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

        scene->add ( newkart -> getModelTransform() ) ;
        m_kart.push_back(newkart);
        pos++;
    }  // for i

    resetAllKarts();

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
    //ssgSetBackFaceCollisions ( m_race_setup.mirror ) ;
#endif

    Highscores::HighscoreType hst = (m_race_setup.m_mode==RaceSetup::RM_TIME_TRIAL) 
                                  ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                  : Highscores::HST_RACE_OVERALL_TIME;

    m_highscores   = highscore_manager->getHighscores(hst, (int)m_kart.size(),
                                                      m_race_setup.m_difficulty, 
                                                      m_track->getName(),
                                                      m_race_setup.m_num_laps);

    callback_manager->initAll();
    menu_manager->switchToRace();

    const std::string& MUSIC_NAME= track_manager->getTrack(m_race_setup.m_track)->getMusic();
    if (MUSIC_NAME.size()>0) sound_manager->playMusic(MUSIC_NAME.c_str());

    if(user_config->m_profile)
    {
        m_ready_set_go = -1;
        m_phase        = RACE_PHASE;
    }
    else
    {
        m_phase        = START_PHASE;  // profile starts without countdown
        m_ready_set_go = 3;
    }

#ifdef HAVE_GHOST_REPLAY
    m_replay_recorder.initRecorder( m_race_setup.getNumKarts() );

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
    for(int i=0; i<10; i++) m_physics->update(1.f/60.f);
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

    checkRaceStatus();
    // this line was before checkRaceStatus. but m_clock is set to 0.0 in
    // checkRaceStatus on start, so m_clock would not be synchronized and the
    // first delta would not be added .. that would cause a gap in 
    // replay-recording
    m_clock += dt;

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
        unsigned int karts_to_enter = (m_race_setup.m_mode==RaceSetup::RM_TIME_TRIAL) 
                                    ? (unsigned int)m_kart.size() : (unsigned int)m_kart.size()-1;
        for(unsigned int pos=0; pos<karts_to_enter; pos++)
        {
            // Only record times for player karts
            if(!m_kart[index[pos]]->isPlayerKart()) continue;

            PlayerKart *k = (PlayerKart*)m_kart[index[pos]];
            
            Highscores::HighscoreType hst = (m_race_setup.m_mode==RaceSetup::RM_TIME_TRIAL) 
                                  ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                  : Highscores::HST_RACE_OVERALL_TIME;
            if(m_highscores->addData(hst, (int)m_kart.size(),
                     m_race_setup.m_difficulty, 
                     m_track->getName(),
                     k->getName(),
                     k->getPlayer()->getName(),
                     k->getFinishTime(),
                     m_race_setup.m_num_laps)>0)
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
    if( m_phase != START_PHASE ) 
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
    try
    {
        path = loader->getPath( ReplayBase::REPLAY_FOLDER );
    }
    catch(std::runtime_error& e)
    {
        fprintf( stderr, _("Couldn't find replay-path: '%s'\n"), ReplayBase::REPLAY_FOLDER.c_str() );
        return false;
    }
    path += DIR_SEPARATOR + filename + ".";
    path += ReplayBase::REPLAY_FILE_EXTENSION_HUMAN_READABLE;

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
    fprintf(fd, "numplayers: %d\n", m_race_setup.getNumPlayers());
    fprintf(fd, "difficulty: %d\n", m_race_setup.m_difficulty);
    fprintf(fd, "track: %s\n",      m_track->getIdent());

    for (RaceSetup::Karts::const_iterator i = m_race_setup.m_karts.begin() ;
         i != m_race_setup.m_karts.end() ; ++i )
    {
        fprintf(fd, "model %d: %s\n", i-m_race_setup.m_karts.begin(), (*i).c_str());
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

    std::string path = ReplayBase::REPLAY_FOLDER + DIR_SEPARATOR;
    path += filename + ".";
    path += ReplayBase::REPLAY_FILE_EXTENSION_HUMAN_READABLE;

    try
    {
        path = loader->getPath(path.c_str());
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

//-----------------------------------------------------------------------------
void World::checkRaceStatus()
{
    if (m_clock > 1.0 && m_ready_set_go == 0)
    {
        m_ready_set_go = -1;
    }
    else if (m_clock > 2.0 && m_ready_set_go == 1)
    {
        m_ready_set_go = 0;
        m_phase = RACE_PHASE;
        m_clock = 0.0f;
        sound_manager->playSfx(SOUND_START);
#ifdef HAVE_GHOST_REPLAY
        // push positions at time 0.0 to replay-data
        m_replay_recorder.pushFrame();
#endif
    }
    else if (m_clock > 1.0 && m_ready_set_go == 2)
    {
        sound_manager->playSfx(SOUND_PRESTART);
        m_ready_set_go = 1;
    }
    else if (m_clock > 0.0 && m_ready_set_go == 3)
    {
        sound_manager->playSfx(SOUND_PRESTART);
        m_ready_set_go = 2;
    }

    /*if all players have finished, or if only one kart is not finished when
      not in time trial mode, the race is over. Players are the last in the
      vector, so substracting the number of players finds the first player's
      position.*/
    int new_finished_karts   = 0;
    for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
    {
        if ((m_kart[i]->getLap () >= m_race_setup.m_num_laps) && !m_kart[i]->raceIsFinished())
        {
            m_kart[i]->setFinishingState(m_clock);

            race_manager->addKartScore((int)i, m_kart[i]->getPosition());

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
    // Different ending conditions:
    // 1) all players are finished -->
    //    wait TIME_DELAY_TILL_FINISH seconds - if no other end condition
    //    applies, end the game (and make up some artificial times for the
    //    outstanding karts).
    // 2) If only one kart is playing, finish when one kart is finished.
    // 3) Otherwise, wait till all karts except one is finished - the last
    //    kart obviously becoming the last
    if(race_manager->allPlayerFinished() && m_phase == RACE_PHASE)
    {
        m_phase = DELAY_FINISH_PHASE;
        m_finish_delay_start_time = m_clock;
    }
    else if(m_phase==DELAY_FINISH_PHASE &&
            m_clock-m_finish_delay_start_time>TIME_DELAY_TILL_FINISH)
    {
        m_phase = FINISH_PHASE;
        for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
        {
            if(!m_kart[i]->raceIsFinished())
            {
                // FIXME: Add some tenths to have different times - a better solution
                //        would be to estimate the distance to go and use this to
                //        determine better times.
                m_kart[i]->setFinishingState(m_clock+m_kart[i]->getPosition()*0.3f);
                race_manager->addKartScore((int)i, m_kart[i]->getPosition());

            }   // if !raceIsFinished
        }   // for i

    }
    else if(m_race_setup.getNumKarts() == 1)
    {
        if(race_manager->getFinishedKarts() == 1) m_phase = FINISH_PHASE;
    }
    else if(race_manager->getFinishedKarts() >= m_race_setup.getNumKarts() - 1)
    {
        m_phase = FINISH_PHASE;
        for ( Karts::size_type i = 0; i < m_kart.size(); ++i)
        {
            if(!m_kart[i]->raceIsFinished())
            {
                m_kart[i]->setFinishingState(m_clock);
            }   // if !raceIsFinished
        }   // for i
    }   // if getFinishedKarts()>=geNumKarts()

}  // checkRaceStatus

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
    std::string path = "data/";
    path += m_track->getIdent();
    path += ".loc";
    path = loader->getPath(path.c_str());

    // remove old herrings (from previous race), and remove old
    // track specific herring models
    herring_manager->cleanup();
    if(m_race_setup.m_mode == RaceSetup::RM_GRAND_PRIX)
    {
        try
        {
            herring_manager->loadHerringStyle(m_race_setup.getHerringStyle());
        }
        catch(std::runtime_error)
        {
            fprintf(stderr, "The cup '%s' contains an invalid herring style '%s'.\n",
                    race_manager->getGrandPrix()->getName().c_str(),
                    race_manager->getGrandPrix()->getHerringStyle().c_str());
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
    m_ready_set_go = 3;
    m_clock        = 0.0f;
    m_phase        = START_PHASE;

    for ( Karts::iterator i = m_kart.begin(); i != m_kart.end() ; ++i )
    {
        (*i)->reset();
    }
    resetAllKarts();
    herring_manager->reset();
    projectile_manager->cleanup();
    race_manager->reset();
    callback_manager->reset();

#ifdef HAVE_GHOST_REPLAY
    m_replay_recorder.destroy();
    m_replay_recorder.initRecorder( m_race_setup.getNumKarts() );

    if( m_p_replay_player ) 
    {
        m_p_replay_player->reset();
        m_p_replay_player->showReplayAt( 0.0 );
    }
#endif
}

//-----------------------------------------------------------------------------
Kart* World::loadRobot(const KartProperties *kart_properties, int position,
                 sgCoord init_pos)
{
    Kart* currentRobot;
    
    const int NUM_ROBOTS = 1;

    srand((unsigned)std::time(0));

    switch(rand() % NUM_ROBOTS)
    {
        case 0:
            currentRobot = new DefaultRobot(kart_properties, position,
                init_pos);
            break;
        default:
            std::cerr << "Warning: Unknown robot, using default." << std::endl;
            currentRobot = new DefaultRobot(kart_properties, position,
                init_pos);
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
