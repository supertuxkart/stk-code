//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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

#include <math.h>
#include <iostream>
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf  _snprintf
#endif

#define _WINSOCKAPI_
#include <plib/ssg.h>

#include "bullet/Demos/OpenGL/GL_ShapeDrawer.h"
#include "loader.hpp"
#include "coord.hpp"
#include "items/herring_manager.hpp"
#include "file_manager.hpp"
#include "skid_mark.hpp"
#include "user_config.hpp"
#include "constants.hpp"
#include "shadow.hpp"
#include "track.hpp"
#include "kart.hpp"
#include "physics.hpp"
#include "translation.hpp"
#include "smoke.hpp"
#include "material_manager.hpp"
#include "kart_properties_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/race_gui.hpp"
#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "network/race_state.hpp"
#include "network/network_manager.hpp"
#include "utils/ssg_help.hpp"


#if defined(WIN32) && !defined(__CYGWIN__)
   // Disable warning for using 'this' in base member initializer list
#  pragma warning(disable:4355)
#endif

Kart::Kart (const std::string& kart_name, int position,
            const btTransform& init_transform) 
     : TerrainInfo(1),
       Moveable(), m_attachment(this), m_collectable(this)

#if defined(WIN32) && !defined(__CYGWIN__)
#  pragma warning(1:4355)
#endif
{
    m_kart_properties      = kart_properties_manager->getKart(kart_name);
   //m_grid_position        = position;
    m_initial_position     = position;
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
    

    // Set position and heading:
    m_reset_transform      = init_transform;

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

    m_engine_sound = sfx_manager->newSFX(SFXManager::SOUND_ENGINE);
    m_beep_sound = sfx_manager->newSFX(SFXManager::SOUND_BEEP);
    m_crash_sound = sfx_manager->newSFX(SFXManager::SOUND_CRASH);

    if(!m_engine_sound)
    {
        fprintf(stdout, "Error: Could not allocate a sfx object for the kart. Further errors may ensue!\n");
    }

    loadData();
    reset();
}   // Kart

// -----------------------------------------------------------------------------

btTransform Kart::getKartHeading(const float customPitch)
{
    btTransform trans = this->getTrans();
    
    // get heading=trans.getBasis*(0,1,0) ... so save the multiplication:
    btVector3 direction(trans.getBasis()[0][1],
                        trans.getBasis()[1][1],
                        trans.getBasis()[2][1]);
    float heading=atan2(-direction.getX(), direction.getY());
    
    TerrainInfo::update(this->getXYZ());
    float pitch = (customPitch == -1 ? getTerrainPitch(heading) : customPitch);
    
    btMatrix3x3 m;
    m.setEulerZYX(pitch, 0.0f, heading);
    trans.setBasis(m);
    
    return trans;
}   // getKartHeading

