//  $Id: World.cxx,v 1.8 2004/08/12 14:54:55 matzebraun Exp $
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

#include <iostream>
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
#include "gfx.h"
#include "Camera.h"
#include "RaceSetup.h"
#include "WorldLoader.h"
#include "isect.h"
#include "TrackManager.h"

World* World::current_ = 0;

World::World(const RaceSetup& raceSetup_)
  : raceSetup(raceSetup_)
{
  current_ = this;

  scene = NULL;
  track = NULL;

  net = NULL ;
  network_enabled = FALSE ;
  network_testing = FALSE ;

  gfx = NULL;

  //int tuxkartMain (RaceSetup& raceSetup)

  /* Initialise some horrid globals */
  fclock           = new ulClock ;
  
  /* Network initialisation -- NOT WORKING YET */

  net              = new guUDPConnection ;
  network_testing  = FALSE ;
  network_enabled  = FALSE ;

#ifdef ENABLE_NETWORKING
  network_enabled  = TRUE ;
  net->connect ( argv[i] ) ;

  if ( network_enabled && network_testing )
  {
    fprintf ( stderr, "You'll need to run this program\n" ) ;
    fprintf ( stderr, "on the other machine too\n" ) ;
    fprintf ( stderr, "Type ^C to exit.\n" ) ;

    while ( 1 )
    {
      char buffer [ 20 ] ;

      secondSleep ( 1 ) ;

      if ( net->recvMessage( buffer, 20 ) > 0 )
	fprintf ( stderr, "%s\n", buffer ) ;
      else
	fprintf ( stderr, "*" ) ;

      net->sendMessage ( "Testing...", 11 ) ;
    }
  }
#endif

  initMaterials     () ;

  /* Set the SSG loader options */

  loader -> setCreateStateCallback  ( getAppState ) ;
  loader -> setCreateBranchCallback ( process_userdata ) ;

  // Grab the track centerline file
  curr_track = new Track ( track_manager.tracks[raceSetup.track].drv_filename.c_str(),
                           raceSetup.mirror, raceSetup.reverse ) ;
  gfx        = new GFX ( raceSetup.mirror ) ;

  int numSplits = raceSetup.numPlayers;
  if (numSplits == 3)
    numSplits++;
  Camera::setNumSplits ( numSplits ) ;
  initCameras () ;

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

  preProcessObj ( gold_h -> getRoot(),   raceSetup.mirror );
  preProcessObj ( silver_h -> getRoot(), raceSetup.mirror );
  preProcessObj ( red_h -> getRoot(),    raceSetup.mirror );
  preProcessObj ( green_h -> getRoot(),  raceSetup.mirror );

  if (raceSetup.numKarts == -1)
    raceSetup.numKarts = characters.size();

  // Create the karts and fill the kart vector with them
  for ( int i = 0 ; i < raceSetup.numKarts ; i++ )
  {
    /* Kart[0] is always the player. */
    KartDriver* newkart;

    if ( i < raceSetup.numPlayers )
      newkart = new PlayerKartDriver  ( kart_props, i ) ;
    else if ( network_enabled )
      newkart = new NetworkKartDriver ( characters[i], i ) ;
    else
      newkart = new AutoKartDriver    ( characters[i], i ) ;

    sgCoord init_pos = { { 0, 0, 0 }, { 0, 0, 0 } } ;

    init_pos.xyz [ 0 ] = (float)(i-2) * 2.0f ;
    init_pos.xyz [ 1 ] = 2.0f ;

    if ( raceSetup.reverse ) init_pos.hpr[0] = 180.0f ;

    newkart -> setReset ( & init_pos ) ;
    newkart -> reset    () ;
    newkart -> getModel () -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

    scene -> addKid ( newkart -> getRoot() ) ;

    kart.push_back(newkart);
  }

  /* Load the Projectiles */

  for ( int j = 0 ; j < NUM_PROJECTILES ; j++ )
  {
    projectile[j] = new Projectile ( ) ;
    scene -> addKid ( projectile[j] -> getModel() ) ;
    projectile[j]->getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
  }

  // Load the track models
  load_track   ( track_manager.tracks[raceSetup.track].loc_filename.c_str() ) ;
  load_players ( ) ;

  preProcessObj ( scene, raceSetup.mirror ) ;


#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  ssgSetBackFaceCollisions ( raceSetup.mirror ) ;
#endif
	
  guiStack.push_back(GUIS_RACE);
}

World::~World()
{  
/* This whole function needs some serious fixing - whatever was done
   in tuxKartMain (the function immediately above this one) needs
   undoing. I've simply taken the above function, reversed the order
   of all the statements, replaced all the "new"s with "delete"s and
   all the "init()"s with commented out and non-existent
   "deinit()"s */

  //FIXME: in load we had preProcessObj ( scene, raceSetup.mirror ) ;
  
  //FIXME:
  //unload_players ( ) ;
  //unload_track ( ) ;
  
  for ( unsigned int i = 0 ; i < kart.size() ; i++ )
    delete kart[i];

  kart.clear();
  
  for ( int j = 0 ; j < NUM_PROJECTILES ; j++ )
    delete projectile[j];
    
  delete gold_h;
  delete silver_h;
  delete red_h;
  delete green_h;
  
  delete trackBranch ;
  delete scene ; 
  
  //FIXME:
  //deinitCameras () ;
  
  delete gfx ;
  
  //FIXME:
  //deinitMaterials     () ;
  //the destructor of RaceGUI;
  
delete fclock ;

#ifdef ENABLE_NETWORKING
  net->disconnect ( ) ;
#endif  
  delete net ;
}

