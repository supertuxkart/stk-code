//  $Id: hook_manager.cpp,v 1.3 2005/08/19 20:51:56 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
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

#include "hook_manager.hpp"
#include "world.hpp"

HookManager* hook_manager=0;

// -----------------------------------------------------------------------------
void HookManager::update() {
  for(HookList::iterator i=hookList.begin();
                         i!=hookList.end(); i++) {
    if((*i)->hook) {
      (*i)->hook( (*i)->branch, (*i)->param);
    }
  }   // for i
}

// -----------------------------------------------------------------------------
void HookManager::clearAll() {
  for(HookList::iterator i=hookList.begin();
                         i!=hookList.end(); i++) {
    free (*i);
  }   // for i
  // JH FIXME: This might introduce a memory leak, since the ssgBranches
  // created duringautodcsInit might not be freed (but they are put into
  // the ssg as userdata, so plib might free it????
  hookList.clear();
}  // clearAll
// -----------------------------------------------------------------------------
void HookManager::addHook(void  (*hook)(ssgBranch  *, void *),
			  void  (*hit )(ssgBranch  *, void *),
			  ssgBranch *b,
			  void      *param ) {

  Hook *h = new Hook(hook, hit, b, param);
  // For now it appears not to make any sense to add the data to
  // the hooklist, since the update function will test those elements,
  // but not do any calls. I am not even sure if we actually should
  // set the user data at all. JH
  if(hook || hit) {
    hookList.push_back(h);
  }
  b->setUserData(h);
}

// -----------------------------------------------------------------------------
static void to_uppercase ( char *s ) {
  while ( *s != '\0' ) {
    if ( *s >= 'a' && *s <= 'z' ) *s = *s - 'a' + 'A' ;
    s++ ;
  }
}
 
                                                                                 
// =============================================================================
class AutoDCSParam {
public:
  sgCoord init  ;
  sgCoord delta ;
  int   mode  ;
  int   isDCS ;
  float phase ;
  float cycle ;

  AutoDCSParam () {
    sgSetCoord ( &init , 0, 0, 0, 0, 0, 0 ) ;
    sgSetCoord ( &delta, 0, 0, 0, 0, 0, 0 ) ;
    mode  = MODE_FORWARD ;
    phase =  0.0f ;
    cycle = 30.0f ;
    isDCS = TRUE  ;
  }
 
};   // AutoDCSParam
 
// =============================================================================
void autodcsHook ( ssgBranch *br, void *param ) {
  AutoDCSParam *p = (AutoDCSParam *) param ;
 
  sgCoord now, add ;
 
  sgCopyCoord ( & now, & ( p -> init  ) ) ;
  sgCopyCoord ( & add, & ( p -> delta ) ) ;
 
  float timer = world->clock + p -> phase ;
 
  if ( p->cycle != 0.0 && p->mode != MODE_FORWARD ) {
    if ( p->mode == MODE_SHUTTLE ) {
      float ctimer = fmod ( timer, p->cycle ) ;
 
      if ( ctimer > p->cycle / 2.0f )
        timer = p->cycle - ctimer ;
      else
        timer = ctimer ;
    } else {
      if ( p->mode == MODE_SINESHUTTLE )
	timer = sin ( timer * 2.0f * M_PI / p->cycle ) * p->cycle / 2.0f ;
      else
	timer = fmod ( timer, p->cycle ) ;
    }
  }   // mode!=MODE_FORWARD
 
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
 
  if ( br->isAKindOf(ssgTypeTexTrans()) ) {
    now . xyz [ 0 ] = fmod ( now . xyz [ 0 ], 1.0 ) ;
    now . xyz [ 1 ] = fmod ( now . xyz [ 1 ], 1.0 ) ;
    now . xyz [ 2 ] = fmod ( now . xyz [ 2 ], 1.0 ) ;
  }
  ((ssgBaseTransform *) br) -> setTransform ( & now ) ;
}   //autdcsHook
 
 
// -----------------------------------------------------------------------------
static void *autoTexOrDCSInit ( char *data, int isDCS ) {
  AutoDCSParam *param = new AutoDCSParam ;
 
  param -> isDCS = isDCS ;
 
  char *s = data ;
 
  to_uppercase ( s ) ;
 
  while ( s != NULL && *s != '\0' ) {
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
                                                                                

// -----------------------------------------------------------------------------
void *autodcsInit ( ssgBranch **br, char *data ) {
  *br = new ssgTransform () ;
 
  return autoTexOrDCSInit ( data, TRUE ) ;
}   // autodcsInit
 
 
// -----------------------------------------------------------------------------
void *autotexInit ( ssgBranch **br, char *data ) {
  *br = new ssgTexTrans () ;
 
  return autoTexOrDCSInit ( data, FALSE ) ;
}   //autotexInit
 

// -----------------------------------------------------------------------------
void *billboardInit ( ssgBranch **br, char * ) {
  *br = new ssgCutout () ;
  return NULL ;
}   // billboardInit
                                                                                

// -----------------------------------------------------------------------------
void *invisibleInit ( ssgBranch **br, char * ) {
  *br = new ssgInvisible () ;
 
  return NULL ;
}   // invisibleInit
                                                                               

// -----------------------------------------------------------------------------
void *switchInit ( ssgBranch **br, char* ) {
  *br = new ssgSelector ;

  ((ssgSelector *)(*br)) -> select ( 0 ) ;

  return NULL ;
}   // switchInit


// -----------------------------------------------------------------------------
void *animInit ( ssgBranch **br, char *data ) {
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
}   // animInit


// -----------------------------------------------------------------------------
ssgBranch *process_userdata ( char *data ) {
  ssgBranch *b = NULL ;

  if ( data == NULL || data [ 0 ] != '@' )
    return NULL ;

  data++ ;   /* Skip the '@' */

  if ( strncmp ( "billboard", data, strlen ( "billboard" ) ) == 0 ) {
    billboardInit ( &b, data ) ;
    return b ;
  }

  if ( strncmp ( "invisible", data, strlen ( "invisible" ) ) == 0 ) {
    invisibleInit ( &b, data ) ;
    return b ;
  }

  if ( strncmp ( "switch", data, strlen ( "switch" ) ) == 0 ) {
    void *p = switchInit ( &b, data ) ;
    hook_manager->addHook ( NULL, NULL, b, p ) ;
    return b ;
  }

  if ( strncmp ( "animate", data, strlen ( "animate" ) ) == 0 ) {
    void *p = animInit ( &b, data ) ;
    hook_manager->addHook ( NULL, NULL, b, p ) ;
    return b ;
  }

  if ( strncmp ( "autodcs", data, strlen ( "autodcs" ) ) == 0 ) {
    void *p = autodcsInit ( &b, data ) ;
    hook_manager->addHook ( autodcsHook, NULL, b, p ) ;
    return b ;
  }

  if ( strncmp ( "autotex", data, strlen ( "autotex" ) ) == 0 ) {
    void *p = autotexInit ( &b, data ) ;
    hook_manager->addHook ( autodcsHook, NULL, b, p ) ;
    return b ;
  }
  
  return NULL ;
}   // process_userdata

/* EOF */

