/*
     $Id$
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 3 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

     For further information visit http://plib.sourceforge.net
*/

#include "particle_system.hpp"

ParticleSystem::ParticleSystem ( int num, float _create_rate, int _ttf,
                                 float sz, float bsphere_size)
        : ssgVtxTable ( GL_QUADS,
            new ssgVertexArray   ( num * 4 /*, new sgVec3 [ num * 4 ]*/ ),
            new ssgNormalArray   ( num * 4 /*, new sgVec3 [ num * 4 ]*/ ),
            new ssgTexCoordArray ( num * 4 /*, new sgVec2 [ num * 4 ]*/ ),
            new ssgColourArray   ( num * 4 /*, new sgVec4 [ num * 4 ]*/ )
            )
{
    m_turn_to_face = _ttf ;
    m_create_error = 0 ;
    m_create_rate = _create_rate ;

    m_size = sz ;

    bsphere . setRadius ( bsphere_size ) ;
    bsphere . setCenter ( 0, 0, 0 ) ;

    m_num_particles = num ;
    m_num_verts     = num * 4 ;

    m_particles = new Particle [ num ] ;

    int i ;

    for ( i = 0 ; i < m_num_verts ; i++ )
    {
        sgSetVec3  ( getNormal ( i ), 0, -1, 0 ) ;
        sgSetVec4  ( getColour ( i ), 1, 1, 1, 1 ) ;
        sgZeroVec3 ( getVertex ( i ) ) ;
    }

    for ( i = 0 ; i < m_num_particles ; i++ )
    {
        sgSetVec2 ( getTexCoord ( i*4+0 ), 0, 0 ) ;
        sgSetVec2 ( getTexCoord ( i*4+1 ), 1, 0 ) ;
        sgSetVec2 ( getTexCoord ( i*4+2 ), 1, 1 ) ;
        sgSetVec2 ( getTexCoord ( i*4+3 ), 0, 1 ) ;
    }

    m_num_active = 0 ;
}

//-----------------------------------------------------------------------------
void
ParticleSystem::init(int initial_num)
{
    for ( int i = 0 ; i < initial_num ; i++ )
        particle_create(i, & m_particles [ i ] ) ;
}

//-----------------------------------------------------------------------------
void
ParticleSystem::recalcBSphere()
{
    bsphere . setRadius ( 1000.0f ) ;
    bsphere . setCenter ( 0, 0, 0 ) ;
}

//-----------------------------------------------------------------------------
void ParticleSystem::draw_geometry ()
{
    sgVec3 nxny, xxny, xxyy, nxyy ;

    float SZ = m_size / 2.0f ;

    if ( m_turn_to_face )
    {
        sgMat4 mat ;

        glGetFloatv ( GL_MODELVIEW_MATRIX, (float *) mat ) ;

        sgVec3 xx, yy ;

        sgSetVec3 ( xx, mat[0][0] * SZ, mat[1][0] * SZ, mat[2][0] * SZ ) ;
        sgSetVec3 ( yy, mat[0][1] * SZ, mat[1][1] * SZ, mat[2][1] * SZ ) ;

        sgSetVec3 ( nxny, -xx[0]-yy[0], -xx[1]-yy[1], -xx[2]-yy[2] ) ;
        sgSetVec3 ( nxyy, -xx[0]+yy[0], -xx[1]+yy[1], -xx[2]+yy[2] ) ;
        sgSetVec3 ( xxny,  xx[0]-yy[0],  xx[1]-yy[1],  xx[2]-yy[2] ) ;
        sgSetVec3 ( xxyy,  xx[0]+yy[0],  xx[1]+yy[1],  xx[2]+yy[2] ) ;
    }
    else
    {
        sgSetVec3 ( xxny ,  SZ, 0, -SZ ) ;
        sgSetVec3 ( nxny , -SZ, 0, -SZ ) ;
        sgSetVec3 ( nxyy , -SZ, 0,  SZ ) ;
        sgSetVec3 ( xxyy,   SZ, 0,  SZ ) ;
    }

    int j = 0 ;

    for ( int i = 0 ; i < m_num_particles ; i++ )
    {
        /* Make them disappear if not needed */

        if ( m_particles[i].m_time_to_live <= 0.0f )
            continue ;

        sgCopyVec4 ( getColour ( j + 0 ), m_particles[i].m_col ) ;
        sgCopyVec4 ( getColour ( j + 1 ), m_particles[i].m_col ) ;
        sgCopyVec4 ( getColour ( j + 2 ), m_particles[i].m_col ) ;
        sgCopyVec4 ( getColour ( j + 3 ), m_particles[i].m_col ) ;

        sgAddScaledVec3 ( getVertex ( j + 0 ), m_particles[i].m_pos,
                          nxny, m_particles[i].m_size ) ;
        sgAddScaledVec3 ( getVertex ( j + 1 ), m_particles[i].m_pos,
                          xxny, m_particles[i].m_size ) ;
        sgAddScaledVec3 ( getVertex ( j + 2 ), m_particles[i].m_pos,
                          xxyy, m_particles[i].m_size ) ;
        sgAddScaledVec3 ( getVertex ( j + 3 ), m_particles[i].m_pos,
                          nxyy, m_particles[i].m_size ) ;

        j += 4 ;
    }

    rawSetNumVertices ( j ) ; /* Avoid drawing more than 'j' vertices. */

    if ( j > 0 )
    {
        glDisable   ( GL_CULL_FACE ) ;
        glDepthMask ( 0 ) ;

        ssgVtxTable::draw_geometry () ;

        glDepthMask ( 1 ) ;
        glEnable ( GL_CULL_FACE ) ;
    }
}

//-----------------------------------------------------------------------------
ParticleSystem::~ParticleSystem ()
{
    // TODO we should call particle_delete here, but that's not possible, because
    // the functions are virtual (illegal in destructor)

    delete[] m_particles ;
}

//-----------------------------------------------------------------------------
void ParticleSystem::update ( float t )
{
    int i ;

    m_create_error += m_create_rate * t ;

    m_num_active = 0 ;

    /* Call the update routine for all the particles */
    for ( i = 0 ; i < m_num_particles ; i++ )
        if ( m_particles [ i ] . m_time_to_live > 0.0f )
        {
            m_particles [ i ] . update ( t ) ;
            particle_update( t, i, & m_particles [ i ] ) ;
        }

    /* Check for death of particles */
    for ( i = 0 ; i < m_num_particles ; i++ )
        if ( m_particles [ i ] . m_time_to_live <= 0.0 )
        {
            particle_delete ( i, & m_particles [ i ] ) ;

            m_particles [ i ] . m_pos [ 2 ] = -1000000.0f ;

            if ( m_create_error >= 1.0f )
            {
                particle_create( i, & m_particles [ i ] ) ;
                m_create_error -= 1.0f ;
            }
        }
        else
        {
            m_num_active++ ;
        }
}

/* EOF */

