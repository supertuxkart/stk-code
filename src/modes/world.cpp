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

#include "modes/world.hpp"

#include <assert.h>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <ctime>

#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/race_gui.hpp"
#include "io/file_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/auto_kart.hpp"
#include "karts/player_kart.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/network_manager.hpp"
#include "network/race_state.hpp"
#include "physics/btKart.hpp"
#include "race/highscore_manager.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "robots/default_robot.hpp"
#include "robots/new_ai.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

//-----------------------------------------------------------------------------
/** Constructor. Note that in the constructor it is not possible to call any
 *  functions that use RaceManager::getWorld(), since this is only defined
 *  after the constructor. Those functions can be called in the init() 
 *  function, which is called immediately after the constructor.
 */
World::World() : TimedRace()
{
    m_physics  = NULL;
    m_race_gui = NULL;
    TimedRace::setClockMode( CHRONO );
}   // World

// ----------------------------------------------------------------------------
/** This function is called after the World constructor. In init() functions
 *  can be called that use RaceManager::getWorld(). The init function is 
 *  called immediately after the constructor.
 */
void World::init()
{
    race_state            = new RaceState();
    m_track               = NULL;
    m_faster_music_active = false;
    m_fastest_lap         = 9999999.9f;
    m_fastest_kart        = 0;
    m_eliminated_karts    = 0;
    m_eliminated_players  = 0;

    m_use_highscores = true;

    // Create the race gui before anything else is attached to the scene node
    // (which happens when the track is loaded). This allows the race gui to
    // do any rendering on texture.
    m_race_gui = new RaceGUI();

    // Grab the track file
    try
    {
        m_track = track_manager->getTrack(race_manager->getTrackName());
    }
    catch(std::runtime_error)
    {
        std::ostringstream msg;
        msg << "Track '" << race_manager->getTrackName() << "' not found.\n";
        throw std::runtime_error(msg.str());
    }

    // Create the physics
    m_physics = new Physics();

    assert(race_manager->getNumKarts() > 0);

    // Load the track models - this must be done before the karts so that the
    // karts can be positioned properly on (and not in) the tracks.
    m_track->loadTrackModel();

    m_player_karts.resize(race_manager->getNumPlayers());
    m_network_karts.resize(race_manager->getNumPlayers());
    m_local_player_karts.resize(race_manager->getNumLocalPlayers());

    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        btTransform init_pos=m_track->getStartTransform(i);
        const std::string& kart_ident = race_manager->getKartIdent(i);
        int local_player_id           = race_manager->getKartLocalPlayerId(i);
        int global_player_id          = race_manager->getKartGlobalPlayerId(i);
        Kart* newkart = this->createKart(kart_ident, i, local_player_id,  
                                   global_player_id, init_pos);
        m_kart.push_back(newkart);
        newkart->setWorldKartId(m_kart.size()-1);
    }  // for i

    resetAllKarts();
    // Note: track reset must be called after all karts exist, since check
    // objects need to allocate data structures depending on the number
    // of karts.
    m_track->reset();
    m_track->startMusic();

    if(!history->replayHistory()) history->initRecording();
    network_manager->worldLoaded();
}   // init

//-----------------------------------------------------------------------------
/** Creates a kart, having a certain position, starting location, and local
 *  and global player id (if applicable).
 *  \param kart_ident Identifier of the kart to create.
 *  \param index Index of the kart.
 *  \param local_player_id If the kart is a player kart this is the index of
 *         this player on the local machine.
 *  \param global_player_id If the akrt is a player kart this is the index of
 *         this player globally (i.e. including network players).
 *  \param init_pos The start XYZ coordinates.
 */
Kart *World::createKart(const std::string &kart_ident, int index,
                        int local_player_id, int global_player_id,
                        const btTransform &init_pos)
{
    Kart *newkart = NULL;
    int position  = index+1;
    switch(race_manager->getKartType(index))
    {
    case RaceManager::KT_PLAYER:
        std::cout << "===== World : creating player kart for kart #" << index << " which has local_player_id " << local_player_id << " ===========\n";
        newkart = new PlayerKart(kart_ident, position,
                                 StateManager::get()->getActivePlayer(local_player_id),
                                 init_pos, local_player_id);
        m_player_karts[global_player_id] = (PlayerKart*)newkart;
        m_local_player_karts[local_player_id] = static_cast<PlayerKart*>(newkart);
        break;
    case RaceManager::KT_NETWORK_PLAYER:
        newkart = new NetworkKart(kart_ident, position, init_pos,
                                  global_player_id);
        m_network_karts[global_player_id] = static_cast<NetworkKart*>(newkart);
        m_player_karts[global_player_id] = (PlayerKart*)newkart;
        break;
    case RaceManager::KT_AI:
        std::cout << "===== World : creating AI kart for #" << index << "===========\n";

        newkart = loadRobot(kart_ident, position, init_pos);
        break;
    case RaceManager::KT_GHOST:
        break;
    case RaceManager::KT_LEADER:
        break;
    }
    return newkart;
}   // createKart

