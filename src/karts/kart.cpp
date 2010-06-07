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

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/nitro.hpp"
#include "graphics/shadow.hpp"
#include "graphics/skid_marks.hpp"
#include "graphics/slip_stream.hpp"
#include "graphics/smoke.hpp"
#include "graphics/stars.hpp"
#include "graphics/water_splash.hpp"
#include "modes/world.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/controller/end_controller.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/race_state.hpp"
#include "network/network_manager.hpp"
#include "physics/btKart.hpp"
#include "physics/btUprightConstraint.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
   // Disable warning for using 'this' in base member initializer list
#  pragma warning(disable:4355)
#endif

/** The kart constructor. 
 *  \param ident  The identifier for the kart model to use.
 *  \param position The position (or rank) for this kart (between 1 and
 *         number of karts). This is used to determine the start position.
 *  \param init_transform  The initial position and rotation for this kart.
 */
Kart::Kart (const std::string& ident, int position,
            const btTransform& init_transform)
     : TerrainInfo(1),
       Moveable(), EmergencyAnimation(this), m_powerup(this)

#if defined(WIN32) && !defined(__CYGWIN__)
#  pragma warning(1:4355)
#endif
{
    m_kart_properties      = kart_properties_manager->getKart(ident);
    assert(m_kart_properties != NULL);
    m_initial_position     = position;
    m_collected_energy     = 0;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_slipstream_time      = 0.0f;
    m_shadow_enabled       = false;
    m_shadow               = NULL;
    m_smoke_system         = NULL;
    m_stars_effect         = NULL;
    m_water_splash_system  = NULL;
    m_nitro                = NULL;
    m_slip_stream          = NULL;
    m_skidmarks            = NULL;
    m_camera               = NULL;
    m_controller           = NULL;
    m_saved_controller     = NULL;

    m_view_blocked_by_plunger = 0;
    m_race_position           = -1;

    // Initialize custom sound vector (TODO: add back when properly done)
    // m_custom_sounds.resize(SFXManager::NUM_CUSTOMS);

    // Set position and heading:
    m_reset_transform      = init_transform;

    // Neglecting the roll resistance (which is small for high speeds compared
    // to the air resistance), maximum speed is reached when the engine
    // power equals the air resistance force, resulting in this formula:
    m_max_speed               = m_kart_properties->getMaxSpeed();
    m_max_speed_reverse_ratio = m_kart_properties->getMaxSpeedReverseRatio();
    m_speed                   = 0.0f;

    m_wheel_rotation          = 0;

    // Create SFXBase for each custom sound (TODO: add back when properly done)
    /*
    for (int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        int id = m_kart_properties->getCustomSfxId((SFXManager::CustomSFX)n);

        // If id == -1 the custom sound was not defined in the .irrkart config file
        if (id != -1)
        {
            m_custom_sounds[n] = sfx_manager->newSFX(id);
        }
    }*/

    m_engine_sound = sfx_manager->createSoundSource(m_kart_properties->getEngineSfxType());
    m_beep_sound   = sfx_manager->createSoundSource( "beep"  );
    m_crash_sound  = sfx_manager->createSoundSource( "crash" );
    m_goo_sound    = sfx_manager->createSoundSource( "goo"   );
    m_skid_sound   = sfx_manager->createSoundSource( "skid"  );

    if(!m_engine_sound)
    {
        fprintf(stdout, "Error: Could not allocate a sfx object for the kart. Further errors may ensue!\n");
    }

    loadData();
    float length = m_kart_properties->getSlipstreamLength();

    Vec3 p0(-getKartWidth()*0.5f, 0, -getKartLength()*0.5f       );
    Vec3 p1(-getKartWidth()*0.5f, 0, -getKartLength()*0.5f-length);
    Vec3 p2( getKartWidth()*0.5f, 0, -getKartLength()*0.5f-length);
    Vec3 p3( getKartWidth()*0.5f, 0, -getKartLength()*0.5f       );
    m_slipstream_original_quad = new Quad(p0, p1, p2, p3);
    m_slipstream_quad          = new Quad(p0, p1, p2, p3);

    reset();
}   // Kart

// -----------------------------------------------------------------------------
/** Saves the old controller in m_saved_controller and stores a new 
 *  controller. The save controller is needed in case of a reset.
 *  \param controller The new controller to use (atm it's always an
 *         end controller).
 */
void Kart::setController(Controller *controller)
{
    assert(m_saved_controller==NULL);
    m_saved_controller = m_controller;
    m_controller       = controller;
}   // setController