// ----------------------------------------------------------------------------
void Kart::createPhysics(ssgEntity *obj)
{
    // First: Create the chassis of the kart
    // -------------------------------------

    float kart_width  = m_kart_properties->getKartWidth();
    float kart_length = m_kart_properties->getKartLength();
    float kart_height = m_kart_properties->getKartHeight();

    btBoxShape *shape = new btBoxShape(btVector3(0.5f*kart_width,
                                                 0.5f*kart_length,
                                                 0.5f*kart_height));
    btTransform shiftCenterOfGravity;
    shiftCenterOfGravity.setIdentity();
    // Shift center of gravity downwards, so that the kart 
    // won't topple over too easy. 
    shiftCenterOfGravity.setOrigin(getGravityCenterShift());

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
        new btDefaultVehicleRaycaster(RaceManager::getWorld()->getPhysics()->getPhysicsWorld());
    m_tuning  = new btKart::btVehicleTuning();
	m_tuning->m_maxSuspensionTravelCm = m_kart_properties->getSuspensionTravelCM();
    m_vehicle = new btKart(*m_tuning, m_body, m_vehicle_raycaster,
                           m_kart_properties->getTrackConnectionAccel());

    // never deactivate the vehicle
    m_body->setActivationState(DISABLE_DEACTIVATION);
    m_vehicle->setCoordinateSystem(/*right: */ 0,  /*up: */ 2,  /*forward: */ 1);
    
    // Add wheels
    // ----------
    float wheel_radius    = m_kart_properties->getWheelRadius();
    float suspension_rest = m_kart_properties->getSuspensionRest();
    Vec3  front_wheel     = m_kart_properties->getFrontWheelConnection();
    if(front_wheel.getX()==STKConfig::UNDEFINED)
        front_wheel.setX(0.5f*kart_width);
    if(front_wheel.getY()==STKConfig::UNDEFINED)
        front_wheel.setY(0.5f*kart_length-wheel_radius);
    if(front_wheel.getZ()==STKConfig::UNDEFINED)
        front_wheel.setZ(0);
    btVector3 wheel_direction(0.0f, 0.0f, -1.0f);
    btVector3 wheel_axle(1.0f,0.0f,0.0f);

    // right front wheel
    m_vehicle->addWheel(front_wheel, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ true);

    // left front wheel: mirror X axis
    front_wheel.setX(-front_wheel.getX());
    m_vehicle->addWheel(front_wheel, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ true);

    // right rear wheel
    Vec3  rear_wheel = m_kart_properties->getRearWheelConnection();
    if(rear_wheel.getX()==STKConfig::UNDEFINED)
        rear_wheel.setX(0.5f*kart_width);
    if(rear_wheel.getY()==STKConfig::UNDEFINED)
        rear_wheel.setY(-0.5f*kart_length+wheel_radius);
    if(rear_wheel.getZ()==STKConfig::UNDEFINED)
        rear_wheel.setZ(0);
    m_vehicle->addWheel(rear_wheel, wheel_direction, wheel_axle,
                        suspension_rest, wheel_radius, *m_tuning,
                        /* isFrontWheel: */ false);

    // left rear wheel: mirror X axis
    rear_wheel.setX(-rear_wheel.getX());
    m_vehicle->addWheel(rear_wheel, wheel_direction, wheel_axle,
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
    // Obviously these allocs have to be properly managed/freed
    btTransform t;
    t.setIdentity();
    m_uprightConstraint=new btUprightConstraint(*m_body, t);
    m_uprightConstraint->setLimit(m_kart_properties->getUprightTolerance());
    m_uprightConstraint->setBounce(0.0f);
    m_uprightConstraint->setMaxLimitForce(m_kart_properties->getUprightMaxForce());
    m_uprightConstraint->setErp(1.0f);
    m_uprightConstraint->setLimitSoftness(1.0f);
    m_uprightConstraint->setDamping(0.0f);
    RaceManager::getWorld()->getPhysics()->addKart(this, m_vehicle);

    //create the engine sound
    if(m_engine_sound)
    {
        m_engine_sound->speed(0.6f);
        m_engine_sound->loop();
        m_engine_sound->play();
    }
}   // createPhysics

// -----------------------------------------------------------------------------
Kart::~Kart() 
{
    //stop the engine sound
    if(m_engine_sound)
    {
        m_engine_sound->stop();
    }
    sfx_manager->deleteSFX(m_engine_sound);
    sfx_manager->deleteSFX(m_beep_sound);
    sfx_manager->deleteSFX(m_crash_sound);
    

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

    RaceManager::getWorld()->getPhysics()->removeKart(this);
    delete m_vehicle;
    delete m_tuning;
    delete m_vehicle_raycaster;
    delete m_uprightConstraint;
    for(int i=0; i<m_kart_chassis.getNumChildShapes(); i++)
    {
        delete m_kart_chassis.getChildShape(i);
    }
}   // ~Kart

//-----------------------------------------------------------------------------
void Kart::eliminate()
{
    m_eliminated = true;
    RaceManager::getWorld()->getPhysics()->removeKart(this);

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
        RaceManager::getWorld()->getPhysics()->addKart(this, m_vehicle);
    }

    m_attachment.clear();
    m_collectable.reset();

    m_race_position        = 9;
    m_finished_race        = false;
    m_eliminated           = false;
    m_finish_time          = 0.0f;
    m_zipper_time_left     = 0.0f;
    m_num_herrings_gobbled = 0;
    m_wheel_rotation       = 0;
    m_wheelie_angle        = 0.0f;
    m_bounce_back_time     = 0.0f;

    m_controls.lr          = 0.0f;
    m_controls.accel       = 0.0f;
    m_controls.brake       = false;
    m_controls.wheelie     = false;
    m_controls.jump        = false;
    m_controls.fire        = false;

    // Set the brakes so that karts don't slide downhill
    for(int i=0; i<4; i++) m_vehicle->setBrake(5.0f, i);

    setTrans(m_reset_transform);

    m_vehicle->applyEngineForce (0.0f, 2);
    m_vehicle->applyEngineForce (0.0f, 3);

    Moveable::reset();
    for(int j=0; j<m_vehicle->getNumWheels(); j++)
    {
        m_vehicle->updateWheelTransform(j, true);
    }

    // if the kart was being rescued when a restart is called,
    // add the vehicle back into the physical world!
    if(m_rescue)
    {
        RaceManager::getWorld()->getPhysics()->addKart(this, m_vehicle);
    }
    m_rescue               = false;
    TerrainInfo::update(getXYZ());
}   // reset

