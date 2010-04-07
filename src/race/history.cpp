//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "race/history.hpp"

#include <stdio.h>

#include "modes/world.hpp"
#include "karts/kart.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"

History* history = 0;

#define KEEP_OLD_FORMAT

//-----------------------------------------------------------------------------
/** Initialises the history object and sets the mode to none. 
 */
History::History()
{
    m_replay_mode = HISTORY_NONE;
}   // History

//-----------------------------------------------------------------------------
/** Starts replay from the history file in the current directory.
 */
void History::startReplay()
{
    Load();
}   // initReplay

//-----------------------------------------------------------------------------
/** Initialise the history for a new recording. It especially allocates memory
 *  to store the history.
 */
void History::initRecording()
{
    allocateMemory(stk_config->m_max_history);
    m_current = -1;
    m_wrapped = false;
    m_size    = 0;
}   // initRecording

//-----------------------------------------------------------------------------
/** Allocates memory for the history. This is used when recording as well
 *  as when replaying (since in replay the data is read into memory first).
 *  \param number_of_frames Maximum number of frames to store.
 */
void History::allocateMemory(int number_of_frames)
{
    m_all_deltas.resize   (number_of_frames);
    unsigned int num_karts = race_manager->getNumberOfKarts();
    m_all_controls.resize (number_of_frames*num_karts);
    m_all_xyz.resize      (number_of_frames*num_karts);
    m_all_rotations.resize(number_of_frames*num_karts);
}   // allocateMemory

//-----------------------------------------------------------------------------
/** Depending on mode either saves the data for the current time step, or 
 *  replays the data. 
 *  /param dt Time step.
 */
void History::update(float dt)
{
    if(m_replay_mode==HISTORY_NONE)
        updateSaving(dt);
    else
        updateReplay(dt);
}   // update

//-----------------------------------------------------------------------------
/** Saves the current history.
 *  \param dt Time step size.
 */
void History::updateSaving(float dt)
{
    m_current++;
    if(m_current>=(int)m_all_deltas.size())
    {
        m_wrapped = true;
        m_current = 0;
    }
    else
    {
        m_size ++;
    }
    m_all_deltas[m_current] = dt;

    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for(unsigned int i=0; i<num_karts; i++)
    {
        unsigned int index     = m_current*num_karts+i;
        const Kart *kart       = world->getKart(i);
        m_all_controls[index]  = kart->getControls();
        m_all_xyz[index]       = kart->getXYZ();
        m_all_rotations[index] = kart->getRotation();
    }   // for i
}   // updateSaving

//-----------------------------------------------------------------------------
/** Sets the kart position and controls to the recorded history value.
 *  \param dt Time step size.
 */
void History::updateReplay(float dt)
{
    m_current++;
    if(m_current>=(int)m_all_deltas.size())
    {
        printf("Replay finished.\n");
        exit(2);
    }
    World *world = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    for(unsigned k=0; k<num_karts; k++)
    {
        Kart *kart = world->getKart(k);
        unsigned int index=m_current*num_karts+k;
        if(m_replay_mode==HISTORY_POSITION)
        {
            kart->setXYZ(m_all_xyz[index]);
            kart->setRotation(m_all_rotations[index]);
        }
        else
        {
            kart->setControls(m_all_controls[index]);
        }
    }
}   // updateReplay

//-----------------------------------------------------------------------------
/** Saves the history stored in the internal data structures into a file called
 *  history.dat.
 */
void History::Save()
{
    FILE *fd       = fopen("history.dat","w");
    World *world   = World::getWorld();
    int  num_karts = world->getNumKarts();
#ifdef VERSION
    fprintf(fd, "Version:  %s\n",   VERSION);
#endif
    fprintf(fd, "numkarts: %d\n",   num_karts);
    fprintf(fd, "numplayers: %d\n", race_manager->getNumPlayers());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      world->getTrack()->getIdent().c_str());

    int k;
    for(k=0; k<num_karts; k++)
    {
        fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getIdent().c_str());
    }
    fprintf(fd, "size:     %d\n", m_size);

    int j = m_wrapped ? m_current : 0;
    for(int i=0; i<m_size; i++)
    {
        fprintf(fd, "delta: %f\n",m_all_deltas[i]);
        j=(j+1)%m_size;
    }

    for(int k=0; k<num_karts; k++)
    {
        //Kart* kart= RaceManager::getKart(k);
        j = m_wrapped ? m_current : 0;
        for(int i=0; i<m_size; i++)
        {
            // FIXME: kart number is not really necessary
            fprintf(fd, "%d %f %f %d  %f %f %f  %f %f %f %f\n",
                    k,
                    m_all_controls[j].m_steer,
                    m_all_controls[j].m_accel,
                    m_all_controls[j].getButtonsCompressed(),
                    m_all_xyz[j].getX(), 
#ifdef KEEP_OLD_FORMAT
                    m_all_xyz[j].getZ(), m_all_xyz[j].getY(),
#else
                    m_all_xyz[j].getY(), m_all_xyz[j].getZ(),
#endif
                    m_all_rotations[j].getX(), m_all_rotations[j].getY(),
                    m_all_rotations[j].getZ(), m_all_rotations[j].getW()  );
            j=(j+1)%m_size;
        }   // for i
    }   // for k
    fprintf(fd, "History file end.\n");
    fclose(fd);
}   // Save

