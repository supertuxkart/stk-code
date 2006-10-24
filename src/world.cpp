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

#include "world.hpp"
#include "preprocessor.hpp"
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
#include "config.hpp"
#include "callback_manager.hpp"
#include "history.hpp"
#include "constants.hpp"
#include "sound_manager.hpp"
#include "widget_set.hpp"

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
  
  assert(raceSetup.karts.size() > 0);

  // Clear all callbacks, which might still be stored there from a previous race.
  callback_manager->clear(CB_TRACK);

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
    if(config->profile) {
      // In profile mode, load only the old kart
      newkart = new AutoKart (kart_properties_manager->getKart("tuxkart"), pos);
    } else {
      if (std::find(raceSetup.players.begin(),
		    raceSetup.players.end(), pos) != raceSetup.players.end())
      {
	// the given position belongs to a player
	    newkart = new PlayerKart (kart_properties_manager->getKart(*i), pos,
                      &(config->player[playerIndex++]));
      } else {
	newkart = new AutoKart   (kart_properties_manager->getKart(*i), pos);
      }
    }   // if !config->profile
    if(config->replayHistory) {
      history->LoadKartData(newkart, pos);
    }
    sgCoord init_pos = { { 0, 0, 0 }, { 0, 0, 0 } } ;


    //float hot=0.0;
    // Bug fix/workaround: sometimes the first kart would be too close
    // to the first driveline point and not to the last one -->
    // This kart would not get any lap counting done in the first
    // lap! Therefor -1.5 is subtracted from the y position - which
    // is a somewhat arbitrary value.
    init_pos.xyz[0] = (pos % 2 == 0) ? 1.5f : -1.5f ;
    init_pos.xyz[1] = -pos * 1.5f -1.5f;
    float hot = newkart->getIsectData ( init_pos.xyz, init_pos.xyz ) ;
    init_pos.xyz[2] = hot;
    newkart -> setReset ( & init_pos ) ;
    newkart -> reset    () ;
    newkart -> getModel () -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

    scene -> addKid ( newkart -> getModel() ) ;

    kart.push_back(newkart);
    pos++;
  }  // for i

  loadPlayers ( ) ;
  numberCollisions = new int[raceSetup.getNumKarts()];
  for(unsigned int i=0; i<raceSetup.getNumKarts(); i++) numberCollisions[i]=0;
  preProcessObj ( scene ) ;

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  //ssgSetBackFaceCollisions ( raceSetup.mirror ) ;
#endif

  menu_manager->switchToRace();

  const std::string& music_name= track_manager->getTrack(raceSetup.track)->getMusic();
  if (music_name.size()>0) sound_manager->playMusic(music_name.c_str());

  if(config->profile) {
    ready_set_go = -1;
    phase        = RACE_PHASE;
  } else {
    phase        = START_PHASE;  // profile starts without countdown
    ready_set_go = 3;
  }
}

World::~World() {
  for ( unsigned int i = 0 ; i < kart.size() ; i++ )
    delete kart[i];

  kart.clear();
  projectile_manager->cleanup();
  delete [] numberCollisions;
  delete scene ;

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

  // Count the number of collision in the next 'framesForTrafficJam' frames.
  // If a kart has more than one hit, play 'traffic jam' noise.
  static int nCount=0;
  const int framesForTrafficJam=20;
  nCount++;
  if(nCount==framesForTrafficJam) {
    for(unsigned int i=0; i<raceSetup.getNumKarts(); i++) numberCollisions[i]=0;
    nCount=0;
  }
  if( getPhase() == FINISH_PHASE ) {
    widgetSet->tgl_paused();
    menu_manager->pushMenu(MENUID_RACERESULT);
  }

  float inc = 0.05f;
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
  
  for ( Karts::size_type i = 0 ; i < kart.size(); ++i) {
    if(!kart[i]->raceIsFinished()) updateRacePosition((int)i);
  }

  /* Routine stuff we do even when paused */
  callback_manager->update(delta);

  // Check for traffic jam. The sound is played even if it's
  // not a player kart - a traffic jam happens rarely anyway.
  for(unsigned int i=0; i<raceSetup.getNumKarts(); i++) {
    if(numberCollisions[i]>1) {
      sound_manager->playSfx(SOUND_TRAFFIC_JAM);
      nCount = framesForTrafficJam-1;  // sets all fields to zero in next frame
      break;
    }
  }

}