//-----------------------------------------------------------------------------
void Kart::raceFinished(float time)
{
    m_finished_race = true;
    m_finish_time   = time;
    race_manager->RaceFinished(this, time);
}   // raceFinished

//-----------------------------------------------------------------------------
void Kart::collectedHerring(const Herring &herring, int add_info)
{
    const herringType type = herring.getType();

    switch (type)
    {
    case HE_GREEN  : m_attachment.hitGreenHerring(herring, add_info);    break;
    case HE_SILVER : m_num_herrings_gobbled++ ;                          break;
    case HE_GOLD   : m_num_herrings_gobbled += 3 ;                       break;
    case HE_RED    : { 
		         int n=1 + 4*getNumHerring() / MAX_HERRING_EATEN;
                         m_collectable.hitRedHerring(n, herring,add_info);break;
                     }
    default        : break;
    }   // switch TYPE

    // Attachments and collectables are stored in the corresponding
    // functions (hit{Red,Green}Herring), so only coins need to be
    // stored here.
    if(network_manager->getMode()==NetworkManager::NW_SERVER &&
        (type==HE_SILVER || type==HE_GOLD)                       )
    {
        race_state->herringCollected(getWorldKartId(), herring.getHerringId());
    }

    if ( m_num_herrings_gobbled > MAX_HERRING_EATEN )
        m_num_herrings_gobbled = MAX_HERRING_EATEN;

}   // collectedHerring

