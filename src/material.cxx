//  $Id$
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
#include "material.h"

#define UCLAMP   1
#define VCLAMP   2

static ulList *materials = NULL ;


int clearSpheremap ( ssgEntity * )
{
  glDisable   ( GL_TEXTURE_GEN_S ) ;
  glDisable   ( GL_TEXTURE_GEN_T ) ;
  return TRUE ;
}


int setSpheremap ( ssgEntity * )
{
  glTexGeni   ( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
  glTexGeni   ( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
  glEnable    ( GL_TEXTURE_GEN_S ) ;
  glEnable    ( GL_TEXTURE_GEN_T ) ;
  return TRUE ;
}


bool Material::parseBool ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  bool res = ( ( **p == 'Y' ) || ( **p == 'y' ) ) ;

  while ( **p > ' ' && **p != '\0' ) (*p)++ ;

  return res ;
}


int Material::parseInt ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  return strtol ( *p, p, 0 ) ;
}



float Material::parseFloat ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  return strtod ( *p, p ) ;
}



Material::Material ()
{
  texname = new char [ 1 ] ;
  texname [ 0 ] = '\0' ;
  predraw  = postdraw = NULL ;

  init    () ;
  install () ;
}


Material::Material ( char *fname, char *description )
{
  texname = new char [ strlen ( fname ) + 1 ] ;
  strcpy ( texname, fname ) ;
  predraw  = postdraw = NULL ;

  init () ;

  clamp_tex    = parseBool  ( & description ) ? UCLAMP : 0 ;
  clamp_tex   += parseBool  ( & description ) ? VCLAMP : 0 ;

  transparency = parseBool  ( & description ) ;
  alpha_ref    = parseFloat ( & description ) ;
  lighting     = parseBool  ( & description ) ;
  spheremap    = parseBool  ( & description ) ;
  friction     = parseFloat ( & description ) ;
  ignore       = parseBool  ( & description ) ;
  zipper       = parseBool  ( & description ) ;
  resetter     = parseBool  ( & description ) ;
  collideable  = parseBool  ( & description ) ;

  install () ;
}

Material::~Material()
{
  ssgDeRefDelete(state);
  delete[] texname;
}

void Material::init ()
{
  materials -> addEntity ( this ) ;
  index = materials -> searchForEntity ( this ) ;

  clamp_tex    = 0     ;
  transparency = false ;
  alpha_ref    = 0.1   ;
  lighting     = true  ;
  spheremap    = false ;
  friction     = 1.0   ;
  ignore       = false ;
  zipper       = false ;
  resetter     = false ;
  collideable  = true  ;
}


void Material::applyToLeaf ( ssgLeaf *l )
{
  if ( predraw  ) l -> setCallback ( SSG_CALLBACK_PREDRAW , predraw  ) ;
  if ( postdraw ) l -> setCallback ( SSG_CALLBACK_POSTDRAW, postdraw ) ;
}


void Material::install ()
{
  if ( isSphereMap () )
  {
    predraw  =   setSpheremap ;
    postdraw = clearSpheremap ;
  }

  ssgSimpleState *s = new ssgSimpleState ;

  state = s ;

  s -> ref () ;
  s -> setExternalPropertyIndex ( index ) ;

  if ( texname != NULL && texname [ 0 ] != '\0' )
  {
    char fn [ 1024 ] ;
    sprintf ( fn, "images/%s", texname ) ;

    s -> setTexture ( fn, !(clamp_tex & UCLAMP),
                          !(clamp_tex & VCLAMP) ) ;
    s -> enable  ( GL_TEXTURE_2D ) ;
  }
  else
    s -> disable ( GL_TEXTURE_2D ) ;

  if ( lighting )
    s -> enable  ( GL_LIGHTING ) ;
  else
    s -> disable ( GL_LIGHTING ) ;

  s -> setShadeModel ( GL_SMOOTH ) ;
  s -> enable        ( GL_COLOR_MATERIAL ) ;
  s -> enable        ( GL_CULL_FACE      ) ;
  s -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  s -> setMaterial   ( GL_EMISSION, 0, 0, 0, 1 ) ;
  s -> setMaterial   ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  s -> setShininess  ( 0 ) ;

  if ( transparency )
  {
    s -> setTranslucent () ;
    s -> enable         ( GL_ALPHA_TEST ) ;
    s -> setAlphaClamp  ( alpha_ref ) ;
    s -> enable         ( GL_BLEND ) ;
  }
  else
  {
    s -> setOpaque () ;
    s -> disable   ( GL_BLEND ) ;
  }
}


