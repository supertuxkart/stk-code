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
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

Track::Track( std::string filename_, float w, float h, bool stretch )
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

}   // Track

//-----------------------------------------------------------------------------
Track::~Track()
{
}   // ~Track

//-----------------------------------------------------------------------------
/** Finds on which side of the line segment a given point is.
 */
inline float Track::pointSideToLine( const sgVec2 L1, const sgVec2 L2,
    const sgVec2 P ) const
{
    return ( L2[0]-L1[0] )*( P[1]-L1[1] )-( L2[1]-L1[1] )*( P[0]-L1[0] );
}   // pointSideToLine

//-----------------------------------------------------------------------------
/** pointInQuad() works by checking if the given point is 'to the right'
 *  in clock-wise direction (which would be to look towards the inside of
 *  the quad) of each line segment that forms the quad. If it is to the
 *  left of all the segments, then the point is inside. This idea
 *  works for convex polygons, so we have to test it for the two
 *  triangles that compose the quad, in case that the quad is concave,
 *  not for the quad itself.
 */
int Track::pointInQuad
(
    const sgVec2 A,
    const sgVec2 B,
    const sgVec2 C,
    const sgVec2 D,
    const sgVec2 POINT
) const
{
    if(pointSideToLine( C, A, POINT ) >= 0.0 )
    {
        //Test the first triangle
        if( pointSideToLine( A, B, POINT ) >  0.0 &&
            pointSideToLine( B, C, POINT ) >= 0.0    )
            return QUAD_TRI_FIRST;
        return QUAD_TRI_NONE;
    }

    //Test the second triangle
    if( pointSideToLine( C, D, POINT ) > 0.0 &&
        pointSideToLine( D, A, POINT ) > 0.0     )
        return QUAD_TRI_SECOND;

    return QUAD_TRI_NONE;
}   // pointInQuad

//-----------------------------------------------------------------------------
/** findRoadSector returns in which sector on the road the position
 *  xyz is. If xyz is not on top of the road, it returns
 *  UNKNOWN_SECTOR.
 *
 *  The 'sector' could be defined as the number of the closest track
 *  segment to XYZ.
 */
int Track::findRoadSector( const sgVec3 XYZ )const
{
    /* To find in which 'sector' of the track the kart is, we use a
       'point in triangle' algorithm for each triangle in the quad
       that forms each track segment.
     */
    std::vector <SegmentTriangle> possible_segment_tris;
    const unsigned int DRIVELINE_SIZE = m_left_driveline.size();
    int triangle;
    int next;
    for( size_t i = 0; i < DRIVELINE_SIZE ; ++i )
    {
        next = i + 1 <  DRIVELINE_SIZE ? i + 1 : 0;
        triangle = pointInQuad( m_left_driveline[i],     m_right_driveline[i],
                                m_right_driveline[next], m_left_driveline[next], 
                                XYZ );

        if (triangle != QUAD_TRI_NONE)
        {
            possible_segment_tris.push_back(SegmentTriangle(i, triangle));
        }
    }

    /* Since xyz can be on more than one 2D track segment, we have to
       find on top of which one of the possible track segments it is.
     */
    const int POS_SEG_SIZE = possible_segment_tris.size();
    if( POS_SEG_SIZE == 0 )
    {
        //xyz is not on the road
        return UNKNOWN_SECTOR;
    }
    else //POS_SEG_SIZE > 1
    {
        /* To find on top of which track segment the variable xyz is,
           we get which of the possible triangles that are under xyz
           has the lower distance on the height(Y or Z) axis.
         */
        float dist;
        float near_dist = 99999;
        int nearest = QUAD_TRI_NONE;
        size_t segment;
        sgVec4 plane;

        for( int i = 0; i < POS_SEG_SIZE; ++i )
        {
             segment = possible_segment_tris[i].segment;
             next = segment + 1 < DRIVELINE_SIZE ? segment + 1 : 0;

             if( possible_segment_tris[i].triangle == QUAD_TRI_FIRST )
             {
                 sgMakePlane( plane, m_left_driveline[segment],
                     m_right_driveline[segment], m_right_driveline[next] );
             }
             else //possible_segment_tris[i].triangle == QUAD_TRI_SECOND
             {
                  sgMakePlane( plane, m_right_driveline[next],
                      m_left_driveline[next], m_left_driveline[segment] );
             }

             dist = sgHeightAbovePlaneVec3( plane, XYZ );

             /* sgHeightAbovePlaneVec3 gives a negative dist if the plane
                is on top, so we have to rule it out.

                However, for some reason there are cases where we get
                negative values for the track segment we should be on,
                so we just use the absolute values, since the track
                segment you are on will have a smaller absolute value
                of dist anyways.
              */
             if( dist > -3.5 && dist < near_dist)
             {
                 near_dist = dist;
                 nearest = i;
             }
        }

        if( nearest != QUAD_TRI_NONE )
        {
            return possible_segment_tris[nearest].segment;
        }
        else return UNKNOWN_SECTOR; //This only happens if the position is
                                    //under all the possible sectors
    }
}   // findRoadSector

