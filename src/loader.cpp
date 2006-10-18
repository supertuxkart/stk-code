//  $Id$
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


#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#ifdef WIN32
#  include <io.h>
#  include <stdio.h>
#  ifndef __CYGWIN__
#    define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#  endif
#endif
#include "plib/ul.h"
#include "loader.hpp"

Loader* loader = 0;

// -----------------------------------------------------------------------------
Loader::Loader() 
{
  char *datadir;
  currentCallbackType = CB_COLLECTABLE;

  if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
    datadir = getenv ( "SUPERTUXKART_DATADIR" ) ;
  else
#ifdef _MSC_VER
    if ( _access ( "data\\tuxtrack.track", 04 ) == 0 )
#else
    if ( access ( "data/tuxtrack.track", F_OK ) == 0 )
#endif
      datadir = "." ;
    else
#ifdef _MSC_VER
      if ( _access ( "..\\data\\tuxtrack.track", 04 ) == 0 )
#else
      if ( access ( "../data/tuxtrack.track", F_OK ) == 0 )
#endif
	datadir = ".." ;
      else
#ifdef SUPERTUXKART_DATADIR
	datadir = SUPERTUXKART_DATADIR ;
#else
      datadir = "/usr/local/share/games/supertuxkart" ;
#endif
  fprintf ( stderr, "Data files will be fetched from: '%s'\n", datadir ) ;
  addSearchPath(datadir);
}  // Loader

// -----------------------------------------------------------------------------
Loader::~Loader() 
{
}   // ~Loader

// -----------------------------------------------------------------------------
void Loader::make_path(char* path, const char* dir, const char* fname) const 
{
  struct stat mystat;
    
  for(std::vector<std::string>::const_iterator i = searchPath.begin();
      i != searchPath.end(); ++i) 
  {
    sprintf(path, "%s%c%s%c%s", i->c_str(), DIR_SEPARATOR, dir, 
		    DIR_SEPARATOR, fname);
    // convert backslashes and slashes to the native form
    size_t len = strlen(path);
    for(size_t i = 0; i < len; ++i)
      if(path[i] == '\\' || path[i] == '/')
        path[i] = DIR_SEPARATOR;
    
    if(stat(path, &mystat) < 0)
      continue;

    return;
  }

  // error case...
  // hmm ideally we'd throw an exception here, but plib is not prepared for that
  sprintf(path, "NotFound: %s", fname);
}   // make_path

// -----------------------------------------------------------------------------
void Loader::makeModelPath(char* path, const char* fname) const 
{
  make_path(path, getModelDir(), fname);
}   // makeModelPath

// -----------------------------------------------------------------------------
void Loader::makeTexturePath(char* path, const char* fname) const 
{
  make_path(path, getTextureDir(), fname);
}   // makeTexturePath

// -----------------------------------------------------------------------------
void Loader::addSearchPath(const std::string& path) 
{
  searchPath.push_back(path);
}   // addSearchPath

// -----------------------------------------------------------------------------
void Loader::addSearchPath(const char* path)
{
  searchPath.push_back(std::string(path));
}   // addSearchPath

// -----------------------------------------------------------------------------
void Loader::initConfigDir() 
{
#ifdef WIN32
  /*nothing*/
#else
  /*if HOME environment variable exists
    create directory $HOME/.supertuxkart*/
  if(getenv("HOME")!=NULL) 
  {
    std::string pathname;
    pathname = getenv("HOME");
    pathname += "/.supertuxkart";
    mkdir(pathname.c_str(), 0755);
  }
#endif
}   // initConfigDir

// -----------------------------------------------------------------------------
std::string Loader::getPath(const char* fname) const 
{
  struct stat mystat;
  std::string result;

  std::string native_fname=fname;
  size_t len = strlen(fname);
  for(size_t i = 0; i < len; ++i)
      if(native_fname[i] == '\\' || native_fname[i] == '/')
        native_fname[i] = DIR_SEPARATOR;

  for(std::vector<std::string>::const_iterator i = searchPath.begin();
      i != searchPath.end(); ++i) {
    result = *i;
    result += DIR_SEPARATOR;
    result += native_fname;
    
    if(stat(result.c_str(), &mystat) < 0)
      continue;
    
    return result;
  }

  std::stringstream msg;
  msg << "Couldn't find file '" << fname << "'.";
  throw std::runtime_error(msg.str());
}   // getPath

