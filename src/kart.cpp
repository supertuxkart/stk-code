//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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
#include <plib/ssg.h>

#include "herring_manager.hpp"
#include "sound_manager.hpp"
#include "loader.hpp"
#include "skid_mark.hpp"
#include "user_config.hpp"
#include "constants.hpp"
#include "shadow.hpp"
#include "track.hpp"
#include "world.hpp"
#include "kart.hpp"
#include "ssg_help.hpp"
#include "physics.hpp"
#include "gui/menu_manager.hpp"
#include "gui/race_gui.hpp"
#include "translation.hpp"
#ifdef BULLET
#include "../bullet/Demos/OpenGL/GL_ShapeDrawer.h"
#endif
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif

KartParticleSystem::KartParticleSystem(Kart* kart_,
                                       int num, float _create_rate, int _ttf,
                                       float sz, float bsphere_size)
        : ParticleSystem (num, _create_rate, _ttf, sz, bsphere_size),
        m_kart(kart_)
{
    getBSphere () -> setCenter ( 0, 0, 0 ) ;
    getBSphere () -> setRadius ( 1000.0f ) ;
    dirtyBSphere();
}   // KartParticleSystem

//-----------------------------------------------------------------------------
void KartParticleSystem::update ( float t )
{
#if 0
    std::cout << "BSphere: r:" << getBSphere()->radius
    << " ("  << getBSphere()->center[0]
    << ", "  << getBSphere()->center[1]
    << ", "  << getBSphere()->center[2]
    << ")"
    << std::endl;
#endif
    getBSphere () -> setRadius ( 1000.0f ) ;
    ParticleSystem::update(t);
}   // update

//-----------------------------------------------------------------------------
void KartParticleSystem::particle_create(int, Particle *p)
{
    sgSetVec4 ( p -> m_col, 1, 1, 1, 1 ) ; /* initially white */
    sgSetVec3 ( p -> m_pos, 0, 0, 0 ) ;    /* start off on the ground */
    sgSetVec3 ( p -> m_vel, 0, 0, 0 ) ;
    sgSetVec3 ( p -> m_acc, 0, 0, 2.0f ) ; /* Gravity */
    p -> m_size = .5f;
    p -> m_time_to_live = 0.5 ;            /* Droplets evaporate after 5 seconds */

    const sgCoord* POS = m_kart->getCoord();
    const sgCoord* VEL = m_kart->getVelocity();

    const float X_DIRECTION = sgCos (POS->hpr[0] - 90.0f); // Point at the rear
    const float Y_DIRECTION = sgSin (POS->hpr[0] - 90.0f); // Point at the rear

    sgCopyVec3 (p->m_pos, POS->xyz);

    p->m_pos[0] += X_DIRECTION * 0.7f;
    p->m_pos[1] += Y_DIRECTION * 0.7f;

    const float ABS_VEL = sqrt((VEL->xyz[0] * VEL->xyz[0]) + (VEL->xyz[1] * VEL->xyz[1]));

    p->m_vel[0] = X_DIRECTION * -ABS_VEL/2;
    p->m_vel[1] = Y_DIRECTION * -ABS_VEL/2;

    p->m_vel[0] += sgCos ((float)(rand()%180));
    p->m_vel[1] += sgSin ((float)(rand()%180));
    p->m_vel[2] += sgSin ((float)(rand()%100));

    getBSphere () -> setCenter ( POS->xyz[0], POS->xyz[1], POS->xyz[2] ) ;
}   // particle_create

//-----------------------------------------------------------------------------
void KartParticleSystem::particle_update (float delta, int,
        Particle * particle)
{
    particle->m_size    += delta*2.0f;
    particle->m_col[3]  -= delta * 2.0f;

    particle->m_pos[0] += particle->m_vel[0] * delta;
    particle->m_pos[1] += particle->m_vel[1] * delta;
    particle->m_pos[2] += particle->m_vel[2] * delta;
}  // particle_update

//-----------------------------------------------------------------------------
void KartParticleSystem::particle_delete (int , Particle* )
{}   // particle_delete

//=============================================================================
Kart::Kart (const KartProperties* kartProperties_, int position_ ,
            sgCoord init_pos)
        : Moveable(true), m_attachment(this), m_collectable(this)
{
    m_kart_properties      = kartProperties_;
    m_grid_position        = position_ ;
    m_num_herrings_gobbled = 0;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_prev_accel           = 0.0f;
    m_wheelie_angle        = 0.0f;
    m_current_friction     = 1.0f;
#ifdef BULLET
    m_time_since_stuck     = 0.0f;
#endif
    m_smokepuff            = NULL;
    m_smoke_system         = NULL;
    m_exhaust_pipe         = NULL;
    m_skidmark_left        = NULL;
    m_skidmark_right       = NULL;
    m_track_sector         = Track::UNKNOWN_SECTOR;
    sgCopyCoord(&m_reset_pos, &init_pos);
    // Neglecting the roll resistance (which is small for high speeds compared
    // to the air resistance), maximum speed is reached when the engine
    // power equals the air resistance force, resulting in this formula:
#ifdef BULLET
    m_max_speed               = m_kart_properties->getMaximumSpeed();
    m_max_speed_reverse_ratio = m_kart_properties->getMaxSpeedReverseRatio();
    m_speed                   = 0.0f;
#else
    m_max_speed               = sqrt(getMaxPower()/getAirResistance());
#endif

    m_wheel_position = 0;

    m_wheel_front_l  = NULL;
    m_wheel_front_r  = NULL;
    m_wheel_rear_l   = NULL;
    m_wheel_rear_r   = NULL;
    m_lap_start_time = -1.0f;
    loadData();
}   // Kart

// -----------------------------------------------------------------------------v
void Kart::createPhysics(ssgEntity *obj)
{
    // First: Create the chassis of the kart
    // -------------------------------------
    // The size for bullet must be specified in half extends!
    //    ssgEntity *model = getModel();
#ifdef BULLET
    float x_min, x_max, y_min, y_max, z_min, z_max;
    MinMax(obj, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
    float kart_width  = x_max-x_min;
    float kart_length = y_max-y_min;
    if(kart_length<1.2) kart_length=1.5f;

    // The kart height is needed later to reset the physics to the correct
    // position.
    m_kart_height  = z_max-z_min;

    btBoxShape *shape = new btBoxShape(btVector3(0.5*kart_width,
                                              0.5*kart_length,
                                              0.5*m_kart_height));
    m_kart_chassis = new btCompoundShape();
    btTransform shiftCenterOfGravity;
    shiftCenterOfGravity.setIdentity();
    // Shift center of gravity downwards, so that the kart 
    // won't topple over too easy. This must be between 0 and 0.5
    // (it's in units of kart_height)
    const float CENTER_SHIFT = getGravityCenterShift();
    shiftCenterOfGravity.setOrigin(btVector3(0.0f,0.0f,CENTER_SHIFT*m_kart_height));
    m_kart_chassis->addChildShape(shiftCenterOfGravity, shape);

    // Set mass and inertia
    // --------------------
    float mass=getMass();
    btVector3 inertia;
    m_kart_chassis->calculateLocalInertia(mass, inertia);

    // Position the chassis
    // --------------------
    btTransform trans;
    trans.setIdentity();
    m_motion_state = new btDefaultMotionState(trans);

    // Then create a rigid body
    // ------------------------
    m_kart_body = new btRigidBody(mass, m_motion_state, 
                                  m_kart_chassis, inertia);
    m_kart_body->setDamping(m_kart_properties->getChassisLinearDamping(), 
                            m_kart_properties->getChassisAngularDamping() );

    // Reset velocities
    // ----------------
    m_kart_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_kart_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));

    // Create the actual vehicle
    // -------------------------
    m_vehicle_raycaster = 
        new btDefaultVehicleRaycaster(world->getPhysics()->getPhysicsWorld());
    m_tuning = new btRaycastVehicle::btVehicleTuning();
    m_vehicle = new btRaycastVehicle(*m_tuning, m_kart_body, m_vehicle_raycaster);
    // never deactivate the vehicle
    m_kart_body->setActivationState(DISABLE_DEACTIVATION);
    m_vehicle->setCoordinateSystem(/*right: */ 0,  /*up: */ 2,  /*forward: */ 1);
    
    // Add wheels
    // ----------
    float wheel_width  = m_kart_properties->getWheelWidth();
    float wheel_radius = m_kart_properties->getWheelRadius();
    float suspension_rest = m_kart_properties->getSuspensionRest();
    float connection_height = -(0.5-CENTER_SHIFT)*m_kart_height;
    btVector3 wheel_direction(0.0f, 0.0f, -1.0f);
    btVector3 wheel_axle(1.0f,0.0f,0.0f);

    // right front wheel
    btVector3 wheel_coord(0.5f*kart_width-0.3f*wheel_width,
                          0.5f*kart_length-wheel_radius,
                          connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ true);

    // left front wheel
    wheel_coord = btVector3(-0.5f*kart_width+0.3f*wheel_width,
                            0.5f*kart_length-wheel_radius,
                            connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ true);

    // right rear wheel
    wheel_coord = btVector3(0.5*kart_width-0.3f*wheel_width, 
                            -0.5*kart_length+wheel_radius,
                            connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ false);

    // right rear wheel
    wheel_coord = btVector3(-0.5*kart_width+0.3f*wheel_width,
                            -0.5*kart_length+wheel_radius,
                            connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ false);

    for(int i=0; i<m_vehicle->getNumWheels(); i++)
    {
        btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness      = m_kart_properties->getSuspensionStiffness();
        wheel.m_wheelsDampingRelaxation  = m_kart_properties->getWheelDampingRelaxation();
        wheel.m_wheelsDampingCompression = m_kart_properties->getWheelDampingCompression();
        wheel.m_frictionSlip             = m_kart_properties->getFrictionSlip();
        wheel.m_rollInfluence            = m_kart_properties->getRollInfluence();
    }
    world->getPhysics()->addKart(this, m_vehicle);

