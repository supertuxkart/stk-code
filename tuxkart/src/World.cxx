//  $Id: World.cxx,v 1.35 2004/09/24 18:41:26 matzebraun Exp $
//
//  TuxKart - a fun racing game with go-kart
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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "guNet.h"
#include "preprocessor.h"
#include "Explosion.h"
#include "Herring.h"
#include "Projectile.h"
#include "KartDriver.h"
#include "WidgetSet.h"
#include "tuxkart.h"
#include "Loader.h"
#include "material.h"
#include "World.h"
#include "Camera.h"
#include "RaceSetup.h"
#include "WorldLoader.h"
#include "PlayerDriver.h"
#include "AutoDriver.h"
#include "isect.h"
#include "Track.h"
#include "KartManager.h"
#include "TrackManager.h"

World* world = 0;

World::World(const RaceSetup& raceSetup_)
  : raceSetup(raceSetup_)
{
  phase = START_PHASE;

  scene = NULL;
  track = NULL;

  clock           = 0.0f;
  
  // Grab the track centerline file
  track = track_manager->getTrack(raceSetup.track) ;

  // Start building the scene graph
  scene       = new ssgRoot   ;
  trackBranch = new ssgBranch ;
  scene -> addKid ( trackBranch ) ;

  /* Load the Herring */
  sgVec3 yellow = { 1.0, 1.0, 0.4 } ;

  gold_h    = new Herring ( yellow ) ; 
  silver_h  = new Herring ( ssgLoad ( "coin.ac", loader )   ) ;
  red_h     = new Herring ( ssgLoad ( "bonusblock.ac", loader )   ) ; 
  green_h   = new Herring ( ssgLoad ( "banana.ac", loader )   ) ; 

  preProcessObj ( gold_h -> getRoot());
  preProcessObj ( silver_h -> getRoot());
  preProcessObj ( red_h -> getRoot());
  preProcessObj ( green_h -> getRoot());

  // Create the karts and fill the kart vector with them

  assert(raceSetup.karts.size() > 0);

  for (RaceSetup::Karts::iterator i = raceSetup.karts.begin() ; i != raceSetup.karts.end() ; ++i )
  {
    KartDriver* newkart;
    int pos = kart.size();

    if (std::find(raceSetup.players.begin(), raceSetup.players.end(), pos) != raceSetup.players.end())
      { // the given position belongs to a player
        newkart = new KartDriver ( this, kart_manager.getKart(*i), pos, new PlayerDriver ) ;
      }
    else
      newkart = new KartDriver ( this, kart_manager.getKart(*i), pos, new AutoDriver ) ;
    
    sgCoord init_pos = { { 0, 0, 0 }, { 0, 0, 0 } } ;

    init_pos.xyz [ 0 ] = (pos % 2 == 0) ? 1.5f : -1.5f ;
    init_pos.xyz [ 1 ] = -pos * 1.5f ;

    newkart -> setReset ( & init_pos ) ;
    newkart -> reset    () ;
    newkart -> getModel () -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

    scene -> addKid ( newkart -> getRoot() ) ;

    kart.push_back(newkart);
  }

  // Load the track models
  loadTrack   ( ) ;
  loadPlayers ( ) ;

  preProcessObj ( scene ) ;

#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  //ssgSetBackFaceCollisions ( raceSetup.mirror ) ;
#endif
	
  guiStack.push_back(GUIS_RACE);

  std::string music = track_manager->getTrack(raceSetup.track)->music_filename;
  
  if (!music.empty())
    sound -> change_track ( music.c_str() );

  ready_set_go = 3;
}

World::~World()
{  
  for ( unsigned int i = 0 ; i < kart.size() ; i++ )
    delete kart[i];

  kart.clear();
 
  for(Projectiles::iterator i = projectiles.begin();
      i != projectiles.end(); ++i)
    delete *i;
  for ( int i = 0 ; i < NUM_EXPLOSIONS ; ++i )
    delete explosion[i];

  ssgDeRefDelete(projectile_spark);
  ssgDeRefDelete(projectile_missle);
  ssgDeRefDelete(projectile_flamemissle);
    
  delete gold_h;
  delete silver_h;
  delete red_h;
  delete green_h;

  delete scene ; 
}

