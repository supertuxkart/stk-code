//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "track.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>
#define _WINSOCKAPI_
#include <plib/ssgAux.h>
#include "irrlicht.h"

#include "stk_config.hpp"
#include "material_manager.hpp"
#include "callback_manager.hpp"
#include "isect.hpp"
#include "user_config.hpp"
#include "audio/sound_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/scene.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "modes/world.hpp"
#include "physics/moving_physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "race_manager.hpp"
#include "utils/ssg_help.hpp"
#include "utils/string_utils.hpp"

const float Track::NOHIT           = -99999.9f;
const int   Track::QUAD_TRI_NONE   = -1;
const int   Track::QUAD_TRI_FIRST  =  1;
const int   Track::QUAD_TRI_SECOND =  2;
const int   Track::UNKNOWN_SECTOR  = -1;

// ----------------------------------------------------------------------------
Track::Track( std::string filename_, float w, float h, bool stretch )
{
    m_filename           = filename_;
    m_item_style         = "";
    m_track_2d_width     = w;
    m_track_2d_height    = h;
    m_do_stretch         = stretch;
    m_description        = "";
    m_designer           = "";
    m_screenshot         = "";
    m_version            = 0;
    m_track_mesh         = new TriangleMesh();
    m_non_collision_mesh = new TriangleMesh();
    m_all_nodes.clear();
    m_all_meshes.clear();
    m_has_final_camera = false;
    m_is_arena         = false;
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
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        irr_driver->removeNode(m_all_nodes[i]);
    }
    for(unsigned int i=0; i<m_all_meshes.size(); i++)
    {
        irr_driver->removeMesh(m_all_meshes[i]);
    }

    delete m_non_collision_mesh;
    delete m_track_mesh;

    // remove temporary materials loaded by the material manager
    material_manager->popTempMaterial();
}   // cleanup

//-----------------------------------------------------------------------------
/** Finds on which side of the line segment a given point is.
 */