// -----------------------------------------------------------------------------
/** Returns a transform that will align an object with the kart: the heading 
 *  and the pitch will be set appropriately. A custom pitch value can be 
 *  specified in order to overwrite the terrain pitch (which would be used
 *  otherwise).
 *  \param customPitch Pitch value to overwrite the terrain pitch.
 */
btTransform Kart::getKartHeading(const float customPitch)
{
    btTransform trans = getTrans();

    float pitch = (customPitch == -1 ? getTerrainPitch(getHeading()) : customPitch);

    btMatrix3x3 m;
    m.setEulerYPR(-getHeading(), pitch, 0.0f);
    trans.setBasis(m);

    return trans;
}   // getKartHeading

// ----------------------------------------------------------------------------
/** Creates the physical representation of this kart. Atm it uses the actual
 *  extention of the kart model to determine the size of the collision body.
 */
void Kart::createPhysics()
{
    // First: Create the chassis of the kart
    // -------------------------------------
    const KartModel *km = m_kart_properties->getKartModel();
    float kart_width  = km->getWidth();
    float kart_length = km->getLength();
    float kart_height = km->getHeight();

    btBoxShape *shape = new btBoxShape(btVector3(0.5f*kart_width,
                                                 0.5f*kart_height,
                                                 0.5f*kart_length));
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
        new btDefaultVehicleRaycaster(World::getWorld()->getPhysics()->getPhysicsWorld());
    m_tuning  = new btKart::btVehicleTuning();
    m_tuning->m_maxSuspensionTravelCm = m_kart_properties->getSuspensionTravelCM();
    m_vehicle = new btKart(*m_tuning, m_body, m_vehicle_raycaster,
                           m_kart_properties->getTrackConnectionAccel());

    // never deactivate the vehicle
    m_body->setActivationState(DISABLE_DEACTIVATION);
    m_vehicle->setCoordinateSystem(/*right: */ 0,  /*up: */ 1,  /*forward: */ 2);

    // Add wheels
    // ----------
    float wheel_radius    = m_kart_properties->getWheelRadius();
    float suspension_rest = m_kart_properties->getSuspensionRest();

    btVector3 wheel_direction(0.0f, -1.0f, 0.0f);
    btVector3 wheel_axle(-1.0f, 0.0f, 0.0f);

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
    m_uprightConstraint=new btUprightConstraint(this, t);
    m_uprightConstraint->setLimit(m_kart_properties->getUprightTolerance());
    m_uprightConstraint->setBounce(0.0f);
    m_uprightConstraint->setMaxLimitForce(m_kart_properties->getUprightMaxForce());
    m_uprightConstraint->setErp(1.0f);
    m_uprightConstraint->setLimitSoftness(1.0f);
    m_uprightConstraint->setDamping(0.0f);
    World::getWorld()->getPhysics()->addKart(this);

    //create the engine sound
    if(m_engine_sound)
    {
        m_engine_sound->speed(0.6f);
        m_engine_sound->loop();
        m_engine_sound->play();
    }
}   // createPhysics

// ----------------------------------------------------------------------------
/** The destructor frees the memory of this kart, but note that the actual kart
 *  model is still stored in the kart_properties (m_kart_model variable), so 
 *  it is not reloaded).
 */
Kart::~Kart()
{
    //stop the engine sound
    if(m_engine_sound)
    {
        m_engine_sound->stop();
    }

    // Delete all custom sounds (TODO: add back when properly done)
    /*
    for (int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        if (m_custom_sounds[n] != NULL)
            sfx_manager->deleteSFX(m_custom_sounds[n]);
    }*/

    sfx_manager->deleteSFX(m_engine_sound );
    sfx_manager->deleteSFX(m_crash_sound  );
    sfx_manager->deleteSFX(m_skid_sound   );
    sfx_manager->deleteSFX(m_goo_sound    );

    if(m_smoke_system)        delete m_smoke_system;
    if(m_water_splash_system) delete m_water_splash_system;
    if(m_nitro)               delete m_nitro;
    if(m_slip_stream)         delete m_slip_stream;

    delete m_shadow;
    delete m_stars_effect;

    if(m_skidmarks) delete m_skidmarks ;

    World::getWorld()->getPhysics()->removeKart(this);
    delete m_vehicle;
    delete m_tuning;
    delete m_vehicle_raycaster;
    delete m_uprightConstraint;
    for(int i=0; i<m_kart_chassis.getNumChildShapes(); i++)
    {
        delete m_kart_chassis.getChildShape(i);
    }
    delete m_slipstream_original_quad;
    delete m_slipstream_quad;
}   // ~Kart

//-----------------------------------------------------------------------------
/** Eliminates a kart from the race. It removes the kart from the physics
 *  world, and makes the scene node invisible.
 */