void
World::update()
{
  /* Stop updating if we are paused */

  if ( ! widgetSet -> get_paused () )
    {
      fclock->update    () ;
      updateNetworkRead () ;
      
      for ( Karts::size_type i = 0 ; i < kart.size(); ++i) kart[ i ] -> update () ;
      for ( int i = 0 ; i < NUM_PROJECTILES ; i++ ) projectile [ i ] -> update () ;
      for ( int i = 0 ; i < NUM_EXPLOSIONS  ; i++ ) explosion  [ i ] -> update () ;
      for ( int i = 0 ; i < MAX_HERRING     ; i++ ) herring    [ i ] .  update () ;
      for ( Karts::size_type i = 0 ; i < kart.size(); ++i) updateLapCounter ( i ) ;

      updateNetworkWrite () ;
      updateCameras      () ;
      updateWorld        () ;
    }

  /* Routine stuff we do even when paused */

  silver_h -> update () ;
  gold_h   -> update () ;
  red_h    -> update () ;
  green_h  -> update () ;

  updateGFX ( gfx ) ;

  pollEvents();
  kartInput (raceSetup) ;
  updateGUI(raceSetup);
  sound    -> update () ;

  /* Swap graphics buffers last! */

  gfx      -> done   () ;
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
World::updateNetworkRead ()
{
  if ( ! network_enabled ) return ;

  char buffer [ 1024 ] ;
  int len = 0 ;
  int got_one = FALSE ;

  while ( (len = net->recvMessage ( buffer, 1024 )) > 0 )
    got_one = TRUE ;
  
  if ( got_one )
  {
    char *p = buffer ;

    kart[1]->setCoord    ( (sgCoord *) p ) ; p += sizeof(sgCoord) ;
    kart[1]->setVelocity ( (sgCoord *) p ) ; p += sizeof(sgCoord) ;
  }
}

void
World::updateNetworkWrite ()
{
  if ( ! network_enabled ) return ;

  char buffer [ 1024 ] ;
  char *p = buffer ;

  memcpy ( p, kart[0]->getCoord   (), sizeof(sgCoord) ) ; p += sizeof(sgCoord) ;
  memcpy ( p, kart[0]->getVelocity(), sizeof(sgCoord) ) ; p += sizeof(sgCoord) ;

  net->sendMessage ( buffer, p - buffer ) ;
}

void
World::load_players()
{
  char *projectile_files [] =
    {
      "spark.ac",         /* COLLECT_SPARK          */
      "missile.ac",       /* COLLECT_MISSILE        */
      "flamemissile.ac",  /* COLLECT_HOMING_MISSILE */
      NULL
    } ;

  for ( Karts::size_type i = 0 ; i < kart.size() ; ++i )
    {
      kart[i]->load_data();
    }

  for ( int i = 0 ; i < NUM_PROJECTILES ; i++ )
    {
      ssgSelector *sel = new ssgSelector ;
      projectile[i]-> getModel() -> addKid ( sel ) ;

      for ( int j = 0 ; projectile_files [ j ] != NULL ; j++ )
        sel -> addKid ( ssgLoad ( projectile_files [ j ], loader ) ) ;

      projectile[i] -> off () ;
    }

  for ( int i = 0 ; i < NUM_EXPLOSIONS ; i++ )
    {
      ssgBranch *b = (ssgBranch *) ssgLoad ( "explode.ac", loader ) ;
      explosion[i] = new Explosion ( b ) ;
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
  xyz[2] = getHeight ( xyz ) + 0.06 ;
 
  sgCoord c ;
 
  sgSetVec3  ( c.hpr, 0.0f, 0.0f, 0.0f ) ;
  sgCopyVec3 ( c.xyz, xyz ) ;
 
  if ( str[0]=='Y' || str[0]=='y' ){ h->her = gold_h   ; h->type = HE_GOLD   ;}
  if ( str[0]=='G' || str[0]=='g' ){ h->her = green_h  ; h->type = HE_GREEN  ;}
  if ( str[0]=='R' || str[0]=='r' ){ h->her = red_h    ; h->type = HE_RED    ;}
  if ( str[0]=='S' || str[0]=='s' ){ h->her = silver_h ; h->type = HE_SILVER ;}
 
  if ( raceSetup.mirror ) xyz[0] *= -1.0f ;

  sgCopyVec3 ( h->xyz, xyz ) ;
  h->eaten = FALSE ;
  h->scs   = new ssgTransform ;
  h->scs -> setTransform ( &c ) ;
  h->scs -> addKid ( h->her->getRoot () ) ;
  scene  -> addKid ( h->scs ) ;

  num_herring++ ;
}


void
World::load_track(const char *fname )
{
  std::string path = loader->getPath(fname);
  FILE *fd = fopen (path.c_str(), "r" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "tuxkart: Can't open track file '%s'\n", fname ) ;
    exit ( 1 ) ;
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

    if ( sscanf ( s, "MUSIC \"%[^\"]\"", fname ) == 1 )
    {
      sound -> change_track ( fname ) ;
    }
    else
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
	fprintf ( stderr, "tuxkart: Syntax error in '%s':\n", fname ) ;
	fprintf ( stderr, "  %s\n", s ) ;
	exit ( 1 ) ;
      }

      if ( need_hat )
      {
	sgVec3 nrm ;

	loc.xyz[2] = 1000.0f ;
	loc.xyz[2] = getHeightAndNormal ( loc.xyz, nrm ) ;

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
      fprintf ( stderr, "tuxkart: Syntax error in '%s':\n", fname ) ;
      fprintf ( stderr, "  %s\n", s ) ;
      exit ( 1 ) ;
    }
  }

  fclose ( fd ) ;
}

void
World::restartRace()
{
  finishing_position = -1 ;
  
  for ( Karts::iterator i = kart.begin(); i != kart.end() ; ++i )
    (*i)->reset() ;
}

/* EOF */
