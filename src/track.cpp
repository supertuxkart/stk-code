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
#include <plib/ssgAux.h>
#include "file_manager.hpp"
#include "loader.hpp"
#include "track.hpp"
#include "string_utils.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "stk_config.hpp"
#include "translation.hpp"
#include "scene.hpp"
#include "moving_physics.hpp"
#include "world.hpp"
#include "material_manager.hpp"
#include "isect.hpp"
#include "ssg_help.hpp"
#include "user_config.hpp"
#include "herring.hpp"
#include "herring_manager.hpp"
#include "sound_manager.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

const float Track::NOHIT           = -99999.9f;
const int   Track::QUAD_TRI_NONE   = -1;
const int   Track::QUAD_TRI_FIRST  =  1;
const int   Track::QUAD_TRI_SECOND =  2;
const int   Track::UNKNOWN_SECTOR  = -1;

// ----------------------------------------------------------------------------
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
/** Removes the physical body from the world.
 *  Called at the end of a race.
 */
void Track::cleanup()
{
    delete m_non_collision_mesh;
    delete m_track_mesh;

    // remove temporary materials loaded by the material manager
    material_manager->popTempMaterial();
}   // cleanup

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
void Track::findRoadSector( const sgVec3 XYZ, int *sector )const
{
    if(*sector!=UNKNOWN_SECTOR)
    {
        int next = (unsigned)(*sector) + 1 <  m_left_driveline.size() ? *sector + 1 : 0;
        if(pointInQuad( m_left_driveline[*sector], m_right_driveline[*sector],
                        m_right_driveline[next],   m_left_driveline[next], 
                        XYZ ) != QUAD_TRI_NONE)
            // Still in the same sector, no changes
            return;
    }
    /* To find in which 'sector' of the track the kart is, we use a
       'point in triangle' algorithm for each triangle in the quad
       that forms each track segment.
     */
    std::vector <SegmentTriangle> possible_segment_tris;
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_left_driveline.size();
    int triangle;
    int next;

    for( size_t i = 0; i < DRIVELINE_SIZE ; ++i )
    {
        next = (unsigned int)i + 1 <  DRIVELINE_SIZE ? (int)i + 1 : 0;
        triangle = pointInQuad( m_left_driveline[i],     m_right_driveline[i],
                                m_right_driveline[next], m_left_driveline[next], 
                                XYZ );

        if (triangle != QUAD_TRI_NONE && ((XYZ[2]-m_left_driveline[i][2]) < 1.0f))
        {
            possible_segment_tris.push_back(SegmentTriangle((int)i, triangle));
        }
    }

    /* Since xyz can be on more than one 2D track segment, we have to
       find on top of which one of the possible track segments it is.
     */
    const int POS_SEG_SIZE = (int)possible_segment_tris.size();
    if( POS_SEG_SIZE == 0 )
    {
        //xyz is not on the road
        *sector = UNKNOWN_SECTOR;
        return;
    }

    //POS_SEG_SIZE > 1
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
        next = segment + 1 < DRIVELINE_SIZE ? (int)segment + 1 : 0;
        
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
           negative values for the track segment we should be on.
        */
        if( dist > -2.0 && dist < near_dist)
        {
            near_dist = dist;
            nearest = i;
        }
    }
    
    if( nearest != QUAD_TRI_NONE )
    {
        *sector=possible_segment_tris[nearest].segment;
        return;
    }
    *sector = UNKNOWN_SECTOR;
    return;                         // This only happens if the position is
                                    // under all the possible sectors
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
    //FIXME: it can happen that dist is bigger than nearest_dist for all the
    //the points we check (currently a limit of +/- 10), and if so, the
    //function will return UNKNOWN_SECTOR, and if the AI get this, it will
    //trigger an assertion. I increased the nearest_dist default value from
    //99999 to 9999999, which is a lot more than the situation that caused
    //the discovery of this problem, but the best way to solve this, is to
    //find a better way of handling the shortcuts, and maybe a better way of
    //calculating the distance.
    float nearest_dist = 9999999;
    const unsigned int DRIVELINE_SIZE = (unsigned int)m_left_driveline.size();

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

    const unsigned int DRIVELINE_SIZE = (unsigned int)m_driveline.size();
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

    sgVec3 line_eqn;
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

    return (int)p1;
}   // spatialToTrack

