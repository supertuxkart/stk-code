//  $Id: World.cxx,v 1.16 2005/09/30 16:51:53 joh Exp $
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

#include <assert.h>
#include <sstream>
#include <stdexcept>
#include "World.h"
#include "preprocessor.h"
#include "HerringManager.h"
#include "ProjectileManager.h"
#include "gui/BaseGUI.h"
#include "Loader.h"
#include "PlayerKart.h"
#include "AutoKart.h"
#include "isect.h"
#include "Track.h"
#include "KartManager.h"
#include "TrackManager.h"
#include "Config.h"
#include "HookManager.h"
#include "History.h"
#include "constants.h"
#include "sound.h"

World* world = 0;

World::World(const RaceSetup& raceSetup_) : raceSetup(raceSetup_) {
  delete world;
  world = this;
  phase = START_PHASE;

  scene = NULL;
  track = NULL;

  clock = 0.0f;

  // Grab the track file
  try {
    track = track_manager->getTrack(raceSetup.track) ;
  } catch(std::runtime_error) {
    printf("Track '%s' not found.\n",raceSetup.track.c_str());
    exit(1);
  }

  // Start building the scene graph
  scene       = new ssgRoot   ;
  trackBranch = new ssgBranch ;
  scene -> addKid ( trackBranch ) ;

  //Clear textures that might be stored from things like the character select
  //screen, otherwise, the tracks could get textures where they aren't
  //suppposed to be, and if there is no texture, it just looks white.
  if(raceSetup.mode != RaceSetup::RM_GRAND_PRIX)
      loader->shared_textures.removeAll();

  assert(raceSetup.karts.size() > 0);

  // Clear all hooks, which might still be stored there from a previous race.
  hook_manager->clearAll();

  // Load the track models - this must be done before the karts so that the
  // karts can be positioned properly on (and not in) the tracks.
  loadTrack   ( ) ;

  staticSSG = new StaticSSG(trackBranch, 1000);
  //  staticSSG->Draw(scene);
  //  exit(-1);
  int pos = 0;
  int playerIndex = 0;
  for (RaceSetup::Karts::iterator i = raceSetup.karts.begin() ;
                                  i != raceSetup.karts.end() ; ++i ) {
    Kart* newkart;
    if(config->profile)
    {
      // In profile mode, load only the old kart
      newkart = new AutoKart (kart_manager->getKart("tuxkart"), pos);
    } else {
      if (std::find(raceSetup.players.begin(),
		    raceSetup.players.end(), pos) != raceSetup.players.end())
      {
	// the given position belongs to a player
	    newkart = new PlayerKart (kart_manager->getKart(*i), pos,
                      &(config->player[playerIndex++]));
      } else {
	newkart = new AutoKart   (kart_manager->getKart(*i), pos);
      }
    }   // if config->profile else
    if(config->replayHistory) {
      history->LoadKartData(newkart, pos);
    }
    sgCoord init_pos = { { 0, 0, 0 }, { 0, 0, 0 } } ;


    float hot = newkart->getIsectData ( init_pos.xyz, init_pos.xyz ) ;
    //float hot=0.0;
    // Bug fix/workaround: sometimes the first kart would be too close
    // to the first driveline point and not to the last one -->
    // This kart would not get any lap counting done in the first
    // lap! Therefor -1.5 is subtracted from the y position - which
    // is a somewhat arbitrary value.
    init_pos.xyz[0] = (pos % 2 == 0) ? 1.5f : -1.5f ;
    init_pos.xyz[1] = -pos * 1.5f -1.5;
    init_pos.xyz[2] = hot;
    newkart -> setReset ( & init_pos ) ;
    newkart -> reset    () ;
    newkart -> getModel () -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

    scene -> addKid ( newkart -> getModel() ) ;

    kart.push_back(newkart);
    pos++;
  }  // for i

  loadPlayers ( ) ;

  preProcessObj ( scene ) ;

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  //ssgSetBackFaceCollisions ( raceSetup.mirror ) ;
#endif

  guiStack.push_back(GUIS_RACE);

  const std::string music = track_manager->getTrack(raceSetup.track)->getMusic();

  if (!music.empty())
    sound -> change_track ( music.c_str() );

  ready_set_go = 3;
  phase        = START_PHASE;
}

World::~World() {
  for ( unsigned int i = 0 ; i < kart.size() ; i++ )
    delete kart[i];

  kart.clear();
  projectile_manager->cleanup();

  delete scene ;
}