void
World::draw()
{
  for ( Karts::size_type i = 0 ; i < kart.size(); ++i) kart[ i ] -> placeModel() ;

  ssgGetLight ( 0 ) -> setPosition ( track->sun_position ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_AMBIENT , track->ambientcol  ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_DIFFUSE , track->diffusecol  ) ;
  ssgGetLight ( 0 ) -> setColour ( GL_SPECULAR, track->specularcol ) ;

  ssgCullAndDraw ( world->scene ) ;
}

void
World::update(float delta)
{
  clock += delta;

  checkRaceStatus();

  for ( Karts::size_type i = 0 ; i < kart.size(); ++i) kart[ i ] -> update (delta) ;
  for(Projectiles::iterator i = projectiles.begin();
      i != projectiles.end(); ++i)
    (*i)->update(delta);
          
  for ( int i = 0 ; i < NUM_EXPLOSIONS  ; i++ ) explosion  [ i ] -> update () ;
  for ( int i = 0 ; i < MAX_HERRING     ; i++ ) herring    [ i ] .  update () ;
  for ( Karts::size_type i = 0 ; i < kart.size(); ++i) updateLapCounter ( i ) ;

  /* Routine stuff we do even when paused */
  silver_h -> update () ;
  gold_h   -> update () ;
  red_h    -> update () ;
  green_h  -> update () ;
}