void World::checkRaceStatus() {
  if (clock > 1.0 && ready_set_go == 0) {
      ready_set_go = -1;
  } else if (clock > 2.0 && ready_set_go == 1) {
    ready_set_go = 0;
    phase = RACE_PHASE;
    clock = 0.0f;
    sound_manager->playSfx(SOUND_START);
  } else if (clock > 1.0 && ready_set_go == 2) {
    sound_manager->playSfx(SOUND_PRESTART);
    ready_set_go = 1;
  } else if (clock > 0.0 && ready_set_go == 3) {
    sound_manager->playSfx(SOUND_PRESTART);
    ready_set_go = 2;
  }

  /*if all players have finished, or if only one kart is not finished when
    not in time trial mode, the race is over. Players are the last in the
    vector, so substracting the number of players finds the first player's
    position.*/
  int new_finished_karts   = 0;
  for ( Karts::size_type i = 0; i < kart.size(); ++i)
  {
      if ((kart[i]->getLap () >= raceSetup.numLaps) && !kart[i]->raceIsFinished())
      {
          kart[i]->setFinishingState(clock);

          race_manager->addKartScore((int)i, kart[i]->getPosition());

          ++new_finished_karts;
	  if(kart[i]->isPlayerKart()) {
	    race_manager->PlayerFinishes();
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
  if(race_manager->allPlayerFinished() && phase == RACE_PHASE) {
    phase = DELAY_FINISH_PHASE;
    finishDelayStartTime = clock;
  } else if(phase==DELAY_FINISH_PHASE && 
	    clock-finishDelayStartTime>TIME_DELAY_TILL_FINISH) {
    phase = FINISH_PHASE;
    for ( Karts::size_type i = 0; i < kart.size(); ++i) {
      if(!kart[i]->raceIsFinished()) {
	// FIXME: Add some tenths to have different times - a better solution
	//        would be to estimate the distance to go and use this to
	//        determine better times.
	kart[i]->setFinishingState(clock+kart[i]->getPosition()*0.3f);
      }   // if !raceIsFinished
    }   // for i
    
  } else if(raceSetup.getNumKarts() == 1) {
    if(race_manager->getFinishedKarts() == 1) phase = FINISH_PHASE;
  } else if(race_manager->getFinishedKarts() >= raceSetup.getNumKarts() - 1) {
    phase = FINISH_PHASE;
    for ( Karts::size_type i = 0; i < kart.size(); ++i) {
      if(!kart[i]->raceIsFinished()) {
	kart[i]->setFinishingState(clock);
      }   // if !raceIsFinished
    }   // for i
  }   // if getFinishedKarts()>=geNumKarts()

}  // checkRaceStatus

void
World::updateRacePosition ( int k )
{
  int p = 1 ;

  /* Find position of kart 'k' */

  for ( Karts::size_type j = 0 ; j < kart.size() ; ++j )
  {
    if ( int(j) == k ) continue ;

    // Count karts ahead of the current kart, i.e. kart that are already 
    // finished (the current kart k has not yet finished!!), have done more
    // laps, or the same number of laps, but a greater distance.
    if (kart[j]->raceIsFinished()                                          ||
	kart[j]->getLap() >  kart[k]->getLap()                             ||
        (kart[j]->getLap() == kart[k]->getLap() &&
	 kart[j]->getDistanceDownTrack() > kart[k]->getDistanceDownTrack()) )
      p++ ;
  }

  kart [ k ] -> setPosition ( p ) ;
}   // updateRacePosition

void World::loadPlayers() {
  for ( Karts::size_type i = 0 ; i < kart.size() ; ++i )
    {
      kart[i]->load_data();
    }

}

void World::herring_command (sgVec3 *xyz, char htype, int bNeedHeight ) {

  // if only 2d coordinates are given, let the herring fall from very heigh
  if(bNeedHeight) (*xyz)[2] = 1000000.0f;

  // Even if 3d data are given, make sure that the herring is on the ground
  (*xyz)[2] = getHeight ( trackBranch, *xyz ) + 0.06f;
  herringType type=HE_GREEN;
  if ( htype=='Y' || htype=='y' ){ type = HE_GOLD   ;}
  if ( htype=='G' || htype=='g' ){ type = HE_GREEN  ;}
  if ( htype=='R' || htype=='r' ){ type = HE_RED    ;}
  if ( htype=='S' || htype=='s' ){ type = HE_SILVER ;}
  herring_manager->newHerring(type, xyz);
}   // herring_command


void World::loadTrack() {
  std::string path = "data/";
  path += track->getIdent();
  path += ".loc";
  path = loader->getPath(path.c_str());

  // remove old herrings (from previous race), and remove old
  // track specific herring models
  herring_manager->cleanup();
  if(raceSetup.mode==RaceSetup::RM_GRAND_PRIX) {
  try {
    herring_manager->loadHerringStyle(raceSetup.getHerringStyle());
  } catch(std::runtime_error) {
    fprintf(stderr, "The cup '%s' contains an invalid herring style '%s'.\n",
	    race_manager->getGrandPrix()->getName().c_str(), 
	    race_manager->getGrandPrix()->getHerringStyle().c_str());
    fprintf(stderr, "Please fix the file '%s'.\n",
	    race_manager->getGrandPrix()->getFilename().c_str()); 
  }
  } else {
    try {
      herring_manager->loadHerringStyle(track->getHerringStyle());
    } catch(std::runtime_error) {
    fprintf(stderr, "The track '%s' contains an invalid herring style '%s'.\n",
	    track->getName(), track->getHerringStyle().c_str());
    fprintf(stderr, "Please fix the file '%s'.\n",
	    track->getFilename().c_str()); 
    }
  }

  FILE *fd = fopen (path.c_str(), "r" );
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

    if ( sscanf ( s, "%cHERRING,%f,%f,%f", &htype,
		  &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 4 ) {
      herring_command(&loc.xyz, htype, false) ;
    } else if ( sscanf ( s, "%cHERRING,%f,%f", &htype,
                     &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 ) {
      herring_command (&loc.xyz, htype, true) ;
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

      ssgEntity        *obj   = loader->load(fname, CB_TRACK);
      createDisplayLists(obj);
      ssgRangeSelector *lod   = new ssgRangeSelector ;
      ssgTransform     *trans = new ssgTransform ( & loc ) ;

      float r [ 2 ] = { -10.0f, 2000.0f } ;

      lod         -> addKid    ( obj   ) ;
      trans       -> addKid    ( lod   ) ;
      trackBranch -> addKid    ( trans ) ;
      lod         -> setRanges ( r, 2  ) ;

      #ifdef DEBUG_SHOW_DRIVEPOINTS
      ssgaSphere *sphere;
      sgVec3 center;
      sgVec4 colour;
      for(unsigned int i = 0; i < track->driveline.size(); ++i)
      {
          sphere = new ssgaSphere;
          sgCopyVec3(center, track->driveline[i]);
          sphere->setCenter(center);
          sphere->setSize(track->getWidth()[i] / 4.0f);

          if(i == 0)
          {
              colour[0] = colour[2] = colour[3] = 255;
              colour[1] = 0;
          }
          else
          {
              colour[0] = colour[1] = colour[3] = 255;
              colour[2] = 0;
          }
          sphere->setColour(colour);
          scene->addKid(sphere);
      }
      #endif

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
  clock        = 0.0f;
  phase        = START_PHASE;

  for ( Karts::iterator i = kart.begin(); i != kart.end() ; ++i ) {
    (*i)->reset();
  }
  herring_manager->reset();
  projectile_manager->cleanup();
  race_manager->reset();
}


PlayerKart* World::getPlayerKart(int player) {
  return (PlayerKart*)kart[raceSetup.players[player]];
}

/* EOF */