#endif
}   // createPhysics

// -----------------------------------------------------------------------------
Kart::~Kart() 
{
    delete m_smokepuff;

    sgMat4 wheel_steer;
    sgMakeIdentMat4(wheel_steer);
    if (m_wheel_front_l) m_wheel_front_l->setTransform(wheel_steer);
    if (m_wheel_front_r) m_wheel_front_r->setTransform(wheel_steer);


    ssgDeRefDelete(m_shadow);
    ssgDeRefDelete(m_wheel_front_l);
    ssgDeRefDelete(m_wheel_front_r);
    ssgDeRefDelete(m_wheel_rear_l);
    ssgDeRefDelete(m_wheel_rear_r);

    if(m_skidmark_left ) delete m_skidmark_left ;
    if(m_skidmark_right) delete m_skidmark_right;

#ifdef BULLET
    delete m_tuning;
    delete m_vehicle_raycaster;
    delete m_vehicle;
    for(int i=0; i<m_kart_chassis->getNumChildShapes(); i++)
    {
        delete m_kart_chassis->getChildShape(i);
    }
    delete m_kart_chassis;
    delete m_kart_body;
    delete m_motion_state;
#endif
}   // ~Kart

//-----------------------------------------------------------------------------
/** Returns true if the kart is 'resting'
 *
 * Returns true if the kart is 'resting', i.e. (nearly) not moving.
 */
#ifdef BULLET
bool Kart::isInRest()
{
    return fabs(m_kart_body->getLinearVelocity ().z())<0.2;
}  // isInRest
#endif
//-----------------------------------------------------------------------------
/** Modifies the physics parameter to simulate an attached anvil.
 *  The velocity is multiplicated by f, and the mass of the kart is increased.
 */
void Kart::adjustSpeedWeight(float f)
{
#ifdef BULLET
    m_kart_body->setLinearVelocity(m_kart_body->getLinearVelocity()*f);
    // getMass returns the mass increased by the attachment
    btVector3 inertia;
    float m=getMass();
    m_kart_chassis->calculateLocalInertia(m, inertia);
    m_kart_body->setMassProps(m, inertia);
#else
    getVelocity()->xyz[1] *= f;
#endif
}   // adjustSpeedWeight

//-----------------------------------------------------------------------------
void Kart::reset()
{
    Moveable::reset();

    m_attachment.clear();
    m_collectable.clear();

    m_race_lap             = -1;
    m_lap_start_time       = -1.0f;
    m_shortcut_count       = 0;
    m_shortcut_sector      = Track::UNKNOWN_SECTOR;
    m_shortcut_type        = SC_NONE;
    m_race_position        = 9;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_zipper_time_left     = 0.0f;
    m_rescue               = false;
    m_num_herrings_gobbled = 0;
    m_wheel_position       = 0;
    m_wheelie_angle        = 0.0f;

    m_controls.lr      = 0.0f;
    m_controls.accel   = 0.0f;
    m_controls.brake   = false;
    m_controls.wheelie = false;
    m_controls.jump    = false;
    m_controls.fire    = false;

    world->m_track->findRoadSector(m_curr_pos.xyz, &m_track_sector);

    //If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
    //the road, so we have to use another function to find the sector.
    if (m_track_sector == Track::UNKNOWN_SECTOR )
    {
        m_on_road = false;
        m_track_sector = world->m_track->findOutOfRoadSector(
            m_curr_pos.xyz, Track::RS_DONT_KNOW, Track::UNKNOWN_SECTOR );
    }
    else
    {
        m_on_road = true;
    }

    world->m_track->spatialToTrack( m_curr_track_coords, m_curr_pos.xyz,
        m_track_sector );

#ifdef BULLET
    btTransform *trans=new btTransform();
    trans->setIdentity();
    // Set heading:
    trans->setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), 
				    m_reset_pos.hpr[0]*3.1415926f/180.0f));
    // Set position
    trans->setOrigin(btVector3(m_reset_pos.xyz[0],
                               m_reset_pos.xyz[1],
                               m_reset_pos.xyz[2]+0.5*m_kart_height));
    m_vehicle->applyEngineForce (0.0f, 2);
    m_vehicle->applyEngineForce (0.0f, 3);
    m_kart_body->setCenterOfMassTransform(*trans);
    m_kart_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_kart_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));
    for(int j=0; j<m_vehicle->getNumWheels(); j++)
    {
        m_vehicle->updateWheelTransform(j, true);
    }

#endif
    placeModel();
}   // reset

//-----------------------------------------------------------------------------
void Kart::handleZipper()
{
    m_wheelie_angle  = ZIPPER_ANGLE;
    m_zipper_time_left = ZIPPER_TIME;
}   // handleZipper