//-----------------------------------------------------------------------------
/** findOutOfRoadSector finds the sector where XYZ is, but as it name
    implies, it is more accurate for the outside of the track than the
    inside, and for STK's needs the accuracy on top of the track is
    unacceptable; but if this was a 2D function, the accuracy for out
    of road sectors would be perfect.

    To find the sector we look for the closest line segment from the
    right and left drivelines, and the number of that segment will be
    the sector.

    The SIDE argument is used to speed up the function only; if we know
    that XYZ is on the left or right side of the track, we know that
    the closest driveline must be the one that matches that condition.
    In reality, the side used in STK is the one from the previous frame,
    but in order to move from one side to another a point would go
    through the middle, that is handled by findRoadSector() which doesn't
    has speed ups based on the side.

    NOTE: This method of finding the sector outside of the road is *not*
    perfect: if two line segments have a similar altitude (but enough to
    let a kart get through) and they are very close on a 2D system,
    if a kart is on the air it could be closer to the top line segment
    even if it is supposed to be on the sector of the lower line segment.
    Probably the best solution would be to construct a quad that reaches
    until the next higher overlapping line segment, and find the closest
    one to XYZ.
 */
int Track::findOutOfRoadSector
(
    const sgVec3 XYZ,
    const RoadSide SIDE,
    const int CURR_SECTOR
) const
{
    int sector = UNKNOWN_SECTOR;
    float dist;
    float nearest_dist = 99999;
    const unsigned int DRIVELINE_SIZE = m_left_driveline.size();

    int begin_sector = 0;
    int end_sector = DRIVELINE_SIZE - 1;

    if(CURR_SECTOR != UNKNOWN_SECTOR )
    {
        const int LIMIT = 10; //The limit prevents shortcuts
        if( CURR_SECTOR - LIMIT < 0 )
        {
            begin_sector = DRIVELINE_SIZE - 1 + CURR_SECTOR - LIMIT;
        }
        else begin_sector = CURR_SECTOR - LIMIT;

        if( CURR_SECTOR + LIMIT > (int)DRIVELINE_SIZE - 1 )
        {
            end_sector = CURR_SECTOR + LIMIT - DRIVELINE_SIZE;
        }
        else end_sector = CURR_SECTOR + LIMIT;
    }

    sgLineSegment3 line_seg;
    int next_sector;
    for (int i = begin_sector ; i != end_sector ; i = next_sector )
    {
        next_sector = i + 1 == (int)DRIVELINE_SIZE ? 0 : i + 1;

        if( SIDE != RS_RIGHT)
        {
            sgCopyVec3( line_seg.a, m_left_driveline[i] );
            sgCopyVec3( line_seg.b, m_left_driveline[next_sector] );

            dist = sgDistSquaredToLineSegmentVec3( line_seg, XYZ );

            if ( dist < nearest_dist )
            {
                nearest_dist = dist;
                sector = i ;
            }
        }

        if( SIDE != RS_LEFT )
        {
            sgCopyVec3( line_seg.a, m_right_driveline[i] );
            sgCopyVec3( line_seg.b, m_right_driveline[next_sector] );

            dist = sgDistSquaredToLineSegmentVec3( line_seg, XYZ );

            if ( dist < nearest_dist )
            {
                nearest_dist = dist;
                sector = i ;
            }
        }
    }   // for i

    return sector;
}   // findOutOfRoadSector

//-----------------------------------------------------------------------------
/** spatialToTrack() takes absolute coordinates (coordinates in OpenGL
 *  space) and transforms them into coordinates based on the track. It is
 *  for 2D coordinates, thought it can be used on 3D vectors. The y-axis
 *  of the returned vector is how much of the track the point has gone
 *  through, the x-axis is on which side of the road it is, and the z-axis
 *  contains half the width of the track at this point. The return value
 *  is p1, i.e. the first of the two driveline points between which the
 *  kart is currently located.
 */
