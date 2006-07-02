/*
     PLIB - A Suite of Portable Game Libraries
     Copyright (C) 1998,2002  Steve Baker
 
     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License as published by the Free Software Foundation; either
     version 2 of the License, or (at your option) any later version.
 
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
                  new ssgColourArray   ( num * 4 /*, new sgVec4 [ num * 4 ]*/ ) )
{
  turn_to_face = _ttf ;
  create_error = 0 ;
  create_rate = _create_rate ;

  size = sz ;

  bsphere . setRadius ( bsphere_size ) ;
  bsphere . setCenter ( 0, 0, 0 ) ;

  num_particles = num ;
  num_verts     = num * 4 ;

  particles = new Particle [ num ] ;

  int i ;

  for ( i = 0 ; i < num_verts ; i++ )
  {
    sgSetVec3  ( getNormal ( i ), 0, -1, 0 ) ;
    sgSetVec4  ( getColour ( i ), 1, 1, 1, 1 ) ;
    sgZeroVec3 ( getVertex ( i ) ) ;
  }

  for ( i = 0 ; i < num_particles ; i++ )
  {
    sgSetVec2 ( getTexCoord ( i*4+0 ), 0, 0 ) ;
    sgSetVec2 ( getTexCoord ( i*4+1 ), 1, 0 ) ;
    sgSetVec2 ( getTexCoord ( i*4+2 ), 1, 1 ) ;
    sgSetVec2 ( getTexCoord ( i*4+3 ), 0, 1 ) ;
  }

  num_active = 0 ;
}

void
ParticleSystem::init(int initial_num)
{
  for ( int i = 0 ; i < initial_num ; i++ )
    particle_create(i, & particles [ i ] ) ;
}

void
ParticleSystem::recalcBSphere()
{
  bsphere . setRadius ( 1000.0f ) ;
  bsphere . setCenter ( 0, 0, 0 ) ;
}

void ParticleSystem::draw_geometry ()
{
  sgVec3 nxny, xxny, xxyy, nxyy ;

  float sz = size / 2.0f ;

  if ( turn_to_face )
  {
    sgMat4 mat ;

    glGetFloatv ( GL_MODELVIEW_MATRIX, (float *) mat ) ;

    sgVec3 xx, yy ;

    sgSetVec3 ( xx, mat[0][0] * sz, mat[1][0] * sz, mat[2][0] * sz ) ;
    sgSetVec3 ( yy, mat[0][1] * sz, mat[1][1] * sz, mat[2][1] * sz ) ;

    sgSetVec3 ( nxny, -xx[0]-yy[0], -xx[1]-yy[1], -xx[2]-yy[2] ) ;
    sgSetVec3 ( nxyy, -xx[0]+yy[0], -xx[1]+yy[1], -xx[2]+yy[2] ) ;
    sgSetVec3 ( xxny,  xx[0]-yy[0],  xx[1]-yy[1],  xx[2]-yy[2] ) ;
    sgSetVec3 ( xxyy,  xx[0]+yy[0],  xx[1]+yy[1],  xx[2]+yy[2] ) ;
  }
  else
  {
    sgSetVec3 ( xxny ,  sz, 0, -sz ) ;
    sgSetVec3 ( nxny , -sz, 0, -sz ) ;
    sgSetVec3 ( nxyy , -sz, 0,  sz ) ;
    sgSetVec3 ( xxyy,   sz, 0,  sz ) ;
  }

  int j = 0 ;

  for ( int i = 0 ; i < num_particles ; i++ )
  {
    /* Make them disappear if not needed */

    if ( particles[i].time_to_live <= 0.0f )
      continue ;

    sgCopyVec4 ( getColour ( j + 0 ), particles[i].col ) ;
    sgCopyVec4 ( getColour ( j + 1 ), particles[i].col ) ;
    sgCopyVec4 ( getColour ( j + 2 ), particles[i].col ) ;
    sgCopyVec4 ( getColour ( j + 3 ), particles[i].col ) ;

    sgAddScaledVec3 ( getVertex ( j + 0 ), particles[i].pos,
                                     nxny, particles[i].size ) ;
    sgAddScaledVec3 ( getVertex ( j + 1 ), particles[i].pos,
                                     xxny, particles[i].size ) ;
    sgAddScaledVec3 ( getVertex ( j + 2 ), particles[i].pos,
                                     xxyy, particles[i].size ) ;
    sgAddScaledVec3 ( getVertex ( j + 3 ), particles[i].pos,
                                     nxyy, particles[i].size ) ;

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

ParticleSystem::~ParticleSystem ()
{
  // TODO we should call particle_delete here, but that's not possible, because
  // the functions are virtual (illegal in destructor)
  
  delete[] particles ;
}

void ParticleSystem::update ( float t )
{
  int i ;

  create_error += create_rate * t ;

  num_active = 0 ;

  /* Call the update routine for all the particles */
  for ( i = 0 ; i < num_particles ; i++ )
    if ( particles [ i ] . time_to_live > 0.0f )
      {
        particles [ i ] . update ( t ) ;
        particle_update( t, i, & particles [ i ] ) ;
      }

  /* Check for death of particles */
  for ( i = 0 ; i < num_particles ; i++ )
    if ( particles [ i ] . time_to_live <= 0.0 )
    {
      particle_delete ( i, & particles [ i ] ) ;

      particles [ i ] . pos [ 2 ] = -1000000.0f ;

      if ( create_error >= 1.0f )
      {
	particle_create( i, & particles [ i ] ) ;
	create_error -= 1.0f ;
      }
    }
    else
    {
      num_active++ ;
    }
}

/* EOF */