//-----------------------------------------------------------------------------
void Kart::doLapCounting ()
{
    bool newLap = m_last_track_coords[1] > 300.0f && m_curr_track_coords[1] <  20.0f;
    if (  newLap   &&
         (world->m_race_setup.m_difficulty==RD_EASY                         ||
          world->m_race_setup.m_difficulty==RD_MEDIUM && m_shortcut_count<2 ||
          world->m_race_setup.m_difficulty==RD_HARD   && m_shortcut_count<1   ) )
    {
        // Only increase the lap counter and set the new time if the
        // kart hasn't already finished the race (otherwise the race_gui
        // will begin another countdown).
        if(m_race_lap+1<=world->m_race_setup.m_num_laps)
        {
            setTimeAtLap(world->m_clock);
            m_race_lap++ ;
        }

        m_shortcut_count = 0;
        // Only do timings if original time was set properly. Driving backwards
        // over the start line will cause the lap start time to be set to 0.
        if(m_lap_start_time>=0.0)
        {
            float time_per_lap=world->m_clock-m_lap_start_time;
            if(time_per_lap < world->getFastestLapTime() )
            {
                world->setFastestLap(this, time_per_lap);
                RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
                if(m)
                {
                    m->addMessage(_("New fastest lap"), NULL, 
                                  2.0f, 40, 100, 210, 100);
                    char s[20];
                    m->TimeToString(time_per_lap, s);
                    snprintf(m_fastest_lap_message, sizeof(m_fastest_lap_message),
                             "%s: %s",s, getName().c_str());
                    m->addMessage(m_fastest_lap_message, NULL, 
                                  2.0f, 40, 100, 210, 100);
                }   // if m
            }   // if time_per_lap < world->getFasterstLapTime()
            if(isPlayerKart())
            {
                // Put in in the highscore list???
                //printf("Time per lap: %s %f\n", getName().c_str(), time_per_lap);
            }
        }
        m_lap_start_time = world->m_clock;
    }
    else if ( newLap )
    {
        // Might happen if the option menu is called
        RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
        if(m)
        {
            m->addMessage(_("Lap not counted"),  this, 2.0f, 60);
            m->addMessage(_("(shortcut taken)"), this, 2.0f, 60);
        }
        m_shortcut_count = 0;
    }
    else if ( m_curr_track_coords[1] > 300.0f && m_last_track_coords[1] <  20.0f)
    {
        m_race_lap-- ;
        // Prevent cheating by setting time to a negative number, indicating
        // that the line wasn't crossed properly.
        m_lap_start_time = -1.0f;
    }
}   // doLapCounting

//-----------------------------------------------------------------------------
void Kart::doObjectInteractions ()
{
    int i;
    for ( i = 0 ; i < m_grid_position ; i++ )
    {
        sgVec3 xyz ;
        Kart *other_kart = world->getKart(i);
        sgSubVec3(xyz, getCoord()->xyz, other_kart->getCoord()->xyz );
        //FIXME
        // the dist calculation is very very basic, and 1.1f is far too low
        // must be from 2.4f to 3.0f
        float dist = sgLengthSquaredVec3(xyz);
        if ( dist < 1.1f )
        {
            // Avoid division by zero
            if(dist>0.00001) sgNormalizeVec2(xyz) ;

            world->addCollisions(m_grid_position, 1);
            world->addCollisions(i,             1);
            if ( m_velocity.xyz[1] > other_kart->getVelocity()->xyz[1] )
            {
                forceCrash () ;
                sgSubVec2 ( other_kart->getCoord()->xyz, xyz ) ;
            }
            else
            {
                other_kart->forceCrash () ;
                sgAddVec2 ( getCoord()->xyz, xyz ) ;
            }
            if(m_attachment.getType()==ATTACH_BOMB &&
               m_attachment.getPreviousOwner()!=other_kart) 
            {
                m_attachment.moveBombFromTo(this, other_kart);
            }
            if(other_kart->m_attachment.getType()==ATTACH_BOMB &&
               other_kart->m_attachment.getPreviousOwner()!=this)
            {
                m_attachment.moveBombFromTo(other_kart, this);
            }
        }   // if sgLengthSquaredVec2(xy)<1.0
    }   // for i

    // Check if any herring was hit.
    herring_manager->hitHerring(this);
}   // doObjectInteractions

//-----------------------------------------------------------------------------
void Kart::collectedHerring(Herring* herring)
{
    const herringType TYPE = herring->getType();
    const int OLD_HERRING_GOBBLED = m_num_herrings_gobbled;

    switch (TYPE)
    {
    case HE_GREEN  : m_attachment.hitGreenHerring(); break;
    case HE_SILVER : m_num_herrings_gobbled++ ;       break;
    case HE_GOLD   : m_num_herrings_gobbled += 3 ;    break;
    case HE_RED    : int n=1 + 4*getNumHerring() / MAX_HERRING_EATEN;
        m_collectable.hitRedHerring(n); break;
    }   // switch TYPE

    if ( m_num_herrings_gobbled > MAX_HERRING_EATEN )
        m_num_herrings_gobbled = MAX_HERRING_EATEN;

    if(OLD_HERRING_GOBBLED < m_num_herrings_gobbled &&
       m_num_herrings_gobbled == MAX_HERRING_EATEN)
        sound_manager->playSfx(SOUND_FULL);
}   // hitHerring

//-----------------------------------------------------------------------------
void Kart::doZipperProcessing (float delta)
{
    if ( m_zipper_time_left > delta )
    {
        m_zipper_time_left -= delta ;
        if ( m_velocity.xyz[1] < ZIPPER_VELOCITY )
            m_velocity.xyz[1] = ZIPPER_VELOCITY ;
    }
    else m_zipper_time_left = 0.0f ;
}   // doZipperProcessing

#ifdef BULLET
//-----------------------------------------------------------------------------
float Kart::getActualWheelForce()
{
    const std::vector<float>& gear_ratio=m_kart_properties->getGearSwitchRatio();
    for(unsigned int i=0; i<gear_ratio.size(); i++)
    {
        if(m_speed <= m_max_speed*gear_ratio[i]) 
            return getMaxPower()*m_kart_properties->getGearPowerIncrease()[i];
    }
    return getMaxPower();

}   // getActualWheelForce
#endif

//-----------------------------------------------------------------------------
void Kart::handleExplosion(const sgVec3& pos, bool direct_hit)
{
#ifdef BULLET
    if(direct_hit) {
        btVector3 velocity = m_kart_body->getLinearVelocity();

        velocity.setX( 0.0f );
        velocity.setY( 0.0f );
        velocity.setZ( 3.5f );

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );
    }
    else   // only affected by a distant explosion
    {
        sgVec3 diff;
        sgSubVec3(diff, getCoord()->xyz, pos);
        float len2=sgLengthSquaredVec3(diff);

        // The correct forumale would be to first normalise diff,
        // then apply the impulse (which decreases 1/r^2 depending
        // on the distance r), so:
        // diff/len(diff) * impulseSize/len(diff)^2
        // = diff*impulseSize/len(diff)^3
        // We use diff*impulseSize/len(diff)^2 here, this makes the impulse
        // somewhat larger, which is actually more fun :)
        sgScaleVec3(diff,stk_config->m_explosion_impulse/len2);
        btVector3 impulse(diff[0],diff[1], diff[2]);
        getVehicle()->getRigidBody()->applyCentralImpulse(impulse);
    }

#else
    // only the kart hit directly is affected.
    if(direct_hit) forceCrash();
#endif
}   // handleExplosion

//-----------------------------------------------------------------------------
void Kart::forceCrash ()
{
    m_wheelie_angle = CRASH_PITCH ;
#ifdef BULLET
    btVector3 velocity         = m_kart_body->getLinearVelocity();

    velocity.setY( 0.0f );
    velocity.setX( 0.0f );

    getVehicle()->getRigidBody()->setLinearVelocity( velocity );
#else
    m_velocity.xyz[0] = m_velocity.xyz[1] = m_velocity.xyz[2] =
    m_velocity.hpr[0] = m_velocity.hpr[1] = m_velocity.hpr[2] = 0.0f ;
#endif
}  // forceCrash

//-----------------------------------------------------------------------------
void Kart::doCollisionAnalysis ( float delta, float hot )
{
    if ( m_collided )
    {
        if ( m_velocity.xyz[1] > MIN_COLLIDE_VELOCITY )
        {
            m_velocity.xyz[1] -= COLLIDE_BRAKING_RATE * delta ;
        }
        else if ( m_velocity.xyz[1] < -MIN_COLLIDE_VELOCITY )
        {
            m_velocity.xyz[1] += COLLIDE_BRAKING_RATE * delta ;
        }
    }   // if collided

    if ( m_crashed && m_velocity.xyz[1] > MIN_CRASH_VELOCITY )
    {
        forceCrash () ;
    }
    else if ( m_wheelie_angle < 0.0f )
    {
        m_wheelie_angle += getWheelieRestoreRate() * delta;
        if ( m_wheelie_angle >= 0.0f ) m_wheelie_angle = 0.0f ;
    }

    /* Make sure that the car doesn't go through the floor */
    if ( isOnGround() )
    {
        m_velocity.xyz[2] = 0.0f ;
    }   // isOnGround
}   // doCollisionAnalysis

