//  $Id: tuxkart.cxx,v 1.38 2004/08/05 14:35:42 grumbel Exp $
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

#include "tuxkart.h"
#include "Loader.h"
#include "Herring.h"
#include "Driver.h"
#include "Explosion.h"
#include "isect.h"

#include "status.h"
#include "Camera.h"
#include "level.h"
#include "WidgetSet.h"
#include "BaseGUI.h"
#include "oldgui.h"
#include "WorldLoader.h"

#include "gfx.h"
#include "preprocessor.h"
#include "material.h"


int mirror  = 0 ;
int reverse = 0 ;

int finishing_position = -1 ;

guUDPConnection *net = NULL ;

int network_enabled = FALSE ;
int network_testing = FALSE ;

Herring *silver_h ;
Herring *gold_h   ;
Herring *red_h    ;
Herring *green_h  ;
 
Track     *curr_track  ;
ssgBranch *trackBranch ;

int num_herring   ;
int num_laps_in_race ;

char playersfname [ 256 ] ;
char *trackname = "tuxtrack" ;

HerringInstance herring [ MAX_HERRING ] ;

char *traffic_files [] =
{
  "icecreamtruck.ac", "truck1.ac",
} ;


char *projectile_files [] =
{
  "spark.ac",         /* COLLECT_SPARK          */
  "missile.ac",       /* COLLECT_MISSILE        */
  "flamemissile.ac",  /* COLLECT_HOMING_MISSILE */
  NULL
} ;


char *tinytux_file   = "tinytux_magnet.ac" ;
char *explosion_file = "explode.ac"    ;
char *parachute_file = "parachute.ac"  ;
char *magnet_file    = "magnet.ac"     ;
char *magnet2_file   = "magnetbzzt.ac" ;
char *anvil_file     = "anvil.ac"      ;



ulClock      *fclock = NULL ;
SoundSystem  *sound = NULL ;
GFX          *gfx = NULL ;
WidgetSet          *widgetSet = NULL ;
BaseGUI	*gui = NULL;
OldGUI       *oldgui = NULL ;
Level        level ;

GUISwitch guiSwitch = GUIS_CURRENT;

Karts kart;
TrafficDriver *traffic [ NUM_TRAFFIC     ] ;
Projectile *projectile [ NUM_PROJECTILES ] ;
Explosion   *explosion [ NUM_EXPLOSIONS  ] ;

ssgRoot      *scene = NULL ;
Track        *track = NULL ;


void load_players ( )
{
  std::vector<std::string> player_files;

  player_files.push_back("tuxkart.ac");
  player_files.push_back("geekokart.ac");
  player_files.push_back("bsodkart.ac");
  player_files.push_back("gownkart.ac");

  assert( player_files.size() >= kart.size() );

  for ( Karts::size_type i = 0 ; i < kart.size() ; ++i )
  {
    ssgEntity *pobj1 = ssgLoad ( parachute_file, loader ) ;
    ssgEntity *pobj2 = ssgLoad ( magnet_file   , loader ) ;
    ssgEntity *pobj3 = ssgLoad ( magnet2_file  , loader ) ;
    ssgEntity *pobj4 = ssgLoad ( anvil_file    , loader ) ;

    sgCoord cc ;
    sgSetCoord ( &cc, 0, 0, 2, 0, 0, 0 ) ;
    ssgTransform *ttt = new ssgTransform ( & cc ) ;
    ttt -> addKid ( ssgLoad ( tinytux_file  , loader ) ) ;

    ssgEntity *pobj5 = ttt ;

    float r [ 2 ] = { -10.0f, 100.0f } ;

    ssgEntity *obj = ssgLoad ( player_files [ i ].c_str(), loader ) ;

    sgSetCoord ( &cc, 0, 0, 0, 0, 0, 0 ) ;
    ssgTransform *xxx = new ssgTransform ( & cc ) ;
    xxx -> addKid ( obj ) ;
    obj = xxx ;
    
    ssgRangeSelector *lod = new ssgRangeSelector ;

    lod -> addKid ( obj ) ;
    lod -> setRanges ( r, 2 ) ;

    kart[i]-> getModel() -> addKid ( lod ) ;
    kart[i]-> addAttachment ( pobj1 ) ;
    kart[i]-> addAttachment ( pobj2 ) ;
    kart[i]-> addAttachment ( pobj3 ) ;
    kart[i]-> addAttachment ( pobj4 ) ;
    kart[i]-> addAttachment ( pobj5 ) ;
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
    ssgBranch *b = (ssgBranch *) ssgLoad ( explosion_file, loader ) ;
    explosion[i] = new Explosion ( b ) ;
  }
}


 
static void herring_command ( char *s, char *str )
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
 
  if ( mirror ) xyz[0] *= -1.0f ;
  sgCopyVec3 ( h->xyz, xyz ) ;
  h->eaten = FALSE ;
  h->scs   = new ssgTransform ;
  h->scs -> setTransform ( &c ) ;
  h->scs -> addKid ( h->her->getRoot () ) ;
  scene  -> addKid ( h->scs ) ;

  num_herring++ ;
}


void load_track (const char *fname )
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

    if ( sscanf ( s, "PLAYERS \"%[^\"]\"", fname ) == 1 )
    {
      strcpy ( playersfname, fname ) ;
    }
    else
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


