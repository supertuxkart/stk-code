
#include "tuxkart.h"

#define MIN_CAM_DISTANCE      5.0f  
#define MAX_CAM_DISTANCE     10.0f  // Was 15

int player=0;

int finishing_position = -1 ;

static ulClock ck2 ;
extern float tt[6];
guUDPConnection *net = NULL ;
ssgLoaderOptions *loader_opts = NULL ;

int network_enabled = FALSE ;
int network_testing = FALSE ;

Herring *silver_h ;
Herring *gold_h   ;
Herring *red_h    ;
Herring *green_h  ;
 
Track *curr_track ;
int num_herring   ;
int num_laps_in_race ;

char *trackname = "tuxtrack" ;

HerringInstance herring [ MAX_HERRING ] ;

sgCoord steady_cam ;
 
char player_files [ NUM_KARTS ][ 256 ] ;

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
GFX            *gfx = NULL ;
GUI            *gui = NULL ;

KartDriver       *kart [ NUM_KARTS       ] ;
TrafficDriver *traffic [ NUM_TRAFFIC     ] ;
Projectile *projectile [ NUM_PROJECTILES ] ;
Explosion   *explosion [ NUM_EXPLOSIONS  ] ;

int       num_karts = 0 ;
char       *datadir = NULL ;
ssgRoot      *scene = NULL ;
Track        *track = NULL ;
int      cam_follow =  0 ;
float    cam_delay  = 10.0f ;

#define MAX_FIXED_CAMERA 9

sgCoord fixedpos [ MAX_FIXED_CAMERA ] =
{
  { {    0,    0, 500 }, {    0, -90, 0 } },

  { {    0,  180,  30 }, {  180, -15, 0 } },
  { {    0, -180,  40 }, {    0, -15, 0 } },

  { {  300,    0,  60 }, {   90, -15, 0 } },
  { { -300,    0,  60 }, {  -90, -15, 0 } },

  { {  200,  100,  30 }, {  120, -15, 0 } },
  { {  200, -100,  40 }, {   60, -15, 0 } },
  { { -200, -100,  30 }, {  -60, -15, 0 } },
  { { -200,  100,  40 }, { -120, -15, 0 } }
} ;

Level level ;