//-----------------------------------------------------------------------------
void Kart::update (float dt)
{
#ifdef BULLET
    // check if kart is stuck
    if(!isPlayerKart() && getVehicle()->getRigidBody()->getLinearVelocity().length()<2.0f
            && !m_rescue && world->getPhase() != World::START_PHASE)
    {
        m_time_since_stuck += dt;
    }
    else
    {
        m_time_since_stuck = 0.0f;
    }
#endif

    //m_wheel_position gives the rotation around the X-axis, and since velocity's
    //timeframe is the delta time, we don't have to multiply it with dt.
    m_wheel_position += m_velocity.xyz[1];

    if ( m_rescue )
    {
        // Let the kart raise 2m in the 2 seconds of the rescue
        const float rescue_time   = 2.0f;
#ifdef BULLET
        const float rescue_height = 2.0f;
#endif
        if(m_attachment.getType() != ATTACH_TINYTUX)
        {
            if(isPlayerKart()) sound_manager -> playSfx ( SOUND_BZZT );
            m_attachment.set( ATTACH_TINYTUX, rescue_time ) ;
#ifdef BULLET
            m_rescue_pitch = m_curr_pos.hpr[1];
            m_rescue_roll  = m_curr_pos.hpr[2];
            world->getPhysics()->removeKart(this, m_vehicle);
#endif
        }
#ifdef BULLET
        m_curr_pos.xyz[2] += rescue_height*dt/rescue_time;

        btTransform pos=m_kart_body->getCenterOfMassTransform();
        pos.setOrigin(btVector3(m_curr_pos.xyz[0],m_curr_pos.xyz[1],m_curr_pos.xyz[2]));
        btQuaternion q_roll (btVector3(0., 1., 0.),
                             -m_rescue_roll*dt/rescue_time*M_PI/180.0);
        btQuaternion q_pitch(btVector3(1., 0., 0.),
                             -m_rescue_pitch*dt/rescue_time*M_PI/180.0);
        pos.setRotation(pos.getRotation()*q_roll*q_pitch);
        m_kart_body->setCenterOfMassTransform(pos);
        m_motion_state->setWorldTransform(pos);
        //printf("Set %f %f %f\n",pos.getOrigin().x(),pos.getOrigin().y(),pos.getOrigin().z());
#else
        sgZeroVec3 ( m_velocity.xyz ) ;
        sgZeroVec3 ( m_velocity.hpr ) ;
        m_velocity.xyz[2] = 1.1f * GRAVITY * dt *10.0f;
#endif        
    }   // if m_rescue
    m_attachment.update(dt, &m_velocity);

    /*smoke drawing control point*/
    if ( user_config->m_smoke )
    {
        if (m_smoke_system != NULL)
            m_smoke_system->update (dt);
    }  // user_config->smoke
    doZipperProcessing (dt) ;
    updatePhysics(dt);

    sgCopyVec2  ( m_last_track_coords, m_curr_track_coords );
    if(m_material_hot && m_on_ground)
    {
        float r=m_material_hot->getFriction();
        if(r<m_current_friction) 
        {
            m_velocity.xyz[1]-= (m_current_friction*m_current_friction-r*r)
                                *m_velocity.xyz[1];
        }   // r<m_current_friction
        m_current_friction = r;
    }   // if m_material_hot
    Moveable::update (dt);
    doObjectInteractions();


    // Save the last valid sector for forced rescue on shortcuts
    if(m_track_sector  != Track::UNKNOWN_SECTOR && 
       !m_rescue                                    ) 
    {
        m_shortcut_sector = m_track_sector;
    }

    int prev_sector = m_track_sector;
    if(!m_rescue)
        world->m_track->findRoadSector(m_curr_pos.xyz, &m_track_sector);

    // Check if the kart is taking a shortcut (if it's not already doing one):
    if(m_shortcut_type!=SC_SKIPPED_SECTOR && !m_rescue)
    {
        if(world->m_track->isShortcut(prev_sector, m_track_sector))
        {
            // Skipped sectors are more severe then getting outside the 
            // road, so count this as two. But if the kart is already
            // outside the track, only one is added (since the outside
            // track shortcut already added 1).
            
            // This gets subtracted again when doing the rescue
            m_shortcut_count+= m_shortcut_type==SC_NONE ? 2 : 1;
            m_shortcut_type  = SC_SKIPPED_SECTOR;
            if(isPlayerKart())
            {
                forceRescue();  // bring karts back to where they left the track.
                RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
                // Can happen if the option menu is called
                if(m)
                    m->addMessage(_("Invalid short-cut!!"), this, 2.0f, 60);
            }
        }
    } 
    else
    {   // The kart is already doing a skipped sector --> reset
        // the flag, since from now on (it's on a new sector) it's
        // not a shortcut anymore.
        m_shortcut_type=SC_NONE;
    }

    if (m_track_sector == Track::UNKNOWN_SECTOR && !m_rescue)
    {
        m_on_road = false;
        if( m_curr_track_coords[0] > 0.0 )
            m_track_sector = world->m_track->findOutOfRoadSector(
               m_curr_pos.xyz, Track::RS_RIGHT, prev_sector );
        else
            m_track_sector = world->m_track->findOutOfRoadSector(
               m_curr_pos.xyz, Track::RS_LEFT, prev_sector );
    }
    else
    {
        m_on_road = true;
    }

    int sector = world->m_track->spatialToTrack( m_curr_track_coords, 
                                                 m_curr_pos.xyz,
                                                 m_track_sector      );
    // If the kart is more thanm_max_road_distance away from the border of
    // the track, the kart is considered taking a shortcut (but not on level
    // easy, and not while being rescued)

    if(world->m_race_setup.m_difficulty             != RD_EASY               &&
       !m_rescue                                                             &&
       m_shortcut_type                              != SC_SKIPPED_SECTOR     &&
         fabsf(m_curr_track_coords[0])-stk_config->m_max_road_distance 
         >  m_curr_track_coords[2] ) 
    {
        m_shortcut_sector = sector;
        // Increase the error count the first time this happens
        if(m_shortcut_type==SC_NONE)
            m_shortcut_count++;
        m_shortcut_type   = SC_OUTSIDE_TRACK;
    }
    else 
    {
        // Kart was taking a shortcut before, but it finished. So increase the
        // overall shortcut count.
        if(m_shortcut_type == SC_OUTSIDE_TRACK) 
            m_shortcut_type = SC_NONE;
    }
    doLapCounting () ;
    processSkidMarks();
}   // update

//-----------------------------------------------------------------------------
#define sgn(x) ((x<0)?-1.0f:((x>0)?1.0f:0.0f))