//-----------------------------------------------------------------------------
void Track::trackToSpatial ( sgVec3 xyz, const int SECTOR ) const
{
    sgCopyVec3 ( xyz, m_driveline [ SECTOR ] ) ;
}   // trackToSpatial

//-----------------------------------------------------------------------------
/** Returns the start coordinates for a kart on a given position pos
    (with 0<=pos).
 */
void Track::getStartCoords(unsigned int pos, sgCoord* coords) const {
  // Bug fix/workaround: sometimes the first kart would be too close
  // to the first driveline point and not to the last one -->
  // This kart would not get any lap counting done in the first
  // lap! Therefor -1.5 is subtracted from the y position - which
  // is a somewhat arbitrary value.
  coords->xyz[0] = pos<m_start_x.size() ? m_start_x[pos] : ((pos%2==0)?1.5f:-1.5f);
  coords->xyz[1] = pos<m_start_y.size() ? m_start_y[pos] : -1.5f*pos-1.5f;
   // height must be larger than the actual hight for which hot is computed.
  coords->xyz[2] = pos<m_start_z.size() ? m_start_z[pos] : 1.0f;

  coords->hpr[0] = pos<m_start_heading.size() ? m_start_heading[pos] : 0.0f;
  coords->hpr[1] = 0.0f;
  coords->hpr[2] = 0.0f;

  btVector3 tmp_pos(coords->xyz[0],coords->xyz[1],coords->xyz[2]);

  btVector3 normal;
  const Material *material=NULL;
  getTerrainInfo(tmp_pos, &(coords->xyz[2]), &normal, &material);

}   // getStartCoords

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
    if(2*distance_sectors > (unsigned int)m_driveline.size())
        distance_sectors = (unsigned int)m_driveline.size() - distance_sectors;
    return (distance_sectors>stk_config->m_shortcut_segments);
}   // isShortcut

