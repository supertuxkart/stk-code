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

#include "history.hpp"
#include "world.hpp"
#include "kart.hpp"
#include "track.hpp"
#include "race_manager.hpp"

History* history = 0;

void History::SetSize(int n)
{
    m_size      = n;
    m_all_deltas  = new float[m_size];
    m_current    = -1;
    m_wrapped    = false;
}   // History

//-----------------------------------------------------------------------------
void History::StoreDelta(float delta)
{
    this->m_current++;
    if(m_current>=m_size)
    {
        m_wrapped = true;
        m_current = 0;
    }
    m_all_deltas[m_current] = delta;
}   // StoreDT

//-----------------------------------------------------------------------------
float History::GetNextDelta()
{
    m_current++;
    if(m_current>=m_size)
    {
        fprintf(stderr,"History: finished.\n");
        exit(-3);
    }
    return m_all_deltas[m_current];
}   // GetNextDT

//-----------------------------------------------------------------------------
void History::Save()
{
    FILE *fd = fopen("history.dat","w");
    int  nKarts = race_manager->getNumKarts();
#ifdef VERSION
    fprintf(fd, "Version:  %s\n",   VERSION);
#endif
    fprintf(fd, "numkarts: %d\n",   nKarts);
    fprintf(fd, "numplayers: %d\n", race_manager->getNumPlayers());
    fprintf(fd, "difficulty: %d\n", race_manager->getDifficulty());
    fprintf(fd, "track: %s\n",      world->getTrack()->getIdent().c_str());

    int k;
    for(k=0; k<nKarts; k++)
    {
        fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getName().c_str());
    }
    fprintf(fd, "size:     %d\n", GetCount());

    int j = m_wrapped ? m_current : 0;
    for(int i=0; i<GetCount(); i++)
    {
        fprintf(fd, "delta: %f\n",m_all_deltas[i]);
        j=(j+1)%m_size;
    }

    for(int k=0; k<nKarts; k++)
    {
        Kart* kart= world->getKart(k);
        char s[1024];
        j = m_wrapped ? m_current : 0;
        for(int i=0; i<GetCount(); i++)
        {
            kart->WriteHistory(s, k, j);
            fprintf(fd, "%s\n",s);
            j=(j+1)%m_size;
        }   // for i
    }   // for k
    fprintf(fd, "History file end.\n");
    fclose(fd);
}   // Save

//-----------------------------------------------------------------------------
void History::Load()
{
    char s[1024], s1[1024];
    int  n, numKarts;
    m_fd = fopen("history.dat","r");

    fgets(s, 1023, m_fd);
    if(sscanf(s,"Version: %s",s1)!=1)
    {
        fprintf(stderr, "WARNING: no Version information found in history file.\n");
        exit(-2);
    }
#ifdef VERSION
    if(strcmp(s1,VERSION))
    {
        fprintf(stderr, "WARNING: history is version '%s'\n",s1);
        fprintf(stderr, "         tuxracer version is '%s'\n",VERSION);
    }
#endif
    fgets(s, 1023, m_fd);
    if(sscanf(s, "numkarts: %d",&numKarts)!=1)
    {
        fprintf(stderr,"WARNING: No number of karts found in history file.\n");
        exit(-2);
    }
    race_manager->setNumKarts(numKarts);

    fgets(s, 1023, m_fd);
    if(sscanf(s, "numplayers: %d",&n)!=1)
    {
        fprintf(stderr,"WARNING: No number of players found in history file.\n");
        exit(-2);
    }
    race_manager->setNumPlayers(n);

    fgets(s, 1023, m_fd);
    if(sscanf(s, "difficulty: %d",&n)!=1)
    {
        fprintf(stderr,"WARNING: No difficulty found in history file.\n");
        exit(-2);
    }
    race_manager->setDifficulty((RaceManager::Difficulty)n);

    fgets(s, 1023, m_fd);
    if(sscanf(s, "track: %s",s1)!=1)
    {
        fprintf(stderr,"WARNING: Track not found in history file.\n");
    }
    race_manager->setTrack(s1);
    // This value doesn't really matter, but should be defined, otherwise
    // the racing phase can switch to 'ending'
    race_manager->setNumLaps(10);

    for(int i=0; i<numKarts; i++)
    {
        fgets(s, 1023, m_fd);
        if(sscanf(s, "model %d: %s",&n, s1)!=2)
        {
            fprintf(stderr,"WARNING: No model information for kart %d found.\n",
                    i);
            exit(-2);
        }
    }   // for i<nKarts
    // JH: The model information is currently ignored
    fgets(s, 1023, m_fd);
    if(sscanf(s,"size: %d",&n)!=1)
    {
        fprintf(stderr,"WARNING: Number of records not found in history file.\n");
        exit(-2);
    }
    SetSize(n);
    for(int i=0; i<m_size; i++)
    {
        fgets(s, 1023, m_fd);
        sscanf(s, "delta: %f\n",m_all_deltas+i);
    }
    m_current = -1;
}   // Load

//-----------------------------------------------------------------------------
void History::LoadKartData(Kart* k, int kartNumber)
{
    char s[1024];
    for(int i=0; i<m_size; i++)
    {
        fgets(s, 1023, m_fd);
        k->ReadHistory(s, kartNumber, i);
    }   // for i<m_current
}   // LoadKartData