//-----------------------------------------------------------------------------
// Simulates gears
float Kart::getActualWheelForce()
{
    float zipperF=(m_zipper_time_left>0.0f) ? stk_config->m_zipper_force : 0.0f;
    const std::vector<float>& gear_ratio=m_kart_properties->getGearSwitchRatio();
    for(unsigned int i=0; i<gear_ratio.size(); i++)
    {
        if(m_speed <= m_max_speed*gear_ratio[i])
        {
            m_current_gear_ratio = gear_ratio[i];
            return getMaxPower()*m_kart_properties->getGearPowerIncrease()[i]+zipperF;
        }
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
void Kart::handleExplosion(const Vec3& pos, bool direct_hit)
{
    if(direct_hit) 
    {
        btVector3 diff((float)(rand()%16/16), (float)(rand()%16/16), 2.0f);
        diff.normalize();
        diff*=stk_config->m_explosion_impulse/5.0f;
        m_uprightConstraint->setDisableTime(10.0f);
        getVehicle()->getRigidBody()->applyCentralImpulse(diff);
        getVehicle()->getRigidBody()->applyTorqueImpulse(btVector3(float(rand()%32*5),
                                                                   float(rand()%32*5),
                                                                   float(rand()%32*5)));
    }
    else  // only affected by a distant explosion
    {
        btVector3 diff=getXYZ()-pos;
        //if the z component is negative, the resulting impulse could push the 
        // kart through the floor. So in this case ignore z.
        if(diff.getZ()<0) diff.setZ(0.0f);
        float len2=diff.length2();

        // The correct formhale would be to first normalise diff,
        // then apply the impulse (which decreases 1/r^2 depending
        // on the distance r), so:
        // diff/len(diff) * impulseSize/len(diff)^2
        // = diff*impulseSize/len(diff)^3
        // We use diff*impulseSize/len(diff)^2 here, this makes the impulse
        // somewhat larger, which is actually more fun :)
        diff *= stk_config->m_explosion_impulse/len2;
        getVehicle()->getRigidBody()->applyCentralImpulse(diff);
    }
}   // handleExplosion

//-----------------------------------------------------------------------------
void Kart::update(float dt)
{
    // Store the actual kart controls at the start of update in the server
    // state. This makes it easier to reset some fields when they are not used 
    // anymore (e.g. controls.fire).
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        race_state->storeKartControls(*this);
    }

    // On a client fiering is done upon receiving the command from the server.
    if ( m_controls.fire && network_manager->getMode()!=NetworkManager::NW_CLIENT 
         && !isRescue())
    {
        // use() needs to be called even if there currently is no collecteable
        // since use() can test if something needs to be switched on/off.
        m_collectable.use() ;
        m_controls.fire = false;
    }

    // Only use the upright constraint if the kart is in the air!
    if(isOnGround())
        m_uprightConstraint->setLimit(M_PI);
    else
        m_uprightConstraint->setLimit(m_kart_properties->getUprightTolerance());


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
            m_attachment.set( ATTACH_TINYTUX, rescue_time ) ;
            m_rescue_pitch = getHPR().getPitch();
            m_rescue_roll  = getHPR().getRoll();
            RaceManager::getWorld()->getPhysics()->removeKart(this);
            race_state->herringCollected(getWorldKartId(), -1, -1);
        }
        btQuaternion q_roll (btVector3(0.f, 1.f, 0.f),
                             -m_rescue_roll*dt/rescue_time*M_PI/180.0f);
        btQuaternion q_pitch(btVector3(1.f, 0.f, 0.f),
                             -m_rescue_pitch*dt/rescue_time*M_PI/180.0f);
        setXYZRotation(getXYZ()+Vec3(0, 0, rescue_height*dt/rescue_time),
                       getRotation()*q_roll*q_pitch);
    }   // if m_rescue
    m_attachment.update(dt);

    //smoke drawing control point
    if ( user_config->m_smoke )
    {
        if (m_smoke_system != NULL)
            m_smoke_system->update (dt);
    }  // user_config->smoke
    updatePhysics(dt);

    //kart_info.m_last_track_coords = kart_info.m_curr_track_coords;

    Moveable::update(dt);

    m_engine_sound->position(getXYZ());
    m_beep_sound->position(getXYZ());
    m_crash_sound->position(getXYZ());

    // Check if a kart is (nearly) upside down and not moving much --> automatic rescue
    if((fabs(getHPR().getRoll())>60 && fabs(getSpeed())<3.0f) )
    {
        forceRescue();
    }

    btTransform trans=getTrans();
    // Add a certain epsilon (0.2) to the height of the kart. This avoids
    // problems of the ray being cast from under the track (which happened
    // e.g. on tux tollway when jumping down from the ramp).
    // FIXME: this should be more thoroughly fixed, the constant is probably
    // dependent on the wheel size, suspension data etc.: when jumping,
    // the wheel suspension will be fully compressed, resulting in the
    // ray to start too low (under the track).
    Vec3 pos_plus_epsilon = trans.getOrigin()+btVector3(0,0,0.2f);
    //btVector3 pos_plus_epsilon (-56.6874237, -137.48851, -3.06826854);
    TerrainInfo::update(pos_plus_epsilon);

    const Material* material=getMaterial();
    if (getHoT()==Track::NOHIT)   // kart falling off the track
    {
        forceRescue();    
    } 
    else if(material)
    {
        // Sometimes the material can be 0. This can happen if a kart is above
        // another kart (e.g. mass collision, or one kart falling on another 
        // kart). Bullet does not have any triangle information in this case,
        // and so material can not be set. In this case it is simply ignored 
        // since it can't hurt (material is only used for friction, zipper and
        // rescue, so those things are not triggered till the kart is on the 
        // track again)
        if     (material->isReset()  && isOnGround()) forceRescue();
        else if(material->isZipper() && isOnGround()) handleZipper();
        else if(user_config->m_skidding)  // set friction otherwise if it's enabled
        {
            for(int i=0; i<m_vehicle->getNumWheels(); i++)
            {
                // terrain dependent friction
                m_vehicle->getWheelInfo(i).m_frictionSlip = 
                                getFrictionSlip() * material->getFriction();
            }   // for i<getNumWheels
        } // neither reset nor zipper material
    }   // if there is material

    // Check if any herring was hit.
    herring_manager->hitHerring(this);
    
    processSkidMarks();

}
//-----------------------------------------------------------------------------
// Set zipper time, and apply one time additional speed boost
void Kart::handleZipper()
{
    // Ignore a zipper that's activated while braking
    if(m_controls.brake) return;
    m_zipper_time_left  = stk_config->m_zipper_time;
    const btVector3& v  = m_body->getLinearVelocity();
    float current_speed = v.length();
    float speed         = std::min(current_speed+stk_config->m_zipper_speed_gain, 
                                   getMaxSpeed());
    // Avoid NAN problems, which can happen if e.g. a kart is rescued on
    // top of zipper, and then dropped.
    if(current_speed>0.00001) m_body->setLinearVelocity(v*(speed/current_speed));
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
    RaceManager::getWorld()->getPhysics()->debugDraw(m, m_body->getCollisionShape(), 
                                   wire_color);
    btCylinderShapeX wheelShape( btVector3(0.1f,
                                        m_kart_properties->getWheelRadius(),
                                        m_kart_properties->getWheelRadius()));
    btVector3 wheelColor(1,0,0);
    for(int i=0; i<m_vehicle->getNumWheels(); i++)
    {
        m_vehicle->updateWheelTransform(i, true);
        float m[16];
        m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
        RaceManager::getWorld()->getPhysics()->debugDraw(m, &wheelShape, wheelColor);
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
        // Disable the upright constraint, since it will otherwise
        // work against the wheelie
        m_uprightConstraint->setLimit(M_PI);

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
    if(m_wheelie_angle <=0.0f) 
    {
        m_uprightConstraint->setLimit(m_kart_properties->getUprightTolerance());
        return 0.0f;
    }

    const btTransform& chassisTrans = getTrans();
    btVector3 targetUp(0.0f, 0.0f, 1.0f);
    btVector3 forwardW (chassisTrans.getBasis()[0][1],
                        chassisTrans.getBasis()[1][1],
                        chassisTrans.getBasis()[2][1]);
    btVector3 crossProd = targetUp.cross(forwardW);
    crossProd.normalize();

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
void Kart::crashed(Kart *k)
{
    // After a collision disable the engine for a short time so that karts 
    // can 'bounce back' a bit (without this the engine force will prevent
    // karts from bouncing back, they will instead stuck towards the obstable).
    if(m_bounce_back_time<=0.0f)
    {
        m_crash_sound->play();
        m_bounce_back_time = 0.5f;
    }
}   // crashed

// -----------------------------------------------------------------------------
void Kart::beep()
{
    m_beep_sound->play();
} // beep

// -----------------------------------------------------------------------------
void Kart::updatePhysics (float dt) 
{
    m_bounce_back_time-=dt;
    float engine_power = getActualWheelForce() + handleWheelie(dt);
    if(m_attachment.getType()==ATTACH_PARACHUTE) engine_power*=0.2f;

    if(m_controls.accel)   // accelerating
    {   // For a short time after a collision disable the engine,
        // so that the karts can bounce back a bit from the obstacle.
        if(m_bounce_back_time>0.0f) engine_power = 0.0f;
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

    if(m_controls.jump && isOnGround())
    { 
      //Vector3 impulse(0.0f, 0.0f, 10.0f);
      //        getVehicle()->getRigidBody()->applyCentralImpulse(impulse);
        btVector3 velocity         = m_body->getLinearVelocity();
        velocity.setZ( m_kart_properties->getJumpVelocity() );

        getBody()->setLinearVelocity( velocity );

    }
    if(m_wheelie_angle<=0.0f)
    {
        const float steering = DEGREE_TO_RAD(getMaxSteerAngle()) * m_controls.lr;
        m_vehicle->setSteeringValue(steering, 0);
        m_vehicle->setSteeringValue(steering, 1);
    } 
    else 
    {
        m_vehicle->setSteeringValue(0.0f, 0);
        m_vehicle->setSteeringValue(0.0f, 1);
    }

    // Only compute the current speed if this is not the client. On a client the
    // speed is actually received from the server.
    if(network_manager->getMode()!=NetworkManager::NW_CLIENT)
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

    // when going faster, use higher pitch for engine
    if(m_engine_sound && sfx_manager->sfxAllowed())
    {
        m_engine_sound->speed(0.6 + (float)(m_speed / max_speed)*0.7f);
        m_engine_sound->position(getXYZ());
    }
   
}   // updatePhysics

//-----------------------------------------------------------------------------
void Kart::forceRescue()
{
    m_rescue=true;
}   // forceRescue
//-----------------------------------------------------------------------------
/** Drops a kart which was rescued back on the track.
 */
void Kart::endRescue()
{
    m_rescue = false ;

    m_body->setLinearVelocity (btVector3(0.0f,0.0f,0.0f));
    m_body->setAngularVelocity(btVector3(0.0f,0.0f,0.0f));

    // let the mode decide where to put the kart
    RaceManager::getWorld()->moveKartAfterRescue(this, m_body);
    
    RaceManager::getWorld()->getPhysics()->addKart(this, m_vehicle);
}   // endRescue

//-----------------------------------------------------------------------------
void Kart::processSkidMarks()
{
    // FIXME: disable skidmarks for now, they currently look ugly, and are
    //        sometimes hovering in the air
    return;
    assert(m_skidmark_left);
    assert(m_skidmark_right);
    const float threshold = 0.3f;
    //FIXME    const float ANGLE     = 43.0f;
    //FIXME    const float LENGTH    = 0.57f;
    bool skid_front = m_vehicle->getWheelInfo(0).m_skidInfo < threshold ||
                      m_vehicle->getWheelInfo(1).m_skidInfo < threshold;
    bool skid_rear  = m_vehicle->getWheelInfo(2).m_skidInfo < threshold ||
                      m_vehicle->getWheelInfo(3).m_skidInfo < threshold;
    if(skid_rear || skid_front)
    {
        if(isOnGround())
        {
	  //FIXME: no getCoord anymore  m_skidmark_left ->add(*getCoord(),  ANGLE, LENGTH);
          //FIXME  m_skidmark_right->add(*getCoord(),  ANGLE, LENGTH);            
        }
        else
        {   // not on ground
            //FIXME m_skidmark_left->addBreak(*getCoord(),  ANGLE, LENGTH);
            //FIRME m_skidmark_right->addBreak(*getCoord(), ANGLE, LENGTH);
        }   // on ground
    }
    else
    {   // !skid_rear && !skid_front    
        //FIXME if(m_skidmark_left->wasSkidMarking())
        //FIXME     m_skidmark_left->addBreak(*getCoord(),  ANGLE, LENGTH);

        //FIXME if(m_skidmark_right->wasSkidMarking())
        //FIXME    m_skidmark_right->addBreak(*getCoord(), ANGLE, LENGTH);
    }
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
                m_wheel_front_l = SSGHelp::add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else if (strcmp(i->getName(), "WheelFront.R") == 0)
            {
                m_wheel_front_r = SSGHelp::add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else if (strcmp(i->getName(), "WheelRear.L") == 0)
            {
                m_wheel_rear_l = SSGHelp::add_transform(dynamic_cast<ssgTransform*>(i));
            }
            else if (strcmp(i->getName(), "WheelRear.R") == 0)
            {
                m_wheel_rear_r = SSGHelp::add_transform(dynamic_cast<ssgTransform*>(i));
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
    m_smokepuff->setTexture(material_manager->getMaterial("smoke.rgb")->getState()->getTexture());
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

    SSGHelp::createDisplayLists(obj);  // create all display lists
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

    // 
    m_skidmark_left  = new SkidMark(/* angle sign */ -1);
    m_skidmark_right = new SkidMark(/* angle sign */  1);

    m_shadow = createShadow(m_kart_properties->getShadowFile(), -1, 1, -1, 1);
    m_shadow->ref();
    m_model_transform->addKid ( m_shadow );
}   // loadData

//-----------------------------------------------------------------------------
void Kart::updateGraphics(const Vec3& off_xyz,  const Vec3& off_hpr)
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

    Vec3        center_shift  = getGravityCenterShift();
    center_shift.setZ(-center_shift.getZ()
                      + 0.3f*fabs(sin(DEGREE_TO_RAD(m_wheelie_angle))) );
    const float offset_pitch  = DEGREE_TO_RAD(m_wheelie_angle);
    
    Moveable::updateGraphics(center_shift, Vec3(0, offset_pitch, 0));
}   // updateGraphics

/* EOF */