// -----------------------------------------------------------------------------
#ifdef BULLET
void Kart::draw()
{
    float m[16];
    btTransform t;
    m_motion_state->getWorldTransform(t);
    t.getOpenGLMatrix(m);

    btVector3 wire_color(0.5f, 0.5f, 0.5f);
    world->getPhysics()->debugDraw(m, m_kart_body->getCollisionShape(), 
                                   wire_color);
    btCylinderShapeX *wheelShape = 
        new btCylinderShapeX( btVector3(0.3,
                                        m_kart_properties->getWheelRadius(),
                                        m_kart_properties->getWheelRadius()) );
    btVector3 wheelColor(1,0,0);
    for(int i=0; i<m_vehicle->getNumWheels(); i++)
    {
        m_vehicle->updateWheelTransform(i, true);
        float m[16];
        m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
        world->getPhysics()->debugDraw(m, wheelShape, wheelColor);
    }
}   // draw
#endif
// -----------------------------------------------------------------------------
/** Returned an additional engine power boost when doing a wheele.
***/
#ifdef BULLET
float Kart::handleWheelie(float dt)
{
    // Handle wheelies
    // ===============
    if ( m_controls.wheelie && 
         m_speed >= getMaxSpeed()*getWheelieMaxSpeedRatio())
    {
        if ( m_wheelie_angle < getWheelieMaxPitch() )
            m_wheelie_angle += getWheeliePitchRate() * dt;
        else
            m_wheelie_angle = getWheelieMaxPitch();
    }
    else if ( m_wheelie_angle > 0.0f )
    {
        m_wheelie_angle -= getWheelieRestoreRate() * dt;
        if ( m_wheelie_angle <= 0.0f ) m_wheelie_angle = 0.0f ;
    }
    if(m_wheelie_angle <=0.0f) return 0.0f;

    const btTransform& chassisTrans = m_kart_body->getCenterOfMassTransform();
    btVector3 targetUp(0.0f, 0.0f, 1.0f);
    btVector3 forwardW (chassisTrans.getBasis()[0][1],
                        chassisTrans.getBasis()[1][1],
                        chassisTrans.getBasis()[2][1]);
    btVector3 crossProd = targetUp.cross(forwardW);
    crossProd.normalize();
    
    const float gLeanRecovery   = m_kart_properties->getWheelieLeanRecovery();
    const float step            = m_kart_properties->getWheelieStep();
    const float balance_recovery= m_kart_properties->getWheelieBalanceRecovery();
    float alpha                 = (targetUp.dot(forwardW));
    float deltaalpha            = m_wheelie_angle*M_PI/180.0f - alpha;
    btVector3 angvel            = m_kart_body->getAngularVelocity();
    float projvel               = angvel.dot(crossProd);
    float deltavel              = -projvel    * gLeanRecovery    / step
                                  -deltaalpha * balance_recovery / step;
    btVector3 deltaangvel       = deltavel * crossProd;
    
    angvel                     += deltaangvel;
    m_kart_body->setAngularVelocity(angvel);
    return m_kart_properties->getWheeliePowerBoost() * getMaxPower()
          * m_wheelie_angle/getWheelieMaxPitch();
}   // handleWheelie
#endif

// -----------------------------------------------------------------------------
void Kart::updatePhysics (float dt) 
{

#ifdef BULLET
    float engine_power = getActualWheelForce() + handleWheelie(dt);
    if(m_attachment.getType()==ATTACH_PARACHUTE) engine_power*=0.2;

    if(m_controls.accel)
    {   // accelerating
        m_vehicle->applyEngineForce(engine_power, 2);
        m_vehicle->applyEngineForce(engine_power, 3);
    }
    else
    {   // not accelerating
        if(m_controls.brake)
        {   // braking or moving backwards
            if(m_speed > 0.f)
            {   // going forward, apply brake force
                m_vehicle->applyEngineForce(-1.0f*getBrakeFactor()*engine_power, 2);
                m_vehicle->applyEngineForce(-1.0f*getBrakeFactor()*engine_power, 3);
            }
            else
            {   // going backward, apply reverse gear ratio
                if ( fabs(m_speed) <  m_max_speed*m_max_speed_reverse_ratio )
                {
                    m_vehicle->applyEngineForce(-m_controls.brake*engine_power, 2);
                    m_vehicle->applyEngineForce(-m_controls.brake*engine_power, 3);
                }
                else
                {
                    m_vehicle->applyEngineForce(0.f, 2);
                    m_vehicle->applyEngineForce(0.f, 3);
                }
            }
        }
        else
        {   // lift the foot from throttle, brakes with 10% engine_power
            m_vehicle->applyEngineForce(-m_controls.accel*engine_power*0.1f, 2);
            m_vehicle->applyEngineForce(-m_controls.accel*engine_power*0.1f, 3);
        }
    }

    if(m_controls.jump)
    { 
      //Vector3 impulse(0.0f, 0.0f, 10.0f);
      //        getVehicle()->getRigidBody()->applyCentralImpulse(impulse);
        btVector3 velocity         = m_kart_body->getLinearVelocity();
        velocity.setZ( 3.0f );

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );

    }
    const float steering = getMaxSteerAngle() * m_controls.lr * 0.00444;
    m_vehicle->setSteeringValue(steering, 0);
    m_vehicle->setSteeringValue(steering, 1);

    //store current velocity
    m_speed = getVehicle()->getRigidBody()->getLinearVelocity().length();

    // calculate direction of m_speed
    const btTransform& chassisTrans = getVehicle()->getChassisWorldTransform();
    btVector3 forwardW (
               chassisTrans.getBasis()[0][1],
               chassisTrans.getBasis()[1][1],
               chassisTrans.getBasis()[2][1]);

    if (forwardW.dot(getVehicle()->getRigidBody()->getLinearVelocity()) < btScalar(0.))
        m_speed *= -1.f;

    //cap at maximum velocity
    const float max_speed = m_kart_properties->getMaximumSpeed();
    if ( m_speed >  max_speed )
    {
        const float velocity_ratio = max_speed/m_speed;
        m_speed                    = max_speed;
        btVector3 velocity         = m_kart_body->getLinearVelocity();

        velocity.setY( velocity.getY() * velocity_ratio );
        velocity.setX( velocity.getX() * velocity_ratio );

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );

    }
    //at low velocity, forces on kart push it back and forth so we ignore this
    if(fabsf(m_speed) < 0.2f) // quick'n'dirty workaround for bug 1776883
         m_speed = 0;
    