//-----------------------------------------------------------------------------
void Track::addDebugToScene(int type) const
{
    if(type&1)
    {
        ssgaSphere *sphere;
        sgVec3 center;
        sgVec4 colour;
        for(unsigned int i = 0; i < m_driveline.size(); ++i)
        {
            sphere = new ssgaSphere;
            sgCopyVec3(center, m_driveline[i]);
            sphere->setCenter(center);
            sphere->setSize(getWidth()[i] / 4.0f);
        
            if(i == 0)
            {
                colour[0] = colour[2] = colour[3] = 255;
                colour[1] = 0;
            }
            else
            {
                colour[0] = colour[1] = colour[3] = 255;
                colour[2] = 0;
            }
            sphere->setColour(colour);
            scene->add(sphere);
        }   // for i
    }  /// type ==1
    if(type&2)
    {
        ssgVertexArray* v_array = new ssgVertexArray();
        ssgColourArray* c_array = new ssgColourArray();
        for(unsigned int i = 0; i < m_driveline.size(); i++)
        {
            int ip1 = i==m_driveline.size()-1 ? 0 : i+1;
            // The segment display must be slightly higher than the
            // track, otherwise it's not clearly visible.
            sgVec3 v;
            sgCopyVec3(v,m_left_driveline [i  ]); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,m_right_driveline[i  ]); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,m_right_driveline[ip1]); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,m_left_driveline [ip1]); v[2]+=0.1f; v_array->add(v);
            sgVec4 vc;
            vc[0] = i%2==0 ? 1.0f : 0.0f;
            vc[1] = 1.0f-v[0];
            vc[2] = 0.0f;
            vc[3] = 0.1f;
            c_array->add(vc);c_array->add(vc);c_array->add(vc);c_array->add(vc);
        }   // for i
        // if GL_QUAD_STRIP is used, the colours are smoothed, so the changes
        // from one segment to the next are not visible.
        ssgVtxTable* l = new ssgVtxTable(GL_QUADS, v_array,
                                         (ssgNormalArray*)NULL,
                                         (ssgTexCoordArray*)NULL,
                                         c_array);
        scene->add(l);
    }
}   // addDebugToScene

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
    sgVec2 sc;
    sgSubVec2 ( sc, m_driveline_max, m_driveline_min );

    float sx = w / sc[0];
    float sy = h / sc[1];

    if( sx > sy )
    {
        sx = sy;
        x += w/2 - sc[0]*sx/2;
    }
    else
    {
        sy = sx;
    }

    const unsigned int DRIVELINE_SIZE = (unsigned int)m_driveline.size();

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

    const unsigned int DRIVELINE_SIZE = (unsigned int)m_driveline.size();

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
    m_fog_density               = 1.0f/100.0f;
    m_fog_start                 = 0.0f;
    m_fog_end                   = 1000.0f;
    m_gravity                   = 9.80665f;
    m_AI_angle_adjustment       = 1.0f;
    m_AI_curve_speed_adjustment = 1.0f;

    sgSetVec3 ( m_sun_position,  0.4f, 0.4f, 0.4f      );
    sgSetVec4 ( m_sky_color,     0.3f, 0.7f, 0.9f, 1.0f );
    sgSetVec4 ( m_fog_color,     0.3f, 0.7f, 0.9f, 1.0f );
    sgSetVec4 ( m_ambient_col,   0.5f, 0.5f, 0.5f, 1.0f );
    sgSetVec4 ( m_specular_col,  1.0f, 1.0f, 1.0f, 1.0f );
    sgSetVec4 ( m_diffuse_col,   1.0f, 1.0f, 1.0f, 1.0f );

    lisp::Parser parser;
    const lisp::Lisp* const ROOT = parser.parse(m_filename);

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

    LISP->get      ("name",                  m_name);
    LISP->get      ("description",           m_description);
    std::vector<std::string> filenames;
    LISP->getVector("music",                 filenames);
    getMusicInformation(filenames, m_music);
    LISP->get      ("herring",               m_herring_style);
    LISP->get      ("screenshot",            m_screenshot);
    LISP->get      ("topview",               m_top_view);
    LISP->get      ("sky-color",             m_sky_color);
    LISP->getVector("start-x",               m_start_x);
    LISP->getVector("start-y",               m_start_y);
    LISP->getVector("start-z",               m_start_z);
    LISP->getVector("start-heading",         m_start_heading);
    LISP->get      ("use-fog",               m_use_fog);
    LISP->get      ("fog-color",             m_fog_color);
    LISP->get      ("fog-density",           m_fog_density);
    LISP->get      ("fog-start",             m_fog_start);
    LISP->get      ("fog-end",               m_fog_end);
    LISP->get      ("sun-position",          m_sun_position);
    LISP->get      ("sun-ambient",           m_ambient_col);
    LISP->get      ("sun-specular",          m_specular_col);
    LISP->get      ("sun-diffuse",           m_diffuse_col);
    LISP->get      ("gravity",               m_gravity);
    LISP->get      ("AI-angle-adjust",       m_AI_angle_adjustment);
    LISP->get      ("AI-curve-speed-adjust", m_AI_curve_speed_adjustment);

    // Set the correct paths
    m_screenshot = file_manager->getTrackFile(m_screenshot, getIdent());
    m_top_view   = file_manager->getTrackFile(m_top_view,   getIdent());
    
    delete ROOT;
}   // loadTrack

//-----------------------------------------------------------------------------
void Track::getMusicInformation(std::vector<std::string>&             filenames, 
                                std::vector<MusicInformation const *>& music    )
{
    for(int i=0; i<(int)filenames.size(); i++)
    {
        std::string full_path = file_manager->getTrackFile(filenames[i], getIdent());
        const MusicInformation* mi;
        try
        {
            mi = sound_manager->getMusicInformation(full_path);
        }
        catch(std::runtime_error)
        {
            mi = sound_manager->getMusicInformation(file_manager->getMusicFile(filenames[i]));
        }
        if(!mi)
        {
            fprintf(stderr, "Music information file '%s' not found - ignored.\n",
                    filenames[i].c_str());
            continue;
        }
        m_music.push_back(mi);
    }   // for i in filenames
}   // getMusicInformation

//-----------------------------------------------------------------------------
void Track::playMusic() const {
    sound_manager->playMusic(m_music[rand()% m_music.size()]);
}   // getMusic