//-----------------------------------------------------------------------------
/** Loads a history from history.dat in the current directory.
 */
void History::Load()
{
    char s[1024], s1[1024];
    int  n;
    FILE *fd = fopen("history.dat","r");
    if (fd == NULL)
    {
        fprintf(stderr, "ERROR: could not open history.dat\n");
        exit(-2);
    }
    
    if (fgets(s, 1023, fd) == NULL)
    {
        fprintf(stderr, "ERROR: could not read history.dat\n");
        exit(-2);
    }
    
    if(sscanf(s,"Version: %s",s1)!=1)
    {
        fprintf(stderr, "WARNING: no Version information found in history file.\n");
        
#ifdef VERSION
        if(strcmp(s1,VERSION))
        {
            fprintf(stderr, "WARNING: history is version '%s'\n",s1);
            fprintf(stderr, "         STK version is '%s'\n",VERSION);
        }
#endif
    }
    
    if (fgets(s, 1023, fd) == NULL)
    {
        fprintf(stderr, "ERROR: could not read history.dat\n");
        exit(-2);
    }
    
    unsigned int num_karts;
    if(sscanf(s, "numkarts: %d",&num_karts)!=1)
    {
        fprintf(stderr,"WARNING: No number of karts found in history file.\n");
        exit(-2);
    }
    race_manager->setNumKarts(num_karts);

    fgets(s, 1023, fd);
    if(sscanf(s, "numplayers: %d",&n)!=1)
    {
        fprintf(stderr,"WARNING: No number of players found in history file.\n");
        exit(-2);
    }
    race_manager->setNumPlayers(n);

    fgets(s, 1023, fd);
    if(sscanf(s, "difficulty: %d",&n)!=1)
    {
        fprintf(stderr,"WARNING: No difficulty found in history file.\n");
        exit(-2);
    }
    race_manager->setDifficulty((RaceManager::Difficulty)n);

    fgets(s, 1023, fd);
    if(sscanf(s, "track: %s",s1)!=1)
    {
        fprintf(stderr,"WARNING: Track not found in history file.\n");
    }
    race_manager->setTrack(s1);
    // This value doesn't really matter, but should be defined, otherwise
    // the racing phase can switch to 'ending'
    race_manager->setNumLaps(10);

    for(unsigned int i=0; i<num_karts; i++)
    {
        fgets(s, 1023, fd);
        if(sscanf(s, "model %d: %s",&n, s1)!=2)
        {
            fprintf(stderr,"WARNING: No model information for kart %d found.\n",
                    i);
            exit(-2);
        }
        if(i<race_manager->getNumPlayers())
        {
            race_manager->setLocalKartInfo(i, s1);
        }
    }   // for i<nKarts
    // FIXME: The model information is currently ignored
    fgets(s, 1023, fd);
    if(sscanf(s,"size: %d",&m_size)!=1)
    {
        fprintf(stderr,"WARNING: Number of records not found in history file.\n");
        exit(-2);
    }
    allocateMemory(m_size);
    m_current = -1;

    for(int i=0; i<m_size; i++)
    {
        fgets(s, 1023, fd);
        sscanf(s, "delta: %f\n",&m_all_deltas[i]);
    }

    for(unsigned int k=0; k<num_karts; k++)
    {
        int j=0;
        for(int i=0; i<m_size; i++)
        {
            fgets(s, 1023, fd);
            int buttonsCompressed;
            float x,y,z,rx,ry,rz,rw;
            sscanf(s, "%d %f %f %d  %f %f %f  %f %f %f %f\n",
                    &j, 
                    &m_all_controls[i].m_steer,
                    &m_all_controls[i].m_accel,
                    &buttonsCompressed,
                    &x, 
#ifdef KEEP_OLD_FORMAT
                    &z, &y,
                    //xyz
                    //yxz 
                    //yzx
                    &rx, &rz, &ry, &rw
#else
                    &y, &z,
                    &rx, &ry, &rz, &rw
#endif
                    );
            m_all_xyz[i]       = Vec3(x,y,z);
            m_all_rotations[i] = btQuaternion(rx,ry,rz,rw);
            m_all_controls[i].setButtonsCompressed(char(buttonsCompressed));
        }   // for i
    }   // for k
    fprintf(fd, "History file end.\n");
    fclose(fd);
}   // Load