void load_players ( char *fname )
{
  ssgEntity *obj;

  ssgEntity *pobj1 = ssgLoad ( parachute_file, loader_opts ) ;
  ssgEntity *pobj2 = ssgLoad ( magnet_file   , loader_opts ) ;
  ssgEntity *pobj3 = ssgLoad ( magnet2_file  , loader_opts ) ;
  ssgEntity *pobj4 = ssgLoad ( anvil_file    , loader_opts ) ;

  sgCoord cc ;
  sgSetCoord ( &cc, 0, 0, 2, 0, 0, 0 ) ;
  ssgTransform *ttt = new ssgTransform ( & cc ) ;
  ttt -> addKid ( ssgLoad ( tinytux_file  , loader_opts ) ) ;

  ssgEntity *pobj5 = ttt ;
  int i ;
 
  FILE *fd = fopen ( fname, "r" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "tuxkart: Can't open '%s':\n", fname ) ;
    exit ( 1 ) ;
  }

  num_karts = 0 ;

  while ( num_karts < NUM_KARTS )
  {
    if ( fgets ( player_files [ num_karts ], 256, fd ) != NULL )
    {
      if ( player_files [ num_karts ][ 0 ] <= ' ' ||
           player_files [ num_karts ][ 0 ] == '#' )
        continue ;

      /* Trim off the '\n' */

      int len = strlen ( player_files [ num_karts ] ) - 1 ;

      if ( player_files [ num_karts ][ len ] <= ' ' )
        player_files [ num_karts ][ len ] = '\0' ;

fprintf(stderr,"Kart %d == '%s'\n", num_karts, player_files [ num_karts ] ) ;

      num_karts++ ;
    }
    else
      break ;
  }
 
  fclose ( fd ) ;

  if ( player >= num_karts )
    player = 0 ;

  for ( i = 0 ; i < num_karts ; i++ )
  {
    ssgRangeSelector *lod = new ssgRangeSelector ;
    float r [ 2 ] = { -10.0f, 100.0f } ;
    int kart_id ;

    if ( i == 0 )
      kart_id = player ;
    else
    if ( i == player )
      kart_id = 0 ;
    else
      kart_id = i ;
 
    obj = ssgLoad ( player_files [ kart_id ], loader_opts ) ;

    lod -> addKid ( obj ) ;
    lod -> setRanges ( r, 2 ) ;
    
    kart[i]-> getModel() -> addKid ( lod ) ;
    kart[i]-> addAttachment ( pobj1 ) ;
    kart[i]-> addAttachment ( pobj2 ) ;
    kart[i]-> addAttachment ( pobj3 ) ;
    kart[i]-> addAttachment ( pobj4 ) ;
    kart[i]-> addAttachment ( pobj5 ) ;
  }

  for ( i = 0 ; i < NUM_PROJECTILES ; i++ )
  {
    ssgSelector *sel = new ssgSelector ;
    projectile[i]-> getModel() -> addKid ( sel ) ;

    for ( int j = 0 ; projectile_files [ j ] != NULL ; j++ )
      sel -> addKid ( ssgLoad ( projectile_files [ j ], loader_opts ) ) ;

    projectile[i] -> off () ;
  }

  for ( i = 0 ; i < NUM_EXPLOSIONS ; i++ )
  {
    explosion[i] = new Explosion ( (ssgBranch *) ssgLoad ( explosion_file,
                                                    loader_opts ) ) ;
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
 
  sgCopyVec3 ( h->xyz, xyz ) ;
  sgSetVec3  ( c.hpr, 0.0f, 0.0f, 0.0f ) ;
  sgCopyVec3 ( c.xyz, h->xyz ) ;
 
  if ( str[0]=='Y' || str[0]=='y' ){ h->her = gold_h   ; h->type = HE_GOLD   ;}
  if ( str[0]=='G' || str[0]=='g' ){ h->her = green_h  ; h->type = HE_GREEN  ;}
  if ( str[0]=='R' || str[0]=='r' ){ h->her = red_h    ; h->type = HE_RED    ;}
  if ( str[0]=='S' || str[0]=='s' ){ h->her = silver_h ; h->type = HE_SILVER ;}
 
  h->eaten = FALSE ;
  h->scs   = new ssgTransform ;
  h->scs -> setTransform ( &c ) ;
  h->scs -> addKid ( h->her->getRoot () ) ;
  scene  -> addKid ( h->scs ) ;
 
  num_herring++ ;
}



void load_track ( ssgBranch *trackb, char *fname )
{
  FILE *fd = fopen ( fname, "r" ) ;
  char playersfname [ 256 ] ;

  strcpy ( playersfname, "data/players.dat" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "tuxkart: Can't open track file '%s'\n", fname ) ;
    exit ( 1 ) ;
  }

  init_hooks () ;

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

      ssgEntity        *obj   = ssgLoad ( fname, loader_opts ) ;
      ssgRangeSelector *lod   = new ssgRangeSelector ;
      ssgTransform     *trans = new ssgTransform ( & loc ) ;

      float r [ 2 ] = { -10.0f, 2000.0f } ;

      lod    -> addKid ( obj   ) ;
      trans  -> addKid ( lod   ) ;
      trackb -> addKid ( trans ) ;
      lod -> setRanges ( r, 2  ) ;
    }
    else
    {
      fprintf ( stderr, "tuxkart: Syntax error in '%s':\n", fname ) ;
      fprintf ( stderr, "  %s\n", s ) ;
      exit ( 1 ) ;
    }
  }

  fclose ( fd ) ;

  sgSetVec3  ( steady_cam.xyz, 0.0f, 0.0f, 0.0f ) ;
  sgSetVec3  ( steady_cam.hpr, 0.0f, 0.0f, 0.0f ) ;

  load_players ( playersfname ) ;
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

