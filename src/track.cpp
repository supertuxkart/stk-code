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
#include "translation.hpp"

Track::Track (std::string filename_, float w, float h, bool stretch)
{
    m_filename        = filename_;
    m_herring_style   = "";
    m_track_2d_width  = w;
    m_track_2d_height = h;
    m_do_stretch      = stretch;
    m_description     = "";
    m_screenshot      = "";
    m_top_view        = "";

    loadTrack(m_filename);
    loadDriveline();

}

//-----------------------------------------------------------------------------
Track::~Track()
{}

//-----------------------------------------------------------------------------
int Track::spatialToTrack ( sgVec3 res, sgVec3 xyz, int hint ) const
{
    size_t nearest = 0 ;
    float d ;
    float nearest_d = 99999 ;

    const unsigned int DRIVELINE_SIZE = m_driveline.size();
    int temp_i;
    //Checks from the previous two hints to the next two which is the closest,
    //checking from the previous to the next works on my machine but might not
    //work on slower computers.
    for(int i = hint - 2; i < hint + 3; ++i)
    {
        temp_i = i;
        if(temp_i < 0) temp_i = DRIVELINE_SIZE - temp_i;
        if(temp_i >= (int)DRIVELINE_SIZE) temp_i -= DRIVELINE_SIZE;

        d = sgDistanceVec2 ( m_driveline[temp_i], xyz ) ;
        if ( d < nearest_d )
        {
            nearest_d = d;
            nearest = temp_i;
        }
    }

    /*
      OK - so we have the closest point
    */

    const size_t PREV = ( nearest   ==   0              ) ? DRIVELINE_SIZE - 1 : (nearest - 1);
    const size_t NEXT = ( nearest+1 >= DRIVELINE_SIZE ) ?      0             : (nearest + 1);

    const float D_PREV = sgDistanceVec2 ( m_driveline[PREV], xyz ) ;
    const float D_NEXT = sgDistanceVec2 ( m_driveline[NEXT], xyz ) ;

    size_t p1, p2 ;
    float  d1, d2 ;

    if ( D_NEXT < D_PREV )
    {
        p1 = nearest   ; p2 =  NEXT ;
        d1 = nearest_d ; d2 = D_NEXT ;
    }
    else
    {
        p1 =  PREV ; p2 = nearest   ;
        d1 = D_PREV ; d2 = nearest_d ;
    }

    sgVec3 line_eqn ;
    sgVec3 tmp ;

    sgMake2DLine ( line_eqn, m_driveline[p1], m_driveline[p2] ) ;

    res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

    sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

    // m_distance_from_start[p1] contains the sum of the distances between
    // all driveline points up to point p1.
    res [ 1 ] = sgDistanceVec2 ( tmp, m_driveline[p1] ) + m_distance_from_start[p1];

    return nearest ;
}

//-----------------------------------------------------------------------------
int Track::absSpatialToTrack ( sgVec3 res, sgVec3 xyz ) const
{
    size_t nearest = 0 ;
    float d ;
    float nearest_d = 99999 ;

    /*
      Search all the points on the track to find our nearest
      centerline point
    */

    const unsigned int DRIVELINE_SIZE = m_driveline.size();
    for (size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        d = sgDistanceVec2 ( m_driveline[i], xyz ) ;

        if ( d < nearest_d )
        {
            nearest_d = d ;
            nearest = i ;
        }
    }   // for i

    /*
      OK - so we have the closest point
    */

    const size_t PREV = ( nearest   ==   0              ) ? DRIVELINE_SIZE - 1 : (nearest - 1);
    const size_t NEXT = ( nearest+1 >= DRIVELINE_SIZE ) ?      0             : (nearest + 1);

    const float D_PREV = sgDistanceVec2 ( m_driveline[PREV], xyz ) ;
    const float D_NEXT = sgDistanceVec2 ( m_driveline[NEXT], xyz ) ;

    size_t p1, p2 ;
    float  d1, d2 ;

    if ( D_NEXT < D_PREV )
    {
        p1 = nearest   ; p2 =  NEXT ;
        d1 = nearest_d ; d2 = D_NEXT ;
    }
    else
    {
        p1 =  PREV ; p2 = nearest   ;
        d1 = D_PREV ; d2 = nearest_d ;
    }

    sgVec3 line_eqn ;
    sgVec3 tmp ;

    sgMake2DLine ( line_eqn, m_driveline[p1], m_driveline[p2] ) ;

    res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

    sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

    res [ 1 ] = sgDistanceVec2 ( tmp, m_driveline[p1] ) + m_distance_from_start[p1];

    return nearest ;
}