//-----------------------------------------------------------------------------
Kart* World::loadRobot(const std::string& kart_name, int position,
                       const btTransform& init_pos)
{
    Kart* currentRobot;

    const int NUM_ROBOTS = 1;

    // For now: instead of random switching, use each
    // robot in turns: switch(m_random.get(NUM_ROBOTS))
    static int turn=1;
    turn=1-turn;
    switch(turn)
    {
        case 0:
            currentRobot = new DefaultRobot(kart_name, position, init_pos, m_track);
            break;
        case 1:
            currentRobot = new NewAI(kart_name, position, init_pos, m_track);
            break;
        default:
            std::cerr << "Warning: Unknown robot, using default." << std::endl;
            currentRobot = new DefaultRobot(kart_name, position, init_pos, m_track);
            break;
    }

    return currentRobot;
}   // loadRobot

//-----------------------------------------------------------------------------
World::~World()
{
    delete m_race_gui;
    delete race_state;
    // In case that a race is aborted (e.g. track not found) m_track is 0.
    if(m_track)
        m_track->cleanup();

    for ( unsigned int i = 0 ; i < m_kart.size() ; i++ )
        delete m_kart[i];

    m_kart.clear();
    projectile_manager->cleanup();
    // In case that the track is not found, m_physics is still undefined.
    if(m_physics)
        delete m_physics;

    sound_manager -> stopMusic();
}   // ~World

//-----------------------------------------------------------------------------
void World::terminateRace()
{
    updateHighscores();
    TimedRace::pause();
    // TODO - race results GUI
    //menu_manager->pushMenu(MENUID_RACERESULT);
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
    //Project karts onto track from above. This will lower each kart so
    //that at least one of its wheel will be on the surface of the track
    for ( Karts::iterator i=m_kart.begin(); i!=m_kart.end(); i++)
    {
        ///start projection from top of kart
        btVector3 up_offset(0, 0, 0.5f * ((*i)->getKartHeight()));
        (*i)->getVehicle()->getRigidBody()->translate (up_offset);

        bool kart_over_ground = m_physics->projectKartDownwards(*i);

        if (!kart_over_ground)
        {
            fprintf(stderr, "ERROR: no valid starting position for kart %d on track %s.\n",
                    (int)(i-m_kart.begin()), m_track->getIdent().c_str());
            exit(-1);
        }
    }

    bool all_finished=false;
    // kart->isInRest() is not fully correct, since it only takes the
    // velocity in count, which might be close to zero when the kart
    // is just hitting the floor, before being pushed up again by
    // the suspension. So we just do a longer initial simulation,
    // which should be long enough for all karts to be firmly on ground.
    for(int i=0; i<60; i++) m_physics->update(1.f/60.f);

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

    // Now store the current (i.e. in rest) suspension length for each kart,
    // so that the karts can visualise the suspension.
    for ( Karts::iterator i=m_kart.begin(); i!=m_kart.end(); i++)
        (*i)->setSuspensionLength();
    for(unsigned int i=0; i<m_player_karts.size(); i++)
        m_player_karts[i]->getCamera()->setInitialTransform();
}   // resetAllKarts

//-----------------------------------------------------------------------------
/** Called during rendering. In this function direct calls to the graphics
 *  are possible, e.g. using an irrlicht font object to directly print to
 *  the screen, ...
 */
void World::render()
{
    m_race_gui->render();
}   // render

//-----------------------------------------------------------------------------
void World::update(float dt)
{
    if(history->replayHistory()) dt=history->getNextDelta();
    TimedRace::update(dt);
    // Clear race state so that new information can be stored
    race_state->clear();

    if(network_manager->getMode()!=NetworkManager::NW_CLIENT &&
        !history->dontDoPhysics())
    {
        m_physics->update(dt);
    }

    const int kart_amount = m_kart.size();
    for (int i = 0 ; i < kart_amount; ++i)
    {
        // Update all karts that are not eliminated
        if(!m_kart[i]->isEliminated()) m_kart[i]->update(dt) ;
    }
    // The order of updates is rather important: if track update would
    // be called before kart update, then the check manager (called from
    // track update) will be using the old kart position to determine
    // e.g. if a kart has started a new line. But linear world (from
    // which this is called in case of a race) will be using the new
    // position of the karts to determine the driveline quad a kart
    // is on. So if a kart just arrived at quad 0 (meaning the distance
    // along the track goes from close-to-lap-length to close-to-zero),
    // the order of karts will be incorrect, since this kart will not
    // have started the next lap. While this will only last for one
    // frame (since in the next frame the check manager will detect
    // the new lap), it causes an unwanted display of icons in the
    // icon display.
    m_track->update(dt);

    projectile_manager->update(dt);
    m_race_gui->update(dt);
}   // update