void World::draw() {

  ssgGetLight ( 0 ) -> setPosition ( track->getSunPos() ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT , track->getAmbientCol()  ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_DIFFUSE , track->getDiffuseCol() ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, track->getSpecularCol() ) ;

  ssgCullAndDraw ( world->scene ) ;
}

void World::update(float delta) {
  if(config->replayHistory) delta=history->GetNextDelta();
  clock += delta;

  checkRaceStatus();

  if( getPhase() == World::FINISH_PHASE ) {
    guiStack.push_back ( GUIS_NEXTRACE );
  }

  float inc = 0.05;
  float dt  = delta;
  while (dt>0.0) {
    if(dt>=inc) {
      dt-=inc;
      if(config->replayHistory) delta=history->GetNextDelta();
    } else {
      inc=dt;
      dt=0.0;
    }
    // The same delta is stored over and over again! This helps to use
    // the same index in History:allDeltas, and the history* arrays here,
    // and makes writing easier, since we had to write delta the first
    // time, and then inc from then on.
    if(!config->replayHistory) history->StoreDelta(delta);
    for ( Karts::size_type i = 0 ; i < kart.size(); ++i) {
      kart[i]->update(inc) ;
    }
  }   // while dt>0

  projectile_manager->update(delta);
  herring_manager->update(delta);
  
  for ( Karts::size_type i = 0 ; i < kart.size(); ++i) updateLapCounter ( i ) ;

  /* Routine stuff we do even when paused */
  hook_manager->update();
}

void World::checkRaceStatus() {
  if (clock > 1.0 && ready_set_go == 0) {
      ready_set_go = -1;
  } else if (clock > 2.0 && ready_set_go == 1) {
    ready_set_go = 0;
    phase = RACE_PHASE;
    clock = 0.0f;
  } else if (clock > 1.0 && ready_set_go == 2) {
    ready_set_go = 1;
  } else if (clock > 0.0 && ready_set_go == 3) {
    ready_set_go = 2;
  }

  /*if all players have finished, or if only one kart is not finished when
    not in time trial mode, the race is over. Players are the last in the
    vector, so substracting the number of players finds the first player's
    position.*/
  unsigned int finished_karts = 0;
  unsigned int finished_plyrs = 0;
  for ( Karts::size_type i = 0; i < kart.size(); ++i)
  {
      if ( world->kart[i]->getLap () >= raceSetup.numLaps )
      {
          ++finished_karts;
          if(i >= kart.size() - raceSetup.players.size()) ++finished_plyrs;
      }
  }
  if(finished_plyrs == raceSetup.players.size()) phase = FINISH_PHASE;
  //JH debug only, to be able to run with a single player kart
  // !!!!!!!!!!!!!!!!!!!!!
  // else if(finished_karts == kart.size() - 1 && raceSetup.mode != RaceSetup::RM_TIME_TRIAL) phase = FINISH_PHASE;
}

void
World::updateLapCounter ( int k )
{
  int p = 1 ;

  /* Find position of kart 'k' */

  for ( Karts::size_type j = 0 ; j < kart.size() ; ++j )
  {
    if ( int(j) == k ) continue ;

    if ( kart[j]->getLap() >  kart[k]->getLap() ||
         ( kart[j]->getLap() == kart[k]->getLap() &&
           kart[j]->getDistanceDownTrack() >
                            kart[k]->getDistanceDownTrack() ))
      p++ ;
  }

  kart [ k ] -> setPosition ( p ) ;
}

void World::loadPlayers() {
  for ( Karts::size_type i = 0 ; i < kart.size() ; ++i )
    {
      kart[i]->load_data();
    }

}

void World::herring_command (char *s, char *str ) {

  sgVec3 xyz ;

  sscanf ( s, "%f,%f", &xyz[0], &xyz[1] ) ;
  // The height must be defined here, since getHeight only looks below
  xyz[2] = 1000000.0f;
  xyz[2] = getHeight ( trackBranch, xyz ) + 0.06 ;

  herringType type=HE_GREEN;
  if ( str[0]=='Y' || str[0]=='y' ){ type = HE_GOLD   ;}
  if ( str[0]=='G' || str[0]=='g' ){ type = HE_GREEN  ;}
  if ( str[0]=='R' || str[0]=='r' ){ type = HE_RED    ;}
  if ( str[0]=='S' || str[0]=='s' ){ type = HE_SILVER ;}
  herring_manager->newHerring(type, xyz);
}   // herring_command


