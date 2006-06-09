//  $Id: Track.cxx,v 1.5 2005/08/31 17:25:25 joh Exp $
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

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "Loader.h"
#include "Track.h"
#include "StringUtils.h"
#include "lisp/Lisp.h"
#include "lisp/Parser.h"


Track::Track (const std::string& filename, float w, float h, bool stretch) {
  herringStyle  = "";
  track2DWidth  = w;
  track2DHeight = h;
  doStretch     = stretch;
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

  prev = ( nearest   ==   0              ) ? driveline.size()-1 : (nearest - 1);
  next = ( nearest+1 >= driveline.size() ) ?      0             : (nearest + 1);

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

  // driveline[i][2] contains the sum of the distances between
  // all driveline points up to point i.
  res [ 1 ] = sgDistanceVec2 ( tmp, driveline[p1] ) + driveline[p1][2] ;

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

  prev = ( nearest   ==   0              ) ? driveline.size()-1 : (nearest - 1);
  next = ( nearest+1 >= driveline.size() ) ?      0             : (nearest + 1);

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

  // driveline[i][2] contains the sum of the distances between
  // all driveline points up to point i.
  res [ 1 ] = sgDistanceVec2 ( tmp, driveline[p1] ) + driveline[p1][2] ;

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
void Track::drawScaled2D(float x, float y, float w, 
			 float h, bool stretch) const {
  sgVec2 sc ;
  sgSubVec2 ( sc, driveline_max, driveline_center ) ;

  float sx = w / sc[0] ;
  float sy = h / sc[1] ;

  if(!stretch) sx = sy = std::min(sx, sy);

  const unsigned int driveline_size = driveline.size();
  glBegin ( GL_LINE_LOOP ) ;
  for ( size_t i = 0 ; i < driveline_size ; ++i ) {
      glVertex2f ( x + ( driveline[i][0] - driveline_center[0] ) * sx,
                   y + ( driveline[i][1] - driveline_center[1] ) * sy ) ;
  }
  glEnd () ;

}   // drawScaled2D

// -----------------------------------------------------------------------------
void Track::draw2Dview (float x, float y) const {

  const unsigned int driveline_size = driveline.size();
  glBegin ( GL_LINE_LOOP ) ;
  for ( size_t i = 0 ; i < driveline_size ; ++i ) {
      glVertex2f ( x + ( driveline[i][0] - driveline_center[0] ) * scaleX,
                   y + ( driveline[i][1] - driveline_center[1] ) * scaleY ) ;
  }
  glEnd () ;
}   // draw2Dview

// -----------------------------------------------------------------------------
void Track::loadTrack(const std::string& filename) {
  ident = StringUtils::basename(StringUtils::without_extension(filename));
  std::string path = StringUtils::without_extension(filename);

  // Default values
  use_fog = false;
  sgSetVec4 ( fog_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  fog_density = 1.0f/100.0f;
  fog_start   = 0.0f;
  fog_end     = 1000.0f;
  gravity     = 9.80665f;

  sgSetVec3 ( sun_position, 0.4, 0.4, 0.4 ) ;
  sgSetVec4 ( sky_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( fog_color  , 0.3, 0.7, 0.9, 1.0 ) ;
  sgSetVec4 ( ambientcol , 0.5, 0.5, 0.5, 1.0 ) ;
  sgSetVec4 ( specularcol, 1.0, 1.0, 1.0, 1.0 ) ;
  sgSetVec4 ( diffusecol , 1.0, 1.0, 1.0, 1.0 ) ;

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
  lisp->get("music",         music_filename);
  lisp->get("herring",       herringStyle);
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
  driveline.reserve(driveline_size);
  path_width.reserve(driveline_size);
  angle.reserve(driveline_size);
  for(unsigned int i = 0; i < driveline_size; ++i)
  {
      //Reuse the left_driveline as the center driveline to avoid copying
      sgAddVec3(left_driveline[i], right_driveline[i]);
      sgScaleVec3(left_driveline[i], 0.5f);
      driveline.push_back(left_driveline[i]);

      //Reuse the right_driveline as the width of the track to avoid copying
      sgSubVec3(right_driveline[i], left_driveline[i]);
      width = sgLengthVec3(right_driveline[i]);
      if(width > 0.0f) path_width.push_back(width);
      else path_width.push_back(-width);
  }

  size_t next;
  const int NUM_POINTS = 1;
  SGfloat adjacent_line, opposite_line, theta, prev[NUM_POINTS];

  for(int i = 0; i < NUM_POINTS; ++i) prev[i] = 0.0f;

  for(unsigned int i = 0; i < driveline_size; ++i)
  {
      next = i + 1 >= driveline_size ? 0 : i + 1;
      adjacent_line = left_driveline[next][0] - left_driveline[i][0];
      opposite_line = left_driveline[next][1] - left_driveline[i][1];

      theta = atanf(opposite_line/adjacent_line) * SG_RADIANS_TO_DEGREES;

      if (adjacent_line < 0.0f) theta = theta + 90.0f;
      else theta = theta - 90.0f;


      SGfloat add = 0.0f;
      for(int j = 1; j < NUM_POINTS; ++j) add += prev[i];
      angle.push_back((theta + add) / NUM_POINTS);
      for(int j = 1; j < NUM_POINTS; ++j) prev[j] = prev[j - 1];
      prev[0] = theta;
  }

  sgSetVec2 ( driveline_min,  SG_MAX/2.0f,  SG_MAX/2.0f ) ;
  sgSetVec2 ( driveline_max, -SG_MAX/2.0f, -SG_MAX/2.0f ) ;

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

    driveline [i][2] = d ;

    if ( i == driveline_size - 1 )
      d += sgDistanceVec2 ( driveline[i], driveline[0] ) ;
    else
      d += sgDistanceVec2 ( driveline[i], driveline[i+1] ) ;
  }

  total_distance = d;
  sgAddScaledVec2(driveline_center, driveline_min, driveline_max, 0.5);

  sgVec2 sc ;
  sgSubVec2 ( sc, driveline_max, driveline_center ) ;

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
  path = loader->getPath(path);

  FILE *fd = fopen ( path.c_str(), "ra" ) ;

  if ( fd == NULL )
    {
      fprintf ( stderr, "Can't open '%s' for reading.\n", path.c_str() ) ;
      exit ( 1 ) ;
    }

  static int prev_point;
  static SGfloat prev_distance;
  prev_point = -1;
  prev_distance = 1.51f;
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
          fprintf ( stderr, "Syntax error in '%s'\n", path.c_str() ) ;
          exit ( 1 ) ;
        }

      sgVec3 point;
      point[0] = x;
      point[1] = y;
      //FIXME: the driveline probably should be made 2D instead of this hack.
      point[2] = 0.0f;

      SGfloat distance;
      if(prev_point != -1)
      {
          distance = sgDistanceVec3 ( point, line[prev_point]);
          if(distance < 0.0f) distance = -distance;
          prev_distance += distance;
      }


//FIXME: make a warning for points with more than 15.0f distance between them.
      if(prev_distance > 1.5f)
      {
          line.push_back(point);
          ++prev_point;
          prev_distance -= 1.5f;
      }
      else std::cerr << "In file " << path << " point " << prev_point + 1 <<
          " is too close to previous point." << std::endl;
    }

  fclose ( fd ) ;
}