//-----------------------------------------------------------------------------
void Track::trackToSpatial ( sgVec3 xyz, int hint ) const
{
    sgCopyVec3 ( xyz, m_driveline [ hint ] ) ;
}

/** It's not the nices solution to have two very similar version of a function,
 *  i.e. drawScaled2D and draw2Dview - but to keep both versions const, the
 *  values m_scale_x/m_scale_y can not be changed, but they are needed in glVtx.
 *  So two functions are provided: one which uses temporary variables, and one
 *  which uses the pre-computed attributes (see constructor/loadDriveline)
 *  - which saves a bit of time at runtime as well.
 *  drawScaled2D is called from gui/TrackSel, draw2Dview from RaceGUI.
 */
void Track::drawScaled2D(float x, float y, float w, float h) const
{
    sgVec2 sc ;
    sgSubVec2 ( sc, m_driveline_max, m_driveline_min ) ;

    float sx = (w-20.0f) / sc[0]; // leave 10 pix space left and right
    x+=10.0;
    float sy = h / sc[1] ;

    if(sx>sy)
    {
        sx = sy;
        x += w/2 - sc[0]*sx/2;
    }
    else
    {
        sy = sx;
    }

    const unsigned int DRIVELINE_SIZE = m_driveline.size();

    glPushAttrib ( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_TEXTURE_2D);

    glColor4f ( 1, 1, 1, 0.5) ;

    glBegin ( GL_QUAD_STRIP ) ;

    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
      glVertex2f ( x + ( m_left_driveline[i][0] - m_driveline_min[0] ) * sx,
          y + ( m_left_driveline[i][1] - m_driveline_min[1] ) * sy) ;

      glVertex2f ( x + ( m_right_driveline[i][0] - m_driveline_min[0] ) * sx,
          y + ( m_right_driveline[i][1] - m_driveline_min[1] ) * sy ) ;
    }
    glVertex2f ( x + ( m_left_driveline[0][0] - m_driveline_min[0] ) * sx,
        y + ( m_left_driveline[0][1] - m_driveline_min[1] ) * sy) ;
    glVertex2f ( x + ( m_right_driveline[0][0] - m_driveline_min[0] ) * sx,
        y + ( m_right_driveline[0][1] - m_driveline_min[1] ) * sy ) ;

    glEnd () ;

    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POINT_SMOOTH );
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glLineWidth(1);
    glPointSize(1);

    glColor4f ( 0, 0, 0, 1 ) ;

    glBegin ( GL_LINES ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE - 1 ; ++i )
    {
        /*Draw left driveline of the map*/
        glVertex2f ( x + ( m_left_driveline[i][0] - m_driveline_min[0] ) * sx,
            y + ( m_left_driveline[i][1] - m_driveline_min[1] ) * sy ) ;

        glVertex2f ( x + ( m_left_driveline[i+1][0] - m_driveline_min[0] ) * sx,
            y + ( m_left_driveline[i+1][1] - m_driveline_min[1] ) * sy ) ;


        /*Draw left driveline of the map*/
        glVertex2f ( x + ( m_right_driveline[i][0] - m_driveline_min[0] ) * sx,
	        y + ( m_right_driveline[i][1] - m_driveline_min[1] ) * sy ) ;

        glVertex2f ( x + ( m_right_driveline[i+1][0] - m_driveline_min[0] ) * sx,
	        y + ( m_right_driveline[i+1][1] - m_driveline_min[1] ) * sy ) ;
    }

    //Close the left driveline
    glVertex2f ( x + ( m_left_driveline[DRIVELINE_SIZE - 1][0] - m_driveline_min[0] ) * sx,
        y + ( m_left_driveline[DRIVELINE_SIZE - 1][1] - m_driveline_min[1] ) * sy ) ;

    glVertex2f ( x + ( m_left_driveline[0][0] - m_driveline_min[0] ) * sx,
        y + ( m_left_driveline[0][1] - m_driveline_min[1] ) * sy ) ;


    //Close the right driveline
    glVertex2f ( x + ( m_right_driveline[DRIVELINE_SIZE - 1][0] - m_driveline_min[0] ) * sx,
        y + ( m_right_driveline[DRIVELINE_SIZE - 1][1] - m_driveline_min[1] ) * sy ) ;

    glVertex2f ( x + ( m_right_driveline[0][0] - m_driveline_min[0] ) * sx,
        y + ( m_right_driveline[0][1] - m_driveline_min[1] ) * sy ) ;
    glEnd () ;

