
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef WIN32
#ifdef __CYGWIN__
#include <unistd.h>
#endif
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <math.h>

#include <plib/pw.h>
#include <plib/ssg.h>
#include <plib/sl.h>
#include <plib/js.h>
#include <plib/fnt.h>

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
void initMaterials   () ;
ssgBranch *process_userdata ( char *data ) ;

extern int cam_follow ;

#include "sound.h"
#include "Track.h"
#include "Herring.h"
#include "joystick.h"
#include "Driver.h"
#include "Explosion.h"
#include "gfx.h"
#include "gui.h"
#include "material.h"
#include "status.h"
#include "loader.h"
#include "level.h"
#include "isect.h"

#define NUM_KARTS        8
#define NUM_TRAFFIC      2
#define NUM_PROJECTILES  8
#define NUM_EXPLOSIONS   6

#define MAX_HOME_DIST  50.0f
#define MAX_HOME_DIST_SQD  (MAX_HOME_DIST * MAX_HOME_DIST)

#define DEFAULT_NUM_LAPS_IN_RACE 5

extern int num_karts ;
extern int num_laps_in_race ;
extern KartDriver *kart       [ NUM_KARTS       ] ;
extern Projectile *projectile [ NUM_PROJECTILES ] ;
extern Explosion   *explosion [ NUM_EXPLOSIONS  ] ;

extern int stats_enabled ;

extern int finishing_position ;