#else      // ! BULLET
    m_skid_front = m_skid_rear = false;
    sgVec3 AirResistance, SysResistance;
    // Get some values once, to avoid calling them more than once.
    const float  WORLD_GRAVITY     = world->getGravity();
    const float  WHEEL_BASE   = getWheelBase();
    const float  MASS        = getMass();         // includes m_attachment.WeightAdjust
    const float  AIR_FRICTION = getAirResistance(); // includes attachmetn.AirFrictAdjust
    const float  ROLL_RESIST  = getRollResistance();

    if(m_wheelie_angle>0.0f)
    {
        m_velocity.xyz[1]-=getWheelieSpeedBoost()*m_wheelie_angle/getWheelieMaxPitch();
        if(m_velocity.xyz[1]<0) m_velocity.xyz[1]=0.0;
    }


    // Handle throttle and brakes
    // ==========================
    float throttle;

    if(m_on_ground)
    {
        if(m_controls.brake)
        {
            throttle = m_velocity.xyz[1]<0.0f ? -1.0f : -getBrakeFactor();
        }
        else
        {   // not braking
            throttle =  m_velocity.xyz[1]<0.0f ? getBrakeFactor() 
                     : m_controls.accel*m_current_friction*m_current_friction;
        }
        // Handle wheelies
        // ===============
        if ( m_controls.wheelie && m_velocity.xyz[1] >=
             getMaxSpeed()*getWheelieMaxSpeedRatio())
        {
            m_velocity.hpr[0]=0.0;
            if ( m_wheelie_angle < getWheelieMaxPitch() )
                m_wheelie_angle += getWheeliePitchRate() * dt;
            else
                m_wheelie_angle = getWheelieMaxPitch();
        }
        else if ( m_wheelie_angle > 0.0f )
        {
            m_wheelie_angle -= getWheelieRestoreRate() * dt;
            if ( m_wheelie_angle <= 0.0f ) m_wheelie_angle = 0.0f ;
        }
    }
    else
    {   // not on ground
        throttle = 0.0;
    }   // if !m_on_ground

    float ForceLong = throttle * getMaxPower();
    // apply air friction and system friction
    AirResistance[0] = 0.0f;
    AirResistance[1] = AIR_FRICTION*m_velocity.xyz[1]*fabs(m_velocity.xyz[1]);
    AirResistance[2] = 0.0f;
    SysResistance[0] = ROLL_RESIST*m_velocity.xyz[0];;
    SysResistance[1] = ROLL_RESIST*m_velocity.xyz[1];
    SysResistance[2] = 0.0f;

    //
    // Compute longitudinal acceleration for slipping
    // ----------------------------------------------
    const float FORCE_ON_REAR_TIRE = 0.5f*MASS*WORLD_GRAVITY + m_prev_accel*MASS*getHeightCOG()/WHEEL_BASE;
    const float FORCE_ON_FRONT_TIRE = MASS*WORLD_GRAVITY - FORCE_ON_REAR_TIRE;
	float maxGrip = std::max(FORCE_ON_REAR_TIRE,FORCE_ON_FRONT_TIRE)*getTireGrip();

    // If the kart is on ground, modify the grip by the friction
    // modifier for the texture/terrain.
    if(m_on_ground && m_material_hot) maxGrip *= m_material_hot->getFriction();

    // Gravity handling
    // ================
    float ForceGravity;
    if(m_on_ground)
    {
        if(m_normal_hot)
        {
            // Adjust pitch and roll according to the current terrain. To compute
            // the correspondant angles, we consider first a normalised line
            // pointing into the direction the kart is facing (pointing from (0,0,0)
            // to (x,y,0)). The angle between this line and the triangle the kart is
            // on determines the pitch. Similartly the roll is computed, using a
            // normalised line pointing to the right of the kart, for which we simply
            // use (-y,x,0).
            const float kartAngle = m_curr_pos.hpr[0]*M_PI/180.0f;
            const float X = -sin(kartAngle);
            const float Y =  cos(kartAngle);
            // Compute the angle between the normal of the plane and the line to
            // (x,y,0).  (x,y,0) is normalised, so are the coordinates of the plane,
            // simplifying the computation of the scalar product.
            float pitch = ( (*m_normal_hot)[0]*X + (*m_normal_hot)[1]*Y );  // use ( x,y,0)
            float roll  = (-(*m_normal_hot)[0]*Y + (*m_normal_hot)[1]*X );  // use (-y,x,0)

            // The actual angle computed above is between the normal and the (x,y,0)
            // line, so to compute the actual angles 90 degrees must be subtracted.
            pitch = acosf(pitch)/M_PI*180.0f-90.0f;
            roll  = acosf(roll )/M_PI*180.0f-90.0f;
            // if dt is too big, the relaxation will overshoot, and the
            // karts will either be hopping, or even turn upside down etc.
            if(dt<=0.05)
            {
# define RELAX(oldVal, newVal) (oldVal + (newVal-oldVal)*dt*20.0f)
                m_curr_pos.hpr[1] = RELAX(m_curr_pos.hpr[1],pitch);
                m_curr_pos.hpr[2] = RELAX(m_curr_pos.hpr[2],roll );
            }
            else
            {
                m_curr_pos.hpr[1] = pitch;
                m_curr_pos.hpr[2] = roll ;
            }
            if(fabsf(m_curr_pos.hpr[1])>fabsf(pitch)) m_curr_pos.hpr[1]=pitch;
            if(fabsf(m_curr_pos.hpr[2])>fabsf(roll )) m_curr_pos.hpr[2]=roll;
        }
        if(m_controls.jump)
        { // ignore gravity down when jumping
            ForceGravity = stk_config->m_jump_impulse*WORLD_GRAVITY;
        }
        else
        {   // kart is on groud and not jumping
            if(user_config->m_improved_physics)
            {
                // FIXME:
                // While these physics computation is correct, it is very difficult
                // to drive, esp. the sandtrack: the grades (with the current
                // physics parameters) are too steep, so the kart needs a very high
                // initial velocity to be able to reach the top. Especially the
                // AI gets stuck very easily! Perhaps reduce the forces somewhat?
                const float PITCH_IN_RAD = m_curr_pos.hpr[1]*M_PI/180.0f;
                ForceGravity   = -WORLD_GRAVITY * MASS * cos(PITCH_IN_RAD);
                ForceLong     -=  WORLD_GRAVITY * MASS * sin(PITCH_IN_RAD);
            }
            else
            {
                ForceGravity   = -WORLD_GRAVITY * MASS;
            }
        }
    }
    else
    {  // kart is not on ground, gravity applies all to z axis.
        ForceGravity = -WORLD_GRAVITY * MASS;
    }
    m_velocity.xyz[2] += ForceGravity / MASS * dt;


    if(m_wheelie_angle <= 0.0f && m_on_ground)
    {
        // At low speed, the advanced turning mode can result in 'flickering', i.e.
        // very quick left/right movements. The reason might be:
        // 1) integration timestep to big
        // 2) the kart turning too much, thus 'oversteering', which then gets
        //    corrected (which might be caused by wrongly tuned physics parameters,
        //    or the too big timestep mentioned above
        // Since at lower speed the simple turning algorithm is good enough,
        // the advanced sliding turn algorithm is only used at higher speeds.

        // FIXME: for now, only use the simple steering algorithm.
        //        so the test for 'lower speed' is basically disabled for now,
        //        since the velocity will always be lower than 1.5*100000.0f
        if(fabsf(m_velocity.xyz[1])<150000.0f)
        {
            const float MSA       = getMaxSteerAngle();
            m_velocity.hpr[0] = m_controls.lr * ((m_velocity.xyz[1]>=0.0f) ? MSA : -MSA);
            if(m_velocity.hpr[0]> MSA) m_velocity.hpr[0] =  MSA;  // In case the AI sets
            if(m_velocity.hpr[0]<-MSA) m_velocity.hpr[0] = -MSA;  // m_controls.lr >1 or <-1
        }
        else
        {
            const float STEER_ANGLE    = m_controls.lr*getMaxSteerAngle()*M_PI/180.0f;
            const float TURN_DISTANCE   = m_velocity.hpr[0]*M_PI/180.0f * WHEEL_BASE/2.0f;
            const float SLIP_ANGLE_FRONT = atan((m_velocity.xyz[0]+TURN_DISTANCE)
                                        /fabsf(m_velocity.xyz[1]))
                                   - sgn(m_velocity.xyz[1])*STEER_ANGLE;
            const float SLIP_ANGLE_REAR  = atan((m_velocity.xyz[0]-TURN_DISTANCE)
                                        /fabsf(m_velocity.xyz[1]));
            const float FORCE_LAT_FRONT  = NormalizedLateralForce(SLIP_ANGLE_FRONT, getCornerStiffF())
                                   * FORCE_ON_FRONT_TIRE - SysResistance[0]*0.5f;
            const float FORCE_LAT_REAR   = NormalizedLateralForce(SLIP_ANGLE_REAR,  getCornerStiffR())
                                   * FORCE_ON_REAR_TIRE  - SysResistance[0]*0.5f;
            const float CORNER_FORCE    = FORCE_LAT_REAR + cos(STEER_ANGLE)*FORCE_LAT_FRONT;
            m_velocity.xyz[0]      = CORNER_FORCE/MASS*dt;
            const float TORQUE         =                  FORCE_LAT_REAR *WHEEL_BASE/2
                                                    - cos(STEER_ANGLE)*FORCE_LAT_FRONT*WHEEL_BASE/2;
            const float ANGLE_ACCELERATION         = TORQUE/getInertia();

            m_velocity.hpr[0] += ANGLE_ACCELERATION*dt*180.0f/M_PI;
        }   // fabsf(m_velocity.xyz[1])<0.5
    }   // m_wheelie_angle <=0.0f && m_on_ground

    // Longitudinal accelleration
    // ==========================
    const float EFECTIVE_FORCE  = (ForceLong-AirResistance[1]-SysResistance[1]);
    // Slipping: more force than what can be supported by the back wheels
    // --> reduce the effective force acting on the kart - currently
    //     by an arbitrary value.
    if(fabs(EFECTIVE_FORCE)>maxGrip)
    {
        //    EFECTIVE_FORCE *= 0.4f;
        //    m_skid_rear  = true;
    }   // while EFECTIVE_FORCE>maxGrip
    float ACCELERATION       = EFECTIVE_FORCE / MASS;

    m_velocity.xyz[1] += ACCELERATION           * dt;
    m_prev_accel        = ACCELERATION;

    if(m_wheelie_angle>0.0f)
    {
        m_velocity.xyz[1]+=
            getWheelieSpeedBoost()*m_wheelie_angle/getWheelieMaxPitch();
        if(m_velocity.xyz[1]<0) m_velocity.xyz[1]=0.0;
    }
