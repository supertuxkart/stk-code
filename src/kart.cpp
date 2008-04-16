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
#include <math.h>
#include <iostream>
#include <plib/ssg.h>
#include "bullet/Demos/OpenGL/GL_ShapeDrawer.h"

#include "loader.hpp"
#include "herring_manager.hpp"
#include "sound_manager.hpp"
#include "file_manager.hpp"
#include "skid_mark.hpp"
#include "user_config.hpp"
#include "constants.hpp"
#include "shadow.hpp"
#include "track.hpp"
#include "world.hpp"
#include "kart.hpp"
#include "ssg_help.hpp"
#include "physics.hpp"
#include "kart_properties_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/race_gui.hpp"
#include "translation.hpp"
#include "smoke.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif


Kart::Kart (const std::string& kart_name, int position_ ,
            sgCoord init_pos) 
    : TerrainInfo(1),
#if defined(WIN32) && !defined(__CYGWIN__)
   // Disable warning for using 'this' in base member initializer list
#  pragma warning(disable:4355)
#endif
       Moveable(true), m_attachment(this), m_collectable(this)
#if defined(WIN32) && !defined(__CYGWIN__)
#  pragma warning(1:4355)
#endif
{
    m_kart_properties      = kart_properties_manager->getKart(kart_name);
    m_grid_position        = position_ ;
    m_num_herrings_gobbled = 0;
    m_eliminated           = false;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_wheelie_angle        = 0.0f;
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
    m_max_speed               = m_kart_properties->getMaximumSpeed();
    m_max_speed_reverse_ratio = m_kart_properties->getMaxSpeedReverseRatio();
    m_speed                   = 0.0f;

    // Setting rescue to false is important! If rescue is set when reset() is
    // called, it is assumed that this was triggered by a restart, and that
    // the vehicle must be added back to the physics world. Since reset() is
    // also called at the very start, it must be guaranteed that rescue is
    // not set.
    m_rescue                  = false;
    m_wheel_rotation          = 0;

    m_wheel_front_l           = NULL;
    m_wheel_front_r           = NULL;
    m_wheel_rear_l            = NULL;
    m_wheel_rear_r            = NULL;
    m_lap_start_time          = -1.0f;
    loadData();
}   // Kart

