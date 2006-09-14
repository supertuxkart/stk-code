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

#include <iostream>
#include <stdexcept>
#include <sstream>
#include "loader.hpp"
#include "track.hpp"
#include "string_utils.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"


Track::Track (std::string filename_, float w, float h, bool stretch) {
  filename      = filename_;
  herringStyle  = "";
  track2DWidth  = w;
  track2DHeight = h;
  doStretch     = stretch;
  description   = "";
  screenshot    = "";
  topview       = "";
  loadTrack(filename);
  loadDriveline();

}

// -----------------------------------------------------------------------------
Track::~Track() {
}

// -----------------------------------------------------------------------------
int Track::spatialToTrack ( sgVec3 res, sgVec3 xyz, int hint ) const {
  size_t nearest = 0 ;
  float d ;
  float nearest_d = 99999 ;

  const unsigned int driveline_size = driveline.size();
  int temp_i;
  //Checks from the previous two hints to the next two which is the closest,
  //checking from the previous to the next works on my machine but might not
  //work on slower computers.
  for(int i = hint - 2; i < hint + 3; ++i)
  {
      temp_i = i;
      if(temp_i < 0) temp_i = driveline_size - temp_i;
      if(temp_i >= (int)driveline_size) temp_i -= driveline_size;

      d = sgDistanceVec2 ( driveline[temp_i], xyz ) ;
      if ( d < nearest_d ) {
        nearest_d = d;
        nearest = temp_i;
      }
  }

  /*
    OK - so we have the closest point
  */

  size_t prev,  next ;
  float  dprev, dnext ;

  prev = ( nearest   ==   0              ) ? driveline_size - 1 : (nearest - 1);
  next = ( nearest+1 >= driveline_size ) ?      0             : (nearest + 1);

  dprev = sgDistanceVec2 ( driveline[prev], xyz ) ;
  dnext = sgDistanceVec2 ( driveline[next], xyz ) ;

  size_t p1, p2 ;
  float  d1, d2 ;

  if ( dnext < dprev ) {
    p1 = nearest   ; p2 =  next ;
    d1 = nearest_d ; d2 = dnext ;
  } else {
    p1 =  prev ; p2 = nearest   ;
    d1 = dprev ; d2 = nearest_d ;
  }

  sgVec3 line_eqn ;
  sgVec3 tmp ;

  sgMake2DLine ( line_eqn, driveline[p1], driveline[p2] ) ;

  res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

  sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

  // distance_from_start[p1] contains the sum of the distances between
  // all driveline points up to point p1.
  res [ 1 ] = sgDistanceVec2 ( tmp, driveline[p1] ) + distance_from_start[p1];

  return nearest ;
}

// -----------------------------------------------------------------------------
int Track::absSpatialToTrack ( sgVec3 res, sgVec3 xyz ) const {
  size_t nearest = 0 ;
  float d ;
  float nearest_d = 99999 ;

  /*
    Search all the points on the track to find our nearest
    centerline point
  */

  const unsigned int driveline_size = driveline.size();
  for (size_t i = 0 ; i < driveline_size ; ++i ) {
    d = sgDistanceVec2 ( driveline[i], xyz ) ;

    if ( d < nearest_d ) {
      nearest_d = d ;
      nearest = i ;
    }
  }   // for i

  /*
    OK - so we have the closest point
  */

  size_t prev,  next ;
  float  dprev, dnext ;

  prev = ( nearest   ==   0              ) ? driveline_size-1 : (nearest - 1);
  next = ( nearest+1 >= driveline_size ) ?      0             : (nearest + 1);

  dprev = sgDistanceVec2 ( driveline[prev], xyz ) ;
  dnext = sgDistanceVec2 ( driveline[next], xyz ) ;

  size_t p1, p2 ;
  float  d1, d2 ;

  if ( dnext < dprev ) {
    p1 = nearest   ; p2 =  next ;
    d1 = nearest_d ; d2 = dnext ;
  } else {
    p1 =  prev ; p2 = nearest   ;
    d1 = dprev ; d2 = nearest_d ;
  }

  sgVec3 line_eqn ;
  sgVec3 tmp ;

  sgMake2DLine ( line_eqn, driveline[p1], driveline[p2] ) ;

  res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

  sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

  res [ 1 ] = sgDistanceVec2 ( tmp, driveline[p1] ) + distance_from_start[p1];

  return nearest ;
}