inline float Track::pointSideToLine( const Vec3& L1, const Vec3& L2,
    const Vec3& P ) const
{
    return ( L2.getX()-L1.getX() )*( P.getY()-L1.getY() )-( L2.getY()-L1.getY() )*( P.getX()-L1.getX() );
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
    const Vec3& A,
    const Vec3& B,
    const Vec3& C,
    const Vec3& D,
    const Vec3& POINT
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
 *  \param XYZ Position for which the segment should be determined.
 *  \param sector Contains the previous sector (as a shortcut, since usually
 *         the sector is the same as the last one), and on return the result
 *  \param with_tolerance If true, the drivelines with tolerance are used.
 *         This reduces the impact of driving slightly off road.
 */
void Track::findRoadSector(const Vec3& XYZ, int *sector, 
                           bool with_tolerance )const
{
    if(*sector!=UNKNOWN_SECTOR)
    {
        int next = (unsigned)(*sector) + 1 <  m_left_driveline.size() ? *sector + 1 : 0;
        if(with_tolerance)
        {
            if(pointInQuad(m_dl_with_tolerance_left[*sector],
                           m_dl_with_tolerance_right[*sector],
                           m_dl_with_tolerance_right[next],
                           m_dl_with_tolerance_left[next], 
                           XYZ                                ) != QUAD_TRI_NONE)
                // Still in the same sector, no changes
                return;
        }
        else
        {
            if(pointInQuad(m_left_driveline[*sector],
                           m_right_driveline[*sector],
                           m_right_driveline[next],   
                           m_left_driveline[next], XYZ ) != QUAD_TRI_NONE)
                // Still in the same sector, no changes
                return;
        }
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
        triangle = with_tolerance 
                ? pointInQuad(m_dl_with_tolerance_left[i], 
                              m_dl_with_tolerance_right[i],
                              m_dl_with_tolerance_right[next],
                              m_dl_with_tolerance_left[next], XYZ )
                 : pointInQuad(m_left_driveline[i], m_right_driveline[i],
                               m_right_driveline[next], m_left_driveline[next],
                               XYZ );

        if (triangle != QUAD_TRI_NONE && ((XYZ.getZ()-m_left_driveline[i].getZ()) < 1.0f))
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
        
        // Note: we can make the plane with the normal driveliens
        // (not the one with tolerance), since the driveliens with
        // tolerance lie in the same plane.
        if( possible_segment_tris[i].triangle == QUAD_TRI_FIRST )
        {
            sgMakePlane( plane, m_left_driveline[segment].toFloat(),
                                m_right_driveline[segment].toFloat(), 
                                m_right_driveline[next].toFloat() );
        }
        else //possible_segment_tris[i].triangle == QUAD_TRI_SECOND
        {
            sgMakePlane( plane, m_right_driveline[next].toFloat(),
                                m_left_driveline[next].toFloat(),
                                m_left_driveline[segment].toFloat() );
        }
        
        dist = sgHeightAbovePlaneVec3( plane, XYZ.toFloat() );
        
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
    const Vec3& XYZ,
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
    const int DRIVELINE_SIZE = (int)m_left_driveline.size();

    int begin_sector = 0;
    int count      = DRIVELINE_SIZE;
    if(CURR_SECTOR != UNKNOWN_SECTOR )
    {
        const int LIMIT = 10; //The limit prevents shortcuts
        if( CURR_SECTOR - LIMIT < 0 )
        {
            begin_sector = DRIVELINE_SIZE - 1 + CURR_SECTOR - LIMIT;
        }
        else begin_sector = CURR_SECTOR - LIMIT;
        count = 2*LIMIT;
    }

    sgLineSegment3 line_seg;
    int next_sector;
    for(int j=0; j<count; j++)
    {
        next_sector  = begin_sector+1 == DRIVELINE_SIZE ? 0 : begin_sector+1;

        if( SIDE != RS_RIGHT)
        {
            sgCopyVec3( line_seg.a, m_left_driveline[begin_sector].toFloat() );
            sgCopyVec3( line_seg.b, m_left_driveline[next_sector].toFloat() );
            dist = sgDistSquaredToLineSegmentVec3( line_seg, XYZ.toFloat() );
            if ( dist < nearest_dist )
            {
                nearest_dist = dist;
                sector       = begin_sector;
            }
        }   // SIDE != RS_RIGHT

        if( SIDE != RS_LEFT )
        {
            sgCopyVec3( line_seg.a, m_right_driveline[begin_sector].toFloat() );
            sgCopyVec3( line_seg.b, m_right_driveline[next_sector].toFloat() );
            dist = sgDistSquaredToLineSegmentVec3( line_seg, XYZ.toFloat() );
            if ( dist < nearest_dist )
            {
                nearest_dist = dist;
                sector       = begin_sector;
            }
        }   // SIDE != RS_LEFT
        begin_sector = next_sector;
    }   // for j

    if(sector==UNKNOWN_SECTOR || sector >=DRIVELINE_SIZE)
    {
        printf("unknown sector found.\n");
    }
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
    Vec3& dst, /* out */
    const Vec3& POS,
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

    const float DIST_PREV = (m_driveline[PREV]-POS).length2_2d();
    const float DIST_NEXT = (m_driveline[NEXT]-POS).length2_2d();

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

    sgMake2DLine ( line_eqn, m_driveline[p1].toFloat(), m_driveline[p2].toFloat() );

    dst.setX(sgDistToLineVec2 ( line_eqn, POS.toFloat() ) );

    sgAddScaledVec2 ( tmp, POS.toFloat(), line_eqn, -dst.getX() );

    float dist_from_driveline_p1 = sgDistanceVec2 ( tmp, m_driveline[p1].toFloat() );
    dst.setY(dist_from_driveline_p1 + m_distance_from_start[p1]);
    // Set z-axis to half the width (linear interpolation between the
    // width at p1 and p2) - m_path_width is actually already half the width
    // of the track. This is used to determine if a kart is too far
    // away from the road and is therefore considered taking a shortcut.

    float fraction = dist_from_driveline_p1
                   / (m_distance_from_start[p2]-m_distance_from_start[p1]);
    dst.setZ(m_path_width[p1]*(1-fraction)+fraction*m_path_width[p2]);

    return (int)p1;
}   // spatialToTrack

//-----------------------------------------------------------------------------
const Vec3& Track::trackToSpatial(const int SECTOR ) const
{
    return m_driveline[SECTOR];
}   // trackToSpatial

//-----------------------------------------------------------------------------
/** Returns the start coordinates for a kart on a given position pos
    (with pos ranging from 0 to kart_num-1).
*/
btTransform Track::getStartTransform(unsigned int pos) const
{

    Vec3 orig;
    
    if(isArena())
    {
        assert(pos < m_start_positions.size());
        orig.setX( m_start_positions[pos][0] );
        orig.setY( m_start_positions[pos][1] );
        orig.setZ( m_start_positions[pos][2] );
    }
    else
    {
        // sometimes the first kart would be too close
        // to the first driveline point and not to the last one -->
        // This kart would not get any lap counting done in the first
        // lap! Therefore an offset is substracted from its Y location,
        // and this offset is calculated based on the drivelines
        float offset = 1.5f;
        if(m_left_driveline[0].getY() > 0 || m_right_driveline[0].getY() > 0)
            offset += std::max(m_left_driveline[0].getY(), m_left_driveline[0].getY());
        
        orig.setX( pos<m_start_x.size() ? m_start_x[pos] : ((pos%2==0)?1.5f:-1.5f) );
        orig.setY( pos<m_start_y.size() ? m_start_y[pos] : -1.5f*pos-offset        );
        orig.setZ( pos<m_start_z.size() ? m_start_z[pos] : 1.0f                    );
    }
    btTransform start;
    start.setOrigin(orig);
    start.setRotation(btQuaternion(btVector3(0, 0, 1), 
                                   pos<m_start_heading.size() 
                                   ? DEGREE_TO_RAD(m_start_heading[pos]) 
                                   : 0.0f ));
    return start;
}   // getStartTransform

//-----------------------------------------------------------------------------
/** Determines if a kart moving from sector OLDSEC to sector NEWSEC
 *  would be taking a shortcut, i.e. if the distance is larger
 *  than a certain delta
 */
bool Track::isShortcut(const int OLDSEC, const int NEWSEC) const
{
    // If the kart was off the road, don't do any shortcuts
    if(OLDSEC==UNKNOWN_SECTOR || NEWSEC==UNKNOWN_SECTOR) return false;
    int next_sector = OLDSEC==(int)m_driveline.size()-1 ? 0 : OLDSEC+1;
    if(next_sector==NEWSEC) 
        return false;

    int distance_sectors = (int)(m_distance_from_start[std::max(NEWSEC, OLDSEC)] -
                                 m_distance_from_start[std::min(NEWSEC, OLDSEC)]  );
    
    // Handle 'warp around'
    const int track_length = (int)m_distance_from_start[m_driveline.size()-1];
    if( distance_sectors < 0 ) distance_sectors += track_length;
    //else if( distance_sectors > track_length*3.0f/4.0f) distance_sectors -= track_length;
    
    if(std::max(NEWSEC, OLDSEC) > (int)RaceManager::getTrack()->m_distance_from_start.size()-6 &&
       std::min(NEWSEC, OLDSEC) < 6) distance_sectors -= track_length; // crossed start line
    
    return (distance_sectors > stk_config->m_shortcut_length);
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
            sgCopyVec3(center, m_driveline[i].toFloat());
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
            stk_scene->add(sphere);
        }   // for i
    }  /// type ==1
    // 2: drivelines, 4: driveline with tolerance
    if(type&6)
    {
        ssgVertexArray* v_array = new ssgVertexArray();
        ssgColourArray* c_array = new ssgColourArray();
        const std::vector<Vec3> &left  = type&2 ? m_left_driveline 
                                                : m_dl_with_tolerance_left;
        const std::vector<Vec3> &right = type&2 ? m_right_driveline 
                                               : m_dl_with_tolerance_right;
        for(unsigned int i = 0; i < m_driveline.size(); i++)
        {
            int ip1 = i==m_driveline.size()-1 ? 0 : i+1;
            // The segment display must be slightly higher than the
            // track, otherwise it's not clearly visible.
            sgVec3 v;
            sgCopyVec3(v,left [i  ].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,right[i  ].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,right[ip1].toFloat()); v[2]+=0.1f; v_array->add(v);
            sgCopyVec3(v,left [ip1].toFloat()); v[2]+=0.1f; v_array->add(v);
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
        stk_scene->add(l);
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
  float width = m_driveline_max.getX() - m_driveline_min.getX();

    float sx = w / width;
    float sy = h / ( m_driveline_max.getY()-m_driveline_min.getY() );

    if( sx > sy )
    {
        sx = sy;
        x += w/2 - width*sx/2;
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
      glVertex2f ( x + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * sx,
          y + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * sy) ;

      glVertex2f ( x + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * sx,
          y + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;
    }
    glVertex2f ( x + ( m_left_driveline[0].getX() - m_driveline_min.getX() ) * sx,
        y + ( m_left_driveline[0].getY() - m_driveline_min.getY() ) * sy) ;
    glVertex2f ( x + ( m_right_driveline[0].getX() - m_driveline_min.getX() ) * sx,
        y + ( m_right_driveline[0].getY() - m_driveline_min.getY() ) * sy ) ;

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
        glVertex2f ( x + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * sx,
            y + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;

        glVertex2f ( x + ( m_left_driveline[i+1].getX() - m_driveline_min.getX() ) * sx,
            y + ( m_left_driveline[i+1].getY() - m_driveline_min.getY() ) * sy ) ;


        /*Draw left driveline of the map*/
        glVertex2f ( x + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * sx,
	        y + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;

        glVertex2f ( x + ( m_right_driveline[i+1].getX() - m_driveline_min.getX() ) * sx,
	        y + ( m_right_driveline[i+1].getY() - m_driveline_min.getY() ) * sy ) ;
    }

    //Close the left driveline
    glVertex2f ( x + ( m_left_driveline[DRIVELINE_SIZE - 1].getX() - m_driveline_min.getX() ) * sx,
        y + ( m_left_driveline[DRIVELINE_SIZE - 1].getY() - m_driveline_min.getY() ) * sy ) ;

    glVertex2f ( x + ( m_left_driveline[0].getX() - m_driveline_min.getX() ) * sx,
        y + ( m_left_driveline[0].getY() - m_driveline_min.getY() ) * sy ) ;


    //Close the right driveline
    glVertex2f ( x + ( m_right_driveline[DRIVELINE_SIZE - 1].getX() - m_driveline_min.getX() ) * sx,
        y + ( m_right_driveline[DRIVELINE_SIZE - 1].getY() - m_driveline_min.getY() ) * sy ) ;

    glVertex2f ( x + ( m_right_driveline[0].getX() - m_driveline_min.getX() ) * sx,
        y + ( m_right_driveline[0].getY() - m_driveline_min.getY() ) * sy ) ;
    glEnd () ;

#if 0
  //FIXME: We are not sure if it's a videocard problem, but on Linux with a
  //Nvidia Geforce4 mx 440, we get problems with GL_LINE_LOOP;
  //If this issue is solved, using GL_LINE_LOOP is a better solution than
  //GL_LINES
    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * sx,
            y + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;
    }
    glEnd () ;

    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * sx,
	        y + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;
    }
    glEnd () ;
#endif


    glBegin ( GL_POINTS ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
      glVertex2f ( x + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * sx,
          y + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;

      glVertex2f ( x + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * sx,
          y + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * sy ) ;
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
    glColor4f ( 1.0f, 1.0f, 1.0f, 0.4f) ;


/*FIXME: Too much calculations here, we should be generating scaled driveline arrays
 * in Track::loadDriveline so all we'd be doing is pumping out predefined
 * vertexes in-game.
 */
    /*Draw white filling of the map*/
    glBegin ( GL_QUAD_STRIP ) ;

    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i ) {
      glVertex2f ( x_offset + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
          y_offset + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y) ;
      glVertex2f ( x_offset + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
          y_offset + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
    }
    glVertex2f ( x_offset + ( m_left_driveline[0].getX() - m_driveline_min.getX() ) * m_scale_x,
        y_offset + ( m_left_driveline[0].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
    glVertex2f ( x_offset + ( m_right_driveline[0].getX() - m_driveline_min.getX() ) * m_scale_x,
        y_offset + ( m_right_driveline[0].getY() - m_driveline_min.getY() ) * m_scale_y ) ;

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
        glVertex2f ( x_offset + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
            y_offset + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;

        glVertex2f ( x_offset + ( m_left_driveline[i+1].getX() - m_driveline_min.getX() ) * m_scale_x,
            y_offset + ( m_left_driveline[i+1].getY() - m_driveline_min.getY() ) * m_scale_y ) ;


        /*Draw left driveline of the map*/
        glVertex2f ( x_offset + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
	        y_offset + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;

        glVertex2f ( x_offset + ( m_right_driveline[i+1].getX() - m_driveline_min.getX() ) * m_scale_x,
	        y_offset + ( m_right_driveline[i+1].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
    }

    //Close the left driveline
    glVertex2f ( x_offset + ( m_left_driveline[DRIVELINE_SIZE - 1].getX() - m_driveline_min.getX() ) * m_scale_x,
		 y_offset + ( m_left_driveline[DRIVELINE_SIZE - 1].getY() - m_driveline_min.getY() ) * m_scale_y ) ;

    glVertex2f ( x_offset + ( m_left_driveline[0].getX() - m_driveline_min.getX() ) * m_scale_x,
        y_offset + ( m_left_driveline[0].getY() - m_driveline_min.getY() ) * m_scale_y ) ;


    //Close the right driveline
    glVertex2f ( x_offset + ( m_right_driveline[DRIVELINE_SIZE - 1].getX() - m_driveline_min.getX() ) * m_scale_x,
        y_offset + ( m_right_driveline[DRIVELINE_SIZE - 1].getY() - m_driveline_min.getY() ) * m_scale_y ) ;

    glVertex2f ( x_offset + ( m_right_driveline[0].getX() - m_driveline_min.getX() ) * m_scale_x,
        y_offset + ( m_right_driveline[0].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
    glEnd () ;

#if 0
  //FIXME: We are not sure if it's a videocard problem, but on Linux with a
  //Nvidia Geforce4 mx 440, we get problems with GL_LINE_LOOP;
  //If this issue is solved, using GL_LINE_LOOP is a better solution than
  //GL_LINES
    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x_offset + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
            y_offset + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
    }
    glEnd () ;

    glBegin ( GL_LINE_LOOP ) ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        glVertex2f ( x_offset + ( m_right_driveline[i].get() - m_driveline_min.getX() ) * m_scale_x,
	        y_offset + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
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
        glVertex2f ( x_offset + ( m_left_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
            y_offset + ( m_left_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;

        glVertex2f ( x_offset + ( m_right_driveline[i].getX() - m_driveline_min.getX() ) * m_scale_x,
	      y_offset + ( m_right_driveline[i].getY() - m_driveline_min.getY() ) * m_scale_y ) ;
    }
    glEnd () ;

    glPopAttrib();

}   // draw2Dview

//-----------------------------------------------------------------------------
void Track::loadTrack(const std::string &filename)
{
    m_filename          = filename;
    m_ident             = StringUtils::basename(
                                StringUtils::without_extension(m_filename));
    std::string path    = StringUtils::without_extension(m_filename);

    // Default values
    m_use_fog           = false;
    m_fog_density       = 1.0f/100.0f;
    m_fog_start         = 0.0f;
    m_fog_end           = 1000.0f;
    m_gravity           = 9.80665f;
    m_sun_position      = core::vector3df(0.4f, 0.4f, 0.4f);
    m_sky_color         = video::SColorf(0.3f, 0.7f, 0.9f, 1.0f);
    m_fog_color         = video::SColorf(0.3f, 0.7f, 0.9f, 1.0f);
    m_ambient_color     = video::SColorf(0.5f, 0.5f, 0.5f, 1.0f);
    m_specular_color    = video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_diffuse_color     = video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);    
    XMLReader *xml      = file_manager->getXMLReader(m_filename);
    const XMLNode *node = xml->getNode("track");
    if(!node)
    {
        std::ostringstream o;
        o<<"Can't load track '"<<filename<<"', no track element.";
        throw std::runtime_error(o.str());
    }
    node->get("name",                  &m_name);
    node->get("description",           &m_description);
    node->get("designer",              &m_designer);
    node->get("version",               &m_version);
    std::vector<std::string> filenames;
    node->get("music",                 &filenames);
    getMusicInformation(filenames, m_music);
    node->get("item",                  &m_item_style);
    node->get("screenshot",            &m_screenshot);
    node->get("item",                  &m_item_style);
    node->get("screenshot",            &m_screenshot);
    node->get("sky-color",             &m_sky_color);
    node->get("start-x",               &m_start_x);
    node->get("start-y",               &m_start_y);
    node->get("start-z",               &m_start_z);
    node->get("start-heading",         &m_start_heading);
    node->get("use-fog",               &m_use_fog);
    node->get("fog-color",             &m_fog_color);
    node->get("fog-density",           &m_fog_density);
    node->get("fog-start",             &m_fog_start);
    node->get("fog-end",               &m_fog_end);
    node->get("sun-position",          &m_sun_position);
    node->get("sun-ambient",           &m_ambient_color);
    node->get("sun-specular",          &m_specular_color);
    node->get("sun-diffuse",           &m_diffuse_color);
    node->get("gravity",               &m_gravity);
    node->get("arena",                 &m_is_arena);
    node->get("groups",                &m_groups);
    if(m_groups.size()==0)
        m_groups.push_back("standard");
    // if both camera position and rotation are defined,
    // set the flag that the track has final camera position
    m_has_final_camera  = node->get("camera-final-position", 
                                    &m_camera_final_position)!=1;
    m_has_final_camera &= node->get("camera-final-hpr",
                                    &m_camera_final_hpr)     !=1;
    m_camera_final_hpr.degreeToRad();

    m_sky_type = SKY_NONE;
    node = xml->getNode("sky-dome");
    if(node)
    {
        m_sky_type            = SKY_DOME;
        m_sky_vert_segments   = 16;
        m_sky_hori_segments   = 16;
        m_sky_sphere_percent  = 1.0f;
        m_sky_texture_percent = 1.0f;
        std::string s;
        node->get("texture",          &s                    );
        m_sky_textures.push_back(s);
        node->get("vertical",        &m_sky_vert_segments  );
        node->get("horizontal",      &m_sky_hori_segments  );
        node->get("sphere-percent",  &m_sky_sphere_percent );
        node->get("texture-percent", &m_sky_texture_percent);

    }   // if sky-dome
    node = xml->getNode("sky-box");
    if(node)
    {
        std::string s;
        node->get("texture", &s);
        m_sky_textures = StringUtils::split(s, ' ');
        if(m_sky_textures.size()!=6)
        {
            fprintf(stderr, "A skybox needs 6 textures, but %d are specified\n",
                    (int)m_sky_textures.size());
            fprintf(stderr, "in '%s'.\n", filename.c_str());

        }
        else
        {
            m_sky_type = SKY_BOX;
        }
    }   // if sky-box

    // Set the correct paths
    m_screenshot = file_manager->getTrackFile(m_screenshot, getIdent());
}   // loadTrack

//-----------------------------------------------------------------------------
void Track::getMusicInformation(std::vector<std::string>&       filenames, 
                                std::vector<MusicInformation*>& music    )
{
    for(int i=0; i<(int)filenames.size(); i++)
    {
        std::string full_path = file_manager->getTrackFile(filenames[i], getIdent());
        MusicInformation* mi;
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
void Track::startMusic() const 
{
    // In case that the music wasn't found (a warning was already printed)
    if(m_music.size()>0)
        sound_manager->startMusic(m_music[rand()% m_music.size()]);
}   // startMusic

//-----------------------------------------------------------------------------
void Track::loadDriveline()
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

    m_dl_with_tolerance_left.reserve(DRIVELINE_SIZE);
    m_dl_with_tolerance_right.reserve(DRIVELINE_SIZE);
    m_driveline.reserve(DRIVELINE_SIZE);
    m_path_width.reserve(DRIVELINE_SIZE);
    m_angle.reserve(DRIVELINE_SIZE);
    for(unsigned int i = 0; i < DRIVELINE_SIZE; ++i)
    {
        Vec3 center_point = (m_left_driveline[i]+m_right_driveline[i])*0.5;
        m_driveline.push_back(center_point);

        float width = ( m_right_driveline[i] - center_point ).length();
        m_path_width.push_back(width);

        // Compute the drivelines with tolerance
        Vec3 diff = (m_left_driveline[i] - m_right_driveline[i]) 
                  * stk_config->m_offroad_tolerance;
        m_dl_with_tolerance_left.push_back(m_left_driveline[i]+diff);
        m_dl_with_tolerance_right.push_back(m_right_driveline[i]-diff);

    }

    for(unsigned int i = 0; i < DRIVELINE_SIZE; ++i)
    {
        unsigned int next = i + 1 >= DRIVELINE_SIZE ? 0 : i + 1;
        float dx = m_driveline[next].getX() - m_driveline[i].getX();
        float dy = m_driveline[next].getY() - m_driveline[i].getY();

        float theta = -atan2(dx, dy);
        m_angle.push_back(theta);
    }

    m_driveline_min = Vec3( SG_MAX/2.0f);
    m_driveline_max = Vec3(-SG_MAX/2.0f);


    m_distance_from_start.reserve(DRIVELINE_SIZE);
    float d = 0.0f ;
    for ( size_t i = 0 ; i < DRIVELINE_SIZE ; ++i )
    {
        //Both drivelines must be checked to get the true size of
        //the drivelines, and using the center driveline is not
        //good enough.
        m_driveline_min.min(m_right_driveline[i]);
        m_driveline_min.min(m_left_driveline[i] );
        m_driveline_max.max(m_right_driveline[i]);
        m_driveline_max.max(m_left_driveline[i] );

        m_distance_from_start.push_back(d);  // dfs[i] is not valid in windows here!
        d += (m_driveline[i]-m_driveline[ i==DRIVELINE_SIZE-1 ? 0 : i+1 ]).length();
    }
    m_total_distance = d;
    Vec3 sc = m_driveline_max - m_driveline_min;

    m_scale_x = m_track_2d_width  / sc.getX();
    m_scale_y = m_track_2d_height / sc.getY();

    if(!m_do_stretch) m_scale_x = m_scale_y = std::min(m_scale_x, m_scale_y);
    
}   // loadDriveline

//-----------------------------------------------------------------------------
void
Track::readDrivelineFromFile(std::vector<Vec3>& line, const std::string& file_ext)
{
    std::string path = file_manager->getTrackFile(m_ident+file_ext);
    FILE *fd = fopen ( path.c_str(), "r" ) ;

    if ( fd == NULL )
    {
        std::ostringstream msg;
        msg<<"Can't open '"<<path<<"' for reading.\n";
        throw std::runtime_error(msg.str());
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
            std::ostringstream msg;
            msg<<"Syntax error in '"<<path<<"'\n";
            throw std::runtime_error(msg.str());
        }

        Vec3 point(x,y,z);

        if(prev_sector != UNKNOWN_SECTOR) 
            prev_distance = (point-line[prev_sector]).length2_2d();

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
    
    // Remove the temporary track rigid body, and then convert all objects
    // (i.e. the track and all additional objects) into a new rigid body
    // and convert this again. So this way we have an optimised track
    // rigid body which includes all track objects.
    // Note that removing the rigid body does not remove the already collected
    // triangle information, so there is no need to convert the actual track
    // (first element in m_track_mesh) again!
    m_track_mesh->removeBody();
    for(unsigned int i=1; i<m_all_meshes.size(); i++)
    {
        convertTrackToBullet(m_all_meshes[i]);
    }
    m_track_mesh->createBody();
    m_non_collision_mesh->createBody(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    
}   // createPhysicsModel

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Track::convertTrackToBullet(const scene::IMesh *mesh)
{
    for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++) {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        // FIXME: take translation/rotation into account
        if(mb->getVertexType()!=video::EVT_STANDARD) {
            fprintf(stderr, "WARNING: Physics::convertTrack: Ignoring type '%d'!", 
                mb->getVertexType());
            continue;
        }
        video::SMaterial &irrMaterial=mb->getMaterial();
        video::ITexture* t=irrMaterial.getTexture(0);

        const Material* material=0;
        TriangleMesh *tmesh = m_track_mesh;
        if(t) {
            std::string image = std::string(t->getName().c_str());
            material=material_manager->getMaterial(StringUtils::basename(image));
            if(material->isZipper()) tmesh = m_non_collision_mesh;
        } 

        u16 *mbIndices = mb->getIndices();
        Vec3 vertices[3];
        irr::video::S3DVertex* mbVertices=(video::S3DVertex*)mb->getVertices();
        for(unsigned int j=0; j<mb->getIndexCount(); j+=3) {
            for(unsigned int k=0; k<3; k++) {
                int indx=mbIndices[j+k];
                vertices[k] = Vec3(mbVertices[indx].Pos);
            }   // for k
            if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                         vertices[2], material     );
        }   // for j
    }   // for i<getMeshBufferCount

}   // convertTrackToBullet
// ----------------------------------------------------------------------------
/** Loads the main track model (i.e. all other objects contained in the
 *  scene might use raycast on this track model to determine the actual
 *  height of the terrain.
 */
bool Track::loadMainTrack(const XMLNode &node)
{
    std::string model_name;
    node.get("model", &model_name);
    std::string full_path = file_manager->getTrackFile(model_name, 
                                                       getIdent());
    scene::IMesh *mesh = irr_driver->getAnimatedMesh(full_path);
    if(!mesh)
    {
        fprintf(stderr, "Warning: Main track model '%s' in '%s' not found, aborting.\n",
                node.getName().c_str(), model_name.c_str());
        exit(-1);
    }
    m_all_meshes.push_back(mesh);

    Vec3 min, max;
    MeshTools::minMax3D(mesh, &min, &max);
    RaceManager::getWorld()->getPhysics()->init(min, max);
    // This will (at this stage) only convert the main track model.
    convertTrackToBullet(mesh);
    m_track_mesh->createBody();


    scene::ISceneNode *scene_node = irr_driver->addOctTree(mesh);
    core::vector3df xyz(0,0,0);
    node.getXYZ(&xyz);
    core::vector3df hpr(0,0,0);
    node.getHPR(&hpr);
    scene_node->setPosition(xyz);
    scene_node->setRotation(hpr);
    m_all_nodes.push_back(scene_node);
    scene_node->setMaterialFlag(video::EMF_LIGHTING, false);


    return true;
}   // loadMainTrack

// ----------------------------------------------------------------------------
/** Creates a water node.
 *  \param node The XML node containing the specifications for the water node.
 */
void Track::createWater(const XMLNode &node)
{
    std::string model_name;
    node.get("model", &model_name);
    std::string full_path = file_manager->getTrackFile(model_name, 
                                                       getIdent());

    //scene::IMesh *mesh = irr_driver->getMesh(full_path);
    scene::IAnimatedMesh *mesh = irr_driver->getSceneManager()->getMesh(full_path.c_str());
    irr_driver->getSceneManager()->addWaterSurfaceSceneNode(mesh->getMesh(0));
//    scene::IAnimatedMesh *mesh = irr_driver->getSceneManager()->addHillPlaneMesh("myHill",
//                core::dimension2d<f32>(20,20),
//                core::dimension2d<u32>(40,40), 0, 0,
//                core::dimension2d<f32>(0,0),
//                core::dimension2d<f32>(10,10));

    scene::SMeshBuffer b(*(scene::SMeshBuffer*)(mesh->getMesh(0)->getMeshBuffer(0)));
    //scene::SMeshBuffer* buffer = new scene::SMeshBuffer(*(scene::SMeshBuffer*)(mesh->getMeshBuffer(0)));
    
    float wave_height  = 2.0f;
    float wave_speed   = 300.0f;
    float wave_length  = 10.0f;
    node.get("height", &wave_height);
    node.get("speed",  &wave_speed);
    node.get("length", &wave_length);
    scene::ISceneNode* scene_node = irr_driver->addWaterNode(mesh,
                                                             wave_height, 
                                                             wave_speed,
                                                             wave_length);

    if(!mesh || !scene_node)
    {
        fprintf(stderr, "Warning: Water model '%s' in '%s' not found, ignored.\n",
                node.getName().c_str(), model_name.c_str());
        return;
    }
    m_all_meshes.push_back(mesh);
    core::vector3df xyz(0,0,0);
    node.getXYZ(&xyz);
    core::vector3df hpr(0,0,0);
    node.getHPR(&hpr);
    scene_node->setPosition(xyz);
    scene_node->setRotation(hpr);
    m_all_nodes.push_back(scene_node);
}   // createWater

// ----------------------------------------------------------------------------
void Track::loadTrackModel()
{
    // Add the track directory to the texture search path
    file_manager->pushTextureSearchPath(file_manager->getTrackFile("",getIdent()));
    file_manager->pushModelSearchPath  (file_manager->getTrackFile("",getIdent()));
    // First read the temporary materials.dat file if it exists
    try
    {
        std::string materials_file = file_manager->getTrackFile("materials.xml",getIdent());
        material_manager->pushTempMaterial(materials_file);
    }
    catch (std::exception& e)
    {
        // no temporary materials.dat file, ignore
        (void)e;
    }

    // Start building the scene graph
#ifdef HAVE_IRRLICHT
    std::string path = file_manager->getTrackFile(getIdent()+".scene");
    XMLReader *xml = file_manager->getXMLReader(path);

    // Make sure that we have a track (which is used for raycasts to 
    // place other objects).
    const XMLNode *node = xml->getNode("track");
    if(!node)
    {
        std::ostringstream msg;
        msg<< "No track model defined in '"<<path
           <<"', aborting.";
        throw std::runtime_error(msg.str());
    }
    loadMainTrack(*node);
    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        const std::string name = node->getName();
        // The track object was already converted before the loop
        if(name=="track") continue;
        if(name=="object")
        {
            MovingPhysics *mp = new MovingPhysics(node);
            callback_manager->addCallback(mp, CB_TRACK);
        }
        else if(name=="water")
        {
            createWater(*node);
        }
        else if(name=="model")
        {
            std::string model_name;
            node->get("model", &model_name);
            std::string full_path = file_manager->getTrackFile(model_name, 
                getIdent());
            scene::IMesh *mesh = irr_driver->getAnimatedMesh(full_path);
            if(!mesh)
            {
                fprintf(stderr, "Warning: Main track model '%s' in '%s' not found, aborting.\n",
                    node->getName().c_str(), model_name.c_str());
                exit(-1);
            }
            m_all_meshes.push_back(mesh);
            scene::ISceneNode *scene_node = irr_driver->addOctTree(mesh);
            core::vector3df xyz(0,0,0);
            node->getXYZ(&xyz);
            core::vector3df hpr(0,0,0);
            node->getHPR(&hpr);
            scene_node->setPosition(xyz);
            scene_node->setRotation(hpr);
            m_all_nodes.push_back(scene_node);
            scene_node->setMaterialFlag(video::EMF_LIGHTING, false);
        }
        else if(name=="banana"      || name=="item" || 
                name=="small-nitro" || name=="big-nitro")
        {
            Item::ItemType type=Item::ITEM_BANANA;
            if     (name=="banana"     ) type = Item::ITEM_BANANA;
            else if(name=="item"       ) type = Item::ITEM_BONUS_BOX;
            else if(name=="small-nitro") type = Item::ITEM_SILVER_COIN;
            else                         type = Item::ITEM_GOLD_COIN;
            Vec3 xyz;
            int bits = node->getXYZ(&xyz);
            // Height is needed if bit 2 (for z) is not set
            itemCommand(xyz, type, /* need_height */ !XMLNode::hasZ(bits) );
        }
        else
        {
            fprintf(stderr, "Warning: element '%s' not found.\n",
                    node->getName().c_str());
        }

    }

#else
    std::string path = file_manager->getTrackFile(getIdent()+".loc");

    FILE *fd = fopen (path.c_str(), "r" );
    if ( fd == NULL )
    {
        std::ostringstream msg;
        msg<<"Can't open track location file '"<<path<<"'.";
        throw std::runtime_error(msg.str());
    }
    m_model = new ssgBranch ;
    stk_scene->add(m_model);

    char s [ 1024 ] ;

    while ( fgets ( s, 1023, fd ) != NULL )
    {
        if ( *s == '#' || *s < ' ' )
            continue ;

        int need_hat = false ;
        int fit_skin = false ;
        char fname [ 1024 ] ;
        sgCoord loc;
        sgZeroVec3 ( loc.xyz ) ;
        sgZeroVec3 ( loc.hpr ) ;

        char htype = '\0' ;

        /* the first 2 are for backwards compatibility. Don't use 'herring' names in any new track */
        if ( sscanf ( s, "%cHERRING,%f,%f,%f", &htype,
                      &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 4 )
        {
            Item::ItemType type=Item::ITEM_BANANA;
            if ( htype=='Y' || htype=='y' ) { type = Item::ITEM_GOLD_COIN   ;}
            if ( htype=='G' || htype=='g' ) { type = Item::ITEM_BANANA  ;}
            if ( htype=='R' || htype=='r' ) { type = Item::ITEM_BONUS_BOX    ;}
            if ( htype=='S' || htype=='s' ) { type = Item::ITEM_SILVER_COIN ;}
            itemCommand(&loc.xyz, type, false) ;
        }
        else if ( sscanf ( s, "%cHERRING,%f,%f", &htype,
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 3 )
        {
            Item::ItemType type=Item::ITEM_BANANA;
            if ( htype=='Y' || htype=='y' ) { type = Item::ITEM_GOLD_COIN   ;}
            if ( htype=='G' || htype=='g' ) { type = Item::ITEM_BANANA  ;}
            if ( htype=='R' || htype=='r' ) { type = Item::ITEM_BONUS_BOX    ;}
            if ( htype=='S' || htype=='s' ) { type = Item::ITEM_SILVER_COIN ;}
            itemCommand (&loc.xyz, type, true) ;
        }
        /* and now the new names */
        else if ( sscanf ( s, "BBOX,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 3 )
        {
            itemCommand(&loc.xyz, Item::ITEM_BONUS_BOX, false);
        }
        else if ( sscanf ( s, "BBOX,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 2 )
        {
            itemCommand(&loc.xyz, Item::ITEM_BONUS_BOX, true);
        }
        
        else if ( sscanf ( s, "BANA,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 3 )
        {
            itemCommand(&loc.xyz, Item::ITEM_BANANA, false);
        }
        
        else if ( sscanf ( s, "BANA,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 2 )
        {
            itemCommand(&loc.xyz, Item::ITEM_BANANA, true);
        }
        
        else if ( sscanf ( s, "COIN,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 3 )
        {
            itemCommand(&loc.xyz, Item::ITEM_SILVER_COIN, false);
        }
        else if ( sscanf ( s, "COIN,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 2 )
        {
            itemCommand(&loc.xyz, Item::ITEM_SILVER_COIN, true);
        }
        
        else if ( sscanf ( s, "GOLD,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 3 )
        {
            itemCommand(&loc.xyz, Item::ITEM_GOLD_COIN, false);
        }
        else if ( sscanf ( s, "GOLD,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]) ) == 2 )
        {
            itemCommand(&loc.xyz, Item::ITEM_GOLD_COIN, true);
        }
        
        else if ( sscanf ( s, "START,%f,%f,%f",
                           &(loc.xyz[0]), &(loc.xyz[1]), &(loc.xyz[2]) ) == 3 )
        {
            m_start_positions.push_back(Vec3(loc.xyz[0], loc.xyz[1], loc.xyz[2]));
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
                std::ostringstream msg;
                msg<< "Syntax error in '"<<path<<"': "<<s;
                throw std::runtime_error(msg.str());
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

            ssgEntity        *obj   = load(file_manager->getModelFile(fname),
                                                   CB_TRACK,
                                                   /* optimise   */  true,
                                                   /*is_full_path*/  true);
            if(!obj)
            {
                fclose(fd);
                std::ostringstream msg;
                msg<<"Can't open track model '"<<fname<<"'.";
                file_manager->popTextureSearchPath();
                file_manager->popModelSearchPath  ();
                throw std::runtime_error(msg.str());
            }
            SSGHelp::createDisplayLists(obj);
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
            fprintf(stderr, "Warning: Syntax error in '%s': %s",
                     path.c_str(), s);
        }
    }   // while fgets

    fclose ( fd ) ;
#endif

    if(m_sky_type==SKY_DOME)
    {
        m_all_nodes.push_back(irr_driver->addSkyDome(m_sky_textures[0],
                                                     m_sky_hori_segments, 
                                                     m_sky_vert_segments, 
                                                     m_sky_texture_percent, 
                                                     m_sky_sphere_percent) );
    }
    else if(m_sky_type==SKY_BOX)
    {
        m_all_nodes.push_back(irr_driver->addSkyBox(m_sky_textures));
    }
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath  ();

    const core::vector3df &sun_pos = getSunPos();
    m_light = irr_driver->getSceneManager()->addLightSceneNode(0, sun_pos);
    video::SLight light;
    m_light->setLightData(light);
    irr_driver->setAmbientLight(video::SColor(255, 255, 255, 255));
    // Note: the physics world for irrlicht is created in loadMainTrack
    createPhysicsModel();
}   // loadTrack

//-----------------------------------------------------------------------------
void Track::itemCommand(const Vec3 &xyz, Item::ItemType type, 
                        int bNeedHeight)
{
    // Some modes (e.g. time trial) don't have any bonus boxes
    if(type==Item::ITEM_BONUS_BOX && 
       !RaceManager::getWorld()->enableBonusBoxes()) 
        return;

    Vec3 loc(xyz);
    // if only 2d coordinates are given, let the item fall from very high
    if(bNeedHeight)
    {
        loc.setZ(1000);
        loc.setZ(getTerrainHeight(loc));
    }

    // Don't tilt the items, since otherwise the rotation will look odd,
    // i.e. the items will not rotate around the normal, but 'wobble'
    // around.
    Vec3 normal(0, 0, 0.0f);
    item_manager->newItem(type, loc, normal);
}   // itemCommand

// ----------------------------------------------------------------------------
void  Track::getTerrainInfo(const Vec3 &pos, float *hot, Vec3 *normal, 
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
             else
             {
                 // This can happen if the raycast hits a kart. This should 
                 // actually be impossible (since the kart is removed from
                 // the collision group), but now and again the karts don't
                 // have the broadphase handle set (kart::update() for 
                 // details), and it might still happen. So in this case
                 // just ignore this callback and don't add it.
                 return 1.0f;
             }
             return btCollisionWorld::ClosestRayResultCallback::AddSingleResult(rayResult, 
                                                                                normalInWorldSpace);
        }   // AddSingleResult
    };   // myCollision
    MaterialCollision rayCallback(pos, to_pos);
    RaceManager::getWorld()->getPhysics()->getPhysicsWorld()->rayTest(pos, to_pos, rayCallback);

    if(!rayCallback.HasHit()) 
    {
        *hot      = NOHIT;
        *material = NULL;
        return;
    }

    *hot      = rayCallback.m_hitPointWorld.getZ();
    *normal   = rayCallback.m_hitNormalWorld;
    *material = rayCallback.m_material;
    // Note: material might be NULL. This happens if the ray cast does not
    // hit the track, but another rigid body (kart, moving_physics) - e.g.
    // assume two karts falling down, one over the other. Bullet does not
    // have any triangle/material information in this case!
}   // getTerrainInfo

// ----------------------------------------------------------------------------
/** Simplified version to determine only the height of the terrain.
 *  \param pos Position at which to determine the height (x,y coordinates
 *             are only used).
 *  \return The height at the x,y coordinates.
 */
float Track::getTerrainHeight(const Vec3 &pos) const
{
    float hot;
    Vec3  normal;
    const Material *m;
    getTerrainInfo(pos, &hot, &normal, &m);
    return hot;
}   // getTerrainHeight