// -----------------------------------------------------------------------------v
void Kart::createPhysics(ssgEntity *obj)
{
    // First: Create the chassis of the kart
    // -------------------------------------
    // The size for bullet must be specified in half extends!
    //    ssgEntity *model = getModel();
    float x_min, x_max, y_min, y_max, z_min, z_max;
    MinMax(obj, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
    if(getName()=="Hexley" || getName()=="Wilber")
    {
        // These kart models are too small, so we get problems with stability. 
        // Till we find either better (bigger) models or improve their physics 
        // parameters to become playable, we just adjust the size of their 
        // physical models to be the same as the tuxkart model
        x_min=-0.473799f;
        x_max= 0.486361f;
        y_min=-0.772244f;
        y_max= 0.739075f;
        z_min= 0.002806f;
        z_max= 0.701095f;
    }
    float kart_width  = x_max-x_min;
    m_kart_length = y_max-y_min;
    if(m_kart_length<1.2) m_kart_length=1.5f;

    // The kart height is needed later to reset the physics to the correct
    // position.
    m_kart_height  = z_max-z_min;

    btBoxShape *shape = new btBoxShape(btVector3(0.5f*kart_width,
                                                 0.5f*m_kart_length,
                                                 0.5f*m_kart_height));
    btTransform shiftCenterOfGravity;
    shiftCenterOfGravity.setIdentity();
    // Shift center of gravity downwards, so that the kart 
    // won't topple over too easy. This must be between 0 and 0.5
    // (it's in units of kart_height)
    const float CENTER_SHIFT = getGravityCenterShift();
    shiftCenterOfGravity.setOrigin(btVector3(0.0f,0.0f,CENTER_SHIFT*m_kart_height));

    m_kart_chassis.addChildShape(shiftCenterOfGravity, shape);

    // Set mass and inertia
    // --------------------
    float mass=getMass();

    // Position the chassis
    // --------------------
    btTransform trans;
    trans.setIdentity();
    createBody(mass, trans, &m_kart_chassis);
    m_user_pointer.set(this);
    m_body->setDamping(m_kart_properties->getChassisLinearDamping(), 
                       m_kart_properties->getChassisAngularDamping() );

    // Reset velocities
    // ----------------
    m_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));

    // Create the actual vehicle
    // -------------------------
    m_vehicle_raycaster = 
        new btDefaultVehicleRaycaster(world->getPhysics()->getPhysicsWorld());
    m_tuning  = new btRaycastVehicle::btVehicleTuning();
    m_vehicle = new btRaycastVehicle(*m_tuning, m_body, m_vehicle_raycaster);

    // never deactivate the vehicle
    m_body->setActivationState(DISABLE_DEACTIVATION);
    m_vehicle->setCoordinateSystem(/*right: */ 0,  /*up: */ 2,  /*forward: */ 1);
    
    // Add wheels
    // ----------
    float wheel_width  = m_kart_properties->getWheelWidth();
    float wheel_radius = m_kart_properties->getWheelRadius();
    float suspension_rest = m_kart_properties->getSuspensionRest();
    float connection_height = -(0.5f-CENTER_SHIFT)*m_kart_height;
    btVector3 wheel_direction(0.0f, 0.0f, -1.0f);
    btVector3 wheel_axle(1.0f,0.0f,0.0f);

    // right front wheel
    btVector3 wheel_coord(0.5f*kart_width-0.3f*wheel_width,
                          0.5f*m_kart_length-wheel_radius,
                          connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ true);

    // left front wheel
    wheel_coord = btVector3(-0.5f*kart_width+0.3f*wheel_width,
                            0.5f*m_kart_length-wheel_radius,
                            connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ true);

    // right rear wheel
    wheel_coord = btVector3(0.5f*kart_width-0.3f*wheel_width, 
                            -0.5f*m_kart_length+wheel_radius,
                            connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ false);

    // right rear wheel
    wheel_coord = btVector3(-0.5f*kart_width+0.3f*wheel_width,
                            -0.5f*m_kart_length+wheel_radius,
                            connection_height);
    m_vehicle->addWheel(wheel_coord, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ false);

    for(int i=0; i<m_vehicle->getNumWheels(); i++)
    {
        btWheelInfo& wheel               = m_vehicle->getWheelInfo(i);
        wheel.m_suspensionStiffness      = m_kart_properties->getSuspensionStiffness();
        wheel.m_wheelsDampingRelaxation  = m_kart_properties->getWheelDampingRelaxation();
        wheel.m_wheelsDampingCompression = m_kart_properties->getWheelDampingCompression();
        wheel.m_frictionSlip             = m_kart_properties->getFrictionSlip();
        wheel.m_rollInfluence            = m_kart_properties->getRollInfluence();
    }
    world->getPhysics()->addKart(this, m_vehicle);

}   // createPhysics

// -----------------------------------------------------------------------------
Kart::~Kart() 
{
    if(m_smokepuff) delete m_smokepuff;
    if(m_smoke_system != NULL) delete m_smoke_system;

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

    delete m_vehicle;
    delete m_tuning;
    delete m_vehicle_raycaster;
    world->getPhysics()->removeKart(this);
    for(int i=0; i<m_kart_chassis.getNumChildShapes(); i++)
    {
        delete m_kart_chassis.getChildShape(i);
    }
}   // ~Kart

//-----------------------------------------------------------------------------
void Kart::eliminate()
{
    m_eliminated = true;
    world->getPhysics()->removeKart(this);

    // make the kart invisible by placing it way under the track
    sgVec3 hell; hell[0]=0.0f; hell[1]=0.0f; hell[2] = -10000.0f;
    getModelTransform()->setTransform(hell);
}   // eliminate

//-----------------------------------------------------------------------------
/** Returns true if the kart is 'resting'
 *
 * Returns true if the kart is 'resting', i.e. (nearly) not moving.
 */