static void cmdline_help ()
{
  banner () ;

  printf ( "Usage:\n\n" ) ;
  printf ( "    tuxkart [OPTIONS] [machine_name]\n\n" ) ;
  printf ( "Options:\n" ) ;
  printf ( "  -h     Display this help message.\n" ) ;
  printf ( "  -t     Run a network test.\n" ) ;
  printf ( "\n" ) ;
}


int tuxkart_main ( int num_laps, char *level_name )
{
  net   = new guUDPConnection ;
  fclock = new ulClock ;

  if ( loader_opts == NULL )
  {
    loader_opts = new ssgLoaderOptions () ;
    loader_opts -> setCreateStateCallback ( getAppState ) ;
    loader_opts -> setCreateBranchCallback ( process_userdata ) ;
    ssgSetCurrentOptions ( loader_opts ) ;
  }

  num_laps_in_race = num_laps ;

  trackname = level_name ;

  network_testing = FALSE ;
  network_enabled = FALSE ;
/*
  network_enabled = TRUE ;
  net->connect ( argv[i] ) ;
*/

  if ( network_enabled && network_testing )
  {
    fprintf ( stderr, "You'll need to run this program\n" ) ;
    fprintf ( stderr, "on the other machine too\n" ) ;
    fprintf ( stderr, "Type ^C to exit.\n" ) ;

    while ( 1 )
    {
      char buffer [ 20 ] ;

#ifdef _MSC_VER
      Sleep ( 1000 ) ;
#else
	  sleep ( 1 ) ;
#endif

      if ( net->recvMessage( buffer, 20 ) > 0 )
	fprintf ( stderr, "%s\n", buffer ) ;
      else
	fprintf ( stderr, "*" ) ;

      net->sendMessage ( "Testing...", 11 ) ;
    }
  }

  /* Set tux_aqfh_datadir to the correct directory */

  if ( datadir == NULL )
  {
    if ( getenv ( "TUXKART_DATADIR" ) != NULL )
      datadir = getenv ( "TUXKART_DATADIR" ) ;
    else
#ifdef _MSC_VER
    if ( _access ( "data/levels.dat", 04 ) == 0 )
#else
    if ( access ( "data/levels.dat", F_OK ) == 0 )
#endif
      datadir = "." ;
    else
#ifdef _MSC_VER
    if ( _access ( "../data/levels.dat", 04 ) == 0 )
#else
    if ( access ( "../data/levels.dat", F_OK ) == 0 )
#endif
      datadir = ".." ;
    else
#ifdef TUXKART_DATADIR
      datadir = TUXKART_DATADIR ;
#else
      datadir = "/usr/local/share/games/tuxkart" ;
#endif
  }

  fprintf ( stderr, "Data files will be fetched from: '%s'\n",
                                                    datadir ) ;

#ifdef _MSC_VER
  if ( _chdir ( datadir ) == -1 )
#else
  if ( chdir ( datadir ) == -1 )
#endif
  {
    fprintf ( stderr, "Couldn't chdir() to '%s'.\n", datadir ) ;
    exit ( 1 ) ;
  }

  banner () ;

  char fname [ 100 ] ;
  sprintf ( fname, "data/%s.drv", trackname ) ;

  curr_track = new Track ( fname ) ;
  gfx   = new GFX ;

  sound = new SoundSystem ;
  // sound -> change_track ( "mods/Boom_boom_boom.mod" ) ;
  gui   = new GUI ;

  pwSetCallbacks ( keystroke, mousefn, motionfn, reshape, NULL ) ;

  ssgModelPath   ( "models" ) ;
  ssgTexturePath ( "images" ) ;

  ssgBranch *trackb ;

  scene  = new ssgRoot ;
  trackb = new ssgBranch ;
  scene -> addKid ( trackb ) ;

  sgVec3 cyan   = { 0.4, 1.0, 1.0 } ;
  sgVec3 yellow = { 1.0, 1.0, 0.4 } ;
  sgVec3 red    = { 0.8, 0.0, 0.0 } ;
  sgVec3 green  = { 0.0, 0.8, 0.0 } ;
 
  silver_h  = new Herring ( cyan   ) ;
  gold_h    = new Herring ( yellow ) ;
  red_h     = new Herring ( red    ) ;
  green_h   = new Herring ( green  ) ;

  kart[0] = new PlayerKartDriver ( 0, new ssgTransform ) ;
  scene -> addKid ( kart[0] -> getModel() ) ;
  kart[0]->getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

  if ( network_enabled )
    kart[1] = new NetworkKartDriver ( 1, new ssgTransform ) ;
  else
    kart[1] = new AutoKartDriver ( 1, new ssgTransform ) ;

  scene -> addKid ( kart[1] -> getModel() ) ;
  kart[1]->getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);

  int i;
  for ( i = 2 ; i < NUM_KARTS ; i++ )
  {
    kart[i] = new AutoKartDriver ( i, new ssgTransform ) ;
    scene -> addKid ( kart[i] -> getModel() ) ;
    kart[i]->getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
  }


  for ( i = 0 ; i < NUM_PROJECTILES ; i++ )
  {
    projectile[i] = new Projectile ( new ssgTransform ) ;
    scene -> addKid ( projectile[i] -> getModel() ) ;
    projectile[i]->getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
  }

  /*
    Load the models - optimise them a bit
    and then add them into the scene.
  */

  sprintf ( fname, "data/%s.loc", trackname ) ;

  load_track ( trackb, fname ) ;