//-----------------------------------------------------------------------------
void
Track::loadDriveline()
{
    readDrivelineFromFile(m_left_driveline, ".drvl");

    const unsigned int DRIVELINE_SIZE = (unsigned int)m_left_driveline.size();
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
        //Both drivelines must be checked to get the true size of
        //the drivelines, and using the center driveline is not
        //good enough.
        if ( m_right_driveline[i][0] < m_driveline_min[0] )
            m_driveline_min[0] = m_right_driveline[i][0] ;
        if ( m_right_driveline[i][1] < m_driveline_min[1] )
            m_driveline_min[1] = m_right_driveline[i][1] ;
        if ( m_right_driveline[i][0] > m_driveline_max[0] )
            m_driveline_max[0] = m_right_driveline[i][0] ;
        if ( m_right_driveline[i][1] > m_driveline_max[1] )
            m_driveline_max[1] = m_right_driveline[i][1] ;

        if ( m_left_driveline[i][0] < m_driveline_min[0] )
            m_driveline_min[0] = m_left_driveline[i][0] ;
        if ( m_left_driveline[i][1] < m_driveline_min[1] )
            m_driveline_min[1] = m_left_driveline[i][1] ;
        if ( m_left_driveline[i][0] > m_driveline_max[0] )
            m_driveline_max[0] = m_left_driveline[i][0] ;
        if ( m_left_driveline[i][1] > m_driveline_max[1] )
            m_driveline_max[1] = m_left_driveline[i][1] ;


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
    std::string path = file_manager->getTrackFile(m_ident+file_ext);
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
}   // readDrivelineFromFile

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Track::createPhysicsModel()
{
    if(!m_model) return;

    m_track_mesh         = new TriangleMesh();
    m_non_collision_mesh = new TriangleMesh();
    
    // Collect all triangles in the track_mesh
    sgMat4 mat;
    sgMakeIdentMat4(mat);
    convertTrackToBullet(m_model, mat);
    m_track_mesh->createBody();
    m_non_collision_mesh->createBody(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    
}   // createPhysicsModel

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Track::convertTrackToBullet(ssgEntity *track, sgMat4 m)
{
    if(!track) return;
    MovingPhysics *mp = dynamic_cast<MovingPhysics*>(track);
    if(mp)
    {
        // If the track contains obect of type MovingPhysics,
        // these objects will be real rigid body and are already
        // part of the world. So these objects must not be converted
        // to triangle meshes.
    } 
    else if(track->isAKindOf(ssgTypeLeaf()))
    {
        ssgLeaf  *leaf     = (ssgLeaf*)(track);
        Material *material = material_manager->getMaterial(leaf);
        // Don't convert triangles with material that is ignored (e.g. fuzzy_sand)
        if(!material || material->isIgnore()) return;

        for(int i=0; i<leaf->getNumTriangles(); i++) 
        {
            short v1,v2,v3;
            sgVec3 vv1, vv2, vv3;
            
            leaf->getTriangle(i, &v1, &v2, &v3);
            sgXformPnt3 ( vv1, leaf->getVertex(v1), m );
            sgXformPnt3 ( vv2, leaf->getVertex(v2), m );
            sgXformPnt3 ( vv3, leaf->getVertex(v3), m );
            btVector3 vb1(vv1[0],vv1[1],vv1[2]);
            btVector3 vb2(vv2[0],vv2[1],vv2[2]);
            btVector3 vb3(vv3[0],vv3[1],vv3[2]);
            if(material->isZipper()) 
            {
                m_non_collision_mesh->addTriangle(vb1, vb2, vb3, material);
            }
            else
            {
                m_track_mesh->addTriangle(vb1, vb2, vb3, material);
            }
        }
        
    }   // if(track isAKindOf leaf)
    else if(track->isAKindOf(ssgTypeTransform()))
    {
        ssgBaseTransform *t = (ssgBaseTransform*)(track);
        sgMat4 tmpT, tmpM;
        t->getTransform(tmpT);
        sgCopyMat4(tmpM, m);
        sgPreMultMat4(tmpM,tmpT);
        for(ssgEntity *e = t->getKid(0); e!=NULL; e=t->getNextKid())
        {
            convertTrackToBullet(e, tmpM);
        }   // for i
    }
    else if (track->isAKindOf(ssgTypeBranch())) 
    {
        ssgBranch *b =(ssgBranch*)track;
        for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid()) {
            convertTrackToBullet(e, m);
        }   // for i<getNumKids
    }
    else
    {
        assert(!"Unkown ssg type in convertTrackToBullet");
    }
}   // convertTrackToBullet