bool Kart::isInRest() const
{
    return fabs(m_body->getLinearVelocity ().z())<0.2;
}  // isInRest

//-----------------------------------------------------------------------------
/** Modifies the physics parameter to simulate an attached anvil.
 *  The velocity is multiplicated by f, and the mass of the kart is increased.
 */
void Kart::adjustSpeedWeight(float f)
{
    m_body->setLinearVelocity(m_body->getLinearVelocity()*f);
    // getMass returns the mass increased by the attachment
    btVector3 inertia;
    float m=getMass();
    m_kart_chassis.calculateLocalInertia(m, inertia);
    m_body->setMassProps(m, inertia);
}   // adjustSpeedWeight

//-----------------------------------------------------------------------------
void Kart::reset()
{
    if(m_eliminated)
    {
        world->getPhysics()->addKart(this, m_vehicle);
    }
    Moveable::reset();

    m_attachment.clear();
    m_collectable.clear();

    m_race_lap             = -1;
    m_lap_start_time       = -1.0f;
    m_time_at_last_lap     = 99999.9f;
    m_shortcut_sector      = Track::UNKNOWN_SECTOR;
    m_race_position        = 9;
    m_finished_race        = false;
    m_eliminated           = false;
    m_finish_time          = 0.0f;
    m_zipper_time_left     = 0.0f;
    m_num_herrings_gobbled = 0;
    m_wheel_rotation       = 0;
    m_wheelie_angle        = 0.0f;

    m_controls.lr      = 0.0f;
    m_controls.accel   = 0.0f;
    m_controls.brake   = false;
    m_controls.wheelie = false;
    m_controls.jump    = false;
    m_controls.fire    = false;

    // Set the brakes so that karts don't slide downhill
    for(int i=0; i<4; i++) m_vehicle->setBrake(5.0f, i);

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

    m_vehicle->applyEngineForce (0.0f, 2);
    m_vehicle->applyEngineForce (0.0f, 3);
    // Set heading:
    m_transform.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), 
                                         DEGREE_TO_RAD(m_reset_pos.hpr[0])) );
    // Set position
    m_transform.setOrigin(btVector3(m_reset_pos.xyz[0],
                                    m_reset_pos.xyz[1],
                                    m_reset_pos.xyz[2]+0.5f*m_kart_height));
    m_body->setCenterOfMassTransform(m_transform);
    m_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));
    m_motion_state->setWorldTransform(m_transform);
    for(int j=0; j<m_vehicle->getNumWheels(); j++)
    {
        m_vehicle->updateWheelTransform(j, true);
    }

    // if the kart was being rescued when a restart is called,
    // add the vehicle back into the physical world!
    if(m_rescue)
    {
        world->getPhysics()->addKart(this, m_vehicle);
    }
    m_rescue               = false;

    placeModel();
    TerrainInfo::update(m_transform.getOrigin());
}   // reset