static void banner ()
{
  printf ( "\n\n" ) ;
  printf ( "   TUXEDO T. PENGUIN stars in TUXKART!\n" ) ;
  printf ( "               by Steve and Oliver Baker\n" ) ;
  printf ( "                 <sjbaker1@airmail.net>\n" ) ;
  printf ( "                  http://tuxkart.sourceforge.net\n" ) ;
  printf ( "\n\n" ) ;
}


int tuxkartMain ( int _numLaps, int _mirror, int _reverse,
                  char *_levelName, int numPlayers, int numKarts )
{
  /* Say "Hi!" to the nice user. */

  banner () ;

  /* Initialise some horrid globals */

  fclock           = new ulClock ;
  mirror           = _mirror     ;
  reverse          = _reverse    ;
  num_laps_in_race = _numLaps    ;
  trackname        = _levelName  ;
  
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
  initStatusDisplay () ;

  /* Set the SSG loader options */

  loader -> setCreateStateCallback  ( getAppState ) ;
  loader -> setCreateBranchCallback ( process_userdata ) ;

  /* Grab the track centerline file */

  char fname [ 100 ] ;
  sprintf ( fname, "data/%s.drv", trackname ) ;

  curr_track = new Track ( fname, mirror, reverse ) ;
  gfx        = new GFX ( mirror ) ;

  Camera::setNumSplits ( numPlayers ) ;
  initCameras () ;


  /* Start building the scene graph */

  scene       = new ssgRoot   ;
  trackBranch = new ssgBranch ;
  scene -> addKid ( trackBranch ) ;

  /* Load the Herring */

  //sgVec3 cyan   = { 0.4, 1.0, 1.0 } ;
  sgVec3 yellow = { 1.0, 1.0, 0.4 } ;
  //sgVec3 red    = { 0.8, 0.0, 0.0 } ;
  //sgVec3 green  = { 0.0, 0.8, 0.0 } ;
 
  //red_h     = new Herring ( red    ) ; preProcessObj ( red_h -> getRoot(), mirror ) ;
  //green_h   = new Herring ( green  ) ; preProcessObj ( green_h -> getRoot(), mirror ) ;
  //silver_h  = new Herring ( cyan   ) ; preProcessObj ( silver_h -> getRoot(), mirror ) ;
  
  gold_h    = new Herring ( yellow ) ; preProcessObj ( gold_h -> getRoot(), mirror ) ;
  silver_h     = new Herring ( ssgLoad ( "coin.ac", loader )   ) ; preProcessObj ( silver_h -> getRoot(), mirror ) ;
  red_h     = new Herring ( ssgLoad ( "bonusblock.ac", loader )   ) ; preProcessObj ( red_h -> getRoot(), mirror ) ;
  green_h   = new Herring ( ssgLoad ( "banana.ac", loader )   ) ; preProcessObj ( green_h -> getRoot(), mirror ) ;

  // Create the karts and fill the kart vector with them
  for ( int i = 0 ; i < numKarts ; i++ )
  {
    /* Kart[0] is always the player. */
    KartDriver* newkart;

    if ( i == 0 )
      newkart = new PlayerKartDriver  ( i, new ssgTransform ) ;
    else
    if ( network_enabled )
      newkart = new NetworkKartDriver ( i, new ssgTransform ) ;
    else
      newkart = new AutoKartDriver    ( i, new ssgTransform ) ;

    sgCoord init_pos = { { 0, 0, 0 }, { 0, 0, 0 } } ;

    init_pos.xyz [ 0 ] = (float)(i-2) * 2.0f ;
    init_pos.xyz [ 1 ] = 2.0f ;

    if ( reverse ) init_pos.hpr[0] = 180.0f ;

    newkart -> setReset ( & init_pos ) ;
    newkart -> reset    () ;
    newkart -> getModel () -> clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

    scene -> addKid ( newkart -> getModel() ) ;

    kart.push_back(newkart);
  }

  /* Load the Projectiles */

  for ( int j = 0 ; j < NUM_PROJECTILES ; j++ )
  {
    projectile[j] = new Projectile ( new ssgTransform ) ;
    scene -> addKid ( projectile[j] -> getModel() ) ;
    projectile[j]->getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
  }

  /* Load the track models */

  sprintf ( fname, "data/%s.loc", trackname ) ;
  load_track   ( fname ) ;
  load_players ( ) ;

  preProcessObj ( scene, mirror ) ;


#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
  ssgSetBackFaceCollisions ( mirror ) ;
#endif
	
	guiSwitch = GUIS_RACE;

  /* Play Ball! */

  tuxKartMainLoop () ;
  return TRUE ;
}


void updateLapCounter ( int k )
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


void updateNetworkRead ()
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


void updateNetworkWrite ()
{
  if ( ! network_enabled ) return ;

  char buffer [ 1024 ] ;
  char *p = buffer ;

  memcpy ( p, kart[0]->getCoord   (), sizeof(sgCoord) ) ; p += sizeof(sgCoord) ;
  memcpy ( p, kart[0]->getVelocity(), sizeof(sgCoord) ) ; p += sizeof(sgCoord) ;

  net->sendMessage ( buffer, p - buffer ) ;
}



void tuxKartMainLoop ()
{
  /* Loop forever updating everything */

  while ( 1 )
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
    kartInput () ;
    updateGUI();
    oldgui      -> update () ;
    sound    -> update () ;

    /* Swap graphics buffers last! */

    gfx      -> done   () ;
  }
}

void shutdown()
{
	if (gui)
		delete gui;
	
	if (widgetSet)
		delete widgetSet;

	shutdownVideo ();
  
	exit (0);
}

/* EOF */
