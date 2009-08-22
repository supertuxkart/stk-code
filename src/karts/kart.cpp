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

#include "karts/kart.hpp"

#include <math.h>
#include <iostream>

#include "bullet/Demos/OpenGL/GL_ShapeDrawer.h"

#include "audio/sound_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/nitro.hpp"
#include "graphics/shadow.hpp"
#include "graphics/skid_marks.hpp"
#include "graphics/smoke.hpp"
#include "graphics/water_splash.hpp"
#include "modes/world.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/race_state.hpp"
#include "network/network_manager.hpp"
#include "physics/btKart.hpp"
#include "physics/btUprightConstraint.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/coord.hpp"
#include "audio/sfx_manager.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
   // Disable warning for using 'this' in base member initializer list
#  pragma warning(disable:4355)
#endif

Kart::Kart (const std::string& kart_name, int position,
            const btTransform& init_transform)
     : TerrainInfo(1),
       Moveable(), m_attachment(this), m_powerup(this)

#if defined(WIN32) && !defined(__CYGWIN__)
#  pragma warning(1:4355)
#endif
{
    m_kart_properties      = kart_properties_manager->getKart(kart_name);
    assert(m_kart_properties != NULL);
    m_initial_position     = position;
    m_collected_energy     = 0;
    m_eliminated           = false;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_slipstream_time      = 0.0f;
    m_shadow_enabled       = false;
    m_shadow               = NULL;
    m_smoke_system         = NULL;
    m_water_splash_system  = NULL;
    m_nitro                = NULL;
    m_skidmarks            = NULL;
    m_animated_node        = NULL;

    m_view_blocked_by_plunger = 0;

    // Initialize custom sound vector
    m_custom_sounds.resize(SFXManager::NUM_CUSTOMS);

    // Set position and heading:
    m_reset_transform      = init_transform;

    // Neglecting the roll resistance (which is small for high speeds compared
    // to the air resistance), maximum speed is reached when the engine
    // power equals the air resistance force, resulting in this formula:
    m_max_speed               = m_kart_properties->getMaxSpeed();
    m_max_speed_reverse_ratio = m_kart_properties->getMaxSpeedReverseRatio();
    m_speed                   = 0.0f;

    // Setting rescue to false is important! If rescue is set when reset() is
    // called, it is assumed that this was triggered by a restart, and that
    // the vehicle must be added back to the physics world. Since reset() is
    // also called at the very start, it must be guaranteed that rescue is
    // not set.
    m_rescue                  = false;
    m_wheel_rotation          = 0;

    // Create SFXBase for each custom sound
    for (int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        int id = m_kart_properties->getCustomSfxId((SFXManager::CustomSFX)n);

        // If id == -1 the custom sound was not defined in the .irrkart config file
        if (id != -1)
            m_custom_sounds[n] = sfx_manager->newSFX(id);
    }

    m_engine_sound = sfx_manager->newSFX(m_kart_properties->getEngineSfxType());
    m_beep_sound   = sfx_manager->newSFX(  SFXManager::SOUND_BEEP             );
    m_crash_sound  = sfx_manager->newSFX(  SFXManager::SOUND_CRASH            );
    m_goo_sound    = sfx_manager->newSFX(  SFXManager::SOUND_GOO              );
    m_skid_sound   = sfx_manager->newSFX(  SFXManager::SOUND_SKID             );

    if(!m_engine_sound)
    {
        fprintf(stdout, "Error: Could not allocate a sfx object for the kart. Further errors may ensue!\n");
    }

    loadData();
    float l = m_kart_properties->getSlipstreamLength();

    Vec3 p0(-getKartWidth()*0.5f, -getKartLength()*0.5f, 0);
    Vec3 p1(-getKartWidth()*0.5f, -getKartLength()*0.5f-l, 0);
    Vec3 p2( getKartWidth()*0.5f, -getKartLength()*0.5f-l, 0);
    Vec3 p3( getKartWidth()*0.5f, -getKartLength()*0.5f, 0);
    m_slipstream_original_area = new Quad(p0, p1, p2, p3);
    m_slipstream_area          = new Quad(p0, p1, p2, p3);

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
void Kart::createPhysics()
{
    // First: Create the chassis of the kart
    // -------------------------------------
    const KartModel *km = m_kart_properties->getKartModel();
    float kart_width  = km->getWidth();
    float kart_length = km->getLength();
    float kart_height = km->getHeight();

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

    btVector3 wheel_direction(0.0f, 0.0f, -1.0f);
    btVector3 wheel_axle(1.0f,0.0f,0.0f);

    for(unsigned int i=0; i<4; i++)
    {
        bool is_front_wheel = i<2;
        btWheelInfo& wheel = m_vehicle->addWheel(
                            m_kart_properties->getKartModel()->getWheelPhysicsPosition(i),
                            wheel_direction, wheel_axle, suspension_rest,
                            wheel_radius, *m_tuning, is_front_wheel);
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
    RaceManager::getWorld()->getPhysics()->addKart(this);

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

    // Delete all custom sounds
    for (int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        if (m_custom_sounds[n] != NULL)
            sfx_manager->deleteSFX(m_custom_sounds[n]);
    }

    sfx_manager->deleteSFX(m_engine_sound );
    sfx_manager->deleteSFX(m_crash_sound  );
    sfx_manager->deleteSFX(m_skid_sound   );
    sfx_manager->deleteSFX(m_goo_sound    );

    if(m_smoke_system)        delete m_smoke_system;
    if(m_water_splash_system) delete m_water_splash_system;
    if(m_nitro)               delete m_nitro;

    delete m_shadow;

    if(m_skidmarks) delete m_skidmarks ;

    RaceManager::getWorld()->getPhysics()->removeKart(this);
    delete m_vehicle;
    delete m_tuning;
    delete m_vehicle_raycaster;
    delete m_uprightConstraint;
    for(int i=0; i<m_kart_chassis.getNumChildShapes(); i++)
    {
        delete m_kart_chassis.getChildShape(i);
    }
    delete m_slipstream_original_area;
    delete m_slipstream_area;
}   // ~Kart

//-----------------------------------------------------------------------------
void Kart::eliminate()
{
    m_eliminated = true;
    if (!m_rescue)
    {
        RaceManager::getWorld()->getPhysics()->removeKart(this);
    }

    // make the kart invisible by placing it way under the track
    Vec3 hell(0, 0, -10000.0f);
    getNode()->setPosition(hell.toIrrVector());
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
/** Multiplies the velocity of the kart by a factor f (both linear
 *  and angular). This is used by anvils, which suddenly slow down the kart
 *  when they are attached.
 */
void Kart::adjustSpeed(float f)
{
    m_body->setLinearVelocity(m_body->getLinearVelocity()*f);
    m_body->setAngularVelocity(m_body->getAngularVelocity()*f);
}   // adjustSpeed

//-----------------------------------------------------------------------------
/** This method is to be called every time the mass of the kart is updated,
 *  which includes attaching an anvil to the kart (and detaching).
 */
void Kart::updatedWeight()
{
    // getMass returns the mass increased by the attachment
    btVector3 inertia;
    float m=getMass();
    m_kart_chassis.calculateLocalInertia(m, inertia);
    m_body->setMassProps(m, inertia);
}   // updatedWeight

//-----------------------------------------------------------------------------
void Kart::reset()
{
    // If the kart was eliminated or rescued, the body was removed from the
    // physics world. Add it again.
    if(m_eliminated || m_rescue)
    {
        RaceManager::getWorld()->getPhysics()->addKart(this);
    }

    m_view_blocked_by_plunger = 0.0;
    m_attachment.clear();
    m_powerup.reset();

    m_race_position        = 9;
    m_finished_race        = false;
    m_eliminated           = false;
    m_rescue               = false;
    m_finish_time          = 0.0f;
    m_zipper_time_left     = 0.0f;
    m_collected_energy     = 0;
    m_wheel_rotation       = 0;
    m_bounce_back_time     = 0.0f;
    m_skidding             = 1.0f;
    m_time_last_crash      = 0.0f;
    m_max_speed_reduction  = 0.0f;
    m_power_reduction      = 50.0f;

    m_controls.m_steer     = 0.0f;
    m_controls.m_accel     = 0.0f;
    m_controls.m_brake     = false;
    m_controls.m_nitro     = false;
    m_controls.m_drift     = false;
    m_controls.m_fire      = false;
    m_controls.m_look_back = false;

    m_vehicle->deactivateZipper();

    // Set the brakes so that karts don't slide downhill
    for(int i=0; i<4; i++) m_vehicle->setBrake(5.0f, i);

    setTrans(m_reset_transform);

    m_vehicle->applyEngineForce (0.0f, 2);
    m_vehicle->applyEngineForce (0.0f, 3);

    Moveable::reset();
    if(m_skidmarks) m_skidmarks->reset();
    for(int j=0; j<m_vehicle->getNumWheels(); j++)
    {
        m_vehicle->updateWheelTransform(j, true);
    }

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
void Kart::collectedItem(const Item &item, int add_info)
{
    const Item::ItemType type = item.getType();

    switch (type)
    {
    case Item::ITEM_BANANA: 
        m_attachment.hitBanana(item, add_info); 
        break;

    case Item::ITEM_SILVER_COIN: m_collected_energy++ ;                  break;
    case Item::ITEM_GOLD_COIN  : m_collected_energy += 3 ;               break;
    case Item::ITEM_BONUS_BOX  :
        {
            // In wheelie style, karts get more items depending on energy,
            // in nitro mode it's only one item.
            int n = 1;
            m_powerup.hitBonusBox(n, item,add_info);
            break;
        }
    case Item::ITEM_BUBBLEGUM:
        // slow down
        m_body->setLinearVelocity(m_body->getLinearVelocity()*0.3f);
        m_goo_sound->position(getXYZ());
        m_goo_sound->play();
        // Play appropriate custom character sound
        playCustomSFX(SFXManager::CUSTOM_GOO);
        break;
    default        : break;
    }   // switch TYPE

    // Attachments and powerups are stored in the corresponding
    // functions (hit{Red,Green}Item), so only coins need to be
    // stored here.
    if(network_manager->getMode()==NetworkManager::NW_SERVER &&
        (type==Item::ITEM_SILVER_COIN || type==Item::ITEM_GOLD_COIN)                       )
    {
        race_state->itemCollected(getWorldKartId(), item.getItemId());
    }

    if ( m_collected_energy > MAX_ITEMS_COLLECTED )
        m_collected_energy = MAX_ITEMS_COLLECTED;

}   // collectedItem

//-----------------------------------------------------------------------------
// Simulates gears
float Kart::getActualWheelForce()
{
    float zipperF=(m_zipper_time_left>0.0f) ? stk_config->m_zipper_force : 0.0f;
    const std::vector<float>& gear_ratio=m_kart_properties->getGearSwitchRatio();
    for(unsigned int i=0; i<gear_ratio.size(); i++)
    {
        if(m_speed <= getMaxSpeed()*gear_ratio[i])
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
/** The kart is near the ground, but not necesarily on it (small jumps). This
 *  is used to determine when to switch off the upright constraint, so that
 *  explosions can be more violent, while still
*/

bool Kart::isNearGround() const
{
    if(getHoT()==Track::NOHIT)
        return false;
    else
        return ((getXYZ().getZ() - getHoT()) < stk_config->m_near_ground);
}   // isNearGround
//-----------------------------------------------------------------------------
void Kart::handleExplosion(const Vec3& pos, bool direct_hit)
{
    int sign_bits = rand(); // To select plus or minus randomnly, assuming 15 bit at least
    if(direct_hit)
    {
        // Play associated kart sound
        playCustomSFX(SFXManager::CUSTOM_EXPLODE);
        float sign_a = (sign_bits & (0x1 <<  8)) ? 1.0f : -1.0f;
        float sign_b = (sign_bits & (0x1 <<  9)) ? 1.0f : -1.0f;
        float sign_c = (sign_bits & (0x1 << 10)) ? 1.0f : -1.0f;
        float sign_d = (sign_bits & (0x1 << 11)) ? 1.0f : -1.0f;
        float sign_e = (sign_bits & (0x1 << 12)) ? 1.0f : -1.0f;
        btVector3 diff(sign_a * (float)(rand()%16/16), sign_b * (float)(rand()%16/16), 2.0f);
        diff.normalize();
        diff*=stk_config->m_explosion_impulse/5.0f;
        m_uprightConstraint->setDisableTime(10.0f);
        getVehicle()->getRigidBody()->applyCentralImpulse(diff);
        getVehicle()->getRigidBody()->applyTorqueImpulse(btVector3(sign_c * float(rand()%32*5),
                                                                   sign_d * float(rand()%32*5),
                                                                   sign_e * float(rand()%32*5)));
    }
    else  // only affected by a distant explosion
    {
        btVector3 diff=getXYZ()-pos;
        //if the z component is negative, the resulting impulse could push the
        // kart through the floor. So in this case ignore z.
        if(diff.getZ()<0) diff.setZ(0.0f);
        float len2=diff.length2();

        // The correct formulae would be to first normalise diff,
        // then apply the impulse (which decreases 1/r^2 depending
        // on the distance r), so:
        // diff/len(diff) * impulseSize/len(diff)^2
        // = diff*impulseSize/len(diff)^3
        // We use diff*impulseSize/len(diff)^2 here, this makes the impulse
        // somewhat larger, which is actually more fun :)
        diff *= stk_config->m_explosion_impulse/len2;
        getVehicle()->getRigidBody()->applyCentralImpulse(diff);
        // Even if just pushed, give some random rotation to simulate the lost of control by the shake
        float sign_a = (sign_bits & (0x1 << 8)) ? 1.0f : -1.0f;
        getVehicle()->getRigidBody()->applyTorqueImpulse(btVector3(0, 0, sign_a * float(rand()%32*5)));
    }
}   // handleExplosion

//-----------------------------------------------------------------------------
void Kart::update(float dt)
{
    // if its view is blocked by plunger, decrease remaining time
    if(m_view_blocked_by_plunger > 0) m_view_blocked_by_plunger -= dt;

    // Store the actual kart controls at the start of update in the server
    // state. This makes it easier to reset some fields when they are not used
    // anymore (e.g. controls.fire).
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
    {
        race_state->storeKartControls(*this);
    }

    // On a client fiering is done upon receiving the command from the server.
    if ( m_controls.m_fire && network_manager->getMode()!=NetworkManager::NW_CLIENT
         && !isRescue())
    {
        // use() needs to be called even if there currently is no collecteable
        // since use() can test if something needs to be switched on/off.
        m_powerup.use() ;
        m_controls.m_fire = false;
    }

    // When really on air, free fly, when near ground, try to glide / adjust for landing
    if(!isNearGround())
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
            race_state->itemCollected(getWorldKartId(), -1, -1);
        }
        RaceManager::getWorld()->getPhysics()->removeKart(this);

        btQuaternion q_roll (btVector3(0.f, 1.f, 0.f),
                             -m_rescue_roll*dt/rescue_time*M_PI/180.0f);
        btQuaternion q_pitch(btVector3(1.f, 0.f, 0.f),
                             -m_rescue_pitch*dt/rescue_time*M_PI/180.0f);
        setXYZRotation(getXYZ()+Vec3(0, 0, rescue_height*dt/rescue_time),
                       getRotation()*q_roll*q_pitch);
    }   // if m_rescue
    m_attachment.update(dt);

    //smoke drawing control point
    if ( UserConfigParams::m_graphical_effects )
    {
        m_smoke_system->update(dt);
        m_water_splash_system->update(dt);
        m_nitro->update(dt);
    }  // UserConfigParams::m_graphical_effects

    updatePhysics(dt);

    Moveable::update(dt);

    for (int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        if (m_custom_sounds[n] != NULL) m_custom_sounds[n]->position   ( getXYZ() );
    }

    m_beep_sound->position   ( getXYZ() );
    m_engine_sound->position ( getXYZ() );
    m_crash_sound->position  ( getXYZ() );
    m_skid_sound->position   ( getXYZ() );

    // Check if a kart is (nearly) upside down and not moving much --> automatic rescue
    if((fabs(getHPR().getRoll())>60 && fabs(getSpeed())<3.0f) )
    {
        forceRescue();
    }

    btTransform trans=getTrans();
    // Add a certain epsilon (0.3) to the height of the kart. This avoids
    // problems of the ray being cast from under the track (which happened
    // e.g. on tux tollway when jumping down from the ramp, when the chassis
    // partly tunnels through the track). While tunneling should not be
    // happening (since Z velocity is clamped), the epsilon is left in place
    // just to be on the safe side (it will not hit the chassis itself).
    Vec3 pos_plus_epsilon = trans.getOrigin()+btVector3(0,0,0.3f);
    // These values cause the track not to be hit in tuxtrack. I leave
    // them in as a test case if additional debugging should be needed.
    // Note: it might be that the kart chassis is actually 'in' the track,
    // i.e. it's a tunneling problem!
    //btVector3 pos_plus_epsilon (-54.449902, -139.99402, -3.4524240);
    // motionstate:               -52.449902, -139.99402, -3.6524241
    // collision object           -52.221024, -139.99614, -3.5276926

    // Make sure that the ray doesn't hit the kart. This is done by
    // resetting the collision filter group, so that this collision
    // object is ignored during raycasting.
    short int old_group = 0;
    if(m_body->getBroadphaseHandle())
    {
        old_group = m_body->getBroadphaseHandle()->m_collisionFilterGroup;
        m_body->getBroadphaseHandle()->m_collisionFilterGroup = 0;
    }
    TerrainInfo::update(pos_plus_epsilon);
    if(m_body->getBroadphaseHandle())
    {
        m_body->getBroadphaseHandle()->m_collisionFilterGroup = old_group;
    }
    const Material* material=getMaterial();
    m_power_reduction = 50.0f;
    if (getHoT()==Track::NOHIT)   // kart falling off the track
    {
        // let kart fall a bit before rescuing
        if(fabs( getXYZ().getZ()
                -RaceManager::getTrack()->getQuadGraph().getQuad(0).getCenter().getZ() ) > 17)
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
        else
        {
            m_power_reduction = material->getSlowDown();
            // Normal driving on terrain. Adjust for maximum terrain speed
            float max_speed_here = material->getMaxSpeedFraction()*getMaxSpeed();
            // If the speed is too fast, reduce the maximum speed gradually.
            // The actual capping happens in updatePhysics
            if(max_speed_here<m_speed)
                m_max_speed_reduction += dt*material->getSlowDown();
            else
                m_max_speed_reduction = 0.0f;
        }
    }   // if there is material

    // Check if any item was hit.
    item_manager->hitItem(this);
    if(m_kart_properties->hasSkidmarks())
        m_skidmarks->update(dt);

    // Remove the shadow if the kart is not on the ground (if a kart
    // is rescued isOnGround might still be true, since the kart rigid
    // body was removed from the physics, but still retain the old
    // values for the raycasts).
    if( (!isOnGround() || m_rescue) && m_shadow_enabled)
    {
        m_shadow_enabled = false;
        m_shadow->disableShadow();
    }
    if(!m_shadow_enabled && isOnGround() && !m_rescue)
    {
        m_shadow->enableShadow();
        m_shadow_enabled = true;
    }
}   // update

//-----------------------------------------------------------------------------
/** Sets zipper time, and apply one time additional speed boost.
 */
void Kart::handleZipper()
{
    // Ignore a zipper that's activated while braking
    if(m_controls.m_brake) return;
    m_zipper_time_left  = stk_config->m_zipper_time;

    btVector3 v         = m_body->getLinearVelocity();
    float current_speed = v.length();
    float speed         = std::min(current_speed+stk_config->m_zipper_speed_gain,
                                   getMaxSpeedOnTerrain() *
                                   (1 + stk_config->m_zipper_max_speed_fraction));

    m_vehicle->activateZipper(speed);
    // Play custom character sound (weee!)
    playCustomSFX(SFXManager::CUSTOM_ZIPPER);
}   // handleZipper

//-----------------------------------------------------------------------------
void Kart::draw()
{
    float m[16];
    btTransform t=getTrans();
    t.getOpenGLMatrix(m);

    btVector3 wire_color(0.5f, 0.5f, 0.5f);
    //RaceManager::getWorld()->getPhysics()->debugDraw(m, m_body->getCollisionShape(),
    //                               wire_color);
    btCylinderShapeX wheelShape( btVector3(0.1f,
                                        m_kart_properties->getWheelRadius(),
                                        m_kart_properties->getWheelRadius()));
    btVector3 wheelColor(0,0,1);
    for(int i=0; i<m_vehicle->getNumWheels(); i++)
    {
        m_vehicle->updateWheelTransform(i, true);
        float m[16];
        m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(m);
        RaceManager::getWorld()->getPhysics()->debugDraw(m, &wheelShape, wheelColor);
    }
}   // draw

// -----------------------------------------------------------------------------
/** Returned an additional engine power boost when using nitro.
 *  \param dt Time step size.
 */
float Kart::handleNitro(float dt)
{
    if(!m_controls.m_nitro) return 0.0;
    m_collected_energy -= dt;
    if(m_collected_energy<0)
    {
        m_collected_energy = 0;
        return 0.0;
    }
    return m_kart_properties->getNitroPowerBoost() * getMaxPower();

}   // handleNitro

//-----------------------------------------------------------------------------
/** This function manages slipstreaming. It adds up the time a kart was
 *  slipstreaming, and returns the potential power boost due to coming
 *  out of slipstream.
 */
float Kart::handleSlipstream(float dt)
{
    m_slipstream_original_area->transform(getTrans(), m_slipstream_area);

    // Note: there is a slight inconsistency here: Karts are updated one
    // after another. So if the position of this kart is compared with the
    // slipstream area of a kart already updated, it will use the new
    // slipstream area of that kart, but for karts not yet updated the
    // old position will be used. The differences should not be noticable,
    // and simplifies the update process (which would otherwise have to be
    // done in two stages).
    unsigned int n     = race_manager->getNumKarts();
    bool is_sstreaming = false;
    for(unsigned int i=0; i<n; i++)
    {
        Kart *kart = race_manager->getKart(i);
        // Don't test for slipstream with itself.
        if(kart==this) continue;

        // Quick test: the kart must be not more than
        // slipstream length+kart_length() away from the other kart
        Vec3 delta = getXYZ() - kart->getXYZ();
        float l    = kart->m_kart_properties->getSlipstreamLength() + kart->getKartLength()*0.5f;
        if(delta.length2_2d() > l*l) continue;

        if(kart->m_slipstream_area->pointInQuad(getXYZ()))
        {
            is_sstreaming = true;
            break;
        }
    }

    float add_power = 0;


    if(m_slipstream_time >0 && !is_sstreaming)
    {
        // Kart is using slipstream advantage
        add_power         = getMaxPower() * m_kart_properties->getSlipstreamAddPower();
        m_slipstream_time = std::max(m_slipstream_time - dt, 0.0f);
        printf("Add power %f, t=%f for '%s'\n", m_slipstream_time, add_power, getIdent().c_str());
    }
    else if(is_sstreaming)
    {
        // Kart is collecting sliptstream advantage
        m_slipstream_time = std::min(m_slipstream_time + dt,
                                     m_kart_properties->getSlipstreamTime());

    }

    return add_power;
}   // handleSlipstream


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
    /** If a kart is crashing against the track, the collision is often
     *  reported more than once, resulting in a machine gun effect, and too
     *  long disabling of the engine. Therefore, this reaction is disabled
     *  for 0.5 seconds after a crash.
     */
    if(RaceManager::getWorld()->getTime()-m_time_last_crash < 0.5f) return;

    m_time_last_crash = RaceManager::getWorld()->getTime();
    // After a collision disable the engine for a short time so that karts
    // can 'bounce back' a bit (without this the engine force will prevent
    // karts from bouncing back, they will instead stuck towards the obstable).
    if(m_bounce_back_time<=0.0f)
    {
        // In case that the sfx is longer than 0.5 seconds, only play it if
        // it's not already playing.
        if(m_crash_sound->getStatus() != SFXManager::SFX_PLAYING)
            m_crash_sound->play();

        m_bounce_back_time = 0.1f;
    }
}   // crashed

// -----------------------------------------------------------------------------
void Kart::beep()
{
    // If the custom horn can't play (isn't defined) then play the default one
    if (!playCustomSFX(SFXManager::CUSTOM_HORN))
        m_beep_sound->play();

} // beep

// -----------------------------------------------------------------------------
// Plays custom SFX, returns whether or not the sound was played

bool Kart::playCustomSFX(unsigned int type)
{
    bool ret = false;
    if (type < SFXManager::NUM_CUSTOMS) 
        if (m_custom_sounds[type] != NULL)
        {
            ret = true;
            // Don't stutter
            printf("Kart SFX: playing %s for %s.\n", sfx_manager->getCustomTagName(type), m_kart_properties->getIdent().c_str());
            if (m_custom_sounds[type]->getStatus() != SFXManager::SFX_PLAYING)
                m_custom_sounds[type]->play();
        }
    return ret;
}
// -----------------------------------------------------------------------------
void Kart::updatePhysics (float dt)
{

    m_bounce_back_time-=dt;
    float engine_power = getActualWheelForce() + handleNitro(dt)
                                               + handleSlipstream(dt);
    if(m_attachment.getType()==ATTACH_PARACHUTE) engine_power*=0.2f;

    if(m_controls.m_accel)   // accelerating
    {   // For a short time after a collision disable the engine,
        // so that the karts can bounce back a bit from the obstacle.
        if(m_bounce_back_time>0.0f)
            engine_power = 0.0f;
        // let a player going backwards accelerate quickly (e.g. if a player hits a
        // wall, he needs to be able to start again quickly after going backwards)
        else if(m_speed < 0.0f)
            engine_power *= 5.0f;

        // Engine slow down due to terrain (see m_power_reduction is set in
        // update() depending on terrain type. Don't apply this if kart is already
        // going slowly, this would make it hard accelerating to get out of there
        if(m_speed > 4.0)
            engine_power *= m_power_reduction/stk_config->m_slowdown_factor;

        // Lose some traction when skidding, so it is not abused by player
        if(m_controls.m_drift)
            engine_power *= 0.5f;
        m_vehicle->applyEngineForce(engine_power, 2);
        m_vehicle->applyEngineForce(engine_power, 3);
        // Either all or no brake is set, so test only one to avoid
        // resetting all brakes most of the time.
        if(m_vehicle->getWheelInfo(0).m_brake &&
            !RaceManager::getWorld()->isStartPhase())
            resetBrakes();

    }
    else
    {   // not accelerating
        if(m_controls.m_brake)
        {   // check if the player is currently only slowing down or moving backwards
            if(m_speed > 0.0f)
            {   // going forward
                m_vehicle->applyEngineForce(0.f, 2);//engine off
                m_vehicle->applyEngineForce(0.f, 3);

                //apply the brakes
                for(int i=0; i<4; i++) m_vehicle->setBrake(getBrakeFactor(), i);
                m_skidding*= 1.08f;//skid a little when the brakes are hit (just enough to make the skiding sound)
                if(m_skidding>m_kart_properties->getMaxSkid())
                    m_skidding=m_kart_properties->getMaxSkid();
            }
            else   // m_speed < 0
            {
                resetBrakes();
                // going backward, apply reverse gear ratio (unless he goes too fast backwards)
                if ( -m_speed <  getMaxSpeedOnTerrain()*m_max_speed_reverse_ratio )
                {
                    // The backwards acceleration is artificially increased to
                    // allow players to get "unstuck" quicker if they hit e.g.
                    // a wall. At the same time we have to prevent that driving
                    // backards gives an advantage (see m_max_speed_reverse_ratio),
                    // and that a potential slowdown due to the terrain the
                    // kart is driving on feels right. The speedup factor on
                    // normal terrain (power_reduction/slowdown_factor should
                    // be 2.5 (which was experimentally determined to feel
                    // right).
                    float f = 2.5f - 3.8f*(1-m_power_reduction/stk_config->m_slowdown_factor);
                    // Avoid that a kart gets really stuck:
                    if(f<0.1f) f=0.1f;
                    m_vehicle->applyEngineForce(-engine_power*f, 2);
                    m_vehicle->applyEngineForce(-engine_power*f, 3);
                }
                else
                {
                    m_vehicle->applyEngineForce(0.f, 2);
                    m_vehicle->applyEngineForce(0.f, 3);
                }

            }
        }
        else
        {
            // lift the foot from throttle, brakes with 10% engine_power
            m_vehicle->applyEngineForce(-m_controls.m_accel*engine_power*0.1f, 2);
            m_vehicle->applyEngineForce(-m_controls.m_accel*engine_power*0.1f, 3);

#if 1
            // If not giving power (forward or reverse gear), and speed is low
            // we are "parking" the kart, so in battle mode we can ambush people, eg
            if(abs(m_speed) < 5.0f) {
                for(int i=0; i<4; i++) m_vehicle->setBrake(20.0f, i);
            }
#else
            if(!RaceManager::getWorld()->isStartPhase())
                resetBrakes();
#endif
        }
    }
#ifdef ENABLE_JUMP
    if(m_controls.jump && isOnGround())
    {
      //Vector3 impulse(0.0f, 0.0f, 10.0f);
      //        getVehicle()->getRigidBody()->applyCentralImpulse(impulse);
        btVector3 velocity         = m_body->getLinearVelocity();
        velocity.setZ( m_kart_properties->getJumpVelocity() );

        getBody()->setLinearVelocity( velocity );

    }
#endif
    if (isOnGround()){
        if((fabs(m_controls.m_steer) > 0.001f) && m_controls.m_drift)
        {
            m_skidding +=  m_kart_properties->getSkidIncrease()
                          *dt/m_kart_properties->getTimeTillMaxSkid();
            if(m_skidding>m_kart_properties->getMaxSkid())
                m_skidding=m_kart_properties->getMaxSkid();
        }
        else if(m_skidding>1.0f)
        {
            m_skidding *= m_kart_properties->getSkidDecrease();
            if(m_skidding<1.0f) m_skidding=1.0f;
        }
    }
    else
    {
        m_skidding = 1.0f; // Lose any skid factor as soon as we fly
    }
    if(m_skidding>1.0f)
    {
        if(m_skid_sound->getStatus() != SFXManager::SFX_PLAYING &&
            m_kart_properties->hasSkidmarks())
            m_skid_sound->play();
    }
    else if(m_skid_sound->getStatus() == SFXManager::SFX_PLAYING)
    {
        m_skid_sound->stop();
    }
    float steering = getMaxSteerAngle() * m_controls.m_steer*m_skidding;

    m_vehicle->setSteeringValue(steering, 0);
    m_vehicle->setSteeringValue(steering, 1);

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
    const float max_speed = getMaxSpeedOnTerrain();
    if ( m_speed >  max_speed )
    {
        const float velocity_ratio = max_speed/m_speed;
        m_speed                    = max_speed;
        btVector3 velocity         = m_body->getLinearVelocity();

        velocity.setY( velocity.getY() * velocity_ratio );
        velocity.setX( velocity.getX() * velocity_ratio );

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );

    }

    // To avoid tunneling (which can happen on long falls), clamp the
    // velocity in Z direction. Tunneling can happen if the Z velocity
    // is larger than the maximum suspension travel (per frame), since then
    // the wheel suspension can not stop/slow down the fall (though I am
    // not sure if this is enough in all cases!). So the speed is limited
    // to suspensionTravel / dt with dt = 1/60 (since this is the dt
    // bullet is using).
    const Vec3 &v = m_body->getLinearVelocity();
    if(v.getZ() < - m_kart_properties->getSuspensionTravelCM()*0.01f*60)
    {
        Vec3 v_clamped = v;
        // clamp the speed to 99% of the maxium falling speed.
        v_clamped.setZ(-m_kart_properties->getSuspensionTravelCM()*0.01f*60 * 0.99f);
        m_body->setLinearVelocity(v_clamped);
    }

    //at low velocity, forces on kart push it back and forth so we ignore this
    if(fabsf(m_speed) < 0.2f) // quick'n'dirty workaround for bug 1776883
         m_speed = 0;

    // when going faster, use higher pitch for engine
    if(m_engine_sound && sfx_manager->sfxAllowed())
    {
        m_engine_sound->speed(0.6f + (float)(m_speed / max_speed)*0.7f);
        m_engine_sound->position(getXYZ());
    }
#ifdef XX
    printf("forward %f %f %f %f  side %f %f %f %f angVel %f %f %f heading %f\n"
       ,m_vehicle->m_forwardImpulse[0]
       ,m_vehicle->m_forwardImpulse[1]
       ,m_vehicle->m_forwardImpulse[2]
       ,m_vehicle->m_forwardImpulse[3]
       ,m_vehicle->m_sideImpulse[0]
       ,m_vehicle->m_sideImpulse[1]
       ,m_vehicle->m_sideImpulse[2]
       ,m_vehicle->m_sideImpulse[3]
       ,m_body->getAngularVelocity().getX()
       ,m_body->getAngularVelocity().getY()
       ,m_body->getAngularVelocity().getZ()
       ,getHPR().getHeading()
       );
#endif
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

    m_vehicle->deactivateZipper();

    // let the mode decide where to put the kart
    RaceManager::getWorld()->moveKartAfterRescue(this, m_body);

    RaceManager::getWorld()->getPhysics()->addKart(this);
}   // endRescue

//-----------------------------------------------------------------------------

void Kart::loadData()
{
    m_kart_properties->getKartModel()->attachModel(&m_animated_node);
    createPhysics();

    // Attach Particle System
    m_smoke_system        = new Smoke(this);
    m_water_splash_system = new WaterSplash(this);
    m_nitro               = new Nitro(this);

    if(m_kart_properties->hasSkidmarks())
        m_skidmarks = new SkidMarks(*this);

    m_shadow = new Shadow(file_manager->getKartFile(m_kart_properties->getShadowFile(),
                                                    getIdent()                         ),
                          m_animated_node);
}   // loadData

//-----------------------------------------------------------------------------
/** Stores the current suspension length. This function is called from world
 *  after all karts are in resting position (see World::resetAllKarts), so
 *  that the default suspension rest length can be stored. This is then used
 *  later to move the wheels depending on actual suspension, so that when
 *  a kart is in rest, the wheels are at the position at which they were
 *  modelled.
 */
void Kart::setSuspensionLength()
{
    for(unsigned int i=0; i<4; i++)
    {
        m_default_suspension_length[i] =
            m_vehicle->getWheelInfo(i).m_raycastInfo.m_suspensionLength;
    }   // for i
}   // setSuspensionLength

//-----------------------------------------------------------------------------
void Kart::updateGraphics(const Vec3& off_xyz,  const Vec3& off_hpr)
{
    float wheel_z_axis[4];
    KartModel *kart_model = m_kart_properties->getKartModel();
    for(unsigned int i=0; i<4; i++)
    {
        // Set the suspension length
        wheel_z_axis[i] = m_default_suspension_length[i]
                        - m_vehicle->getWheelInfo(i).m_raycastInfo.m_suspensionLength;
    }
#define AUTO_SKID_VISUAL 1.7f
    float auto_skid;
    if (m_skidding>AUTO_SKID_VISUAL) // Above a limit, start counter rotating the wheels to get drifting look
        auto_skid = m_controls.m_steer*30.0f*((AUTO_SKID_VISUAL - m_skidding) / 0.8f); // divisor comes from max_skid - AUTO_SKID_VISUAL
    else
        auto_skid = m_controls.m_steer*30.0f;
    kart_model->update(m_wheel_rotation, auto_skid,
                       getSteerPercent(), wheel_z_axis);

    Vec3        center_shift  = getGravityCenterShift();
    float X = m_vehicle->getWheelInfo(0).m_chassisConnectionPointCS.getZ()
            - m_default_suspension_length[0]
            - m_vehicle->getWheelInfo(0).m_wheelsRadius
            - (kart_model->getWheelGraphicsRadius(0)
               -kart_model->getWheelGraphicsPosition(0).getZ() );
    center_shift.setZ(X);

    if(m_smoke_system)
    {
        float f=0.0f;
        if(getMaterial() && getMaterial()->hasSmoke() && fabsf(m_controls.m_steer) > 0.8 &&
            isOnGround())
            f=250.0f;
        m_smoke_system->setCreationRate((m_skidding-1)*f);
    }
    if(m_water_splash_system)
    {
        float f = getMaterial() && getMaterial()->hasWaterSplash() && isOnGround()
                ? sqrt(getSpeed())*40.0f
                : 0.0f;
        m_water_splash_system->setCreationRate(f);
    }
    if(m_nitro)
        m_nitro->setCreationRate(m_controls.m_nitro && m_collected_energy>0
                                 ? (10.0f + getSpeed()*20.0f) : 0);

    float speed_ratio    = getSpeed()/getMaxSpeed();
    float offset_heading = getSteerPercent()*m_kart_properties->getSkidVisual()
                         * speed_ratio * m_skidding*m_skidding;
    Moveable::updateGraphics(center_shift, Vec3(offset_heading, 0, 0));
}   // updateGraphics

/* EOF */