void Kart::eliminate()
{
    if (!playingEmergencyAnimation())
    {
        World::getWorld()->getPhysics()->removeKart(this);
    }
    m_eliminated = true;

    getNode()->setVisible(false);
}   // eliminate

//-----------------------------------------------------------------------------
/** Returns true if the kart is 'resting', i.e. (nearly) not moving.
 */
bool Kart::isInRest() const
{
    return fabs(m_body->getLinearVelocity ().y())<0.2;
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
/** Reset before a new race. It will remove all attachments, and
 *  puts the kart back at its original start position.
 */
void Kart::reset()
{
    EmergencyAnimation::reset();
    if (m_camera)
    {
        m_camera->reset();
        m_camera->setInitialTransform();
    }
    
    // Stop any animations currently being played.
    m_kart_properties->getKartModel()->setAnimation(KartModel::AF_DEFAULT);
    // If the controller was replaced (e.g. replaced by end controller), 
    //  restore the original controller. 
    if(m_saved_controller)
    {
        m_controller       = m_saved_controller;
        m_saved_controller = NULL;
    }
    m_kart_properties->getKartModel()->setAnimation(KartModel::AF_DEFAULT);
    m_view_blocked_by_plunger = 0.0;
    m_attachment->clear();
    m_powerup.reset();

    m_race_position        = 9;
    m_eliminated           = false;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_zipper_time_left     = 0.0f;
    m_collected_energy     = 0;
    m_wheel_rotation       = 0;
    m_bounce_back_time     = 0.0f;
    m_skidding             = 1.0f;
    m_time_last_crash      = 0.0f;
    m_max_speed_reduction  = 0.0f;
    m_power_reduction      = 1.0f;
    m_slipstream_mode      = SS_NONE;

    m_controls.m_steer     = 0.0f;
    m_controls.m_accel     = 0.0f;
    m_controls.m_brake     = false;
    m_controls.m_nitro     = false;
    m_controls.m_drift     = false;
    m_controls.m_fire      = false;
    m_controls.m_look_back = false;
    // Reset star effect in case that it is currently being shown.
    m_stars_effect->reset();
    m_slip_stream->reset();
    m_vehicle->deactivateZipper();

    // Set the brakes so that karts don't slide downhill
    for(int i=0; i<4; i++) m_vehicle->setBrake(5.0f, i);

    setTrans(m_reset_transform);

    applyEngineForce (0.0f);

    Moveable::reset();
    if(m_skidmarks) m_skidmarks->reset();
    for(int j=0; j<m_vehicle->getNumWheels(); j++)
    {
        m_vehicle->updateWheelTransform(j, true);
    }

    TerrainInfo::update(getXYZ());

    // Reset is also called when the kart is created, at which time
    // m_controller is not yet defined, so this has to be tested here.
    if(m_controller)
        m_controller->reset();

}   // reset

//-----------------------------------------------------------------------------
/** Sets that this kart has finished the race and finishing time. It also
 *  notifies the race_manager about the race completion for this kart.
 *  \param time The finishing time for this kart. It can either be the
 *         actual time when the kart finished (in which case time() = 
 *         world->getTime()), or the estimated time in case that all
 *         player kart have finished the race and all AI karts get
 *         an estimated finish time set.
 */
void Kart::finishedRace(float time)
{
    // m_finished_race can be true if e.g. an AI kart was set to finish
    // because the race was over (i.e. estimating the finish time). If
    // this kart then crosses the finish line (with the end controller)
    // it would trigger a race end again.
    if(m_finished_race) return;
    m_finished_race = true;
    m_finish_time   = time;
    m_controller->finishedRace(time);
    race_manager->kartFinishedRace(this, time);
    
    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_NORMAL_RACE ||
        race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
    {
        // in modes that support it, start end animation
        setController(new EndController(this, m_controller->getPlayer()));
        if(m_race_position<=0.5f*race_manager->getNumberOfKarts() ||
            m_race_position==1)
            m_kart_properties->getKartModel()->setAnimation(KartModel::AF_WIN_START);
        else 
            m_kart_properties->getKartModel()->setAnimation(KartModel::AF_LOSE_START);
        
        // Not all karts have a camera
        if (m_camera) m_camera->setMode(Camera::CM_REVERSE);
        
        RaceGUI* m = World::getWorld()->getRaceGUI();
        if(m)
        {
            m->addMessage((getPosition() == 1 ? _("You won the race!") : _("You finished the race!")) ,
                          this, 2.0f, 60);
        }
    }
    else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {        
        // start end animation
        setController(new EndController(this, m_controller->getPlayer()));
        if(m_race_position<=2)
            m_kart_properties->getKartModel()->setAnimation(KartModel::AF_WIN_START);
        else if(m_race_position>=0.7f*race_manager->getNumberOfKarts())
            m_kart_properties->getKartModel()->setAnimation(KartModel::AF_LOSE_START);
            
        // Not all karts have a camera
        if (m_camera) m_camera->setMode(Camera::CM_REVERSE);
        
        RaceGUI* m = World::getWorld()->getRaceGUI();
        if(m)
        {
            m->addMessage((getPosition() == 2 ? _("You won the race!") : _("You finished the race!")) ,
                          this, 2.0f, 60);
        }
    }
    else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES)
    {
    }             
        
}   // finishedRace