char *parseFileName ( char **str )
{
  char *p = *str ;

  /* Skip leading spaces */

  while ( *p <= ' ' && *p != '\0' ) p++ ;

  /* Skip blank lines and comments */

  if ( *p == '#' || *p == '\0' )
    return NULL ;

  if ( *p != '"' )
  {
    fprintf ( stderr, "ERROR: Material file entries must start with '\"'\n");
    fprintf ( stderr, "ERROR: Offending line is '%s'\n", *str ) ;
    return NULL ;
  }

  /* Filename? */

  char *f = ++p ;

  while ( *p != '"' && *p != '\0' ) p++ ;

  if ( *p != '"' )
  {
    fprintf ( stderr,
      "ERROR: Unterminated string constant '%s' in materials file.\n", *str ) ;
    return NULL ;
  }

  *p = '\0' ;
  *str = ++p ;

  return f ;
}


int parseMaterial ( FILE *fd )
{
  char str [ 1024 ] ;

  while ( ! feof ( fd ) )
  {
    char *s = str ;

    if ( fgets ( s, 1024, fd ) == NULL )
      return false ;

    s [ strlen(s) - 1 ] = '\0' ;

    char *f = parseFileName ( & s ) ;

    if ( f != NULL )
    {
      new Material ( f, s ) ;
      return true ;
    }
  }
 
  return false ;
}


Material *getMaterial ( ssgLeaf *l )
{
  return (Material *) materials -> getEntity (
                                   l -> getExternalPropertyIndex () ) ;
}


Material *getMaterial ( const char *fname )
{
  if ( fname == NULL || fname[0] == '\0' )
    return (Material *) materials -> getEntity ( 0 ) ;

  const char *fn ;

  /* Remove all leading path information. */

  for ( fn = & fname [ strlen ( fname ) - 1 ] ; fn != fname &&
                                               *fn != '/' ; fn-- )
    /* Search back for a '/' */ ;

  if ( *fn == '/' )
    fn++ ;

  char basename [ 1024 ] ;

  strcpy ( basename, fn ) ;

  /* Remove last trailing extension. */

  char* fno;
  for ( fno = & basename [ strlen ( basename ) - 1 ] ; fno != basename &&
                                                     *fno != '.' ; fno-- )
    /* Search back for a '.' */ ;

  if ( *fno == '.' )
    *fno = '\0' ;

  for ( int i = 0 ; i < materials -> getNumEntities () ; i++ )
  {
    char *fname2 = ((Material *)(materials -> getEntity(i)))-> getTexFname () ;

    if ( fname2 != NULL && fname2[0] != '\0' )
    {
      char *fn2 ;

      /* Remove all leading path information. */

      for ( fn2 = & fname2 [ strlen ( fname2 ) -1 ] ; fn2 != fname2 &&
                                                     *fn2 != '/' ; fn2-- )
        /* Search back for a '/' */ ;

      if ( *fn2 == '/' )
        fn2++ ;

      char basename2 [ 1024 ] ;

      strcpy ( basename2, fn2 ) ;

      /* Remove last trailing extension. */

      for ( fn2 = & basename2 [ strlen ( basename2 ) - 1 ] ; fn2 != basename2 &&
                                                         *fn2 != '.' ; fn2-- )
        /* Search back for a '.' */ ;

      if ( *fn2 == '.' )
        *fn2 = '\0' ;

      if ( strcmp ( basename, basename2 ) == 0 )
        return (Material *) materials -> getEntity ( i ) ;
    }
  }

#if 0
  // matze: what is this code good for?
  strcpy ( fname, basename  ) ;
  strcat ( fname, ".png"    ) ;
#endif
  return NULL ;
}


ssgState *getAppState ( char *fname )
{
  Material *m = getMaterial ( fname ) ;

  return ( m == NULL ) ? NULL : m -> getState () ;
}


void initMaterials ()
{
  /* Create list - and default material zero */

  materials = new ulList ( 100 ) ;
  materials -> addEntity ( new Material () ) ;

  char fname [ 1000 ] ;

  sprintf ( fname, "data/materials.dat" ) ;

  std::string path = loader->getPath(fname);
  FILE *fd = fopen ( path.c_str(), "ra" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "FATAL: No Such File as '%s'\n", fname ) ;
    exit ( 1 ) ;
  }

  while ( parseMaterial ( fd ) ) 
    /* Read file */ ;

  fclose ( fd ) ;

  ssgSetAppStateCallback ( getAppState ) ;
}