int Track::spatialToTrack
(
    sgVec3 dst,
    const sgVec2 POS,
    const int SECTOR
) const
{
    if( SECTOR == UNKNOWN_SECTOR )
    {
        std::cerr << "WARNING: UNKNOWN_SECTOR in spatialToTrack().\n";
        return -1;
    }

    const unsigned int DRIVELINE_SIZE = m_driveline.size();
    const size_t PREV = SECTOR == 0 ? DRIVELINE_SIZE - 1 : SECTOR - 1;
    const size_t NEXT = (size_t)SECTOR+1 >= DRIVELINE_SIZE ? 0 : SECTOR + 1;

    const float DIST_PREV = sgDistanceVec2 ( m_driveline[PREV], POS );
    const float DIST_NEXT = sgDistanceVec2 ( m_driveline[NEXT], POS );

    size_t p1, p2;
    if ( DIST_NEXT < DIST_PREV )
    {
        p1 = SECTOR; p2 = NEXT;
    }
    else
    {
        p1 = PREV; p2 = SECTOR;
    }

    sgVec2 line_eqn;
    sgVec2 tmp;

    sgMake2DLine ( line_eqn, m_driveline[p1], m_driveline[p2] );

    dst[0] = sgDistToLineVec2 ( line_eqn, POS );

    sgAddScaledVec2 ( tmp, POS, line_eqn, -dst [0] );

    float dist_from_driveline_p1 = sgDistanceVec2 ( tmp, m_driveline[p1] );
    dst[1] = dist_from_driveline_p1 + m_distance_from_start[p1];
    // Set z-axis to half the width (linear interpolation between the
    // width at p1 and p2) - m_path_width is actually already half the width
    // of the track. This is used to determine if a kart is too far
    // away from the road and is therefore considered taking a shortcut.

    float fraction = dist_from_driveline_p1
                   / (m_distance_from_start[p2]-m_distance_from_start[p1]);
    dst[2] = m_path_width[p1]*(1-fraction)+fraction*m_path_width[p2];

    return p1;
}   // spatialToTrack

//-----------------------------------------------------------------------------
void Track::trackToSpatial ( sgVec3 xyz, const int SECTOR ) const
{
    sgCopyVec3 ( xyz, m_driveline [ SECTOR ] ) ;
}   // trackToSpatial

//-----------------------------------------------------------------------------
/** Determines if a kart moving from sector OLDSEC to sector NEWSEC
 *  would be taking a shortcut, i.e. if the distance is larger
 *  than a certain detla
 */
bool Track::isShortcut(const int OLDSEC, const int NEWSEC) const
{
    // If the kart was off the road, don't do any shortcuts
    if(OLDSEC==UNKNOWN_SECTOR || NEWSEC==UNKNOWN_SECTOR) return false;
    unsigned int distance_sectors = abs(OLDSEC-NEWSEC);
    // Handle 'wrap around': if the distance is more than half the 
    // number of driveline poins, assume it's a 'wrap around'
    if(2*distance_sectors > m_driveline.size())
        distance_sectors = m_driveline.size() - distance_sectors;
    return (distance_sectors>5);
}   // isShortcut

//-----------------------------------------------------------------------------
/** It's not the nicest solution to have two very similar version of a function,
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
    glColor4f ( 1.0f,1.0f,1, 0.4f) ;


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
                 "Couldn't load map '%s': no tuxkart-track node.",
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
        "driveline is " << m_right_driveline.size() << " vertex long " <<
        "and the left driveline is " << m_left_driveline.size()
        << " vertex long. Track is " << m_name << " ." << std::endl;

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
        snprintf (msg, sizeof(msg), "Can't open '%s' for reading.\n", path.c_str() ) ;
        throw std::runtime_error(msg);
    }

    int prev_sector = UNKNOWN_SECTOR;
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
            snprintf (msg, sizeof(msg), "Syntax error in '%s'\n", path.c_str() ) ;
            throw std::runtime_error(msg);
        }

        sgVec3 point;
        point[0] = x;
        point[1] = y;
        point[2] = z;

        if(prev_sector != UNKNOWN_SECTOR) prev_distance = sgDistanceVec2(
            point, line[prev_sector] );

        //1.5f was choosen because it's more or less the length of the tuxkart
        if(prev_distance < 0.0000001)
        {
            fprintf(stderr, "File %s point %d is duplicated!.\n",
                    path.c_str(), prev_sector+1);
        }
#if 0
        else if(prev_distance < 1.5f)
        {
            fprintf(stderr,"File %s point %d is too close(<1.5) to previous point.\n",
                    path.c_str(), prev_sector + 1);
        }
        if(prev_distance > 15.0f)
        {
            fprintf(stderr,"In file %s point %d is too far(>15.0) from next point at %d.\n",
                    path, prev_sector, prev_distance);
        }
#endif

        line.push_back(point);
        ++prev_sector;
        prev_distance -= 1.5f;
    }

    fclose ( fd ) ;
}