//-----------------------------------------------------------------------------
void Kart::doLapCounting ()
{
    bool newLap = m_last_track_coords[1] > 300.0f && m_curr_track_coords[1] <  20.0f;
    if ( newLap )
    {
        // Only increase the lap counter and set the new time if the
        // kart hasn't already finished the race (otherwise the race_gui
        // will begin another countdown).
        if(m_race_lap+1<=race_manager->getNumLaps())
        {
            setTimeAtLap(world->getTime());
            m_race_lap++ ;
        }
        // Sound manager makes sure that only the first call switches the music
        if(m_race_lap==race_manager->getNumLaps()-1)   // last lap started
        {
            sound_manager->switchToFastMusic();
        }
        // Only do timings if original time was set properly. Driving backwards
        // over the start line will cause the lap start time to be set to -1.
        if(m_lap_start_time>=0.0)
        {
            float time_per_lap;
            if (m_race_lap == 1) // just completed first lap
            {
            	time_per_lap=world->getTime();
            }
            else //completing subsequent laps
            {
            	time_per_lap=world->getTime()-m_lap_start_time;
            }
                        
            if(time_per_lap < world->getFastestLapTime() &&
                race_manager->raceHasLaps())
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
        m_lap_start_time = world->getTime();
    }
    else if ( m_curr_track_coords[1] > 300.0f && m_last_track_coords[1] <  20.0f)
    {
        m_race_lap-- ;
        // Prevent cheating by setting time to a negative number, indicating
        // that the line wasn't crossed properly.
        m_lap_start_time = -1.0f;
    } else
    {   // Switch to fast music in case of follow the leader when only 3 karts are left
        if(race_manager->getRaceMode()==RaceManager::RM_FOLLOW_LEADER &&
            world->getCurrentNumKarts()==3)  
        {
            sound_manager->switchToFastMusic();
        }
    }
}   // doLapCounting

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
// Simulates gears
float Kart::getActualWheelForce()
{
    float zipperF=(m_zipper_time_left>0.0f) ? stk_config->m_zipper_force : 0.0f;
    const std::vector<float>& gear_ratio=m_kart_properties->getGearSwitchRatio();
    for(unsigned int i=0; i<gear_ratio.size(); i++)
    {
        if(m_speed <= m_max_speed*gear_ratio[i]) 
            return getMaxPower()*m_kart_properties->getGearPowerIncrease()[i]+zipperF;
    }
    return getMaxPower()+zipperF;

}   // getActualWheelForce

//-----------------------------------------------------------------------------
/** The kart is on ground if all 4 wheels touch the ground
*/
bool Kart::isOnGround() const
{
    return m_vehicle->getWheelInfo(0).m_raycastInfo.m_isInContact &&
           m_vehicle->getWheelInfo(1).m_raycastInfo.m_isInContact &&
           m_vehicle->getWheelInfo(2).m_raycastInfo.m_isInContact &&
           m_vehicle->getWheelInfo(3).m_raycastInfo.m_isInContact;
}   // isOnGround
//-----------------------------------------------------------------------------
void Kart::handleExplosion(const sgVec3& pos, bool direct_hit)
{
    if(direct_hit) {
        btVector3 velocity = m_body->getLinearVelocity();

        velocity.setX( 0.0f );
        velocity.setY( 0.0f );
        velocity.setZ( 3.5f );

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );
    }
    else  // only affected by a distant explosion
    {
        sgVec3 diff;
        sgSubVec3(diff, getCoord()->xyz, pos);
        float len2=sgLengthSquaredVec3(diff);

        // The correct formhale would be to first normalise diff,
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
}   // handleExplosion

//-----------------------------------------------------------------------------
void Kart::update(float dt)
{
    m_zipper_time_left = m_zipper_time_left>0.0f ? m_zipper_time_left-dt : 0.0f;

    //m_wheel_rotation gives the rotation around the X-axis, and since velocity's
    //timeframe is the delta time, we don't have to multiply it with dt.
    m_wheel_rotation += m_speed*dt / m_kart_properties->getWheelRadius();
    m_wheel_rotation=fmodf(m_wheel_rotation, 2*M_PI);

    if ( m_rescue )
    {
        // Let the kart raise 2m in the 2 seconds of the rescue
        const float rescue_time   = 2.0f;
        const float rescue_height = 2.0f;
        if(m_attachment.getType() != ATTACH_TINYTUX)
        {
            if(isPlayerKart()) sound_manager -> playSfx ( SOUND_BZZT );
            m_attachment.set( ATTACH_TINYTUX, rescue_time ) ;
            m_rescue_pitch = m_curr_pos.hpr[1];
            m_rescue_roll  = m_curr_pos.hpr[2];
            world->getPhysics()->removeKart(this);
        }
        m_curr_pos.xyz[2] += rescue_height*dt/rescue_time;

        m_transform.setOrigin(btVector3(m_curr_pos.xyz[0],m_curr_pos.xyz[1],
                                        m_curr_pos.xyz[2]));
        btQuaternion q_roll (btVector3(0.f, 1.f, 0.f),
                             -m_rescue_roll*dt/rescue_time*M_PI/180.0f);
        btQuaternion q_pitch(btVector3(1.f, 0.f, 0.f),
                             -m_rescue_pitch*dt/rescue_time*M_PI/180.0f);
        m_transform.setRotation(m_transform.getRotation()*q_roll*q_pitch);
        m_body->setCenterOfMassTransform(m_transform);

        //printf("Set %f %f %f\n",pos.getOrigin().x(),pos.getOrigin().y(),pos.getOrigin().z());     
    }   // if m_rescue
    m_attachment.update(dt);

    /*smoke drawing control point*/
    if ( user_config->m_smoke )
    {
        if (m_smoke_system != NULL)
            m_smoke_system->update (dt);
    }  // user_config->smoke
    updatePhysics(dt);

    sgCopyVec2  ( m_last_track_coords, m_curr_track_coords );
    
    Moveable::update(dt);

    // Check if a kart is (nearly) upside down and not moving much --> automatic rescue
    if((fabs(m_curr_pos.hpr[2])>60 && fabs(getSpeed())<3.0f) )
    {
        forceRescue();
    }

    btTransform trans=getTrans();
    // Add a certain epsilon (0.1) to the height of the kart. This avoids
    // problems of the ray being cast from under the track (which happened
    // e.g. on tux tollway when jumping down from the ramp).
    btVector3 pos_plus_epsilon = trans.getOrigin()+btVector3(0,0,0.1f);
    TerrainInfo::update(pos_plus_epsilon);
    if (getHoT()==Track::NOHIT                   || 
       (getMaterial()->isReset() && isOnGround())   )
    {
        forceRescue();
    }
    else if(getMaterial()->isZipper())
    {
        handleZipper();
    }
    else
    {
        for(int i=0; i<m_vehicle->getNumWheels(); i++)
        {
            // terrain dependent friction
            m_vehicle->getWheelInfo(i).m_frictionSlip = getFrictionSlip() * 
                                                        getMaterial()->getFriction();
        }   // for i<getNumWheels

    }   // if there is terrain and it's not a reset material

    // Check if any herring was hit.
    herring_manager->hitHerring(this);

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
    if(!m_rescue && world->m_track->isShortcut(prev_sector, m_track_sector))
    {
        if(isPlayerKart())
        {
            forceRescue(/*is rescue*/ true);  // bring karts back to where they left the track.
            RaceGUI* m=(RaceGUI*)menu_manager->getRaceMenu();
            // Can happen if the option menu is called
            if(m)
                m->addMessage(_("Invalid short-cut!!"), this, 2.0f, 60);
        }
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

    doLapCounting () ;
    processSkidMarks();
}   // update

//-----------------------------------------------------------------------------
// Set zipper time, and apply one time additional speed boost
void Kart::handleZipper()
{
    m_zipper_time_left  = stk_config->m_zipper_time;
    const btVector3& v  = m_body->getLinearVelocity();
    float current_speed = v.length();
    float speed         = std::min(current_speed+stk_config->m_zipper_speed_gain, 
                                   getMaxSpeed());
    
    m_body->setLinearVelocity(v/current_speed*speed);
}   // handleZipper
//-----------------------------------------------------------------------------
#define sgn(x) ((x<0)?-1.0f:((x>0)?1.0f:0.0f))

// -----------------------------------------------------------------------------
void Kart::draw()
{
    float m[16];
    btTransform t=getTrans();
    t.getOpenGLMatrix(m);

    btVector3 wire_color(0.5f, 0.5f, 0.5f);
    world->getPhysics()->debugDraw(m, m_body->getCollisionShape(), 
                                   wire_color);
    btCylinderShapeX wheelShape( btVector3(0.3f,
                                        m_kart_properties->getWheelRadius(),
                                        m_kart_properties->getWheelRadius()));
    btVector3 wheelColor(1,0,0);
    for(int i=0; i<m_vehicle->getNumWheels(); i++)
    {
        m_vehicle->updateWheelTransform(i, true);
        float m[16];
        m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
        world->getPhysics()->debugDraw(m, &wheelShape, wheelColor);
    }
}   // draw

// -----------------------------------------------------------------------------
/** Returned an additional engine power boost when doing a wheele.
***/

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

    const btTransform& chassisTrans = getTrans();
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
    btVector3 angvel            = m_body->getAngularVelocity();
    float projvel               = angvel.dot(crossProd);
    float deltavel              = -projvel    * gLeanRecovery    / step
                                  -deltaalpha * balance_recovery / step;
    btVector3 deltaangvel       = deltavel * crossProd;
    
    angvel                     += deltaangvel;
    m_body->setAngularVelocity(angvel);
    
    return m_kart_properties->getWheeliePowerBoost() * getMaxPower()
          * m_wheelie_angle/getWheelieMaxPitch();
}   // handleWheelie

// -----------------------------------------------------------------------------
/** This function is called when the race starts. Up to then all brakes are
    braking (to avoid the kart from rolling downhill), but they need to be set
    to zero (otherwise the brakes will be braking whenever no engine force
    is set, i.e. the kart is not accelerating).
    */
void Kart::resetBrakes()
{
    for(int i=0; i<4; i++) m_vehicle->setBrake(0.0f, i);
}   // resetBrakes
// -----------------------------------------------------------------------------
void Kart::updatePhysics (float dt) 
{
    float engine_power = getActualWheelForce() + handleWheelie(dt);
    if(m_attachment.getType()==ATTACH_PARACHUTE) engine_power*=0.2f;

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
                m_vehicle->applyEngineForce(-getBrakeFactor()*engine_power, 2);
                m_vehicle->applyEngineForce(-getBrakeFactor()*engine_power, 3);
            }
            else
            {   // going backward, apply reverse gear ratio
                if ( fabs(m_speed) <  m_max_speed*m_max_speed_reverse_ratio )
                {
                    m_vehicle->applyEngineForce(-engine_power*m_controls.brake, 2);
                    m_vehicle->applyEngineForce(-engine_power*m_controls.brake, 3);
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

    if(isOnGround() && m_controls.jump)
    { 
      //Vector3 impulse(0.0f, 0.0f, 10.0f);
      //        getVehicle()->getRigidBody()->applyCentralImpulse(impulse);
        btVector3 velocity         = m_body->getLinearVelocity();
        velocity.setZ( m_kart_properties->getJumpVelocity() );

        getBody()->setLinearVelocity( velocity );

    }
    const float steering = DEGREE_TO_RAD(getMaxSteerAngle()) * m_controls.lr;
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
        btVector3 velocity         = m_body->getLinearVelocity();

        velocity.setY( velocity.getY() * velocity_ratio );
        velocity.setX( velocity.getX() * velocity_ratio );

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );

    }
    //at low velocity, forces on kart push it back and forth so we ignore this
    if(fabsf(m_speed) < 0.2f) // quick'n'dirty workaround for bug 1776883
         m_speed = 0;
}   // updatePhysics