// ----------------------------------------------------------------------------

HighscoreEntry* World::getHighscores() const
{
    if(!m_use_highscores) return NULL;

    const HighscoreEntry::HighscoreType type = "HST_" + getIdent();

    HighscoreEntry* highscores =
        highscore_manager->getHighscoreEntry(type,
                                             race_manager->getNumKarts(),
                                             race_manager->getDifficulty(),
                                             race_manager->getTrackName(),
                                             race_manager->getNumLaps());

    return highscores;
}
// ----------------------------------------------------------------------------
/*
 * usually called at the end of a race. Checks if the current times are worth a new
 * score, if so it notifies the HighscoreManager so the new score is added and saved.
 */
void World::updateHighscores()
{
    if(!m_use_highscores) return;

    // Add times to highscore list. First compute the order of karts,
    // so that the timing of the fastest kart is added first (otherwise
    // someone might get into the highscore list, only to be kicked out
    // again by a faster kart in the same race), which might be confusing
    // if we ever decide to display a message (e.g. during a race)
    unsigned int *index = new unsigned int[m_kart.size()];

    const unsigned int kart_amount = m_kart.size();
    for (unsigned int i=0; i<kart_amount; i++ )
    {
        index[i] = 999; // first reset the contents of the array
    }
    for (unsigned int i=0; i<kart_amount; i++ )
    {
        const int pos = m_kart[i]->getPosition()-1;
        if(pos < 0 || pos >= (int)kart_amount) continue; // wrong position
        index[pos] = i;
    }

    for(unsigned int pos=0; pos<kart_amount; pos++)
    {

        if(index[pos] == 999)
        {
            // no kart claimed to be in this position, most likely means
            // the kart location data is wrong

#ifdef DEBUG
            fprintf(stderr, "Error, incorrect kart positions:");
            for (unsigned int i=0; i<m_kart.size(); i++ )
            {
                fprintf(stderr, "i=%d position %d\n",i, m_kart[i]->getPosition());
            }
#endif
            continue;
        }

        // Only record times for player karts and only if they finished the race
        if(!m_kart[index[pos]]->isPlayerKart()) continue;
        if (!m_kart[index[pos]]->hasFinishedRace()) continue;

        assert(index[pos] >= 0);
        assert(index[pos] < m_kart.size());
        PlayerKart *k = (PlayerKart*)m_kart[index[pos]];

        HighscoreEntry* highscores = getHighscores();

        if(highscores->addData(k->getIdent(),
                               k->getPlayer()->getProfile()->getName(),
                               k->getFinishTime())>0 )
        {
            highscore_manager->Save();
        }
    } // next position
    delete []index;

}   // updateHighscores

//-----------------------------------------------------------------------------
/** Called in follow-leader-mode to remove the last kart
*/
void World::removeKart(int kart_number)
{
    Kart *kart = m_kart[kart_number];

    // Display a message about the eliminated kart in the race gui
    for (std::vector<PlayerKart*>::iterator i  = m_player_karts.begin();
        i != m_player_karts.end();  i++ )
    {
        if(*i==kart)
        {
            m_race_gui->addMessage(_("You have been\neliminated!"), *i, 2.0f, 60);
        }
        else
        {
            m_race_gui->addMessage(StringUtils::insertValues(_("'%s' has\nbeen eliminated."),
                                                             kart->getName().c_str()),
                                                             *i, 2.0f, 60);
        }
    }   // for i in kart
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
    race_manager->RaceFinished(kart, TimedRace::getTime());
    kart->eliminate();
    m_eliminated_karts++;

}   // removeKart

//-----------------------------------------------------------------------------
void World::getDefaultCollectibles(int& collectible_type, int& amount )
{
    collectible_type = POWERUP_NOTHING;
    amount = 0;
}
//-----------------------------------------------------------------------------
void World::restartRace()
{
    TimedRace::reset();
	m_track->reset();
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

    projectile_manager->cleanup();
    race_manager->reset();
}   // restartRace

//-----------------------------------------------------------------------------
void  World::pause()
{
    sound_manager->pauseMusic();
    sfx_manager->pauseAll();
    TimedRace::pause();
}

//-----------------------------------------------------------------------------
void  World::unpause()
{
    sound_manager->resumeMusic() ;
    sfx_manager->resumeAll();
    TimedRace::unpause();
    for(unsigned int i=0; i<m_player_karts.size(); i++)
        m_player_karts[i]->resetInputState();
}

/* EOF */