fprintf ( stderr, "READY TO RACE!!\n" ) ;

  tuxKartMainLoop () ;
  return TRUE ;
}


void updateWorld ()
{
  if ( network_enabled )
  {
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

  int i;
  ck2.update() ; tt[1] = ck2.getDeltaTime()*1000.0f ;
  for ( i = 0 ; i < num_karts       ; i++ ) kart       [ i ] -> update () ;
  ck2.update() ; tt[2] = ck2.getDeltaTime()*1000.0f ;
  for ( i = 0 ; i < NUM_PROJECTILES ; i++ ) projectile [ i ] -> update () ;
  ck2.update() ; tt[3] = ck2.getDeltaTime()*1000.0f ;
  for ( i = 0 ; i < NUM_EXPLOSIONS  ; i++ ) explosion  [ i ] -> update () ;

  for ( i = 0 ; i < num_karts ; i++ )
  {
    int p = 1 ;

    for ( int j = 0 ; j < num_karts ; j++ )
    {
      if ( j == i ) continue ;

      if ( kart[j]->getLap() >  kart[i]->getLap() ||
           ( kart[j]->getLap() == kart[i]->getLap() && 
             kart[j]->getDistanceDownTrack() >
                              kart[i]->getDistanceDownTrack() ))
        p++ ;      
    }

    kart [ i ] -> setPosition ( p ) ;
  }

  if ( network_enabled )
  {
    char buffer [ 1024 ] ;
    char *p = buffer ;

    memcpy ( p, kart[0]->getCoord   (), sizeof(sgCoord) ) ;
    p += sizeof(sgCoord) ;
    memcpy ( p, kart[0]->getVelocity(), sizeof(sgCoord) ) ;
    p += sizeof(sgCoord) ;

    net->sendMessage ( buffer, p - buffer ) ;
  }

  if ( cam_follow < 0 )
    cam_follow = 18 + MAX_FIXED_CAMERA - 1 ;
  else
  if ( cam_follow >= 18 + MAX_FIXED_CAMERA )
    cam_follow = 0 ;

  sgCoord final_camera ;

  if ( cam_follow < num_karts )
  {
    sgCoord cam, target, diff ;

    sgCopyCoord ( &target, kart[cam_follow]->getCoord   () ) ;
    sgCopyCoord ( &cam   , kart[cam_follow]->getHistory ( (int)cam_delay ) ) ;

    float dist = 5.0f + sgDistanceVec3 ( target.xyz, cam.xyz ) ;

    if ( dist < MIN_CAM_DISTANCE && cam_delay < 50 )
      cam_delay++ ;

    if ( dist > MAX_CAM_DISTANCE && cam_delay > 1 )
      cam_delay-- ;

    sgVec3 offset ;
    sgMat4 cam_mat ;

    sgSetVec3 ( offset, -0.5f, -5.0f, 1.5f ) ;
    sgMakeCoordMat4 ( cam_mat, &cam ) ;

    sgXformPnt3 ( offset, cam_mat ) ;

    sgCopyVec3 ( cam.xyz, offset ) ;

    cam.hpr[1] = -5.0f ;
    cam.hpr[2] = 0.0f;

    sgSubVec3 ( diff.xyz, cam.xyz, steady_cam.xyz ) ;
    sgSubVec3 ( diff.hpr, cam.hpr, steady_cam.hpr ) ;

    while ( diff.hpr[0] >  180.0f ) diff.hpr[0] -= 360.0f ;
    while ( diff.hpr[0] < -180.0f ) diff.hpr[0] += 360.0f ;
    while ( diff.hpr[1] >  180.0f ) diff.hpr[1] -= 360.0f ;
    while ( diff.hpr[1] < -180.0f ) diff.hpr[1] += 360.0f ;
    while ( diff.hpr[2] >  180.0f ) diff.hpr[2] -= 360.0f ;
    while ( diff.hpr[2] < -180.0f ) diff.hpr[2] += 360.0f ;

    steady_cam.xyz[0] += 0.2f * diff.xyz[0] ;
    steady_cam.xyz[1] += 0.2f * diff.xyz[1] ;
    steady_cam.xyz[2] += 0.2f * diff.xyz[2] ;
    steady_cam.hpr[0] += 0.1f * diff.hpr[0] ;
    steady_cam.hpr[1] += 0.1f * diff.hpr[1] ;
    steady_cam.hpr[2] += 0.1f * diff.hpr[2] ;

    final_camera = steady_cam ;
  }
  else
  if ( cam_follow < num_karts + MAX_FIXED_CAMERA )
  {
    final_camera = fixedpos[cam_follow-num_karts] ;
  }
  else
    final_camera = steady_cam ;

  sgVec3 interfovealOffset ;
  sgMat4 mat ;

  sgSetVec3 ( interfovealOffset, 0.2 * (float)stereoShift(), 0, 0 ) ;
  sgMakeCoordMat4 ( mat, &final_camera ) ;
  sgXformPnt3 ( final_camera.xyz, interfovealOffset, mat ) ;

  ssgSetCamera ( &final_camera ) ;
}



void tuxKartMainLoop ()
{
  while ( 1 )
  {
    if ( ! gui -> isPaused () )
    {
      fclock->update () ;
      ck2.update() ; tt[0] = ck2.getDeltaTime()*1000.0f ;
      updateWorld () ;

      for ( int i = 0 ; i < MAX_HERRING ; i++ )
        if ( herring [ i ] . her != NULL )
          herring [ i ] . update () ;

      silver_h -> update () ;
      gold_h   -> update () ;
      red_h    -> update () ;
      green_h  -> update () ;

      update_hooks () ;
    }
    else
    {
      ck2.update() ; tt[0] = ck2.getDeltaTime()*1000.0f ;
      ck2.update() ; tt[1] = ck2.getDeltaTime()*1000.0f ;
      ck2.update() ; tt[2] = ck2.getDeltaTime()*1000.0f ;
      ck2.update() ; tt[3] = ck2.getDeltaTime()*1000.0f ;
    }

/*track  -> update () ; */

    ck2.update() ; tt[4] = ck2.getDeltaTime()*1000.0f ;
    gfx    -> update () ;
    gui    -> update () ;
    sound  -> update () ;
    gfx    -> done   () ;  /* Swap buffers! */
    ck2.update() ; tt[5] = ck2.getDeltaTime()*1000.0f ;
  }
}