void
World::checkRaceStatus()
{
  if (clock > 1.0 && ready_set_go == 0)
    {
      ready_set_go = -1;
    }
  else if (clock > 2.0 && ready_set_go == 1)
    {
      ready_set_go = 0;
      phase = RACE_PHASE;
      clock = 0.0f;
    }
  else if (clock > 1.0 && ready_set_go == 2)
    {
      ready_set_go = 1;
    }
  else if (clock > 0.0 && ready_set_go == 3)
    {
      ready_set_go = 2;
    }

  if ( world->kart[0]->getLap () >= raceSetup.numLaps )
    {
      phase = FINISH_PHASE;
    }
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

void
World::loadPlayers()
{
  for ( Karts::size_type i = 0 ; i < kart.size() ; ++i )
    {
      kart[i]->load_data();
    }

  projectile_spark = ssgLoad("spark.ac");
  projectile_spark->ref();
  projectile_missle = ssgLoad("missile.ac");
  projectile_missle->ref();
  projectile_flamemissle = ssgLoad("flamemissile.ac");
  projectile_flamemissle->ref();

  for ( int i = 0 ; i < NUM_EXPLOSIONS ; i++ )
    {
      ssgBranch *b = (ssgBranch *) ssgLoad ( "explode.ac", loader ) ;
      explosion[i] = new Explosion ( this, b ) ;
    }
}

void
World::herring_command (char *s, char *str )
{
  if ( num_herring >= MAX_HERRING )
  {
    fprintf ( stderr, "Too many herring\n" ) ;
    return ;
  }
 
  HerringInstance *h = & herring[num_herring] ;
  sgVec3 xyz ;
 
  sscanf ( s, "%f,%f", &xyz[0], &xyz[1] ) ;
 
  xyz[2] = 1000000.0f ;
  xyz[2] = getHeight ( trackBranch, xyz ) + 0.06 ;
 
  sgCoord c ;
 
  sgSetVec3  ( c.hpr, 0.0f, 0.0f, 0.0f ) ;
  sgCopyVec3 ( c.xyz, xyz ) ;
 
  if ( str[0]=='Y' || str[0]=='y' ){ h->her = gold_h   ; h->type = HE_GOLD   ;}
  if ( str[0]=='G' || str[0]=='g' ){ h->her = green_h  ; h->type = HE_GREEN  ;}
  if ( str[0]=='R' || str[0]=='r' ){ h->her = red_h    ; h->type = HE_RED    ;}
  if ( str[0]=='S' || str[0]=='s' ){ h->her = silver_h ; h->type = HE_SILVER ;}
 
  sgCopyVec3 ( h->xyz, xyz ) ;
  h->eaten = FALSE ;
  h->scs   = new ssgTransform ;
  h->scs -> setTransform ( &c ) ;
  h->scs -> addKid ( h->her->getRoot () ) ;
  scene  -> addKid ( h->scs ) ;

  num_herring++ ;
}


void
World::loadTrack()
{
  std::string path = "data/";
  path += track->getIdent();
  path += ".loc";
  path = loader->getPath(path);
  FILE *fd = fopen (path.c_str(), "r" ) ;

  if ( fd == NULL )
  {
    std::stringstream msg;
    msg << "Can't open track location file '" << path << "'.";
    throw std::runtime_error(msg.str());
  }

  initWorld () ;

  char s [ 1024 ] ;

  while ( fgets ( s, 1023, fd ) != NULL )
  {
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
                     &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
    {
      herring_command ( & s [ strlen ( "*HERRING," ) ], s ) ;
    }
    else
    if ( s[0] == '\"' )
    {
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f,%f,%f",
		 fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
			&(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2]) ) == 7 )
      {
	/* All 6 DOF specified */
	need_hat = FALSE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,%f,%f",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]),
			  &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2]) ) == 6 )
      {
	/* All 6 DOF specified - but need height */
	need_hat = TRUE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
			  &(loc.hpr[0]) ) == 5 )
      {
	/* No Roll/Pitch specified - assumed zero */
	need_hat = FALSE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,{},{}",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.hpr[0]) ) == 3 )
      {
	/* All 6 DOF specified - but need height, roll, pitch */
	need_hat = TRUE ;
	fit_skin = TRUE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]),
			  &(loc.hpr[0]) ) == 4 )
      {
	/* No Roll/Pitch specified - but need height */
	need_hat = TRUE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 4 )
      {
	/* No Heading/Roll/Pitch specified - but need height */
	need_hat = FALSE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f,{}",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
      {
	/* No Roll/Pitch specified - but need height */
	need_hat = TRUE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\",%f,%f",
		   fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
      {
	/* No Z/Heading/Roll/Pitch specified */
	need_hat = FALSE ;
      }
      else 
      if ( sscanf ( s, "\"%[^\"]\"", fname ) == 1 )
      {
	/* Nothing specified */
	need_hat = FALSE ;
      }
      else
      {
        fclose(fd);
        std::stringstream msg;
        msg << "Syntax error in '" << path << "': " << s;
        throw std::runtime_error(msg.str());
      }

      if ( need_hat )
      {
	sgVec3 nrm ;

	loc.xyz[2] = 1000.0f ;
	loc.xyz[2] = getHeightAndNormal ( trackBranch, loc.xyz, nrm ) ;

	if ( fit_skin )
	{
	  float sy = sin ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
	  float cy = cos ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
   
	  loc.hpr[2] =  SG_RADIANS_TO_DEGREES * atan2 ( nrm[0] * cy -
							nrm[1] * sy, nrm[2] ) ;
	  loc.hpr[1] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[1] * cy +
							nrm[0] * sy, nrm[2] ) ; 
	}
      }

      ssgEntity        *obj   = ssgLoad ( fname, loader ) ;
      ssgRangeSelector *lod   = new ssgRangeSelector ;
      ssgTransform     *trans = new ssgTransform ( & loc ) ;

      float r [ 2 ] = { -10.0f, 2000.0f } ;

      lod         -> addKid    ( obj   ) ;
      trans       -> addKid    ( lod   ) ;
      trackBranch -> addKid    ( trans ) ;
      lod         -> setRanges ( r, 2  ) ;
    }
    else
    {
      fclose(fd);
      std::stringstream msg;
      msg << "Syntax error in '" << path << "': " << s;
      throw std::runtime_error(msg.str());
    }
  }

  fclose ( fd ) ;
}

void
World::restartRace()
{
  ready_set_go = 3;
  finishing_position = -1 ;
  clock = 0.0f;
  phase = START_PHASE;

  for ( Karts::iterator i = kart.begin(); i != kart.end() ; ++i )
    (*i)->reset() ;
}

KartDriver*
World::getKart(int kartId)
{
  assert(kartId >= 0 && kartId < int(kart.size()));
  return kart[kartId];
}

KartDriver*
World::getPlayerKart(int player)
{
  return kart[raceSetup.players[player]];
}

/* EOF */