void World::loadTrack() {
  std::string path = "data/";
  path += track->getIdent();
  path += ".loc";
  path = loader->getPath(path);

  // remove old herrings (from previous race), and remove old
  // track specific herring models
  herring_manager->cleanup();
  herring_manager->loadHerringData(track->getHerringStyle(),
				   HerringManager::ISTRACKDATA);
  FILE *fd = fopen (path.c_str(), "r" ) ;
  if ( fd == NULL ) {
    std::stringstream msg;
    msg << "Can't open track location file '" << path << "'.";
    throw std::runtime_error(msg.str());
  }

  char s [ 1024 ] ;

  while ( fgets ( s, 1023, fd ) != NULL ) {
    if ( *s == '#' || *s < ' ' )
      continue ;

    int need_hat = FALSE ;
    int fit_skin = FALSE ;
    char fname [ 1024 ] ;
    sgCoord loc ;
    sgZeroVec3 ( loc.xyz ) ;
    sgZeroVec3 ( loc.hpr ) ;

    char htype = '\0' ;

    if ( sscanf ( s, "%cHERRING,%f,%f", &htype,
                     &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 ) {
      herring_command ( & s [ strlen ( "*HERRING," ) ], s ) ;
    } else if ( s[0] == '\"' ) {
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f,%f,%f",
		    fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
		    &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2]) ) == 7 ) {
	/* All 6 DOF specified */
	need_hat = FALSE;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,%f,%f",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]),
			   &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2])) == 6 ){
	/* All 6 DOF specified - but need height */
	need_hat = TRUE ;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
			   &(loc.hpr[0]) ) == 5 ) {
	/* No Roll/Pitch specified - assumed zero */
	need_hat = FALSE ;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,{},{}",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]),
			   &(loc.hpr[0]) ) == 3 ) {
	/* All 6 DOF specified - but need height, roll, pitch */
	need_hat = TRUE ;
	fit_skin = TRUE ;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]),
			   &(loc.hpr[0]) ) == 4 ) {
	/* No Roll/Pitch specified - but need height */
	need_hat = TRUE ;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]),
			   &(loc.xyz[2]) ) == 4 ) {
	/* No Heading/Roll/Pitch specified - but need height */
	need_hat = FALSE ;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{}",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 ) {
	/* No Roll/Pitch specified - but need height */
	need_hat = TRUE ;
      } else if ( sscanf ( s, "\"%[^\"]\",%f,%f",
			   fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 ) {
	/* No Z/Heading/Roll/Pitch specified */
	need_hat = FALSE ;
      } else if ( sscanf ( s, "\"%[^\"]\"", fname ) == 1 ) {
	/* Nothing specified */
	need_hat = FALSE ;
      } else {
        fclose(fd);
        std::stringstream msg;
        msg << "Syntax error in '" << path << "': " << s;
        throw std::runtime_error(msg.str());
      }

      if ( need_hat ) {
	sgVec3 nrm ;

	loc.xyz[2] = 1000.0f ;
	loc.xyz[2] = getHeightAndNormal ( trackBranch, loc.xyz, nrm ) ;

	if ( fit_skin ) {
	  float sy = sin ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
	  float cy = cos ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;

	  loc.hpr[2] =  SG_RADIANS_TO_DEGREES * atan2 ( nrm[0] * cy -
							nrm[1] * sy, nrm[2] ) ;
	  loc.hpr[1] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[1] * cy +
							nrm[0] * sy, nrm[2] ) ;
	}
      }   // if need_hat

      ssgEntity        *obj   = ssgLoad ( fname, loader ) ;
      ssgRangeSelector *lod   = new ssgRangeSelector ;
      ssgTransform     *trans = new ssgTransform ( & loc ) ;

      float r [ 2 ] = { -10.0f, 2000.0f } ;

      lod         -> addKid    ( obj   ) ;
      trans       -> addKid    ( lod   ) ;
      trackBranch -> addKid    ( trans ) ;
      lod         -> setRanges ( r, 2  ) ;
    } else {
      fclose(fd);
      std::stringstream msg;
      msg << "Syntax error in '" << path << "': " << s;
      throw std::runtime_error(msg.str());
    }
  }   // while fgets

  fclose ( fd ) ;
}

void World::restartRace() {
  ready_set_go = 3;
  clock = 0.0f;
  phase = START_PHASE;

  for ( Karts::iterator i = kart.begin(); i != kart.end() ; ++i )
    (*i)->reset() ;
}

Kart* World::getKart(int kartId) {
  assert(kartId >= 0 && kartId < int(kart.size()));
  return kart[kartId];
}

PlayerKart* World::getPlayerKart(int player) {
  return (PlayerKart*)kart[raceSetup.players[player]];
}

/* EOF */