// ----------------------------------------------------------------------------
void Track::loadTrackModel()
{
    // Add the track directory to the texture search path
    file_manager->pushTextureSearchPath(file_manager->getTrackFile("",getIdent()));
    file_manager->pushModelSearchPath  (file_manager->getTrackFile("",getIdent()));
    // First read the temporary materials.dat file if it exists
    try
    {
        std::string materials_file = file_manager->getTrackFile("materials.dat",getIdent());
        material_manager->pushTempMaterial(materials_file);
    }
    catch (std::exception& e)
    {
        // no temporary materials.dat file, ignore
        (void)e;
    }
    std::string path = file_manager->getTrackFile(getIdent()+".loc");

    FILE *fd = fopen (path.c_str(), "r" );
    if ( fd == NULL )
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg),"Can't open track location file '%s'.\n",
                 path.c_str());
        throw std::runtime_error(msg);
    }

    // Start building the scene graph
    m_model = new ssgBranch ;
    scene->add(m_model);

    char s [ 1024 ] ;

    while ( fgets ( s, 1023, fd ) != NULL )
    {
        if ( *s == '#' || *s < ' ' )
            continue ;

        int need_hat = false ;
        int fit_skin = false ;
        char fname [ 1024 ] ;
        sgCoord loc ;
        sgZeroVec3 ( loc.xyz ) ;
        sgZeroVec3 ( loc.hpr ) ;

        char htype = '\0' ;

        if ( sscanf ( s, "%cHERRING,%f,%f,%f", &htype,
                      &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 4 )
        {
            herring_command(&loc.xyz, htype, false) ;
        }
        else if ( sscanf ( s, "%cHERRING,%f,%f", &htype,
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
        {
            herring_command (&loc.xyz, htype, true) ;
        }
        else if ( s[0] == '\"' )
        {
            if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f,%f,%f",
                          fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
                          &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2]) ) == 7 )
            {
                /* All 6 DOF specified */
                need_hat = false;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]), &(loc.hpr[1]), &(loc.hpr[2])) == 6 )
            {
                /* All 6 DOF specified - but need height */
                need_hat = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]),
                               &(loc.hpr[0]) ) == 5 )
            {
                /* No Roll/Pitch specified - assumed zero */
                need_hat = false ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f,{},{}",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]) ) == 4 )
            {
                /* All 6 DOF specified - but need height, roll, pitch */
                need_hat = true ;
                fit_skin = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{},%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.hpr[0]) ) == 4 )
            {
                /* No Roll/Pitch specified - but need height */
                need_hat = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]),
                               &(loc.xyz[2]) ) == 4 )
            {
                /* No Heading/Roll/Pitch specified - but need height */
                need_hat = false ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f,{}",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
            {
                /* No Roll/Pitch specified - but need height */
                need_hat = true ;
            }
            else if ( sscanf ( s, "\"%[^\"]\",%f,%f",
                               fname, &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
            {
                /* No Z/Heading/Roll/Pitch specified */
                need_hat = false ;
            }
            else if ( sscanf ( s, "\"%[^\"]\"", fname ) == 1 )
            {
                /* Nothing specified */
                need_hat = false ;
            }
            else
            {
                fclose(fd);
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), "Syntax error in '%s': %s",
                         path.c_str(), s);
                throw std::runtime_error(msg);
            }

            if ( need_hat )
            {
                sgVec3 nrm ;

                loc.xyz[2] = 1000.0f ;
                loc.xyz[2] = getHeightAndNormal ( m_model, loc.xyz, nrm ) ;

                if ( fit_skin )
                {
                    float sy = sin ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
                    float cy = cos ( -loc.hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;

                    loc.hpr[2] =  SG_RADIANS_TO_DEGREES * atan2 ( nrm[0] * cy -
                                  nrm[1] * sy, nrm[2] ) ;
                    loc.hpr[1] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[1] * cy +
                                 nrm[0] * sy, nrm[2] ) ;
                }
            }   // if need_hat

            ssgEntity        *obj   = loader->load(file_manager->getModelFile(fname),
                                                   CB_TRACK,
                                                   /* optimise   */  true,
                                                   /*is_full_path*/  true);
            if(!obj)
            {
                fclose(fd);
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), "Can't open track model '%s'",fname);
                file_manager->popTextureSearchPath();
                file_manager->popModelSearchPath  ();
                throw std::runtime_error(msg);
            }
            createDisplayLists(obj);
            ssgRangeSelector *lod   = new ssgRangeSelector ;
            ssgTransform     *trans = new ssgTransform ( & loc ) ;

            float r [ 2 ] = { -10.0f, 2000.0f } ;

            lod    -> addKid(obj   );
            trans  -> addKid(lod   );
            m_model-> addKid(trans );
            lod    -> setRanges(r, 2);
            if(user_config->m_track_debug)
                addDebugToScene(user_config->m_track_debug);

        }
        else
        {
//            fclose(fd);
//            char msg[MAX_ERROR_MESSAGE_LENGTH];
//            snprintf(msg, sizeof(msg), "Syntax error in '%s': %s",
            fprintf(stderr, "Warning: Syntax error in '%s': %s",
                     path.c_str(), s);
//            throw std::runtime_error(msg);
        }
    }   // while fgets

    fclose ( fd ) ;
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath  ();

    createPhysicsModel();
}   // loadTrack

