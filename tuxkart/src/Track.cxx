//  $Id: Track.cxx,v 1.18 2004/09/24 18:27:25 matzebraun Exp $
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

#include <stdexcept>
#include <sstream>
#include "tuxkart.h"
#include "Loader.h"
#include "Track.h"
#include "StringUtils.h"
#include "lisp/Lisp.h"
#include "lisp/Parser.h"

Track::Track (const std::string& filename)
{
  loadTrack(filename);
  loadDriveline();
}

Track::~Track()
{
}

extern int check_hint ;

int
Track::spatialToTrack ( sgVec3 res, sgVec3 xyz, int hint ) const
{
  size_t nearest = 0 ;
  float d ;
  float nearest_d = 99999 ;

  /*
    If we don't have a good hint, search all the
    points on the track to find our nearest centerline point
  */

/*
if ( check_hint )
fprintf(stderr,"ih=%d ", hint ) ;
  if ( hint < 0 || hint >= npoints )
*/
  {
    for (size_t i = 0 ; i < driveline.size() ; ++i )
    {
      d = sgDistanceVec2 ( driveline[i], xyz ) ;

      if ( d < nearest_d )
      {
        nearest_d = d ;
        nearest = i ;
      }
    }

    hint = nearest ;
/*
if ( check_hint )
fprintf(stderr,"hint=%d\n", hint ) ;
*/
  }

  /*
    Check the two points on the centerline either side
    of the hint.
  */

/*
  int hp = ( hint <=      0     ) ? (npoints-1) : (hint-1) ;
  int hn = ( hint >= (npoints-1)) ?      0      : (hint+1) ;

  float dp = sgDistanceVec2 ( driveline[ hp ], xyz ) ;
  float d0 = sgDistanceVec2 ( driveline[hint], xyz ) ;
  float dn = sgDistanceVec2 ( driveline[ hn ], xyz ) ;

if ( check_hint )
fprintf(stderr,"d=(%f->%f->%f), %d/%d/%d ", dp,d0,dn, hp,hint,hn ) ;

  if ( d0 < dp && d0 < dn )
  {
    nearest   = hint ;
    nearest_d =  d0  ;
  }
  else
  if ( dp < dn )
  {
    nearest   = hp ;
    nearest_d = dp ;
  }
  else
  {
    nearest   = hn ;
    nearest_d = dn ;
  }

if ( check_hint )
fprintf(stderr,"new hint=%d\n", nearest ) ;
*/
  /*
    OK - so we have the closest point
  */

  size_t    prev,  next ;
  float dprev, dnext ;

  prev = ( nearest   ==   0     ) 
          ? driveline.size() - 1 : (nearest - 1) ;
  next = ( nearest+1 >= driveline.size() ) 
          ?      0        : nearest + 1 ;

  dprev = sgDistanceVec2 ( driveline[prev], xyz ) ;
  dnext = sgDistanceVec2 ( driveline[next], xyz ) ;

  size_t p1, p2 ;
  float d1, d2 ;

  if ( dnext < dprev )
  {
    p1 = nearest   ; p2 =  next ;
    d1 = nearest_d ; d2 = dnext ;
  }
  else
  {
    p1 =  prev ; p2 = nearest   ;
    d1 = dprev ; d2 = nearest_d ;
  }

  sgVec3 line_eqn ;
  sgVec3 tmp ;

  sgMake2DLine ( line_eqn, driveline[p1], driveline[p2] ) ;

  res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

  sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

  res [ 1 ] = sgDistanceVec2 ( tmp, driveline[p1] ) + driveline[p1][2] ;

  return nearest ;
}

int
Track::absSpatialToTrack ( sgVec3 res, sgVec3 xyz ) const
{
  return spatialToTrack ( res, xyz, 100000 ) ;
}


void
Track::trackToSpatial ( sgVec3 xyz, int hint ) const
{
  sgCopyVec3 ( xyz, driveline [ hint ] ) ;
}

void
Track::draw2Dview ( float x, float y, float w, float h, bool stretch ) const
{
  sgVec2 sc ;
  sgSubVec2 ( sc, driveline_max, driveline_center ) ;

  float scale_x = w / sc[0] ;
  float scale_y = h / sc[1] ;

  if(stretch == false)
    scale_x = scale_y = std::min(scale_x, scale_y);
 
  glBegin ( GL_LINE_LOOP ) ;
  for ( size_t i = 0 ; i < driveline.size() ; ++i )
    {
      glVertex2f ( x + ( driveline[i][0] - driveline_center[0] ) * scale_x,
                   y + ( driveline[i][1] - driveline_center[1] ) * scale_y ) ;
    }
  glEnd () ;
}

void
Track::loadTrack(const std::string& filename)
{
  ident = StringUtils::basename(StringUtils::without_extension(filename));
  std::string path = StringUtils::without_extension(filename);

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
  lisp::Parser parser;
  root = parser.parse(loader->getPath(filename));

  const lisp::Lisp* lisp = root->getLisp("tuxkart-track");
  if(!lisp) {
    delete root;
    std::stringstream msg;
    msg << "Couldn't load map '" << filename << "': no tuxkart-track node.";
    throw std::runtime_error(msg.str());
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
  delete root;
}

void
Track::loadDriveline()
{
  std::string path = "data/";
  path += ident;
  path += ".drv";
  path = loader->getPath(path);

  FILE *fd = fopen ( path.c_str(), "ra" ) ;

  if ( fd == NULL )
    {
      fprintf ( stderr, "Can't open '%s' for reading.\n", path.c_str() ) ;
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
          fprintf ( stderr, "Syntax error in '%s'\n", path.c_str() ) ;
          exit ( 1 ) ;
        }

      sgVec3 point;
      point[0] = x;
      point[1] = y;
      point[2] = z;

      driveline.push_back(point);
    }

  fclose ( fd ) ;

  sgSetVec2 ( driveline_min,  SG_MAX/2.0f,  SG_MAX/2.0f ) ;
  sgSetVec2 ( driveline_max, -SG_MAX/2.0f, -SG_MAX/2.0f ) ;

  float d = 0.0f ;
  for ( size_t i = 0 ; i < driveline.size() ; i++ )
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

    if ( i == driveline.size() - 1 )
      d += sgDistanceVec2 ( driveline[i], driveline[0] ) ;
    else
      d += sgDistanceVec2 ( driveline[i], driveline[i+1] ) ;
  }

  total_distance = d;
  sgAddScaledVec2(driveline_center, driveline_min, driveline_max, 0.5);
}

