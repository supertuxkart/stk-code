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

#include <dirent.h>
#include <stdexcept>
#include <sstream>
#if !defined(WIN32) || defined(__CYGWIN__)
#  include <sys/stat.h>
#  include <sys/types.h>
#endif
#include "loader.hpp"

Loader* loader = 0;

Loader::Loader() {
  char *datadir;

  if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
    datadir = getenv ( "SUPERTUXKART_DATADIR" ) ;
  else
#ifdef _MSC_VER
    if ( _access ( "data/tuxtrack.track", 04 ) == 0 )
#else
    if ( access ( "data/tuxtrack.track", F_OK ) == 0 )
#endif
      datadir = "." ;
    else
#ifdef _MSC_VER
      if ( _access ( "../data/tuxtrack.track", 04 ) == 0 )
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
}

Loader::~Loader() {
}

void Loader::make_path(char* path, const char* dir, const char* fname) const {
  struct stat mystat;
    
  for(std::vector<std::string>::const_iterator i = searchPath.begin();
      i != searchPath.end(); ++i) {
    sprintf(path, "%s/%s/%s", i->c_str(), dir, fname);
    // convert backslashes to slashes
    size_t len = strlen(path);
    for(size_t i = 0; i < len; ++i)
      if(path[i] == '\\')
        path[i] = '/';
    
    if(stat(path, &mystat) < 0)
      continue;

    return;
  }

  // error case...
  // hmm ideally we'd throw an exception here, but plib is not prepared for that
  sprintf(path, "NotFound: %s", fname);
}

void Loader::makeModelPath(char* path, const char* fname) const {
  make_path(path, getModelDir(), fname);
}

void Loader::makeTexturePath(char* path, const char* fname) const {
  make_path(path, getTextureDir(), fname);
}

void Loader::addSearchPath(const std::string& path) {
  searchPath.push_back(path);
}

void Loader::addSearchPath(const char* path)
{
  searchPath.push_back(std::string(path));
}

void Loader::initConfigDir() {
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
}

std::string Loader::getPath(const char* fname) const {
  struct stat mystat;
  std::string result;
   
  for(std::vector<std::string>::const_iterator i = searchPath.begin();
      i != searchPath.end(); ++i) {
    result = *i;
    result += '/';
    result += fname;
    
    if(stat(result.c_str(), &mystat) < 0)
      continue;
    
    return result;
  }

  std::stringstream msg;
  msg << "Couldn't find file '" << fname << "'.";
  throw std::runtime_error(msg.str());
}

void Loader::listFiles(std::set<std::string>& result, const std::string& dir) 
     const {
  struct stat mystat;

#ifdef DEBUG
  // don't list directories with a slash on the end, it'll fail on win32
  assert(dir[dir.size()-1] != '/');
#endif

  result.clear();

  for(std::vector<std::string>::const_iterator i = searchPath.begin();
      i != searchPath.end(); ++i) {
    std::string path = *i;
    path += '/';
    path += dir;
    
    if(stat(path.c_str(), &mystat) < 0)
      continue;
    if(! S_ISDIR(mystat.st_mode))
      continue;
    
    DIR* mydir = opendir(path.c_str());
    if(!mydir)
      continue;

    struct dirent* mydirent;
    while( (mydirent = readdir(mydir)) != 0) {
      result.insert(mydirent->d_name);
    }
    closedir(mydir);
  }
}

