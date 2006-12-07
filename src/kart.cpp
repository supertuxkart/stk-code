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
#include "config.hpp"
#include "constants.hpp"
#include "shadow.hpp"
#include "track.hpp"
#include "world.hpp"
#include "kart.hpp"
#include "ssg_help.hpp"
#include "physics.hpp"
#ifdef BULLET
#include "../bullet/Demos/OpenGL/GL_ShapeDrawer.h"
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

    p->m_vel[0] += sgCos (rand()%180);
    p->m_vel[1] += sgSin (rand()%180);
    p->m_vel[2] += sgSin (rand()%100);

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
    m_kart_properties       = kartProperties_;
    m_grid_position        = position_ ;
    m_num_herrings_gobbled  = 0;
    m_finished_race         = false;
    m_finish_time           = 0.0f;
    m_prev_accel            = 0.0f;
    m_power_sliding         = false;
    m_smokepuff            = NULL;
    m_smoke_system         = NULL;
    m_exhaust_pipe         = NULL;
    m_skidmark_left        = NULL;
    m_skidmark_right       = NULL;
    sgCopyCoord(&m_reset_pos, &init_pos);
    // Neglecting the roll resistance (which is small for high speeds compared
    // to the air resistance), maximum speed is reached when the engine
    // power equals the air resistance force, resulting in this formula:
    m_max_speed             = sqrt(getMaxPower()/getAirResistance());

    m_wheel_position = 0;

    m_wheel_front_l = NULL;
    m_wheel_front_r = NULL;
    m_wheel_rear_l  = NULL;
    m_wheel_rear_r  = NULL;
    loadData();
}   // Kart

// -----------------------------------------------------------------------------v
#ifdef BULLET
void Kart::createPhysics(ssgEntity *obj)
{
    // First: Create the chassis of the kart
    // -------------------------------------
    // The size for bullet must be specified in half extends!
    //    ssgEntity *model = getModel();
    float x_min, x_max, y_min, y_max, z_min, z_max;
    MinMax(obj, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
    float kart_width  = x_max-x_min;
    float kart_length = y_max-y_min;
    if(kart_length<1.2) kart_length=1.5f;

    // The kart height is needed later to reset the physics to the correct
    // position.
    m_kart_height     = z_max-z_min;

    btCollisionShape *kart_chassis = new btBoxShape(btVector3(0.5*kart_width,
                                                              0.5*kart_length,
                                                              0.5*m_kart_height));
    // Set mass and inertia
    // --------------------
    float mass=getMass();
    btVector3 inertia;
    kart_chassis->calculateLocalInertia(mass, inertia);

    // Position the chassis
    // --------------------
    btTransform trans;
    trans.setIdentity();
    btDefaultMotionState* myMotionState = new btDefaultMotionState(trans);

    // Then create a rigid body
    // ------------------------
    m_kart_body = new btRigidBody(mass, myMotionState, 
                                  kart_chassis, inertia);
    m_kart_body->setDamping(0.2, 0.2);

    // Reset velocities
    // ----------------
    m_kart_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_kart_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));

    // Create the actual vehicle
    // -------------------------
    btVehicleRaycaster *vehicle_raycaster = 
        new btDefaultVehicleRaycaster(world->getPhysics()->getPhysicsWorld());
    m_tuning = new btRaycastVehicle::btVehicleTuning();
    m_vehicle = new btRaycastVehicle(*m_tuning, m_kart_body, vehicle_raycaster);

    // never deactivate the vehicle
    m_kart_body->setActivationState(DISABLE_DEACTIVATION);
    m_vehicle->setCoordinateSystem(/*right: */ 0,  /*up: */ 2,  /*forward: */ 1);
    
    // Add wheels
    // ----------
    float wheel_width  = 0.3*kart_width;
    float wheel_radius = getWheelRadius();
    float suspension_rest = 0;
    float connection_height = -0.5*m_kart_height;
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
        wheel.m_suspensionStiffness      = 20.0f;
        wheel.m_wheelsDampingRelaxation  = 2.3f;
        wheel.m_wheelsDampingCompression = 4.4f;
        wheel.m_frictionSlip             = 1e30f;
        wheel.m_rollInfluence            = 0.1f;
    }
    world->getPhysics()->addKart(this, m_vehicle);

}   // createPhysics
#endif

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
void Kart::reset()
{
    Moveable::reset();

    m_race_lap             = -1;
    m_race_position        = 9;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_zipper_time_left     = 0.0f ;
    m_rescue               = false;
    m_attachment.clear();
    m_collectable.clear();
    m_num_herrings_gobbled = 0;
    m_wheel_position       = 0;
    m_track_hint = world -> m_track -> absSpatialToTrack(m_curr_track_coords,
                                                         m_curr_pos.xyz);
#ifdef BULLET
    btTransform *trans=new btTransform();
    trans->setIdentity();
    trans->setOrigin(btVector3(m_reset_pos.xyz[0],
                               m_reset_pos.xyz[1],
                               m_reset_pos.xyz[2]+0.5*m_kart_height));
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
    if (      m_last_track_coords[1] > 100.0f && m_curr_track_coords[1] <  20.0f )
    {
        setTimeAtLap(world->m_clock);
        m_race_lap++ ;
    }
    else if ( m_curr_track_coords[1] > 100.0f && m_last_track_coords[1] <  20.0f )
        m_race_lap-- ;
}   // doLapCounting

