//  $Id: TrackData.cxx,v 1.3 2004/08/11 12:33:17 grumbel Exp $
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
#include "lispreader.h"
#include "Loader.h"
#include "TrackData.h"

TrackData::TrackData(const std::string& filename_)
{
  filename = filename_;
  ident    = filename.substr(0, filename.length() - 6);
  loc_filename = ident + ".loc";
  drv_filename = ident + ".drv";
  
  // Default values
  use_fog = false;
  sgSetVec3 ( sun_position, 0.4, 0.4, 0.4 ) ;
  sgSetVec4 ( sky_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( fog_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( ambientcol , 0.5, 0.5, 0.5, 1.0 ) ;
  sgSetVec4 ( specularcol, 1.0, 1.0, 1.0, 1.0 ) ;
  sgSetVec4 ( diffusecol , 1.0, 1.0, 1.0, 1.0 ) ;

  try {
    LispReader* track = LispReader::load(loader ? loader->getPath(filename) : filename, "tuxkart-track");
    assert(track);
    
    LispReader reader(track->get_lisp());
    
    reader.read_string("name", name);
    reader.read_sgVec4("sky-color", sky_color);

    reader.read_bool ("use-fog", use_fog);
    reader.read_sgVec4("fog-color", fog_color);
    reader.read_float("fog-densitiy", fog_density);
    reader.read_float("fog-start", fog_start);
    reader.read_float("fog-end", fog_end);

    reader.read_sgVec3("sun-position", sun_position);
    reader.read_sgVec4("sun-ambient",  ambientcol);
    reader.read_sgVec4("sun-specular", specularcol);
    reader.read_sgVec4("sun-diffuse",  diffusecol);

    delete track;
  } catch(LispReaderException& err) {
    std::cout << "LispReaderException: " << err.message << std::endl;
  } catch(std::exception& err) {
    std::cout << "Catched std::exception: " << err.what() << std::endl;
  }
}

/* EOF */
