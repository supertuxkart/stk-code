//  $Id: TrackData.cxx,v 1.11 2004/09/05 20:09:59 matzebraun Exp $
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

#include <iostream>
#include <stdexcept>
#include "lisp/Lisp.h"
#include "lisp/Parser.h"
#include "Loader.h"
#include "TrackData.h"
#include "StringUtils.h"

TrackData::TrackData(const std::string& filename_)
{
  mirrored = false;
  reversed = false;

  filename = filename_;

  std::string path = StringUtils::without_extension(filename);
  loc_filename = path + ".loc";
  drv_filename = path + ".drv";
  ident    = StringUtils::basename(path);

  // Default values
  use_fog = false;
  sgSetVec4 ( fog_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  fog_density = 1.0f/100.0f;
  fog_start   = 0.0f;
  fog_end     = 1000.0f;

  sgSetVec3 ( sun_position, 0.4, 0.4, 0.4 ) ;
  sgSetVec4 ( sky_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( fog_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( ambientcol , 0.5, 0.5, 0.5, 1.0 ) ;
  sgSetVec4 ( specularcol, 1.0, 1.0, 1.0, 1.0 ) ;
  sgSetVec4 ( diffusecol , 1.0, 1.0, 1.0, 1.0 ) ;

  const lisp::Lisp* root = 0;
  try {
    lisp::Parser parser;
    root = parser.parse(loader->getPath(filename));

    const lisp::Lisp* lisp = root->getLisp("tuxkart-track");
    if(!lisp) {
        throw std::runtime_error("no tuxkart-track node");
    }
        
    lisp->get("name",        name);
    lisp->get("music",       music_filename);
    lisp->get("sky-color",   sky_color);

    lisp->get("use-fog",     use_fog);
    lisp->get("fog-color",   fog_color);
    lisp->get("fog-density", fog_density);
    lisp->get("fog-start",   fog_start);
    lisp->get("fog-end",     fog_end);

    lisp->get("sun-position", sun_position);
    lisp->get("sun-ambient",  ambientcol);
    lisp->get("sun-specular", specularcol);
    lisp->get("sun-diffuse",  diffusecol);
  } catch(std::exception& err) {
    std::cout << "Error while reading '" << filename
              << ": " << err.what() << "\n";
  }
  delete root;

  load_drv();
}

void
TrackData::load_drv()
{
  std::string path = loader->getPath(drv_filename);
  FILE *fd = fopen ( path.c_str(), "ra" ) ;

  if ( fd == NULL )
    {
      fprintf ( stderr, "Can't open '%s' for reading.\n", drv_filename.c_str() ) ;
      exit ( 1 ) ;
    } 

  while(!feof(fd))
    {
      char s [ 1024 ] ;

      if ( fgets ( s, 1023, fd ) == NULL )
        break ;

      if ( *s == '#' || *s < ' ' )
        continue ;

      float x = 0.0f;
      float y = 0.0f;
      float z = 0.0f;

      if ( sscanf ( s, "%f,%f", &x, &y ) != 2 && sscanf ( s, "%f,%f,%f", &x, &y, &z ) != 3 )
        {
          fprintf ( stderr, "Syntax error in '%s'\n", drv_filename.c_str() ) ;
          exit ( 1 ) ;
        } 

      sgVec3 point;
      point[0] = x;
      point[1] = y;
      point[2] = z;

      driveline.push_back(point);
    }

  fclose ( fd ) ;
}

void
TrackData::reverse()
{
  reversed = !reversed;

  std::reverse(driveline.begin(), driveline.end());
}

void
TrackData::mirror()
{
  mirrored = !mirrored;

  for ( int i = 0 ; i < int(driveline.size()); ++i )
    driveline[i][0] = -driveline[i][0];
}

/* EOF */