#endif
}   // updatePhysics

//-----------------------------------------------------------------------------
// PHORS recommends: f=B*alpha/(1+fabs(A*alpha)^p), where A, B, and p
//                   are appropriately chosen constants.
float Kart::NormalizedLateralForce(float alpha, float corner) const
{
    float const MAX_ALPHA=3.14f/4.0f;
    if(fabsf(alpha)<MAX_ALPHA)
    {
        return corner*alpha;
    }
    else
    {
        return alpha>0.0f ? corner*MAX_ALPHA : -corner*MAX_ALPHA;
    }
}   // NormalizedLateralForce

//-----------------------------------------------------------------------------
void Kart::forceRescue()
{
    m_rescue=true;
    // If rescue is triggered while doing a shortcut, reset the kart to the
    // segment where the shortcut started!! And then reset the shortcut
    // flag, so that this shortcut is not counted!
    if(m_shortcut_type!=SC_NONE)
    {
        m_track_sector   = m_shortcut_sector;
        m_shortcut_count-= m_shortcut_type==SC_OUTSIDE_TRACK ? 1 : 2;
        m_shortcut_type  = SC_NONE;
    } 
}   // forceRescue
//-----------------------------------------------------------------------------
/** Drops a kart which was rescued back on the track.
 */
void Kart::endRescue()
{
    if ( m_track_sector > 0 ) m_track_sector-- ;
    world ->m_track -> trackToSpatial ( m_curr_pos.xyz, m_track_sector ) ;
    m_curr_pos.hpr[0] = world->m_track->m_angle[m_track_sector] ;
    m_rescue = false ;
#ifdef BULLET
    world->getPhysics()->addKart(this, m_vehicle);
    m_kart_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_kart_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));
    // FIXME: This code positions the kart correctly back on the track
    // (nearest waypoint) - but if the kart is simply upside down,
    // it feels better if the kart is left where it was. Perhaps
    // this code should only be used if a rescue was not triggered
    // by the kart being upside down??
    btTransform pos=m_kart_body->getCenterOfMassTransform();
    pos.setOrigin(btVector3(m_curr_pos.xyz[0],m_curr_pos.xyz[1],
                            m_curr_pos.xyz[2]+0.5*m_kart_height));
    m_kart_body->setCenterOfMassTransform(pos);
    m_motion_state->setWorldTransform(pos);
                  
#endif

}   // endRescue

//-----------------------------------------------------------------------------
float Kart::getAirResistance() const
{
    return (m_kart_properties->getAirResistance() +
            m_attachment.AirResistanceAdjust()    )
           * stk_config->m_air_res_reduce[world->m_race_setup.m_difficulty];

}

//-----------------------------------------------------------------------------
void Kart::processSkidMarks()
{
    return;
    assert(m_skidmark_left);
    assert(m_skidmark_right);

    if(m_skid_rear || m_skid_front)
    {
        if(m_on_ground)
        {
            const float LENGTH = 0.57f;
            if(m_skidmark_left)
            {
                const float ANGLE  = -43.0f;

                sgCoord wheelpos;
                sgCopyCoord(&wheelpos, getCoord());

                wheelpos.xyz[0] += LENGTH * sgSin(wheelpos.hpr[0] + ANGLE);
                wheelpos.xyz[1] += LENGTH * -sgCos(wheelpos.hpr[0] + ANGLE);

                if(m_skidmark_left->wasSkidMarking())
                    m_skidmark_left->add(&wheelpos);
                else
                    m_skidmark_left->addBreak(&wheelpos);
            }   // if m_skidmark_left

            if(m_skidmark_right)
            {
                const float ANGLE  = 43.0f;

                sgCoord wheelpos;
                sgCopyCoord(&wheelpos, getCoord());

                wheelpos.xyz[0] += LENGTH * sgSin(wheelpos.hpr[0] + ANGLE);
                wheelpos.xyz[1] += LENGTH * -sgCos(wheelpos.hpr[0] + ANGLE);

                if(m_skidmark_right->wasSkidMarking())
                    m_skidmark_right->add(&wheelpos);
                else
                    m_skidmark_right->addBreak(&wheelpos);
            }   // if m_skidmark_right
        }
        else
        {   // not on ground
            if(m_skidmark_left)
            {
                const float LENGTH = 0.57f;
                const float ANGLE  = -43.0f;

                sgCoord wheelpos;
                sgCopyCoord(&wheelpos, getCoord());

                wheelpos.xyz[0] += LENGTH * sgSin(wheelpos.hpr[0] + ANGLE);
                wheelpos.xyz[1] += LENGTH * -sgCos(wheelpos.hpr[0] + ANGLE);

                m_skidmark_left->addBreak(&wheelpos);
            }   // if m_skidmark_left

            if(m_skidmark_right)
            {
                const float LENGTH = 0.57f;
                const float ANGLE  = 43.0f;

                sgCoord wheelpos;
                sgCopyCoord(&wheelpos, getCoord());

                wheelpos.xyz[0] += LENGTH * sgSin(wheelpos.hpr[0] + ANGLE);
                wheelpos.xyz[1] += LENGTH * -sgCos(wheelpos.hpr[0] + ANGLE);

                m_skidmark_right->addBreak(&wheelpos);
            }   // if m_skidmark_right
        }   // on ground
    }
    else
    {   // !m_skid_rear && !m_skid_front
        if(m_skidmark_left)
            if(m_skidmark_left->wasSkidMarking())
            {
                const float ANGLE  = -43.0f;
                const float LENGTH = 0.57f;

                sgCoord wheelpos;
                sgCopyCoord(&wheelpos, getCoord());

                wheelpos.xyz[0] += LENGTH * sgSin(wheelpos.hpr[0] + ANGLE);
                wheelpos.xyz[1] += LENGTH * -sgCos(wheelpos.hpr[0] + ANGLE);

                m_skidmark_left->addBreak(&wheelpos);
            }   // m_skidmark_left->wasSkidMarking

        if(m_skidmark_right)
            if(m_skidmark_right->wasSkidMarking())
            {
                const float ANGLE  = 43.0f;
                const float LENGTH = 0.57f;

                sgCoord wheelpos;
                sgCopyCoord(&wheelpos, getCoord());

                wheelpos.xyz[0] += LENGTH * sgSin(wheelpos.hpr[0] + ANGLE);
                wheelpos.xyz[1] += LENGTH * -sgCos(wheelpos.hpr[0] + ANGLE);

                m_skidmark_right->addBreak(&wheelpos);
            }   // m_skidmark_right->wasSkidMarking
    }   // m_velocity < 20
}   // processSkidMarks

