
#include "tuxkart.h"

 
static void to_uppercase ( char *s )
{
  while ( *s != '\0' )
  {
    if ( *s >= 'a' && *s <= 'z' )
      *s = *s - 'a' + 'A' ;
 
    s++ ;
  }
}
 
class Hook : public ssgBase
{
public:
  Hook ()
  {
    br    = NULL ;
    param = NULL ;
    hook  = NULL ;
    hit   = NULL ;
  }
 
  ssgBranch *br    ;
  void      *param ;
  void (*hook)(ssgBranch *, void *) ;
  void (*hit )(ssgBranch *, void *) ;
} ;
                                                                                

#define MAX_HOOKS 200
int next_hook = 0 ;
Hook *hooklist [ MAX_HOOKS ] ;
 


void init_hooks ()
{
  for ( int i = 0 ; i < MAX_HOOKS ; i++ )
  {
    hooklist [ i ] = new Hook ;
    hooklist [ i ] -> ref () ;
  }
}
                                                                                 
void hit_hook ( ssgEntity *ent, sgMat4 * /*last_mat*/, sgMat4 * /*curr_mat*/ )
{
  /*
*************************
Steve:
    Don't forget that last_mat and/or curr_mat
    are allowed to be NULL.
*************************
  */
 
 
  Hook *h = (Hook *) (ent -> getUserData ()) ;
 
  if ( ent -> isAKindOf ( ssgTypeBaseTransform() ) )
  {
    if ( ent -> isAKindOf ( ssgTypeTexTrans() ) )
    {
    }
    else
    {
    }
  }
 
  if ( h == NULL || h -> hit == NULL )
    return ;
 
  h -> hit ((ssgBranch *) ent, h -> param ) ;
}                                                                               


 
void update_hooks ()
{
  for ( int i = 0 ; i < next_hook ; i++ )
    if ( hooklist [ i ] -> hook != NULL )
      hooklist [ i ] -> hook ( hooklist [ i ] -> br,
                               hooklist [ i ] -> param ) ;
}                                                                               

                                                                                 
class AutoDCSParam
{
public:
  AutoDCSParam ()
  {
    sgSetCoord ( &init , 0, 0, 0, 0, 0, 0 ) ;
    sgSetCoord ( &delta, 0, 0, 0, 0, 0, 0 ) ;
    mode  = MODE_FORWARD ;
    phase =  0.0f ;
    cycle = 30.0f ;
    isDCS = TRUE  ;
  }
 
  sgCoord init  ;
  sgCoord delta ;
  int   mode  ;
  int   isDCS ;
  float phase ;
  float cycle ;
} ;
 
void autodcsHook ( ssgBranch *br, void *param )
{
  AutoDCSParam *p = (AutoDCSParam *) param ;
 
  sgCoord now, add ;
 
  sgCopyCoord ( & now, & ( p -> init  ) ) ;
  sgCopyCoord ( & add, & ( p -> delta ) ) ;
 
  float timer = fclock -> getAbsTime () + p -> phase ;
 
  if ( p->cycle != 0.0 && p->mode != MODE_FORWARD )
  {
    if ( p->mode == MODE_SHUTTLE )
    {
      float ctimer = fmod ( timer, p->cycle ) ;
 
      if ( ctimer > p->cycle / 2.0f )
        timer = p->cycle - ctimer ;
      else
        timer = ctimer ;
    }
    else
    if ( p->mode == MODE_SINESHUTTLE )
      timer = sin ( timer * 2.0f * M_PI / p->cycle ) * p->cycle / 2.0f ;
    else
      timer = fmod ( timer, p->cycle ) ;
  }
 
  sgScaleVec3 ( add . xyz, timer ) ;
  sgScaleVec3 ( add . hpr, timer ) ;
 
  sgAddVec3 ( now . xyz, add . xyz ) ;
  sgAddVec3 ( now . hpr, add . hpr ) ;
 
  /*
    To avoid roundoff problems with very large values
    accumulated after long runs all rotations
    can be modulo-360 degrees.                                                  
  */
 
  now . hpr [ 0 ] = fmod ( now . hpr [ 0 ], 360.0 ) ;
  now . hpr [ 1 ] = fmod ( now . hpr [ 1 ], 360.0 ) ;
  now . hpr [ 2 ] = fmod ( now . hpr [ 2 ], 360.0 ) ;
 
  if ( br->isAKindOf(ssgTypeTexTrans()) )
  {
    now . xyz [ 0 ] = fmod ( now . xyz [ 0 ], 1.0 ) ;
    now . xyz [ 1 ] = fmod ( now . xyz [ 1 ], 1.0 ) ;
    now . xyz [ 2 ] = fmod ( now . xyz [ 2 ], 1.0 ) ;
  }

  ((ssgBaseTransform *) br) -> setTransform ( & now ) ;
}
 
 
static void *autoTexOrDCSInit ( char *data, int isDCS )
{
  AutoDCSParam *param = new AutoDCSParam ;
 
  param -> isDCS = isDCS ;
 
  char *s = data ;
 
  to_uppercase ( s ) ;
 
  while ( s != NULL && *s != '\0' )
  {
    while ( *s > ' ' ) s++ ;     /* Skip previous token */
    while ( *s <= ' ' && *s != '\0' ) s++ ; /* Skip spaces */
 
    if ( *s == '\0' ) break ;
 
    float f ;
 
    if ( sscanf ( s,  "H=%f", & f ) == 1 ) param->delta . hpr [ 0 ] = f ; else
    if ( sscanf ( s,  "P=%f", & f ) == 1 ) param->delta . hpr [ 1 ] = f ; else
    if ( sscanf ( s,  "R=%f", & f ) == 1 ) param->delta . hpr [ 2 ] = f ; else
    if ( sscanf ( s,  "X=%f", & f ) == 1 ) param->delta . xyz [ 0 ] = f ; else
    if ( sscanf ( s,  "Y=%f", & f ) == 1 ) param->delta . xyz [ 1 ] = f ; else
    if ( sscanf ( s,  "Z=%f", & f ) == 1 ) param->delta . xyz [ 2 ] = f ; else
    if ( sscanf ( s,  "C=%f", & f ) == 1 ) param->cycle = f ; else
    if ( sscanf ( s,  "M=%f", & f ) == 1 ) param->mode  = (int) f ; else
    if ( sscanf ( s,  "O=%f", & f ) == 1 ) param->phase = f ; else
      fprintf ( stderr, "Unrecognised @autodcs string: '%s'\n",
                   data ) ;
  }
 
  sgSetCoord ( & ( param -> init ), 0, 0, 0, 0, 0, 0 ) ;
  return param ;
}
                                                                                