//-----------------------------------------------------------------------------
void Track::herring_command (sgVec3 *xyz, char htype, int bNeedHeight )
{

    // if only 2d coordinates are given, let the herring fall from very heigh
    if(bNeedHeight) (*xyz)[2] = 1000000.0f;

    // Even if 3d data are given, make sure that the herring is on the ground
    (*xyz)[2] = getHeight ( m_model, *xyz ) + 0.06f;
    herringType type=HE_GREEN;
    if ( htype=='Y' || htype=='y' ) { type = HE_GOLD   ;}
    if ( htype=='G' || htype=='g' ) { type = HE_GREEN  ;}
    if ( htype=='R' || htype=='r' ) { type = HE_RED    ;}
    if ( htype=='S' || htype=='s' ) { type = HE_SILVER ;}
    herring_manager->newHerring(type, xyz);
}   // herring_command

// ----------------------------------------------------------------------------
void  Track::getTerrainInfo(btVector3 &pos, float *hot, btVector3* normal, 
                            const Material **material) const
{
    btVector3 to_pos(pos);
    to_pos.setZ(-100000.f);

    class MaterialCollision : public btCollisionWorld::ClosestRayResultCallback
    {
    public:
        const Material* m_material;
        MaterialCollision(btVector3 p1, btVector3 p2) : 
            btCollisionWorld::ClosestRayResultCallback(p1,p2) {m_material=NULL;}
        virtual btScalar AddSingleResult(btCollisionWorld::LocalRayResult& rayResult,
                                         bool normalInWorldSpace) {
             if(rayResult.m_localShapeInfo && rayResult.m_localShapeInfo->m_shapePart>=0 )
             {
                 m_material = ((TriangleMesh*)rayResult.m_collisionObject->getUserPointer())->getMaterial(rayResult.m_localShapeInfo->m_triangleIndex);
             }
             return btCollisionWorld::ClosestRayResultCallback::AddSingleResult(rayResult, 
                                                                                normalInWorldSpace);
        }   // AddSingleResult
    };   // myCollision
    MaterialCollision rayCallback(pos, to_pos);
    world->getPhysics()->getPhysicsWorld()->rayTest(pos, to_pos, rayCallback);

    if(!rayCallback.HasHit()) 
    {
        *hot      = NOHIT;
        *material = NULL;
        return;
    }

    *hot      = rayCallback.m_hitPointWorld.getZ();
    *normal   = rayCallback.m_hitNormalWorld;
    *material = rayCallback.m_material;
    // FIXME: material must be set!
}   // getTerrainInfo