#if 0
  //FIXME: We are not sure if it's a videocard problem, but on Linux with a
  //Nvidia Geforce4 mx 440, we get problems with GL_LINE_LOOP;
  //If this issue is solved, using GL_LINE_LOOP is a better solution than
  //GL_LINES
    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x + ( m_left_driveline[i][0] - m_driveline_min[0] ) * sx,
            y + ( m_left_driveline[i][1] - m_driveline_min[1] ) * sy ) ;
    }
    glEnd () ;

    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x + ( m_right_driveline[i][0] - m_driveline_min[0] ) * sx,
	        y + ( m_right_driveline[i][1] - m_driveline_min[1] ) * sy ) ;
    }
    glEnd () ;
#endif


    glBegin ( GL_POINTS ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
      glVertex2f ( x + ( m_left_driveline[i][0] - m_driveline_min[0] ) * sx,
          y + ( m_left_driveline[i][1] - m_driveline_min[1] ) * sy ) ;

      glVertex2f ( x + ( m_right_driveline[i][0] - m_driveline_min[0] ) * sx,
          y + ( m_right_driveline[i][1] - m_driveline_min[1] ) * sy ) ;
    }
    glEnd () ;

    glPopAttrib();

}   // drawScaled2D

//-----------------------------------------------------------------------------
void Track::draw2Dview (float x_offset, float y_offset) const
{

    const unsigned int DRIVELINE_SIZE = m_driveline.size();

    glPushAttrib ( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_LINE_BIT );

    glEnable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_TEXTURE_2D);

    //TODO: maybe colors should be configurable, or at least the alpha value
    glColor4f ( 1,1,1, 0.4) ;


