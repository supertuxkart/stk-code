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

     For further information visit http://plib.sourceforge.net                  */

#include <plib/ssg.h>

/** This is basically the ssgaParticle, but this implementation supports
 *  methods to be used to create, update and delete the objects instead
 *  of function pointer as the original plib.
 */
class Particle
{
public:

    sgVec4 m_col ;
    sgVec3 m_pos ;
    sgVec3 m_vel ;
    sgVec3 m_acc ;

    float m_size ;

    float m_time_to_live ;
    void *m_user_data ;

    void update ( float dt )
    {
        sgAddScaledVec3 ( m_vel, m_acc, dt ) ;
        sgAddScaledVec3 ( m_pos, m_vel, dt ) ;
        m_time_to_live -= dt ;
    }

    Particle ()
    {
        sgSetVec4 ( m_col, 1, 1, 1, 1 ) ;
        sgZeroVec3 ( m_pos ) ;
        sgZeroVec3 ( m_vel ) ;
        sgZeroVec3 ( m_acc ) ;
        m_time_to_live = 0 ;
        m_user_data = 0 ;
        m_size = 1.0f ;
    }

} ;


/** This is basically the ssgaParticleSystem, but this implementation supports
 *  methods to be used to create, update and delete the objects instead
 *  of function pointer as the original plib.
 */

class ParticleSystem : public ssgVtxTable
{
    int m_num_particles  ;
    int m_num_verts      ;
    int m_turn_to_face   ;
    int m_num_active     ;
    Particle* m_particles ;

    float m_create_error ;
    float m_create_rate ;

    float m_size ;

public:

    ParticleSystem ( int num, float _create_rate, int _turn_to_face,
                     float sz);
    virtual ~ParticleSystem () ;
    virtual void update ( float t ) ;

    virtual void particle_create( int index, Particle *p ) = 0;
    virtual void particle_update( float deltaTime, int index, Particle *p ) = 0;
    virtual void particle_delete( int index, Particle* p ) = 0;

    void init(int initial_num);
    void recalcBSphere();

    void setSize ( float sz ) { m_size = sz ; }
    float getSize () const { return m_size ; }

    void draw_geometry () ;

    void  setCreationRate ( float cr ) { m_create_rate = cr ; }
    float getCreationRate () const { return m_create_rate ; }

    int getNumParticles       () const { return m_num_particles ; }
    int getNumActiveParticles () const { return m_num_active    ; }
} ;

/* EOF */