// -----------------------------------------------------------------------------
void Track::trackToSpatial ( sgVec3 xyz, int hint ) const {
  sgCopyVec3 ( xyz, driveline [ hint ] ) ;
}
// -----------------------------------------------------------------------------
// It's not the nices solution to have two very similar version of a function,
// i.e. drawScaled2D and draw2Dview - but to keep both versions const, the
// values scaleX/scaleY can not be changed, but they are needed in glVtx.
// So two functions are provided: one which uses temporary variables, and one
// which uses the pre-computed attributes (see constructor/loadDriveline)
// - which saves a bit of time at runtime as well.
// drawScaled2D is called from gui/TrackSel, draw2Dview from RaceGUI.
void Track::drawScaled2D(float x, float y, float w, float h) const {
  sgVec2 sc ;
  sgSubVec2 ( sc, driveline_max, driveline_min ) ;
  
  float sx = (w-20.0f) / sc[0]; // leave 10 pix space left and right
  x+=10.0;
  float sy = h / sc[1] ;

  if(sx>sy) {
    sx = sy;
    x += w/2 - sc[0]*sx/2;
  } else {
    sy = sx;
  }

  const unsigned int driveline_size = driveline.size();
  glBegin ( GL_LINE_LOOP ) ;
  for ( size_t i = 0 ; i < driveline_size ; ++i ) {
      glVertex2f ( x + ( driveline[i][0] - driveline_min[0] ) * sx,
                   y + ( driveline[i][1] - driveline_min[1] ) * sy ) ;
  }
  glEnd () ;

}   // drawScaled2D

// -----------------------------------------------------------------------------
void Track::draw2Dview (float xOff, float yOff) const {

  const unsigned int driveline_size = driveline.size();
  glBegin ( GL_LINE_LOOP ) ;
  for ( size_t i = 0 ; i < driveline_size ; ++i ) {
    glVertex2f ( xOff + ( driveline[i][0] - driveline_min[0] ) * scaleX,
		 yOff + ( driveline[i][1] - driveline_min[1] ) * scaleY ) ;
  }
  glEnd () ;
}   // draw2Dview

// -----------------------------------------------------------------------------
void Track::loadTrack(std::string filename_) {
  filename      = filename_;

  ident = StringUtils::basename(StringUtils::without_extension(filename));
  std::string path = StringUtils::without_extension(filename);

  // Default values
  use_fog = false;
  sgSetVec4 ( fog_color  , 0.3f, 0.7f, 0.9f, 1.0f ) ;
  fog_density = 1.0f/100.0f;
  fog_start   = 0.0f;
  fog_end     = 1000.0f;
  gravity     = 9.80665f;

  sgSetVec3 ( sun_position, 0.4f, 0.4f, 0.4f );
  sgSetVec4 ( sky_color  ,  0.3f, 0.7f, 0.9f, 1.0f );
  sgSetVec4 ( fog_color  ,  0.3f, 0.7f, 0.9f, 1.0f );
  sgSetVec4 ( ambientcol ,  0.5f, 0.5f, 0.5f, 1.0f );
  sgSetVec4 ( specularcol,  1.0f, 1.0f, 1.0f, 1.0f );
  sgSetVec4 ( diffusecol ,  1.0f, 1.0f, 1.0f, 1.0f );

  const lisp::Lisp* root = 0;
  lisp::Parser parser;
  root = parser.parse(loader->getPath(filename));

  const lisp::Lisp* lisp = root->getLisp("tuxkart-track");
  if(!lisp) {
    delete root;
    std::stringstream msg;
    msg << "Couldn't load map '" << filename << "': no tuxkart-track node.";
    throw std::runtime_error(msg.str());
  }

  lisp->get("name",          name);
  lisp->get("description",   description);
  lisp->get("music",         music_filename);
  lisp->get("herring",       herringStyle);
  lisp->get("screenshot",    screenshot);
  lisp->get("topview",       topview);
  lisp->get("sky-color",     sky_color);

  lisp->get("use-fog",       use_fog);
  lisp->get("fog-color",     fog_color);
  lisp->get("fog-density",   fog_density);
  lisp->get("fog-start",     fog_start);
  lisp->get("fog-end",       fog_end);

  lisp->get("sun-position",  sun_position);
  lisp->get("sun-ambient",   ambientcol);
  lisp->get("sun-specular",  specularcol);
  lisp->get("sun-diffuse",   diffusecol);
  lisp->get("gravity",       gravity);
  delete root;
}