void *autodcsInit ( ssgBranch **br, char *data )
{
  *br = new ssgTransform () ;
 
  return autoTexOrDCSInit ( data, TRUE ) ;
}
 
 
void *autotexInit ( ssgBranch **br, char *data )
{
  *br = new ssgTexTrans () ;
 
  return autoTexOrDCSInit ( data, FALSE ) ;
}
 

void *billboardInit ( ssgBranch **br, char * )
{
  *br = new ssgCutout () ;
  return NULL ;
}
                                                                                

void *invisibleInit ( ssgBranch **br, char * )
{
  *br = new ssgInvisible () ;
 
  return NULL ;
}
                                                                                

void *switchInit ( ssgBranch **br, char *data )
{
  *br = new ssgSelector ;

  ((ssgSelector *)(*br)) -> select ( 0 ) ;

  return NULL ;
}


void *animInit ( ssgBranch **br, char *data )
{
  while ( ! isdigit ( *data ) && *data != '\0' )
    data++ ;

  int   startlim = strtol ( data, &data, 0 ) ;
  int   endlim   = strtol ( data, &data, 0 ) ;
  float timelim  = strtod ( data, &data ) ;

  while ( *data <= ' ' && *data != '\0' )
    data++ ;

  char mode = toupper ( *data ) ;

  *br = new ssgTimedSelector ;

  ((ssgTimedSelector *)(*br)) -> setLimits   ( startlim+1, endlim+1 ) ;
  ((ssgTimedSelector *)(*br)) -> setDuration ( timelim ) ;
  ((ssgTimedSelector *)(*br)) -> setMode     ( (mode=='O') ?  SSG_ANIM_ONESHOT :
                                               (mode=='S') ?  SSG_ANIM_SWING :
                                                              SSG_ANIM_SHUTTLE ) ;
  ((ssgTimedSelector *)(*br)) -> control     ( SSG_ANIM_START ) ;

  return NULL ;
}


void add_hook ( void  (*hook)(ssgBranch  *, void *),
                void  (*hit )(ssgBranch  *, void *),
                ssgBranch *b,
                void      *param )
{

  hooklist [ next_hook ] -> hook  = hook ;
  hooklist [ next_hook ] -> hit   = hit  ;
  hooklist [ next_hook ] -> br    = b    ;
  hooklist [ next_hook ] -> param = param ;

  b -> setUserData ( hooklist [ next_hook ] ) ;

  next_hook++ ;

  if ( next_hook >= MAX_HOOKS )
  {
    fprintf ( stderr, "More than %d hooknodes in database!\n", MAX_HOOKS ) ;
    exit ( 1 ) ;
  }
}

ssgBranch *process_userdata ( char *data )
{
  ssgBranch *b = NULL ;

  if ( data == NULL || data [ 0 ] != '@' )
    return NULL ;

  data++ ;   /* Skip the '@' */

  if ( strncmp ( "billboard", data, strlen ( "billboard" ) ) == 0 )
  {
    billboardInit ( &b, data ) ;
    return b ;
  }

  if ( strncmp ( "invisible", data, strlen ( "invisible" ) ) == 0 )
  {
    invisibleInit ( &b, data ) ;
    return b ;
  }

  if ( strncmp ( "switch", data, strlen ( "switch" ) ) == 0 )
  {
    void *p = switchInit ( &b, data ) ;
    add_hook ( NULL, NULL, b, p ) ;
    return b ;
  }

  if ( strncmp ( "animate", data, strlen ( "animate" ) ) == 0 )
  {
    void *p = animInit ( &b, data ) ;
    add_hook ( NULL, NULL, b, p ) ;
    return b ;
  }

  if ( strncmp ( "autodcs", data, strlen ( "autodcs" ) ) == 0 )
  {
    void *p = autodcsInit ( &b, data ) ;
    add_hook ( autodcsHook, NULL, b, p ) ;
    return b ;
  }

  if ( strncmp ( "autotex", data, strlen ( "autotex" ) ) == 0 )
  {
    void *p = autotexInit ( &b, data ) ;
    add_hook ( autodcsHook, NULL, b, p ) ;
    return b ;
  }
  
  return NULL ;
}


