
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#ifdef __CYGWIN__
#include <unistd.h>
#endif
#include <windows.h>
#include <io.h>
//#include <direct.h>
#else
#include <unistd.h>
#endif
#include <math.h>

#ifdef HAVE_LIBSDL
#include "sdldrv.h"
#else
#include "pwdrv.h"
#endif

#include <plib/ssg.h>
#include <plib/sl.h>
#include <plib/js.h>
#include <plib/fnt.h>
#include <plib/pu.h>

#include "guNet.h"
#include "constants.h"
#include "utils.h"

class GFX ;
class GUI ;
class SoundSystem ;
class Track ;
class Level ;

extern GFX         *gfx   ;
extern GUI         *gui   ;
extern SoundSystem *sound ;
extern Track       *track ;
extern Level        level ;
extern ulClock     *fclock ;

extern int      game_state ;

extern ssgRoot *scene           ;
extern char    *tuxkart_datadir ;

void tuxKartMainLoop () ;
void shutdown() ;
void initMaterials   () ;
ssgBranch *process_userdata ( char *data ) ;

#include "sound.h"
#include "Track.h"
#include "Herring.h"
#include "joystick.h"
#include "Driver.h"
#include "Explosion.h"
#include "Camera.h"
#include "gfx.h"
#include "gui.h"
#include "material.h"
#include "status.h"
#include "loader.h"
#include "level.h"
#include "isect.h"
#include "preprocessor.h"

#define NUM_KARTS        8
#define NUM_TRAFFIC      2
#define NUM_PROJECTILES  8
#define NUM_EXPLOSIONS   6

#define MAX_HOME_DIST  50.0f
#define MAX_HOME_DIST_SQD  (MAX_HOME_DIST * MAX_HOME_DIST)

#define DEFAULT_NUM_LAPS_IN_RACE 5

#ifndef TUXKART_DATADIR
#define TUXKART_DATADIR "/usr/local/share/games/tuxkart"
#endif

extern int num_karts ;
extern int num_laps_in_race ;
extern int finishing_position ;
extern int paused;

extern KartDriver *kart       [ NUM_KARTS       ] ;
extern Projectile *projectile [ NUM_PROJECTILES ] ;
extern Explosion   *explosion [ NUM_EXPLOSIONS  ] ;

extern int tuxkartMain ( int nl, int mirror, int reverse, 
                         char *track, int numPlayers ) ;


