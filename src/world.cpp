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

//#define DEBUG_SHOW_DRIVEPOINTS //This would place a small sphere in every point
//of the driveline, the one at the first point
//is purple, the others are yellow.
#ifdef DEBUG_SHOW_DRIVEPOINTS
#include <plib/ssgAux.h>
#endif

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
#include "isect.hpp"
#include "track.hpp"
#include "kart_properties_manager.hpp"
#include "track_manager.hpp"
#include "race_manager.hpp"
#include "user_config.hpp"
#include "callback_manager.hpp"
#include "history.hpp"
#include "constants.hpp"
#include "sound_manager.hpp"
#include "ssg_help.hpp"
#include "translation.hpp"
#include "highscore_manager.hpp"
#include "scene.hpp"

#include "robots/default_robot.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif

World* world = 0;

World::World(const RaceSetup& raceSetup_) : m_race_setup(raceSetup_)
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

    // Start building the scene graph
    m_track_branch = new ssgBranch ;
    scene->add ( m_track_branch ) ;

    // Create the physics
    m_physics = new Physics(getGravity());

    assert(m_race_setup.m_karts.size() > 0);

    // Clear all callbacks, which might still be stored there from a previous race.
    callback_manager->clear(CB_TRACK);

    // Load the track models - this must be done before the karts so that the
    // karts can be positioned properly on (and not in) the tracks.
    loadTrack   ( ) ;

    m_static_ssg = new StaticSSG(m_track_branch, 1000);

    int pos = 0;
    int playerIndex = 0;
    for (RaceSetup::Karts::iterator i = m_race_setup.m_karts.begin() ;
         i != m_race_setup.m_karts.end() ; ++i )
    {
        // First determine the x/y/z position for the kart
        sgCoord init_pos = { { 0, 0, 0 }, { 0, 0, 0 } } ;
        //float hot=0.0;
        // Bug fix/workaround: sometimes the first kart would be too close
        // to the first driveline point and not to the last one -->
        // This kart would not get any lap counting done in the first
        // lap! Therefor -1.5 is subtracted from the y position - which
        // is a somewhat arbitrary value.
        init_pos.xyz[0] = (pos % 2 == 0) ? 1.5f : -1.5f ;
        init_pos.xyz[1] = -pos * 1.5f -1.5f;
        init_pos.xyz[2] = 1.0f;    // height must be and larger than the actual
                                   // hight for which hot is computed.
        ssgLeaf *leaf;
        sgVec4* normal;
        const float hot = GetHOT(init_pos.xyz, init_pos.xyz, &leaf, &normal);
        init_pos.xyz[2] = hot;

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
                // the given position belongs to a player
                newkart = new PlayerKart (kart_properties_manager->getKart(*i), pos,
                                          &(user_config->m_player[playerIndex++]),
                                          init_pos);
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
        newkart -> getModel () -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

        scene->add ( newkart -> getModel() ) ;
        m_kart.push_back(newkart);
        pos++;
    }  // for i

    resetAllKarts();
    m_number_collisions = new int[m_race_setup.getNumKarts()];
    for(unsigned int i=0; i<m_race_setup.getNumKarts(); i++) m_number_collisions[i]=0;

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
    //ssgSetBackFaceCollisions ( m_race_setup.mirror ) ;
#endif

    Highscores::HighscoreType hst = (m_race_setup.m_mode==RaceSetup::RM_TIME_TRIAL) 
                                  ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                  : Highscores::HST_RACE_OVERALL_TIME;

    m_highscores   = highscore_manager->getHighscores(hst, m_kart.size(),
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
}

//-----------------------------------------------------------------------------
World::~World()
{
    for ( unsigned int i = 0 ; i < m_kart.size() ; i++ )
        delete m_kart[i];

    m_kart.clear();
    projectile_manager->cleanup();
    delete [] m_number_collisions;
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
}

//-----------------------------------------------------------------------------
/** Waits till each kart is resting on the ground
 *
 * Does simulation steps still all karts reach the ground, i.e. are not
 * moving anymore
 */