//-----------------------------------------------------------------------------
void Kart::forceRescue(bool is_shortcut)
{
    m_rescue=true;
    // If rescue is triggered while doing a shortcut, reset the kart to the
    // segment where the shortcut started!! And then reset the shortcut
    // flag, so that this shortcut is not counted!
    if(is_shortcut)
    {
        m_track_sector   = m_shortcut_sector;
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

    m_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));
    // FIXME: This code positions the kart correctly back on the track
    // (nearest waypoint) - but if the kart is simply upside down,
    // it feels better if the kart is left where it was. Perhaps
    // this code should only be used if a rescue was not triggered
    // by the kart being upside down??
    btTransform pos;
    // A certain epsilon is added here to the Z coordinate (0.1), in case
    // that the drivelines are somewhat under the track. Otherwise, the
    // kart will be placed a little bit under the track, triggering
    // a rescue, ...
    pos.setOrigin(btVector3(m_curr_pos.xyz[0],m_curr_pos.xyz[1],
                            m_curr_pos.xyz[2]+0.5f*m_kart_height+0.1f));
    pos.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), 
                                 DEGREE_TO_RAD(world->m_track->m_angle[m_track_sector])));
    m_body->setCenterOfMassTransform(pos);
    world->getPhysics()->addKart(this, m_vehicle);
    setTrans(pos);
}   // endRescue