/*FIXME: Too much calculations here, we should be generating scaled driveline arrays
 * in Track::loadDriveline so all we'd be doing is pumping out predefined
 * vertexes in-game.
 */
    /*Draw white filling of the map*/
    glBegin ( GL_QUAD_STRIP ) ;

    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i ) {
      glVertex2f ( x_offset + ( m_left_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
          y_offset + ( m_left_driveline[i][1] - m_driveline_min[1] ) * m_scale_y) ;
      glVertex2f ( x_offset + ( m_right_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
          y_offset + ( m_right_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;
    }
    glVertex2f ( x_offset + ( m_left_driveline[0][0] - m_driveline_min[0] ) * m_scale_x,
        y_offset + ( m_left_driveline[0][1] - m_driveline_min[1] ) * m_scale_y ) ;
    glVertex2f ( x_offset + ( m_right_driveline[0][0] - m_driveline_min[0] ) * m_scale_x,
        y_offset + ( m_right_driveline[0][1] - m_driveline_min[1] ) * m_scale_y ) ;

    glEnd () ;


    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POINT_SMOOTH );
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glLineWidth(2);
    glPointSize(2);

    glColor4f ( 0,0,0,1) ;


    glBegin ( GL_LINES ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE - 1 ; ++i )
    {
        /*Draw left driveline of the map*/
        glVertex2f ( x_offset + ( m_left_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
            y_offset + ( m_left_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;

        glVertex2f ( x_offset + ( m_left_driveline[i+1][0] - m_driveline_min[0] ) * m_scale_x,
            y_offset + ( m_left_driveline[i+1][1] - m_driveline_min[1] ) * m_scale_y ) ;


        /*Draw left driveline of the map*/
        glVertex2f ( x_offset + ( m_right_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
	        y_offset + ( m_right_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;

        glVertex2f ( x_offset + ( m_right_driveline[i+1][0] - m_driveline_min[0] ) * m_scale_x,
	        y_offset + ( m_right_driveline[i+1][1] - m_driveline_min[1] ) * m_scale_y ) ;
    }

    //Close the left driveline
    glVertex2f ( x_offset + ( m_left_driveline[DRIVELINE_SIZE - 1][0] - m_driveline_min[0] ) * m_scale_x,
        y_offset + ( m_left_driveline[DRIVELINE_SIZE - 1][1] - m_driveline_min[1] ) * m_scale_y ) ;

    glVertex2f ( x_offset + ( m_left_driveline[0][0] - m_driveline_min[0] ) * m_scale_x,
        y_offset + ( m_left_driveline[0][1] - m_driveline_min[1] ) * m_scale_y ) ;


    //Close the right driveline
    glVertex2f ( x_offset + ( m_right_driveline[DRIVELINE_SIZE - 1][0] - m_driveline_min[0] ) * m_scale_x,
        y_offset + ( m_right_driveline[DRIVELINE_SIZE - 1][1] - m_driveline_min[1] ) * m_scale_y ) ;

    glVertex2f ( x_offset + ( m_right_driveline[0][0] - m_driveline_min[0] ) * m_scale_x,
        y_offset + ( m_right_driveline[0][1] - m_driveline_min[1] ) * m_scale_y ) ;
    glEnd () ;

#if 0
  //FIXME: We are not sure if it's a videocard problem, but on Linux with a
  //Nvidia Geforce4 mx 440, we get problems with GL_LINE_LOOP;
  //If this issue is solved, using GL_LINE_LOOP is a better solution than
  //GL_LINES
    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x_offset + ( m_left_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
            y_offset + ( m_left_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;
    }
    glEnd () ;

    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x_offset + ( m_right_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
	        y_offset + ( m_right_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;
    }
    glEnd () ;
#endif

    /*Because of the way OpenGL draws lines of widths higher than 1,
     *we have to draw the joints too, in order to fill small spaces
     *between lines
     */
    glBegin ( GL_POINTS) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x_offset + ( m_left_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
            y_offset + ( m_left_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;

        glVertex2f ( x_offset + ( m_right_driveline[i][0] - m_driveline_min[0] ) * m_scale_x,
	      y_offset + ( m_right_driveline[i][1] - m_driveline_min[1] ) * m_scale_y ) ;
    }
    glEnd () ;

    glPopAttrib();

}   // draw2Dview

//-----------------------------------------------------------------------------
void Track::loadTrack(std::string filename_)
{
    m_filename      = filename_;

    m_ident = StringUtils::basename(StringUtils::without_extension(m_filename));
    std::string path = StringUtils::without_extension(m_filename);

    // Default values
    m_use_fog = false;
    sgSetVec4 ( m_fog_color  , 0.3f, 0.7f, 0.9f, 1.0f ) ;
    m_fog_density = 1.0f/100.0f;
    m_fog_start   = 0.0f;
    m_fog_end     = 1000.0f;
    m_gravity     = 9.80665f;

    sgSetVec3 ( m_sun_position, 0.4f, 0.4f, 0.4f );
    sgSetVec4 ( m_sky_color  ,  0.3f, 0.7f, 0.9f, 1.0f );
    sgSetVec4 ( m_fog_color  ,  0.3f, 0.7f, 0.9f, 1.0f );
    sgSetVec4 ( m_ambient_col ,  0.5f, 0.5f, 0.5f, 1.0f );
    sgSetVec4 ( m_specular_col,  1.0f, 1.0f, 1.0f, 1.0f );
    sgSetVec4 ( m_diffuse_col ,  1.0f, 1.0f, 1.0f, 1.0f );

    lisp::Parser parser;
    const lisp::Lisp* const ROOT = parser.parse(loader->getPath(m_filename));

    const lisp::Lisp* const LISP = ROOT->getLisp("tuxkart-track");
    if(!LISP)
    {
        delete ROOT;
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), 
                 _("Couldn't load map '%s': no tuxkart-track node."),
                 m_filename.c_str());
        throw std::runtime_error(msg);
    }

    LISP->get("name",          m_name);
    LISP->get("description",   m_description);
    LISP->get("music",         m_music_filename);
    LISP->get("herring",       m_herring_style);
    LISP->get("screenshot",    m_screenshot);
    LISP->get("topview",       m_top_view);
    LISP->get("sky-color",     m_sky_color);

    LISP->get("use-fog",       m_use_fog);
    LISP->get("fog-color",     m_fog_color);
    LISP->get("fog-density",   m_fog_density);
    LISP->get("fog-start",     m_fog_start);
    LISP->get("fog-end",       m_fog_end);

    LISP->get("sun-position",  m_sun_position);
    LISP->get("sun-ambient",   m_ambient_col);
    LISP->get("sun-specular",  m_specular_col);
    LISP->get("sun-diffuse",   m_diffuse_col);
    LISP->get("m_gravity",     m_gravity);
    delete ROOT;
}

//-----------------------------------------------------------------------------
void
Track::loadDriveline()
{
    readDrivelineFromFile(m_left_driveline, ".drvl");

    const unsigned int DRIVELINE_SIZE = m_left_driveline.size();
    m_right_driveline.reserve(DRIVELINE_SIZE);
    readDrivelineFromFile(m_right_driveline, ".drvr");

    if(m_right_driveline.size() != m_left_driveline.size())
        std::cout << "Error: driveline's sizes do not match, right " <<
        "driveline is " << m_right_driveline.size() << " points long " <<
        "and the left driveline is " << m_left_driveline.size()
        << " points long. Track is " << m_name << " ." << std::endl;

    SGfloat width;
    sgVec3 center_point, width_vector;
    m_driveline.reserve(DRIVELINE_SIZE);
    m_path_width.reserve(DRIVELINE_SIZE);
    m_angle.reserve(DRIVELINE_SIZE);
    for(unsigned int i = 0; i < DRIVELINE_SIZE; ++i)
    {
        sgAddVec3(center_point, m_left_driveline[i], m_right_driveline[i]);
        sgScaleVec3(center_point, 0.5f);
        m_driveline.push_back(center_point);

        sgSubVec3(width_vector, m_right_driveline[i], center_point);
        width = sgLengthVec3(width_vector);
        if(width > 0.0f) m_path_width.push_back(width);
        else m_path_width.push_back(-width);
    }

    size_t next;
    float adjacent_line, opposite_line;
    SGfloat theta;

    for(unsigned int i = 0; i < DRIVELINE_SIZE; ++i)
    {
        next = i + 1 >= DRIVELINE_SIZE ? 0 : i + 1;
        adjacent_line = m_driveline[next][0] - m_driveline[i][0];
        opposite_line = m_driveline[next][1] - m_driveline[i][1];

        theta = sgATan(opposite_line/adjacent_line);
        theta += adjacent_line < 0.0f ? 90.0f : -90.0f;

        m_angle.push_back(theta);
    }

    sgSetVec2 ( m_driveline_min,  SG_MAX/2.0f,  SG_MAX/2.0f ) ;
    sgSetVec2 ( m_driveline_max, -SG_MAX/2.0f, -SG_MAX/2.0f ) ;


    m_distance_from_start.reserve(DRIVELINE_SIZE);
    float d = 0.0f ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        if ( m_driveline[i][0] < m_driveline_min[0] )
            m_driveline_min[0] = m_driveline[i][0] ;
        if ( m_driveline[i][1] < m_driveline_min[1] )
            m_driveline_min[1] = m_driveline[i][1] ;
        if ( m_driveline[i][0] > m_driveline_max[0] )
            m_driveline_max[0] = m_driveline[i][0] ;
        if ( m_driveline[i][1] > m_driveline_max[1] )
            m_driveline_max[1] = m_driveline[i][1] ;

        m_distance_from_start.push_back(d);  // dfs[i] is not valid in windows here!

        if ( i == DRIVELINE_SIZE - 1 )
            d += sgDistanceVec2 ( m_driveline[i], m_driveline[0] ) ;
        else
            d += sgDistanceVec2 ( m_driveline[i], m_driveline[i+1] ) ;
    }

    m_total_distance = d;

    sgVec2 sc ;
    sgSubVec2 ( sc, m_driveline_max, m_driveline_min ) ;

    m_scale_x = m_track_2d_width  / sc[0] ;
    m_scale_y = m_track_2d_height / sc[1] ;

    if(!m_do_stretch) m_scale_x = m_scale_y = std::min(m_scale_x, m_scale_y);

}   // loadDriveline

//-----------------------------------------------------------------------------
void
Track::readDrivelineFromFile(std::vector<sgVec3Wrapper>& line, const std::string& file_ext)
{
    std::string path = "data";
    path += DIR_SEPARATOR;
    path += m_ident;
    path += file_ext;
    path = loader->getPath(path.c_str());

    FILE *fd = fopen ( path.c_str(), "r" ) ;

    if ( fd == NULL )
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf (msg, sizeof(msg), _("Can't open '%s' for reading.\n"), path.c_str() ) ;
        throw std::runtime_error(msg);
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
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf (msg, sizeof(msg), _("Syntax error in '%s'\n"), path.c_str() ) ;
            throw std::runtime_error(msg);
        }

        sgVec3 point;
        point[0] = x;
        point[1] = y;
        point[2] = z;

        if(prev_hint != -1) prev_distance = sgDistanceVec2 ( point, line[prev_hint]);

        //1.5f was choosen because it's more or less the length of the tuxkart
        if(prev_distance == 0)
        {
            fprintf(stderr, _("File %s point %d is duplicated!.\n"),
                    path.c_str(), prev_hint+1);
        }
        else if(prev_distance < 1.5f)
        {
            fprintf(stderr,_("File %s point %d is too close(<1.5) to previous point.\n"),
                    path.c_str(), prev_hint + 1);
        }
#if 0
        if(prev_distance > 15.0f)
        {
            fprintf(stderr,_("In file %s point %d is too far(>15.0) from next point at %d.\n"),
                    path, prev_hint, prev_distance);
        }
#endif

        line.push_back(point);
        ++prev_hint;
        prev_distance -= 1.5f;
    }

    fclose ( fd ) ;
}