void World::resetAllKarts()
{
#ifdef BULLET
    bool all_finished=false;
    for(int i=0; i<10; i++) m_physics->update(1./60.);
    while(!all_finished)
    {
        m_physics->update(1./60.);
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
#endif
}   // resetAllKarts

//-----------------------------------------------------------------------------
void World::draw()
{
}

//-----------------------------------------------------------------------------
void World::update(float delta)
{
    if(user_config->m_replay_history) delta=history->GetNextDelta();
    m_clock += delta;

    checkRaceStatus();

    // Count the number of collision in the next 'FRAMES_FOR_TRAFFIC_JAM' frames.
    // If a kart has more than one hit, play 'traffic jam' noise.
    static int nCount=0;
    const int FRAMES_FOR_TRAFFIC_JAM=20;
    nCount++;
    if(nCount==FRAMES_FOR_TRAFFIC_JAM)
    {
        for(unsigned int i=0; i<m_race_setup.getNumKarts(); i++) m_number_collisions[i]=0;
        nCount=0;
    }
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
                                    ? m_kart.size() : m_kart.size()-1;
        for(unsigned int pos=0; pos<karts_to_enter; pos++)
        {
            // Only record times for player karts
            if(!m_kart[index[pos]]->isPlayerKart()) continue;

            PlayerKart *k = (PlayerKart*)m_kart[index[pos]];
            
            Highscores::HighscoreType hst = (m_race_setup.m_mode==RaceSetup::RM_TIME_TRIAL) 
                                  ? Highscores::HST_TIMETRIAL_OVERALL_TIME
                                  : Highscores::HST_RACE_OVERALL_TIME;
            if(m_highscores->addData(hst, m_kart.size(),
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

    float inc = 0.05f;
    float dt  = delta;
    while (dt>0.0)
    {
        if(dt>=inc)
        {
            dt-=inc;
            if(user_config->m_replay_history) delta=history->GetNextDelta();
        }
        else
        {
            inc=dt;
            dt=0.0;
        }
        // The same delta is stored over and over again! This helps to use
        // the same index in History:allDeltas, and the history* arrays here,
        // and makes writing easier, since we had to write delta the first
        // time, and then inc from then on.
        if(!user_config->m_replay_history) history->StoreDelta(delta);
        m_physics->update(inc);
        for ( Karts::size_type i = 0 ; i < m_kart.size(); ++i)
        {
            m_kart[i]->update(inc) ;
        }
    }   // while dt>0

    projectile_manager->update(delta);
    herring_manager->update(delta);

    for ( Karts::size_type i = 0 ; i < m_kart.size(); ++i)
    {
        if(!m_kart[i]->raceIsFinished()) updateRacePosition((int)i);
        m_kart[i]->addMessages();
    }

    /* Routine stuff we do even when paused */
    callback_manager->update(delta);

    // Check for traffic jam. The sound is played even if it's
    // not a player kart - a traffic jam happens rarely anyway.
    for(unsigned int i=0; i<m_race_setup.getNumKarts(); i++)
    {
        if(m_number_collisions[i]>1)
        {
            sound_manager->playSfx(SOUND_TRAFFIC_JAM);
            nCount = FRAMES_FOR_TRAFFIC_JAM-1;  // sets all fields to zero in next frame
            break;
        }
    }

}

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
void World::herring_command (sgVec3 *xyz, char htype, int bNeedHeight )
{

    // if only 2d coordinates are given, let the herring fall from very heigh
    if(bNeedHeight) (*xyz)[2] = 1000000.0f;

    // Even if 3d data are given, make sure that the herring is on the ground
    (*xyz)[2] = getHeight ( m_track_branch, *xyz ) + 0.06f;
    herringType type=HE_GREEN;
if ( htype=='Y' || htype=='y' ) { type = HE_GOLD   ;}
    if ( htype=='G' || htype=='g' ) { type = HE_GREEN  ;}
    if ( htype=='R' || htype=='r' ) { type = HE_RED    ;}
    if ( htype=='S' || htype=='s' ) { type = HE_SILVER ;}
    herring_manager->newHerring(type, xyz);
}   // herring_command


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

    FILE *fd = fopen (path.c_str(), "r" );
    if ( fd == NULL )
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg),"Can't open track location file '%s'.\n",
                 path.c_str());
        throw std::runtime_error(msg);
    }

    char s [ 1024 ] ;

    while ( fgets ( s, 1023, fd ) != NULL )
    {
        if ( *s == '#' || *s < ' ' )
            continue ;

        int need_hat = false ;
        int fit_skin = false ;
        char fname [ 1024 ] ;
        sgCoord loc ;
        sgZeroVec3 ( loc.xyz ) ;
        sgZeroVec3 ( loc.hpr ) ;

        char htype = '\0' ;

        if ( sscanf ( s, "%cHERRING,%f,%f,%f", &htype,
                      &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 4 )
        {
            herring_command(&loc.xyz, htype, false) ;
        }
        else if ( sscanf ( s, "%cHERRING,%f,%f", &htype,
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
        {
            herring_command (&loc.xyz, htype, true) ;
        }
        else if ( s[0] == '\"' )
        {
            if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f,%f,%f",
                          fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
                          &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2]) ) == 7 )
            {
                /* All 6 DOF specified */
                need_hat = false;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2])) == 6 )
            {
                /* All 6 DOF specified - but need height */
                need_hat = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
                               &(loc.hpr[0]) ) == 5 )
            {
                /* No Roll/Pitch specified - assumed zero */
                need_hat = false ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,{},{}",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]) ) == 3 )
            {
                /* All 6 DOF specified - but need height, roll, pitch */
                need_hat = true ;
                fit_skin = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]) ) == 4 )
            {
                /* No Roll/Pitch specified - but need height */
                need_hat = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.xyz[2]) ) == 4 )
            {
                /* No Heading/Roll/Pitch specified - but need height */
                need_hat = false ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{}",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
            {
                /* No Roll/Pitch specified - but need height */
                need_hat = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
            {
                /* No Z/Heading/Roll/Pitch specified */
                need_hat = false ;
            }
            else if ( sscanf ( s, "\"%[^\"]\"", fname ) == 1 )
            {
                /* Nothing specified */
                need_hat = false ;
            }
            else
            {
                fclose(fd);
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), "Syntax error in '%s': %s",
                         path.c_str(), s);
                throw std::runtime_error(msg);
            }

            if ( need_hat )
            {
                sgVec3 nrm ;

                loc.xyz[2] = 1000.0f ;
                loc.xyz[2] = getHeightAndNormal ( m_track_branch, loc.xyz, nrm ) ;

                if ( fit_skin )
                {
                    float sy = sin ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
                    float cy = cos ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;

                    loc.hpr[2] =  SG_RADIANS_TO_DEGREES * atan2 ( nrm[0] * cy -
                                  nrm[1] * sy, nrm[2] ) ;
                    loc.hpr[1] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[1] * cy +
                                 nrm[0] * sy, nrm[2] ) ;
                }
            }   // if need_hat

            ssgEntity        *obj   = loader->load(fname, CB_TRACK);
            createDisplayLists(obj);
            ssgRangeSelector *lod   = new ssgRangeSelector ;
            ssgTransform     *trans = new ssgTransform ( & loc ) ;

            float r [ 2 ] = { -10.0f, 2000.0f } ;

            lod         -> addKid    ( obj   ) ;
            trans       -> addKid    ( lod   ) ;
            m_track_branch -> addKid ( trans ) ;
            lod         -> setRanges ( r, 2  ) ;
            if(user_config->m_track_debug)
                m_track->addDebugToScene(user_config->m_track_debug);

        }
        else
        {
            fclose(fd);
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), "Syntax error in '%s': %s",
                     path.c_str(), s);
            throw std::runtime_error(msg);
        }
    }   // while fgets

    fclose ( fd ) ;
#ifdef BULLET
    m_physics->setTrack(m_track_branch);
#endif
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