//-----------------------------------------------------------------------------
void Kart::processSkidMarks()
{
    assert(m_skidmark_left);
    assert(m_skidmark_right);
    const float threshold=0.3f;
    bool skid_front = m_vehicle->getWheelInfo(0).m_skidInfo < threshold ||
                      m_vehicle->getWheelInfo(1).m_skidInfo < threshold;
    bool skid_rear  = m_vehicle->getWheelInfo(2).m_skidInfo < threshold ||
                      m_vehicle->getWheelInfo(3).m_skidInfo < threshold;
    if(skid_rear || skid_front)
    {
        if(isOnGround())
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
    {   // !skid_rear && _skid_front
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
    m_smokepuff -> setTexture        (loader->createTexture("smoke.rgb", true, true, true)) ;
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

    this-> getModelTransform() -> addKid ( lod ) ;

    // Attach Particle System
    //JH  sgCoord pipe_pos = {{0, 0, .3}, {0, 0, 0}} ;
    m_smoke_system = new Smoke(this, 50, 100.0f, true, 0.35f, 1000);
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
    m_model_transform->addKid ( m_shadow );
}   // loadData

//-----------------------------------------------------------------------------
void Kart::placeModel ()
{
    sgMat4 wheel_front;
    sgMat4 wheel_steer;
    sgMat4 wheel_rot;

    sgMakeRotMat4( wheel_rot, 0, RAD_TO_DEGREE(-m_wheel_rotation), 0);
    sgMakeRotMat4( wheel_steer, m_controls.lr * 30.0f , 0, 0);

    sgMultMat4(wheel_front, wheel_steer, wheel_rot);

    if (m_wheel_front_l) m_wheel_front_l->setTransform(wheel_front);
    if (m_wheel_front_r) m_wheel_front_r->setTransform(wheel_front);

    if (m_wheel_rear_l) m_wheel_rear_l->setTransform(wheel_rot);
    if (m_wheel_rear_r) m_wheel_rear_r->setTransform(wheel_rot);
    // We don't have to call Moveable::placeModel, since it does only setTransform

    // Only transfer the bullet data to the plib tree if no history is being
    // replayed.
    if(!user_config->m_replay_history)
    {
        float m[4][4];
        getTrans().getOpenGLMatrix((float*)&m);
        
        //printf(" is %f %f %f\n",t.getOrigin().x(),t.getOrigin().y(),t.getOrigin().z());
        // Transfer the new position and hpr to m_curr_pos
        sgSetCoord(&m_curr_pos, m);
    }
    sgCoord c ;
    sgCopyCoord ( &c, &m_curr_pos ) ;
    //    c.hpr[1] += m_wheelie_angle ;
    //    c.xyz[2] += 0.3f*fabs(sin(m_wheelie_angle*SG_DEGREES_TO_RADIANS));
    const float CENTER_SHIFT = getGravityCenterShift();
    c.xyz[2] -= (0.5f-CENTER_SHIFT)*m_kart_height;   // adjust for center of gravity
    m_model_transform->setTransform(&c);
    Moveable::placeModel();
    
}   // placeModel
//-----------------------------------------------------------------------------
void Kart::setFinishingState(float time)
{
    m_finished_race = true;
    m_finish_time   = time;
}   // setFinishingState

//-----------------------------------------------------------------------------
float Kart::estimateFinishTime  ()
{
    // Estimate the arrival time of any karts that haven't arrived
    // yet by using their average speed up to now and the distance
    // still to race. This approach guarantees that the order of 
    // the karts won't change anymore (karts ahead will have a 
    // higher average speed and therefore finish the race earlier 
    // than karts further behind), so the position doesn't have to
    // be updated to get the correct scoring.
    float distance_covered  = getLap()*world->m_track->getTrackLength()
                            + getDistanceDownTrack();
    // In case that a kart is rescued behind start line, or ...
    if(distance_covered<0) distance_covered =1.0f;

    float average_speed     = distance_covered/world->getTime();

    // Finish time is the time needed for the whole race with 
    // the average speed computed above.
    return race_manager->getNumLaps()*world->getTrack()->getTrackLength() 
          / average_speed;

}   // estimateFinishTime
/* EOF */
