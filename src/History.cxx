//  $Id: History.cxx,v 1.1 2005/09/28 17:00:42 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "History.h"
#include "World.h"
#include "Kart.h"
#include "Track.h"
#include "RaceManager.h"

History* history = 0;

void History::SetSize(int n) {
  nSize      = n; 
  allDeltas  = new float[nSize];
  current    = -1;
  wrapped    = false;
}   // History

// -----------------------------------------------------------------------------
void History::StoreDelta(float delta) {
  this->current++;
  if(current>=nSize) {
    wrapped = true;
    current = 0;
  }
  allDeltas[current] = delta;
}   // StoreDT

// -----------------------------------------------------------------------------
float History::GetNextDelta() {
  current++;
  if(current>=nSize) {
    fprintf(stderr,"History: finished.\n");
    exit(-3);
  }
  return allDeltas[current];
}   // GetNextDT

// -----------------------------------------------------------------------------
void History::Save() {
  FILE *fd = fopen("history.dat","w");
  int  nKarts = world->getNumKarts();
#ifdef VERSION
  fprintf(fd, "Version:  %s\n",   VERSION);
#endif
  fprintf(fd, "numkarts: %d\n",   nKarts);
  fprintf(fd, "numplayers: %d\n", world->raceSetup.getNumPlayers());
  fprintf(fd, "difficulty: %d\n", world->raceSetup.difficulty);
  fprintf(fd, "track: %s\n",      world->track->getIdent());

  int k;
  for(k=0; k<nKarts; k++) {
    fprintf(fd, "model %d: %s\n",k, world->getKart(k)->getKartProperties()->getName());
  }
  fprintf(fd, "size:     %d\n", GetCount());

  int j = wrapped ? current : 0;
  for(int i=0; i<GetCount(); i++) {
    fprintf(fd, "delta: %f\n",allDeltas[i]);
    j=(j+1)%nSize;
  }

  for(int k=0; k<nKarts; k++) {
    Kart* kart= world->getKart(k);
    char s[1024];
    j = wrapped ? current : 0;
    for(int i=0; i<GetCount(); i++) {
      kart->WriteHistory(s, k, j);
      fprintf(fd, "%s\n",s);
      j=(j+1)%nSize;
    }   // for i
  }   // for k
  fprintf(fd, "History file end.\n");
  fclose(fd);
}   // Save

// -----------------------------------------------------------------------------
void History::Load() {
  char s[1024], s1[1024];
  int  n, numKarts;
  fd = fopen("history.dat","r");

  fgets(s, 1023, fd);
  if(sscanf(s,"Version: %s",s1)!=1) {
    fprintf(stderr, "WARNING: no Version information found in history file.\n");
    exit(-2);
  }
  if(strcmp(s1,VERSION)) {
    fprintf(stderr, "WARNING: history is version '%s'\n",s1);
    fprintf(stderr, "         tuxracer version is '%s'\n",VERSION);
  } 

  fgets(s, 1023, fd);
  if(sscanf(s, "numkarts: %d",&numKarts)!=1) {
    fprintf(stderr,"WARNING: No number of karts found in history file.\n");
    exit(-2);
  }
  race_manager->setNumKarts(numKarts);

  fgets(s, 1023, fd);
  if(sscanf(s, "numplayers: %d",&n)!=1) {
    fprintf(stderr,"WARNING: No number of players found in history file.\n");
    exit(-2);
  }
  race_manager->setNumPlayers(n);

  fgets(s, 1023, fd);
  if(sscanf(s, "difficulty: %d",&n)!=1) {
    fprintf(stderr,"WARNING: No difficulty found in history file.\n");
    exit(-2);
  }
  race_manager->setDifficulty((RaceDifficulty)n);
  
  fgets(s, 1023, fd);
  if(sscanf(s, "track: %s",s1)!=1) {
    fprintf(stderr,"WARNING: Track not found in history file.\n");
  }
  race_manager->setTrack(s1);
  // This value doesn't really matter, but should be defined, otherwise
  // the racing phase can switch to 'ending'
  race_manager->setNumLaps(10);

  for(int i=0; i<numKarts; i++) {
    fgets(s, 1023, fd);
    if(sscanf(s, "model %d: %s",&n, s1)!=2) {
      fprintf(stderr,"WARNING: No model information for kart %d found.\n",
	      i);
      exit(-2);
    }
  }   // for i<nKarts 
  // JH: The model information is currently ignored
  fgets(s, 1023, fd);
  if(sscanf(s,"size: %d",&n)!=1) {
    fprintf(stderr,"WARNING: Number of records not found in history file.\n");
    exit(-2);
  }
  SetSize(n);
  for(int i=0; i<nSize; i++) {
    fgets(s, 1023, fd);
    sscanf(s, "delta: %f\n",allDeltas+i);
  }
  current = -1;
}   // Load

// -----------------------------------------------------------------------------
void History::LoadKartData(Kart* k, int kartNumber) {
  char s[1024];
  for(int i=0; i<nSize; i++) {
    fgets(s, 1023, fd);
    k->ReadHistory(s, kartNumber, i);
  }   // for i<current
}   // LoadKartData

// -----------------------------------------------------------------------------