void
Track::loadDriveline()
{
  std::vector<sgVec3Wrapper> left_driveline;
  std::vector<sgVec3Wrapper> right_driveline;

  readDrivelineFromFile(left_driveline, ".drvl");

  const unsigned int driveline_size = left_driveline.size();
  right_driveline.reserve(driveline_size);
  readDrivelineFromFile(right_driveline, ".drvr");

  if(right_driveline.size() != left_driveline.size())
      std::cout << "Error: driveline's sizes do not match, right " <<
          "driveline is " << right_driveline.size() << " points long " <<
          "and the left driveline is " << left_driveline.size()
          << " points long. Track is " << name << " ." << std::endl;

  SGfloat width;
  sgVec3 center_point, width_vector;
  driveline.reserve(driveline_size);
  path_width.reserve(driveline_size);
  angle.reserve(driveline_size);
  for(unsigned int i = 0; i < driveline_size; ++i)
  {
      sgAddVec3(center_point, left_driveline[i], right_driveline[i]);
      sgScaleVec3(center_point, 0.5f);
      driveline.push_back(center_point);

      sgSubVec3(width_vector, right_driveline[i], center_point);
      width = sgLengthVec3(width_vector);
      if(width > 0.0f) path_width.push_back(width);
      else path_width.push_back(-width);
  }

  size_t next;
  float adjacent_line, opposite_line;
  SGfloat theta;

  for(unsigned int i = 0; i < driveline_size; ++i)
  {
      next = i + 1 >= driveline_size ? 0 : i + 1;
      adjacent_line = driveline[next][0] - driveline[i][0];
      opposite_line = driveline[next][1] - driveline[i][1];

      theta = sgATan(opposite_line/adjacent_line);
      theta += adjacent_line < 0.0f ? 90.0f : -90.0f;

      angle.push_back(theta);
  }

  sgSetVec2 ( driveline_min,  SG_MAX/2.0f,  SG_MAX/2.0f ) ;
  sgSetVec2 ( driveline_max, -SG_MAX/2.0f, -SG_MAX/2.0f ) ;


  distance_from_start.reserve(driveline_size);
  float d = 0.0f ;
  for ( size_t i = 0 ; i < driveline_size ; ++i )
  {
    if ( driveline[i][0] < driveline_min[0] )
      driveline_min[0] = driveline[i][0] ;
    if ( driveline[i][1] < driveline_min[1] )
      driveline_min[1] = driveline[i][1] ;
    if ( driveline[i][0] > driveline_max[0] )
      driveline_max[0] = driveline[i][0] ;
    if ( driveline[i][1] > driveline_max[1] )
      driveline_max[1] = driveline[i][1] ;

    distance_from_start[i] = d;

    if ( i == driveline_size - 1 )
      d += sgDistanceVec2 ( driveline[i], driveline[0] ) ;
    else
      d += sgDistanceVec2 ( driveline[i], driveline[i+1] ) ;
  }

  total_distance = d;

  sgVec2 sc ;
  sgSubVec2 ( sc, driveline_max, driveline_min ) ;

  scaleX = track2DWidth  / sc[0] ;
  scaleY = track2DHeight / sc[1] ;

  if(!doStretch) scaleX = scaleY = std::min(scaleX, scaleY);

}   // loadDriveline

void
Track::readDrivelineFromFile(std::vector<sgVec3Wrapper>& line, const std::string& file_ext)
{
  std::string path = "data/";
  path += ident;
  path += file_ext;
  path = loader->getPath(path.c_str());

  FILE *fd = fopen ( path.c_str(), "ra" ) ;

  if ( fd == NULL )
    {
      fprintf ( stderr, "Can't open '%s' for reading.\n", path.c_str() ) ;
      exit ( 1 ) ;
    }

  int prev_hint = -1;
  SGfloat prev_distance = 1.51f;
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

      if (sscanf ( s, "%f,%f,%f", &x, &y, &z ) != 3 )
        {
          fprintf ( stderr, "Syntax error in '%s'\n", path.c_str() ) ;
          exit ( 1 ) ;
        }

      sgVec3 point;
      point[0] = x;
      point[1] = y;
      point[2] = z;

      if(prev_hint != -1) prev_distance = sgDistanceVec2 ( point, line[prev_hint]);

      //1.5f was choosen because it's more or less the length of the tuxkart
      if(prev_distance == 0)
      {
        std::cerr << "File " << path << " point " << prev_hint + 1 << " is " <<
            "duplicated!.\n";
      }
      else if(prev_distance < 1.5f)
      {
        std::cerr << "File " << path << " point " << prev_hint + 1 << " is " <<
            "too close(<1.5) to previous point.\n";
      }
#if 0
      if(prev_distance > 15.0f)
      {
        std::cerr << "In file " << path << " point " <<
            prev_hint << " is too far(>15.0) from next point at " <<
            prev_distance << " .\n";
      }
#endif

          line.push_back(point);
          ++prev_hint;
          prev_distance -= 1.5f;
    }

  fclose ( fd ) ;
}