//-----------------------------------------------------------------------------
void Kart::doObjectInteractions ()
{
    int i;
    for ( i = 0 ; i < m_grid_position ; i++ )
    {
        sgVec3 xyz ;

        sgSubVec3(xyz, getCoord()->xyz, world->getKart(i)->getCoord()->xyz );

        if ( sgLengthSquaredVec2 ( xyz ) < 1.0f )
        {
            sgNormalizeVec2 ( xyz ) ;
            world->addCollisions(m_grid_position, 1);
            world->addCollisions(i,             1);
            if ( m_velocity.xyz[1] > world->getKart(i)->getVelocity()->xyz[1] )
            {
                forceCrash () ;
                sgSubVec2 ( world->getKart(i)->getCoord()->xyz, xyz ) ;
            }
            else
            {
                world->getKart(i)->forceCrash () ;
                sgAddVec2 ( getCoord()->xyz, xyz ) ;
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

//-----------------------------------------------------------------------------
void Kart::forceCrash ()
{

    m_wheelie_angle = CRASH_PITCH ;

    m_velocity.xyz[0] = m_velocity.xyz[1] = m_velocity.xyz[2] =
    m_velocity.hpr[0] = m_velocity.hpr[1] = m_velocity.hpr[2] = 0.0f ;
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
    //m_wheel_position gives the rotation around the X-axis, and since velocity's
    //timeframe is the delta time, we don't have to multiply it with dt.
    m_wheel_position += m_velocity.xyz[1];

    if ( m_rescue )
    {
        if(m_attachment.getType() != ATTACH_TINYTUX)
        {
            if(isPlayerKart()) sound_manager -> playSfx ( SOUND_BZZT );
            m_attachment.set( ATTACH_TINYTUX, 2.0f ) ;
        }
    }
    m_attachment.update(dt, &m_velocity);

    /*smoke drawing control point*/
    if ( config->m_smoke )
    {
        if (m_smoke_system != NULL)
            m_smoke_system->update (dt);
    }  // config->smoke
    doZipperProcessing (dt) ;
    updatePhysics(dt);

    sgCopyVec2  ( m_last_track_coords, m_curr_track_coords );
    Moveable::update (dt) ;
    doObjectInteractions();

    m_track_hint = world->m_track->spatialToTrack(
                    m_curr_track_coords, m_curr_pos.xyz, m_track_hint );

    doLapCounting () ;
    processSkidMarks();

}   // update

//-----------------------------------------------------------------------------
#define sgn(x) ((x<0)?-1.0f:((x>0)?1.0f:0.0f))
#define max(m,n) ((m)>(n) ? (m) : (n))
#define min(m,n) ((m)<(n) ? (m) : (n))

// -----------------------------------------------------------------------------
#ifdef BULLET
void Kart::draw()
{
    float m[16];
    btDefaultMotionState *my_motion_state =
        (btDefaultMotionState*)m_kart_body->getMotionState();
    my_motion_state->m_graphicsWorldTrans.getOpenGLMatrix(m);
    btTransform t;
    my_motion_state->getWorldTransform(t);
    btQuaternion q= t.getRotation();

    btVector3 wire_color(0.5f, 0.5f, 0.5f);
    world->getPhysics()->debugDraw(m, m_kart_body->getCollisionShape(), 
                                   wire_color);
    btCylinderShapeX *wheelShape = new btCylinderShapeX(btVector3(0.3,
                                                                  getWheelRadius(), 
                                                                  getWheelRadius()));
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
void Kart::updatePhysics (float dt) 
{

#ifdef BULLET
    if(m_controls.brake)
    {
        m_vehicle->applyEngineForce(-m_controls.brake*getMaxPower(), 2);
        m_vehicle->applyEngineForce(-m_controls.brake*getMaxPower(), 3);
    }
    else if(m_controls.accel)
    {   // not braking
        m_vehicle->applyEngineForce(m_controls.accel*getMaxPower(), 2);
        m_vehicle->applyEngineForce(m_controls.accel*getMaxPower(), 3);
    }
    if(m_controls.jump)
    { // ignore gravity down when jumping
        // no jumping yet
    }
    const float steering = getMaxSteerAngle() * m_controls.lr*0.00444;
    m_vehicle->setSteeringValue(steering, 0);
    m_vehicle->setSteeringValue(steering, 1);
    
#else      // ! BULLET
    m_skid_front = m_skid_rear = false;
    sgVec3 AirResistance, SysResistance;
    // Get some values once, to avoid calling them more than once.
    const float  WORLD_GRAVITY     = world->getGravity();
    const float  WHEEL_BASE   = getWheelBase();
    const float  MASS        = getMass();         // includes m_attachment.WeightAdjust
    const float  AIR_FRICTION = getAirResistance(); // includes attachmetn.AirFrictAdjust
    const float  ROLL_RESIST  = getRollResistance();

    //  if(materialHOT) ROLL_RESIST +=materialHOT->getFriction();

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
            throttle = m_velocity.xyz[1]<0.0 ? -1.0f : -getBrakeFactor();
        }
        else
        {   // not braking
            throttle =  m_controls.accel;
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
    float maxGrip = max(FORCE_ON_REAR_TIRE,FORCE_ON_FRONT_TIRE)*getTireGrip();

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
            ForceGravity = physicsParameters->m_jump_impulse*WORLD_GRAVITY;
        }
        else
        {   // kart is on groud and not jumping
            if(config->m_improved_physics)
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
void Kart::handleRescue()
{
    if ( m_track_hint > 0 ) m_track_hint-- ;
    world ->m_track -> trackToSpatial ( m_curr_pos.xyz, m_track_hint ) ;
    m_curr_pos.hpr[0] = world->m_track->m_angle[m_track_hint] ;
    m_rescue = false ;
}   // handleRescue

//-----------------------------------------------------------------------------
float Kart::getAirResistance() const
{
    return (m_kart_properties->getAirResistance() +
            m_attachment.AirResistanceAdjust()    )
           * physicsParameters->m_air_res_reduce[world->m_race_setup.m_difficulty];
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
#ifdef BULLET
    createPhysics(obj);
#endif

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
    float m[4][4];
    btTransform t1;
    m_kart_body->getMotionState()->getWorldTransform(t1);
    // expects an array of float, not float[4][4] 
    t1.getOpenGLMatrix((float*)&m);

    // Transfer the new position and hpr to m_curr_pos
    sgSetCoord(&m_curr_pos, m);
    const btVector3 &v=m_kart_body->getLinearVelocity();
    sgSetVec3(m_velocity.xyz, v.x(), v.y(), v.z());

    sgCoord c ;
    sgCopyCoord ( &c, &m_curr_pos ) ;
    c.hpr[1] += m_wheelie_angle ;
    c.xyz[2] -= 0.5*m_kart_height;   // adjust for center of gravity
    c.xyz[2] += 0.3f*fabs(sin(m_wheelie_angle*SG_DEGREES_TO_RADIANS));
    m_model->setTransform(&c);
    
    // Check if a kart needs to be rescued.
    if(fabs(m_curr_pos.hpr[2])>60 &&
       sgLengthVec3(m_velocity.xyz)<3.0f) m_rescue=true;
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

    for ( int i = 0; i < world->getNumKarts() ; ++i )
    {
        if ( world->getKart(i) == this ) continue ;
        if ( world->getKart(i)->getDistanceDownTrack() < getDistanceDownTrack() )
            continue ;

        float d = sgDistanceSquaredVec2 ( getCoord()->xyz,
                                          world->getKart(i)->getCoord()->xyz ) ;
        if ( d < *cdist && d < physicsParameters->m_magnet_range_sq)
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
    if (cdist > physicsParameters->m_magnet_min_range_sq)
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
