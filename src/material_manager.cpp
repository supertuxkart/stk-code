//  $Id: MaterialManager.cxx,v 1.2 2005/07/23 23:05:39 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
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

#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"

ssgState *fuzzy_gst;

MaterialManager *material_manager=0;

MaterialManager::MaterialManager(){
  /* Create list - and default material zero */

  materials = new ulList ( 100 ) ;
  // We can't call init here, since the global variable
  // material_manager has not yet been initialised, and
  // material_manager is used in the Material constructor.
  // Therefore, the code for loading the material had to
  // be moved into a separate function.
  //  Init();
}

int MaterialManager::addEntity(Material *m) {
  materials->addEntity(m);
  return materials->searchForEntity(m);
}

void MaterialManager::loadMaterial() {
  // Create the default/empty material. The Material 
  // constructor will add the material to (this) material_manager
  new Material ();

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
  fuzzy_gst        = getMaterial("fuzzy.rgb")->getState();
}   // MaterialManager

char* MaterialManager::parseFileName(char **str) {
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


int MaterialManager::parseMaterial ( FILE *fd )
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


Material *MaterialManager::getMaterial ( ssgLeaf *l )
{
  return getEntity ( l -> getExternalPropertyIndex () ) ;
}


Material *MaterialManager::getMaterial ( const char* fname )
{
  if ( fname == NULL || fname[0] == '\0' )
    return getEntity ( 0 ) ;

  //This copy is made so the original fname is not modified
  char *fname_copy = strdup(fname);
  const char *fn;
  /* Remove all leading path information. */

  for ( fn = & fname_copy [ strlen ( fname_copy ) - 1 ] ;
        fn != fname_copy && *fn != '/' ; fn-- )
    /* Search back for a '/' */ ;

    if ( *fn == '/' )
      fn++ ;

  char basename [ 1024 ] ;

  strcpy ( basename, fn ) ;
  free(fname_copy);

  /* Remove last trailing extension. */

  char* fno;
  for ( fno = & basename [ strlen ( basename ) - 1 ] ; fno != basename &&
                                                     *fno != '.' ; fno-- )
    /* Search back for a '.' */ ;

  if ( *fno == '.' )
    *fno = '\0' ;

  for ( int i = 0 ; i < getNumEntities () ; i++ )
  {
    char *fname2 = getEntity(i)-> getTexFname () ;

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
        return getEntity ( i ) ;
    }
  }

  return NULL ;
}


ssgState *getAppState ( char *fname )
{
  Material *m = material_manager->getMaterial ( fname ) ;
  return ( m == NULL ) ? NULL : m -> getState () ;
}