// -----------------------------------------------------------------------------
void Loader::listFiles(std::set<std::string>& result, const std::string& dir) 
     const 
{
  struct stat mystat;

#ifdef DEBUG
  // don't list directories with a slash on the end, it'll fail on win32
  assert(dir[dir.size()-1] != '/');
#endif

  result.clear();

  for(std::vector<std::string>::const_iterator i = searchPath.begin();
      i != searchPath.end(); ++i) {
    std::string path = *i;
    path += DIR_SEPARATOR;
    path += dir;
    
    if(stat(path.c_str(), &mystat) < 0)
      continue;
    if(! S_ISDIR(mystat.st_mode))
      continue;
    

    ulDir* mydir = ulOpenDir(path.c_str());
    if(!mydir) continue;

    ulDirEnt* mydirent;
    while( (mydirent = ulReadDir(mydir)) != 0) {
      result.insert(mydirent->d_name);
    }
    ulCloseDir(mydir);
  }
}   // listFiles

// -----------------------------------------------------------------------------
ssgEntity *Loader::load(const std::string& filename, CallbackType t) 
{
  currentCallbackType=t;
  return ssgLoad(filename.c_str(), this);
}   // load

// -----------------------------------------------------------------------------
ssgBranch *Loader::animInit (char *data ) const
{
  while ( ! isdigit ( *data ) && *data != '\0' )
    data++ ;

  int   startlim =        strtol(data, &data, 0 );
  int   endlim   =        strtol(data, &data, 0 );
  float timelim  = (float)strtod(data, &data    );

  while ( *data <= ' ' && *data != '\0' )
    data++ ;

  char mode = toupper ( *data ) ;

  ssgTimedSelector *br = new ssgTimedSelector;

  br->setLimits  (startlim+1, endlim+1 ) ;
  br->setDuration(timelim ) ;
  br->setMode    ((mode=='O') ?  SSG_ANIM_ONESHOT 
		              :  (mode=='S') ?  SSG_ANIM_SWING 
		                             : SSG_ANIM_SHUTTLE ) ;
  br->control    (SSG_ANIM_START ) ;

  return br;
}   // animInit


// -----------------------------------------------------------------------------
// Handle userdata that is stored in the model files. Mostly the userdata
// indicates that a special branch is to be created (e.g. a ssgCutout instead
// of the standard branch). But some userdata indicate that callbacks need
// to be created, which are then handled by the callback manager.

ssgBranch *Loader::createBranch(char *data) const
{

  if ( data == NULL || data[0] != '@' ) return NULL;

  data++ ;   /* Skip the '@' */

  if ( strncmp("billboard", data, strlen("billboard") ) == 0 ) 
    return  new ssgCutout();


  if ( strncmp("invisible", data, strlen("invisible") ) == 0 )
    return new ssgInvisible();

  if ( strncmp ( "switch", data, strlen ( "switch" ) ) == 0 ) 
  {
    ssgSelector *sel = new ssgSelector();
    sel->select(0);
    return sel;
  }
      
  if ( strncmp ( "animate", data, strlen ( "animate" ) ) == 0 )
    return animInit(data);
  

  if ( strncmp ( "autodcs", data, strlen ( "autodcs" ) ) == 0 ) {
    ssgTransform *br = new ssgTransform();
    Callback     *c  = new Callback(data, br);
    br->setUserData(new ssgBase());
    callback_manager->addCallback(c, currentCallbackType);
    return br;
  }

  if ( strncmp ( "autotex", data, strlen ( "autotex" ) ) == 0 ) {
    ssgTexTrans *br = new ssgTexTrans();
    Callback    *c  = new Callback(data, br);
    callback_manager->addCallback(c, currentCallbackType);
    return br;
  }
  
  return NULL ;
}   // createBranch