//-----------------------------------------------------------------------------
/** Called when an item is collected. It will either adjust the collected
 *  energy, or update the attachment or powerup for this kart.
 *  \param item The item that was hit.
 *  \param add_info Additional info, used in networking games to force
 *         a specific item to be used (instead of a random item) to keep
 *         all karts in synch.
 */
void Kart::collectedItem(const Item &item, int add_info)
{
    float old_energy          = m_collected_energy;
    const Item::ItemType type = item.getType();

    switch (type)
    {
    case Item::ITEM_BANANA: 
        m_attachment->hitBanana(item, add_info); 
        break;

    case Item::ITEM_NITRO_SMALL: m_collected_energy++;    break;
    case Item::ITEM_NITRO_BIG:   m_collected_energy += 3; break;
    case Item::ITEM_BONUS_BOX  :
        {
            // In wheelie style, karts get more items depending on energy,
            // in nitro mode it's only one item.
            int n = 1;
            m_powerup.hitBonusBox(n, item, add_info);
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
        (type==Item::ITEM_NITRO_BIG || type==Item::ITEM_NITRO_SMALL) )
    {
        race_state->itemCollected(getWorldKartId(), item.getItemId());
    }

    if ( m_collected_energy > MAX_ITEMS_COLLECTED )
        m_collected_energy = MAX_ITEMS_COLLECTED;
    m_controller->collectedItem(item, add_info, old_energy);

}   // collectedItem

//-----------------------------------------------------------------------------
/** Simulates gears by adjusting the force of the engine. It also takes the
 *  effect of the zipper into account.
 */
float Kart::getActualWheelForce()
{
    float zipperF=(m_zipper_time_left>0.0f) ? stk_config->m_zipper_force : 0.0f;
    const std::vector<float>& gear_ratio=m_kart_properties->getGearSwitchRatio();
    for(unsigned int i=0; i<gear_ratio.size(); i++)
    {
        if(m_speed <= getMaxSpeed()*gear_ratio[i])
        {
            return getMaxPower()*m_kart_properties->getGearPowerIncrease()[i]
                  +zipperF;
        }
    }
    return getMaxPower()+zipperF;

}   // getActualWheelForce

//-----------------------------------------------------------------------------
/** The kart is on ground if all 4 wheels touch the ground, and if no special
 *  animation (rescue, explosion etc.) is happening).
 */
bool Kart::isOnGround() const
{
    return (m_vehicle->getNumWheelsOnGround() == m_vehicle->getNumWheels()
          && !playingEmergencyAnimation());
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
/** Updates the kart in each time step. It updates the physics setting,
 *  particle effects, camera position, etc.
 *  \param dt Time step size.
 */
void Kart::update(float dt)
{
    if(!history->replayHistory())
        m_controller->update(dt);
    if(m_camera)
        m_camera->update(dt);
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
        && !playingEmergencyAnimation())
    {
        // use() needs to be called even if there currently is no collecteable
        // since use() can test if something needs to be switched on/off.
        m_powerup.use() ;
        m_controls.m_fire = false;
    }

    // When really on air, free fly, when near ground, try to glide / adjust for landing
    // If zipped, be stable, so ramp+zipper can allow nice jumps without scripting the fly
    if(!isNearGround() && !(m_zipper_time_left > 0.0f))
        m_uprightConstraint->setLimit(M_PI);
    else
        m_uprightConstraint->setLimit(m_kart_properties->getUprightTolerance());

    // TODO: hiker said this probably will be moved to btKart or so when updating bullet engine.
    // Neutralize any yaw change if the kart leaves the ground, so the kart falls more or less
    // straight after jumping, but still allowing some "boat shake" (roll and pitch).
    // Otherwise many non perfect jumps end in a total roll over or a serious change of
    // direction, sometimes 90 or even full U turn (real but less fun for a karting game).
    // As side effect steering becames a bit less responsive (any wheel on air), but not too bad.
    if(!isOnGround()) {
        btVector3 speed = m_body->getAngularVelocity();
        speed.setX(speed.getX() * 0.95f);
        speed.setY(speed.getY() * 0.25f); // or 0.0f for sharp neutralization of yaw
        speed.setZ(speed.getZ() * 0.95f);
        m_body->setAngularVelocity(speed);
        // This one keeps the kart pointing "100% as launched" instead,
        // like in ski jump sports, too boring but also works.
        //m_body->setAngularVelocity(btVector3(0,0,0));
    }

    m_zipper_time_left = m_zipper_time_left>0.0f ? m_zipper_time_left-dt : 0.0f;

    //m_wheel_rotation gives the rotation around the X-axis, and since velocity's
    //timeframe is the delta time, we don't have to multiply it with dt.
    m_wheel_rotation += m_speed*dt / m_kart_properties->getWheelRadius();
    m_wheel_rotation=fmodf(m_wheel_rotation, 2*M_PI);

    EmergencyAnimation::update(dt);

    m_attachment->update(dt);

    //smoke drawing control point
    if ( UserConfigParams::m_graphical_effects )
    {
        m_smoke_system->update(dt);
        m_water_splash_system->update(dt);
        m_nitro->update(dt);
        m_slip_stream->update(dt);
        // update star effect (call will do nothing if stars are not activated)
        m_stars_effect->update(dt);

    }  // UserConfigParams::m_graphical_effects

    updatePhysics(dt);

    Moveable::update(dt);

    /* (TODO: add back when properly done)
    for (int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        if (m_custom_sounds[n] != NULL) m_custom_sounds[n]->position   ( getXYZ() );
    }
     */
    
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
    Vec3 pos_plus_epsilon = trans.getOrigin()+btVector3(0,0.3f,0);

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
    m_power_reduction = 1.0f;
    if (getHoT()==Track::NOHIT)   // kart falling off the track
    {
        // let kart fall a bit before rescuing
        const Vec3 *min, *max;
        World::getWorld()->getTrack()->getAABB(&min, &max);
        if(min->getY() - getXYZ().getY() > 17)
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
    item_manager->checkItemHit(this);
    if(m_kart_properties->hasSkidmarks())
        m_skidmarks->update(dt);

    // Remove the shadow if the kart is not on the ground (if a kart
    // is rescued isOnGround might still be true, since the kart rigid
    // body was removed from the physics, but still retain the old
    // values for the raycasts).
    if( (!isOnGround() || playingEmergencyAnimation()) && m_shadow_enabled)
    {
        m_shadow_enabled = false;
        m_shadow->disableShadow();
    }
    if(!m_shadow_enabled && isOnGround() && !playingEmergencyAnimation())
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
    m_controller->handleZipper();
}   // handleZipper

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
 *  slipstreaming, and if a kart was slipstreaming long enough, it will
 *  add power to the kart for a certain amount of time.
 */
float Kart::handleSlipstream(float dt)
{
    // First see if we are currently using accumulated slipstream credits:
    // -------------------------------------------------------------------
    if(m_slipstream_mode==SS_USE)
    {
        m_slipstream_time -= dt;
        if(m_slipstream_time<0) m_slipstream_mode=SS_NONE;
        m_slip_stream->setIntensity(2.0f, NULL);
        return m_kart_properties->getSlipstreamAddPower();
    }

    // If this kart is too slow for slipstreaming taking effect, do nothing
    // --------------------------------------------------------------------

    // Define this to get slipstream effect shown even when the karts are
    // not moving. This is useful for debugging the graphics of SS-ing.
#undef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
#ifndef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
    if(getSpeed()<m_kart_properties->getSlipstreamMinSpeed())
    {
        m_slip_stream->setIntensity(0, NULL);
        return 0;
    }
#endif
    // Then test if this kart is in the slipstream range of another kart:
    // ------------------------------------------------------------------
    m_slipstream_original_quad->transform(getTrans(), m_slipstream_quad);

    World *world           = World::getWorld();
    unsigned int num_karts = world->getNumKarts();
    bool is_sstreaming     = false;
    Kart *target_kart;
    for(unsigned int i=0; i<num_karts; i++)
    {
        target_kart = world->getKart(i);
        // Don't test for slipstream with itself.
        if(target_kart==this            || 
            target_kart->isEliminated() || 
            target_kart->hasFinishedRace()) continue;

        // If the kart we are testing against is too slow, no need to test
        // slipstreaming. Note: We compare the speed of the other kart 
        // against the minimum slipstream speed kart of this kart - not 
        // entirely sure if this makes sense, but it makes it easier to 
        // give karts different slipstream properties.
#ifndef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
        if(target_kart->getSpeed()<m_kart_properties->getSlipstreamMinSpeed()) 
            continue;
#endif
        // Quick test: the kart must be not more than
        // slipstream length+kart_length() away from the other kart
        Vec3 delta = getXYZ() - target_kart->getXYZ();
        float l    = target_kart->m_kart_properties->getSlipstreamLength() 
                   + target_kart->getKartLength()*0.5f;
        if(delta.length2_2d() > l*l) continue;
        if(target_kart->m_slipstream_quad->pointInQuad(getXYZ()))
        {
            is_sstreaming     = true;
            break;
        }
    }   // for i < num_karts

    if(!is_sstreaming)
    {
        m_slipstream_time -=dt;
        if(m_slipstream_time<0) m_slipstream_mode = SS_NONE;
        m_slip_stream->setIntensity(0, NULL);
        return 0;
    }   // for i<number of karts

    // Accumulate slipstream credits now
    m_slipstream_time = m_slipstream_mode==SS_NONE ? dt 
                                                   : m_slipstream_time+dt;
    //printf("Collecting slipstream %f\n", m_slipstream_time);
    m_slip_stream->setIntensity(m_slipstream_time, target_kart);

    m_slipstream_mode = SS_COLLECT;
    if(m_slipstream_time>m_kart_properties->getSlipstreamCollectTime())
    {
        m_slipstream_mode = SS_USE;
        //handleZipper(); // FIXME(/REMOVE?) Zipper gives a sharp push, maybe too sharp
        //return 0;       // see below about abusing m_zipper_time_left without zipper
        return m_kart_properties->getSlipstreamAddPower();
    }
    m_zipper_time_left = 5.0f; // FIXME, this is a hack to test higher speed limit without zipper, better would be own counter
    return m_kart_properties->getSlipstreamAddPower();
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
/** Called when the kart crashes against the track (k=NULL) or another kart.
 *  \params k Either a kart if a kart was hit, or NULL if the track was hit.
 */
void Kart::crashed(Kart *k)
{
    m_controller->crashed();
    /** If a kart is crashing against the track, the collision is often
     *  reported more than once, resulting in a machine gun effect, and too
     *  long disabling of the engine. Therefore, this reaction is disabled
     *  for 0.5 seconds after a crash.
     */
    if(World::getWorld()->getTime()-m_time_last_crash < 0.5f) return;

    m_time_last_crash = World::getWorld()->getTime();
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
/*
    playCustomSFX()

    This function will play a particular character voice for this kart.  It
    returns whether or not a character voice sample exists for the particular
    event.  If there is no voice sample, a default can be played instead.

    Use entries from the CustomSFX enumeration as a parameter (see
    sfx_manager.hpp).  eg. playCustomSFX(SFXManager::CUSTOM_CRASH)

    Obviously we don't want a certain character voicing multiple phrases
    simultaneously.  It just sounds bad.  There are two ways of avoiding this:

    1.  If there is already a voice sample playing for the character
        don't play another until it is finished.

    2.  If there is already a voice sample playing for the character
        stop the sample, and play the new one.

    Currently we're doing #2.

    rforder

*/

bool Kart::playCustomSFX(unsigned int type)
{
    // (TODO: add back when properly done)
    return false;
    
    /*
    bool ret = false;

    // Stop all other character voices for this kart before playing a new one
    // we don't want overlapping phrases coming from the same kart
    for (unsigned int n = 0; n < SFXManager::NUM_CUSTOMS; n++)
    {
        if (m_custom_sounds[n] != NULL)
        {
            // If the sound we're trying to play is already playing
            // don't stop it, we'll just let it finish.
            if (type != n) m_custom_sounds[n]->stop();
        }
    }

    if (type < SFXManager::NUM_CUSTOMS) 
    {
        if (m_custom_sounds[type] != NULL)
        {
            ret = true;
            //printf("Kart SFX: playing %s for %s.\n", sfx_manager->getCustomTagName(type), m_kart_properties->getIdent().c_str());
            // If it's already playing, let it finish
            if (m_custom_sounds[type]->getStatus() != SFXManager::SFX_PLAYING)
            {
                m_custom_sounds[type]->play();
            }
        }
    }
    return ret;
     */
}
// ----------------------------------------------------------------------------
/** Updates the physics for this kart: computing the driving force, set 
 *  steering, handles skidding, terrain impact on kart, ...
 *  \param dt Time step size.
 */
void Kart::updatePhysics(float dt)
{
    m_bounce_back_time-=dt;
    float engine_power = getActualWheelForce() + handleNitro(dt)
                                               + handleSlipstream(dt);
    if(m_attachment->getType()==ATTACH_PARACHUTE) engine_power*=0.2f;

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
            engine_power *= m_power_reduction;

        // Lose some traction when skidding, to balance the adventage
        // Up to r5483 AIs were allowed to cheat in medium and high diff levels
        if(m_controls.m_drift)
            engine_power *= 0.5f;
        applyEngineForce(engine_power);

        // Either all or no brake is set, so test only one to avoid
        // resetting all brakes most of the time.
        if(m_vehicle->getWheelInfo(0).m_brake &&
            !World::getWorld()->isStartPhase())
            resetBrakes();
    }
    else
    {   // not accelerating
        if(m_controls.m_brake)
        {   // check if the player is currently only slowing down or moving backwards
            if(m_speed > 0.0f)
            {   // going forward
                applyEngineForce(0.f);

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
                    float f = 2.5f - 3.8f*(1-m_power_reduction);
                    // Avoid that a kart gets really stuck:
                    if(f<0.1f) f=0.1f;
                    applyEngineForce(-engine_power*f);
                }
                else  // -m_speed >= max speed on this terrain
                {
                    applyEngineForce(0.0f);
                }

            }   // m_speed <00
        }
        else   // !m_brake
        {
            // lift the foot from throttle, brakes with 10% engine_power
            applyEngineForce(-m_controls.m_accel*engine_power*0.1f);

            // If not giving power (forward or reverse gear), and speed is low
            // we are "parking" the kart, so in battle mode we can ambush people, eg
            if(abs(m_speed) < 5.0f) {
                for(int i=0; i<4; i++) m_vehicle->setBrake(20.0f, i);
            }
        }   // !m_brake
    }   // not accelerating
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
               chassisTrans.getBasis()[0][2],
               chassisTrans.getBasis()[1][2],
               chassisTrans.getBasis()[2][2]);

    if (forwardW.dot(getVehicle()->getRigidBody()->getLinearVelocity()) < btScalar(0.))
        m_speed *= -1.f;

    //cap at maximum velocity
    float max_speed = getMaxSpeedOnTerrain();
    if (m_zipper_time_left > 0.0f)
        max_speed *= (1.0f + stk_config->m_zipper_max_speed_fraction);
    if ( m_speed >  max_speed )
    {
        const float velocity_ratio = max_speed/m_speed;
        m_speed                    = max_speed;
        btVector3 velocity         = m_body->getLinearVelocity();

        velocity.setZ( velocity.getZ() * velocity_ratio );
        velocity.setX( velocity.getX() * velocity_ratio );
        velocity.setY( velocity.getY() * velocity_ratio ); // Up-down too

        getVehicle()->getRigidBody()->setLinearVelocity( velocity );

    }

    // To avoid tunneling (which can happen on long falls), clamp the
    // velocity in Y direction. Tunneling can happen if the Y velocity
    // is larger than the maximum suspension travel (per frame), since then
    // the wheel suspension can not stop/slow down the fall (though I am
    // not sure if this is enough in all cases!). So the speed is limited
    // to suspensionTravel / dt with dt = 1/60 (since this is the dt
    // bullet is using).
    // Only apply if near ground instead of purely based on speed avoiding
    // the "parachute on top" look.
    const Vec3 &v = m_body->getLinearVelocity();
    if(/*isNearGround() &&*/ v.getY() < - m_kart_properties->getSuspensionTravelCM()*0.01f*60)
    {
        Vec3 v_clamped = v;
        // clamp the speed to 99% of the maxium falling speed.
        v_clamped.setY(-m_kart_properties->getSuspensionTravelCM()*0.01f*60 * 0.99f);
        m_body->setLinearVelocity(v_clamped);
    }

    //at low velocity, forces on kart push it back and forth so we ignore this
    if(fabsf(m_speed) < 0.2f) // quick'n'dirty workaround for bug 1776883
         m_speed = 0;

    // when going faster, use higher pitch for engine
    if(m_engine_sound && sfx_manager->sfxAllowed())
    {
        if(isOnGround())
        {
            // Engine noise is based half in total speed, half in fake gears:
            // With a sawtooth graph like /|/|/| we get 3 even spaced gears,
            // ignoring the gear settings from stk_config, but providing a
            // good enough brrrBRRRbrrrBRRR sound effect. Speed factor makes
            // it a "staired sawtooth", so more acoustically rich.
            float gears = 3.0f * fmod((float)(m_speed / max_speed), 0.333334f);
            m_engine_sound->speed(0.6f +
                                  (float)(m_speed / max_speed) * 0.35f +
                                  gears * 0.35f);
        }
        else
        {
            // When flying, fixed and fast engine noise to make it more scary
            m_engine_sound->speed(1.4f);
        }
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
/** Attaches the right model, creates the physics and loads all special 
 *  effects (particle systems etc.)
 */
void Kart::loadData()
{
    m_kart_properties->getKartModel()->attachModel(&m_node);
    // Attachment must be created after attachModel, since only then the
    // scene node will exist (to which the attachment is added). But the
    // attachment is needed in createPhysics (which gets the mass, which
    // is dependent on the attachment).
    m_attachment = new Attachment(this);
    createPhysics();

    // Attach Particle System
    m_smoke_system        = new Smoke(this);
    m_water_splash_system = new WaterSplash(this);
    m_nitro               = new Nitro(this);
    m_slip_stream         = new SlipStream(this);

    if(m_kart_properties->hasSkidmarks())
        m_skidmarks = new SkidMarks(*this);
       
    m_shadow = new Shadow(m_kart_properties->getShadowTexture(),
                          m_node);

    m_stars_effect = new Stars(m_node);
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
/** Applies engine power to all the wheels that are traction capable,
 *  so other parts of code do not have to be adjusted to simulate different
 *  kinds of vehicles in the general case, only if they are trying to
 *  simulate traction control, diferentials or multiple independent electric
 *  engines, they will have to tweak the power in a per wheel basis.
 */
void Kart::applyEngineForce(float force)
{
    // Split power to simulate a 4WD 40-60, other values possible
    // FWD or RWD is a matter of putting a 0 and 1 in the right place
    float frontForce = force*0.4f;
    float rearForce = force*0.6f;
    // Front wheels
    for(unsigned int i=0; i<2; i++)
    {
        m_vehicle->applyEngineForce (frontForce, i);
    }
    // Rear wheels
    for(unsigned int i=2; i<4; i++)
    {
        m_vehicle->applyEngineForce (rearForce, i);
    }
}   // applyEngineForce

//-----------------------------------------------------------------------------
/** Updates the graphics model. Mainly set the graphical position to be the
 *  same as the physics position, but uses offsets to position and rotation
 *  for special gfx effects (e.g. skidding will turn the karts more). These
 *  variables are actually not used here atm, but are defined here and then
 *  used in Moveable.
 *  \param offset_xyz Offset to be added to the position.
 *  \param rotation Additional rotation.
 */
void Kart::updateGraphics(const Vec3& offset_xyz, 
                          const btQuaternion& rotation)
{
    float wheel_up_axis[4];
    KartModel *kart_model = m_kart_properties->getKartModel();
    for(unsigned int i=0; i<4; i++)
    {
        // Set the suspension length
        wheel_up_axis[i] = m_default_suspension_length[i]
                         - m_vehicle->getWheelInfo(i).m_raycastInfo.m_suspensionLength;
    }
    const float auto_skid_visual=1.7f;
    float auto_skid;
    // FIXME
//    if (m_skidding>auto_skid_visual) // Above a limit, start counter rotating the wheels to get drifting look
//        auto_skid = m_controls.m_steer*30.0f*((auto_skid_visual - m_skidding) / 0.8f); // divisor comes from max_skid - AUTO_SKID_VISUAL
//    else
        auto_skid = m_controls.m_steer*30.0f;
    kart_model->update(m_wheel_rotation, auto_skid,
                       getSteerPercent(), wheel_up_axis);

    Vec3        center_shift  = getGravityCenterShift();
    float y = m_vehicle->getWheelInfo(0).m_chassisConnectionPointCS.getY()
            - m_default_suspension_length[0]
            - m_vehicle->getWheelInfo(0).m_wheelsRadius
            - (kart_model->getWheelGraphicsRadius(0)
               -kart_model->getWheelGraphicsPosition(0).getY() );
    center_shift.setY(y);

    if(m_smoke_system)
    {
        float f=0.0f;
        if(getMaterial() && getMaterial()->hasSmoke() && 
            fabsf(m_controls.m_steer) > 0.8           &&
            isOnGround()                                  )
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
        // fabs(speed) is important, otherwise the negative number will
        // become a huge unsigned number in the particle scene node!
        m_nitro->setCreationRate(m_controls.m_nitro && m_collected_energy>0
                                 ? (10.0f + fabsf(getSpeed())*20.0f) : 0);

    float speed_ratio    = getSpeed()/getMaxSpeed();
    float offset_heading = getSteerPercent()*m_kart_properties->getSkidVisual()
                         * speed_ratio * m_skidding*m_skidding;
    Moveable::updateGraphics(center_shift, 
                             btQuaternion(offset_heading, 0, 0));
}   // updateGraphics

/* EOF */