//-----------------------------------------------------------------------------
void Kart::load_wheels(ssgBranch* branch)
{
    if (!branch) return;

    for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid())
    {
        if (i->getName())
        { // We found something that might be a wheel
            if (strcmp(i->getName(), "WheelFront.L") == 0)
            {
                m_wheel_front_l = add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else if (strcmp(i->getName(), "WheelFront.R") == 0)
            {
                m_wheel_front_r = add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else if (strcmp(i->getName(), "WheelRear.L") == 0)
            {
                m_wheel_rear_l = add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else if (strcmp(i->getName(), "WheelRear.R") == 0)
            {
                m_wheel_rear_r = add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else
            {
                // Wasn't a wheel, continue searching
                load_wheels(dynamic_cast<ssgBranch*>(i));
            }
        }
        else
        { // Can't be a wheel,continue searching
            load_wheels(dynamic_cast<ssgBranch*>(i));
        }
    }   // for i
}   // load_wheels

//-----------------------------------------------------------------------------
void Kart::loadData()
{
    float r [ 2 ] = { -10.0f, 100.0f } ;

    m_smokepuff = new ssgSimpleState ();
    m_smokepuff -> setTexture        (loader->createTexture ("smoke.rgb", true, true, true)) ;
    m_smokepuff -> setTranslucent    () ;
    m_smokepuff -> enable            ( GL_TEXTURE_2D ) ;
    m_smokepuff -> setShadeModel     ( GL_SMOOTH ) ;
    m_smokepuff -> enable            ( GL_CULL_FACE ) ;
    m_smokepuff -> enable            ( GL_BLEND ) ;
    m_smokepuff -> enable            ( GL_LIGHTING ) ;
    m_smokepuff -> setColourMaterial ( GL_EMISSION ) ;
    m_smokepuff -> setMaterial       ( GL_AMBIENT, 0, 0, 0, 1 ) ;
    m_smokepuff -> setMaterial       ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
    m_smokepuff -> setMaterial       ( GL_SPECULAR, 0, 0, 0, 1 ) ;
    m_smokepuff -> setShininess      (  0 ) ;

    ssgEntity *obj = m_kart_properties->getModel();
    createPhysics(obj);

    load_wheels(dynamic_cast<ssgBranch*>(obj));

    // Optimize the model, this can't be done while loading the model
    // because it seems that it removes the name of the wheels or something
    // else needed to load the wheels as a separate object.
    ssgFlatten(obj);

    createDisplayLists(obj);  // create all display lists
    ssgRangeSelector *lod = new ssgRangeSelector ;

    lod -> addKid ( obj ) ;
    lod -> setRanges ( r, 2 ) ;

    this-> getModel() -> addKid ( lod ) ;

    // Attach Particle System
    //JH  sgCoord pipe_pos = {{0, 0, .3}, {0, 0, 0}} ;
    m_smoke_system = new KartParticleSystem(this, 50, 100.0f, true, 0.35f, 1000);
    m_smoke_system -> init(5);
    //JH      m_smoke_system -> setState (getMaterial ("smoke.png")-> getState() );
    //m_smoke_system -> setState ( m_smokepuff ) ;
    //      m_exhaust_pipe = new ssgTransform (&pipe_pos);
    //      m_exhaust_pipe -> addKid (m_smoke_system) ;
    //      comp_model-> addKid (m_exhaust_pipe) ;

    m_skidmark_left  = new SkidMark();
    m_skidmark_right = new SkidMark();

    m_shadow = createShadow(m_kart_properties->getShadowFile(), -1, 1, -1, 1);
    m_shadow->ref();
    m_model->addKid ( m_shadow );
}   // loadData

//-----------------------------------------------------------------------------
void Kart::placeModel ()
{
    sgMat4 wheel_front;
    sgMat4 wheel_steer;
    sgMat4 wheel_rot;

    sgMakeRotMat4( wheel_rot, 0, -m_wheel_position, 0);
    sgMakeRotMat4( wheel_steer, getSteerAngle()/getMaxSteerAngle() * 30.0f , 0, 0);

    sgMultMat4(wheel_front, wheel_steer, wheel_rot);

    if (m_wheel_front_l) m_wheel_front_l->setTransform(wheel_front);
    if (m_wheel_front_r) m_wheel_front_r->setTransform(wheel_front);

    if (m_wheel_rear_l) m_wheel_rear_l->setTransform(wheel_rot);
    if (m_wheel_rear_r) m_wheel_rear_r->setTransform(wheel_rot);
    // We don't have to call Moveable::placeModel, since it does only setTransform

#ifdef BULLET
    // Only transfer the bullet data to the plib tree if no history is being
    // replayed.
    if(!user_config->m_replay_history)
    {
        btTransform t;
        if(m_rescue)
        {
            t=m_kart_body->getCenterOfMassTransform();
            //             m_motion_state->getWorldTransform(t);
        } 
        else
        {
            m_motion_state->getWorldTransform(t);
        }
        float m[4][4];
        t.getOpenGLMatrix((float*)&m);
        
        //printf(" is %f %f %f\n",t.getOrigin().x(),t.getOrigin().y(),t.getOrigin().z());
        // Transfer the new position and hpr to m_curr_pos
        sgSetCoord(&m_curr_pos, m);
        const btVector3 &v=m_kart_body->getLinearVelocity();
        sgSetVec3(m_velocity.xyz, v.x(), v.y(), v.z());
    }
    sgCoord c ;
    sgCopyCoord ( &c, &m_curr_pos ) ;
    //    c.hpr[1] += m_wheelie_angle ;
    //    c.xyz[2] += 0.3f*fabs(sin(m_wheelie_angle*SG_DEGREES_TO_RADIANS));
    const float CENTER_SHIFT = getGravityCenterShift();
    c.xyz[2] -= (0.5-CENTER_SHIFT)*m_kart_height;   // adjust for center of gravity
    m_model->setTransform(&c);
    
    // Check if a kart needs to be rescued.
    if((fabs(m_curr_pos.hpr[2])>60 &&
       sgLengthVec3(m_velocity.xyz)<3.0f) || m_time_since_stuck > 2.0f)
    {
        m_rescue=true;
        m_time_since_stuck=0.0f;
    }
#else
    sgCoord c ;
    sgCopyCoord ( &c, &m_curr_pos ) ;
    c.hpr[1] += m_wheelie_angle ;
    c.xyz[2] += 0.3f*fabs(sin(m_wheelie_angle
                              *SG_DEGREES_TO_RADIANS));
    m_model -> setTransform ( & c ) ;

#endif


}   // placeModel

//-----------------------------------------------------------------------------
void Kart::getClosestKart(float *cdist, int *closest)
{
    *cdist   = SG_MAX ;
    *closest = -1 ;

    for ( unsigned int i = 0; i < world->getNumKarts() ; ++i )
    {
        if ( world->getKart(i) == this ) continue ;
        if ( world->getKart(i)->getDistanceDownTrack() < getDistanceDownTrack() )
            continue ;

        float d = sgDistanceSquaredVec2 ( getCoord()->xyz,
                                          world->getKart(i)->getCoord()->xyz ) ;
        if ( d < *cdist && d < stk_config->m_magnet_range_sq)
        {
            *cdist = d ;
            *closest = i ;
        }
    }   // for i
}   // getClosestKart

//-----------------------------------------------------------------------------
void Kart::handleMagnet(float cdist, int closest)
{

    sgVec3 vec ;
    sgSubVec2 ( vec, world->getKart(closest)->getCoord()->xyz, getCoord()->xyz );
    vec [ 2 ] = 0.0f ;
    sgNormalizeVec3 ( vec ) ;

    sgHPRfromVec3 ( getCoord()->hpr, vec ) ;

    float tgt_velocity = world->getKart(closest)->getVelocity()->xyz[1] ;

    //JH FIXME: that should probably be changes, e.g. by increasing the throttle
    //          to something larger than 1???
    if (cdist > stk_config->m_magnet_min_range_sq)
    {
        if ( m_velocity.xyz[1] < tgt_velocity )
            m_velocity.xyz[1] = tgt_velocity * 1.4f;
    }
    else
        m_velocity.xyz[1] = tgt_velocity ;
}   // handleMagnet

//-----------------------------------------------------------------------------
void Kart::setFinishingState(float time)
{
    m_finished_race = true;
    m_finish_time   = time;
}

/* EOF */
