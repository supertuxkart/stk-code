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
#include <algorithm> // for min and max

#include <ICameraSceneNode.h>
#include <ISceneManager.h>

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/rain.hpp"
#include "graphics/shadow.hpp"
#include "graphics/skid_marks.hpp"
#include "graphics/slip_stream.hpp"
#include "karts/kart_gfx.hpp"
#include "modes/world.hpp"
#include "io/file_manager.hpp"
#include "items/attachment.hpp"
#include "items/item_manager.hpp"
#include "karts/controller/end_controller.hpp"
#include "karts/kart_animation.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/max_speed.hpp"
#include "karts/skidding.hpp"
#include "modes/linear_world.hpp"
#include "network/race_state.hpp"
#include "network/network_manager.hpp"
#include "physics/btKart.hpp"
#include "physics/btKartRaycast.hpp"
#include "physics/btUprightConstraint.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)  && !defined(__MINGW32__)
   // Disable warning for using 'this' in base member initializer list
#  pragma warning(disable:4355)
#endif

/** The kart constructor. 
 *  \param ident  The identifier for the kart model to use.
 *  \param position The position (or rank) for this kart (between 1 and
 *         number of karts). This is used to determine the start position.
 *  \param is_first_kart   Indicates whether this is the first *player* kart
 *  \param init_transform  The initial position and rotation for this kart.
 */
Kart::Kart (const std::string& ident, unsigned int world_kart_id, 
            int position, const btTransform& init_transform)
     : AbstractKart(ident, world_kart_id, position, init_transform)

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#  pragma warning(1:4355)
#endif
{
    m_kart_animation       = new KartAnimation(this);
    m_max_speed            = new MaxSpeed(this);
    m_terrain_info         = new TerrainInfo();
    m_powerup              = new Powerup(this);
    m_vehicle              = NULL;
    m_initial_position     = position;
    m_race_position        = position;
    m_collected_energy     = 0;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_bubblegum_time       = 0.0f;
    m_invulnerable_time    = 0.0f;
    m_squash_time          = 0.0f;
    m_shadow_enabled       = false;
    m_shadow               = NULL;
    m_collision_particles  = NULL;
    m_slipstream           = NULL;
    m_skidmarks            = NULL;
    m_camera               = NULL;
    m_controller           = NULL;
    m_saved_controller     = NULL;
    m_flying               = false;
    m_rain                 = NULL;
    m_sky_particles_emitter= NULL;
    
    m_view_blocked_by_plunger = 0;

    // Initialize custom sound vector (TODO: add back when properly done)
    // m_custom_sounds.resize(SFXManager::NUM_CUSTOMS);

    // Set position and heading:
    m_reset_transform         = init_transform;
    m_speed                   = 0.0f;
    m_wheel_rotation          = 0;

    m_kart_model->setKart(this);

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

    m_engine_sound  = sfx_manager->createSoundSource(m_kart_properties->getEngineSfxType());
    m_beep_sound    = sfx_manager->createSoundSource( "horn"  );
    m_crash_sound   = sfx_manager->createSoundSource( "crash" );
    m_goo_sound     = sfx_manager->createSoundSource( "goo"   );
    m_skid_sound    = sfx_manager->createSoundSource( "skid"  );
    m_terrain_sound          = NULL;
    m_previous_terrain_sound = NULL;

}   // Kart

// -----------------------------------------------------------------------------
/** This is a second initialisation phase, necessary since in the constructor
 *  virtual functions are not called for any superclasses.
 */
void Kart::init(RaceManager::KartType type, bool is_first_kart)
{
    // In multiplayer mode, sounds are NOT positional
    if (race_manager->getNumLocalPlayers() > 1)
    {
        if (type == RaceManager::KT_PLAYER)
        {
            // players have louder sounds than AIs
            const float factor = std::min(1.0f, race_manager->getNumLocalPlayers()/2.0f);
            m_goo_sound->volume( 1.0f / factor );
            m_skid_sound->volume( 1.0f / factor );
            m_crash_sound->volume( 1.0f / factor );
            m_beep_sound->volume( 1.0f / factor );
        }
        else
        {
            m_goo_sound->volume( 1.0f / race_manager->getNumberOfKarts() );
            m_skid_sound->volume( 1.0f / race_manager->getNumberOfKarts() );
            m_crash_sound->volume( 1.0f / race_manager->getNumberOfKarts() );
            m_beep_sound->volume( 1.0f / race_manager->getNumberOfKarts() );
        }
    }
    
    if(!m_engine_sound)
    {
        fprintf(stdout, "Error: Could not allocate a sfx object for the kart. Further errors may ensue!\n");
    }


    bool animations = true;
    const int anims = UserConfigParams::m_show_steering_animations;
    if (anims == ANIMS_NONE)
    {
        animations = false;
    }
    else if (anims == ANIMS_PLAYERS_ONLY && type != RaceManager::KT_PLAYER)
    {
        animations = false;
    }
    loadData(type, is_first_kart, animations);

    m_kart_gfx = new KartGFX(this);
    m_skidding = new Skidding(this, 
                              m_kart_properties->getSkiddingProperties());

    reset();
}   // init

// ----------------------------------------------------------------------------
/** The destructor frees the memory of this kart, but note that the actual kart
 *  model is still stored in the kart_properties (m_kart_model variable), so 
 *  it is not reloaded).
 */
Kart::~Kart()
{
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
    sfx_manager->deleteSFX(m_beep_sound   );
    delete m_kart_gfx;
    if(m_terrain_sound)          sfx_manager->deleteSFX(m_terrain_sound);
    if(m_previous_terrain_sound) sfx_manager->deleteSFX(m_previous_terrain_sound);
    if(m_collision_particles)    delete m_collision_particles;
    if(m_slipstream)             delete m_slipstream;
    if(m_rain)                   delete m_rain;
    if(m_sky_particles_emitter)  delete m_sky_particles_emitter;
    if(m_attachment)             delete m_attachment;

    delete m_shadow;

    if(m_skidmarks) delete m_skidmarks ;

    // Ghost karts don't have a body
    if(m_body)
    {
        World::getWorld()->getPhysics()->removeKart(this);
        delete m_vehicle;
        delete m_vehicle_raycaster;
        delete m_uprightConstraint;
    }

    for(int i=0; i<m_kart_chassis.getNumChildShapes(); i++)
    {
        delete m_kart_chassis.getChildShape(i);
    }
    delete m_skidding;
    delete m_max_speed;
    delete m_kart_animation;
    delete m_terrain_info;
    delete m_powerup;

    if(m_controller)
        delete m_controller;
    if(m_saved_controller)
        delete m_saved_controller;
    if (m_camera) delete m_camera;
}   // ~Kart

//-----------------------------------------------------------------------------
/** Reset before a new race. It will remove all attachments, and
 *  puts the kart back at its original start position.
 */
void Kart::reset()
{
    if (m_flying)
    {
        m_flying = false;
        stopFlying();
    }
    
    m_kart_animation->reset();
    m_max_speed->reset();
    m_powerup->reset();

    if (m_camera)
    {
        m_camera->reset();
        m_camera->setInitialTransform();
    }
    
    // Reset animations and wheels
    m_kart_model->reset();

    // If the controller was replaced (e.g. replaced by end controller), 
    // restore the original controller. 
    if(m_saved_controller)
    {
        m_controller       = m_saved_controller;
        m_saved_controller = NULL;
    }
    m_kart_model->setAnimation(KartModel::AF_DEFAULT);
    m_view_blocked_by_plunger = 0.0;
    m_attachment->clear();
    m_kart_gfx->reset();
    m_skidding->reset();

    
    if (m_collision_particles) 
        m_collision_particles->setCreationRateAbsolute(0.0f);

    m_race_position        = m_initial_position;
    m_finished_race        = false;
    m_finish_time          = 0.0f;
    m_bubblegum_time       = 0.0f;
    m_invulnerable_time    = 0.0f;
    m_squash_time          = 0.0f;
    m_node->setScale(core::vector3df(1.0f, 1.0f, 1.0f));
    m_collected_energy     = 0;
    m_has_started          = false;
    m_wheel_rotation       = 0;
    m_bounce_back_time     = 0.0f;
    m_time_last_crash      = 0.0f;
    m_speed                = 0.0f;
    m_view_blocked_by_plunger = 0.0f;
    
    if(m_terrain_sound)
    {
        sfx_manager->deleteSFX(m_terrain_sound);
    }
    if(m_previous_terrain_sound)
    {
        sfx_manager->deleteSFX(m_previous_terrain_sound);
    }
    
    m_terrain_sound = NULL;
    m_previous_terrain_sound = NULL;
    
    if(m_engine_sound)
        m_engine_sound->stop();

    m_controls.m_steer     = 0.0f;
    m_controls.m_accel     = 0.0f;
    m_controls.m_brake     = false;
    m_controls.m_nitro     = false;
    m_controls.m_skid      = false;
    m_controls.m_fire      = false;
    m_controls.m_look_back = false;
    m_slipstream->reset();
    if(m_vehicle)
    {
        m_vehicle->reset();
    }

    setTrans(m_reset_transform);

    applyEngineForce (0.0f);

    Moveable::reset();
    if (m_skidmarks)
    {
        m_skidmarks->reset();
        const Track *track = 
            track_manager->getTrack( race_manager->getTrackName() );
        m_skidmarks->adjustFog(track->isFogEnabled() );
    }
    

    m_terrain_info->update(getXYZ());

    // Reset is also called when the kart is created, at which time
    // m_controller is not yet defined, so this has to be tested here.
    if(m_controller)
        m_controller->reset();

    // 3 strikes mode can hide the wheels
    scene::ISceneNode** wheels = getKartModel()->getWheelNodes();
    if(wheels[0]) wheels[0]->setVisible(true);
    if(wheels[1]) wheels[1]->setVisible(true);
    if(wheels[2]) wheels[2]->setVisible(true);
    if(wheels[3]) wheels[3]->setVisible(true);
    
}   // reset

// -----------------------------------------------------------------------------
void Kart::increaseMaxSpeed(unsigned int category, float add_speed,
                            float duration, float fade_out_time)
{
    m_max_speed->increaseMaxSpeed(category, add_speed, duration, fade_out_time);
}   // increaseMaxSpeed

// -----------------------------------------------------------------------------
void Kart::setSlowdown(unsigned int category, float max_speed_fraction, 
                       float fade_in_time)
{
    m_max_speed->setSlowdown(category, max_speed_fraction, fade_in_time);
}   // setSlowdown

// -----------------------------------------------------------------------------
float Kart::getCurrentMaxSpeed() const
{
    return m_max_speed->getCurrentMaxSpeed();
}   // getCurrentMaxSpeed
// -----------------------------------------------------------------------------
float Kart::getSpeedIncreaseTimeLeft(unsigned int category) const
{
    return m_max_speed->getSpeedIncreaseTimeLeft(category);
}   // getSpeedIncreaseTimeLeft
// -----------------------------------------------------------------------------
bool Kart::playingRescueAnimation() const
{
    return m_kart_animation->playingRescueAnimation();
}   // playingRescueAnimation

// -----------------------------------------------------------------------------
bool Kart::playingExplosionAnimation() const
{
    return m_kart_animation->playingExplosionAnimation();
}   // playingExplosionAnimation

// -----------------------------------------------------------------------------
/** Returns the timer for the currently played animation. */
float Kart::getAnimationTimer() const
{
    return m_kart_animation->getAnimationTimer();
}   // getAnimationTimer

// -----------------------------------------------------------------------------
/** Returns true if an emergency animation is being played. */
bool Kart::playingEmergencyAnimation() const
{
    return m_kart_animation->playingAnimation();
}   // playingEmergencyAnimation

// -----------------------------------------------------------------------------
/** Returns the current material the kart is on. */
const Material *Kart::getMaterial() const
{
    return m_terrain_info->getMaterial();
}   // getMaterial

// -----------------------------------------------------------------------------
/** Returns the previous material the kart was one (which might be
 *  the same as getMaterial() ). */
const Material *Kart::getLastMaterial() const
{
    return m_terrain_info->getLastMaterial();
}   // getLastMaterial
// -----------------------------------------------------------------------------
/** Returns the pitch of the terrain depending on the heading. */
float Kart::getTerrainPitch(float heading) const
{
    return m_terrain_info->getTerrainPitch(heading);
}   // getTerrainPitch

// -----------------------------------------------------------------------------
/** Returns the height of the terrain. we're currently above */
float Kart::getHoT() const
{
    return m_terrain_info->getHoT();
}   // getHoT

// ----------------------------------------------------------------------------
/** Sets the powerup this kart has collected.
 *  \param t Type of the powerup.
 *  \param n Number of powerups collected.
 */
void Kart::setPowerup(PowerupManager::PowerupType t, int n)
{
    m_powerup->set(t, n); 
}   // setPowerup

// -----------------------------------------------------------------------------
int Kart::getNumPowerup() const
{
    return m_powerup->getNum();
}   // getNumPowerup

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
/** Sets the position in race this kart has . 
 *  The position in this race for this kart (1<=p<=n)
 */
void Kart::setPosition(int p)    
{
    m_controller->setPosition(p);
    m_race_position = p;
}   // setPosition

// -----------------------------------------------------------------------------

/** Returns a transform that will align an object with the kart: the heading 
 *  and the pitch will be set appropriately. A custom pitch value can be 
 *  specified in order to overwrite the terrain pitch (which would be used
 *  otherwise).
 *  \param custom_pitch Pitch value to overwrite the terrain pitch. A value of
 *         -1 indicates that the custom_pitch value should not be used, instead
 *         the actual pitch of the terrain is to be used.
 */
btTransform Kart::getAlignedTransform(const float custom_pitch)
{
    btTransform trans = getTrans();

    float pitch = (custom_pitch == -1 ? getTerrainPitch(getHeading()) 
                                      : custom_pitch);

    btMatrix3x3 m;
    m.setEulerZYX(pitch, getHeading(), 0.0f);
    trans.setBasis(m);

    return trans;
}   // getAlignedTransform

// ----------------------------------------------------------------------------
/** Creates the physical representation of this kart. Atm it uses the actual
 *  extention of the kart model to determine the size of the collision body.
 */
void Kart::createPhysics()
{
    // First: Create the chassis of the kart
    // -------------------------------------
    float kart_width  = getKartWidth();
    float kart_length = getKartLength();
    float kart_height = getKartHeight();

    // improve physics for tall karts
    if (kart_height > kart_length*0.6f)
    {
        kart_height = kart_length*0.6f;
    }

    btCollisionShape *shape;
    const Vec3 &bevel = m_kart_properties->getBevelFactor();
    if(bevel.getX() || bevel.getY() || bevel.getZ())
    {
        Vec3 orig_factor(1, 1, 1-bevel.getZ());
        Vec3 bevel_factor(1.0f-bevel.getX(),
                          1.0f-bevel.getY(),
                          1.0f               );
        btConvexHullShape *hull = new btConvexHullShape();
        for(int x=-1; x<=1; x+=2)
        {
            for(int y=-1; y<=1; y+=2)
            {
                for(int z=-1; z<=1; z+=2)
                {
                    Vec3 p(x*getKartModel()->getWidth()*0.5f,
                          y*getKartModel()->getHeight()*0.5f,
                          z*getKartModel()->getLength()*0.5f);

                    hull->addPoint(p*orig_factor);
                    hull->addPoint(p*bevel_factor);
                }   // for z
            }   // for y
        }   // for x

        // This especially enables proper drawing of the point cloud
        hull->initializePolyhedralFeatures();
        shape = hull;
    }   // bevel.getX()!=0
    else
    {
        shape = new btBoxShape(btVector3(0.5f*kart_width,
                                         0.5f*kart_height,
                                         0.5f*kart_length));
    }

    btTransform shiftCenterOfGravity;
    shiftCenterOfGravity.setIdentity();
    // Shift center of gravity downwards, so that the kart
    // won't topple over too easy.
    shiftCenterOfGravity.setOrigin(m_kart_properties->getGravityCenterShift());
    m_kart_chassis.addChildShape(shiftCenterOfGravity, shape);

    // Set mass and inertia
    // --------------------
    float mass = m_kart_properties->getMass();

    // Position the chassis
    // --------------------
    btTransform trans;
    trans.setIdentity();
    createBody(mass, trans, &m_kart_chassis, 
               m_kart_properties->getRestitution());
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
        new btKartRaycaster(World::getWorld()->getPhysics()->getPhysicsWorld());
    m_vehicle = new btKart(m_body, m_vehicle_raycaster, this);

    // never deactivate the vehicle
    m_body->setActivationState(DISABLE_DEACTIVATION);

    // Add wheels
    // ----------
    float wheel_radius    = m_kart_properties->getWheelRadius();
    float suspension_rest = m_kart_properties->getSuspensionRest();

    btVector3 wheel_direction(0.0f, -1.0f, 0.0f);
    btVector3 wheel_axle(-1.0f, 0.0f, 0.0f);

    btKart::btVehicleTuning tuning;
    tuning.m_maxSuspensionTravelCm = 
        m_kart_properties->getSuspensionTravelCM();
    tuning.m_maxSuspensionForce    = 
        m_kart_properties->getMaxSuspensionForce();

    for(unsigned int i=0; i<4; i++)
    {
        bool is_front_wheel = i<2;
        btWheelInfo& wheel = m_vehicle->addWheel(
                            m_kart_model->getWheelPhysicsPosition(i),
                            wheel_direction, wheel_axle, suspension_rest,
                            wheel_radius, tuning, is_front_wheel);
        wheel.m_suspensionStiffness      = m_kart_properties->getSuspensionStiffness();
        wheel.m_wheelsDampingRelaxation  = m_kart_properties->getWheelDampingRelaxation();
        wheel.m_wheelsDampingCompression = m_kart_properties->getWheelDampingCompression();
        wheel.m_frictionSlip             = m_kart_properties->getFrictionSlip();
        wheel.m_rollInfluence            = m_kart_properties->getRollInfluence();
        wheel.m_rotation                 = 
            btScalar(float(m_kart_properties->hasRandomWheels()) 
            * (rand() % 360));
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

}   // createPhysics

// ----------------------------------------------------------------------------

void Kart::flyUp()
{
    m_flying = true;
    Moveable::flyUp();
}

void Kart::flyDown()
{
    if (isNearGround())
    {
        stopFlying();
        m_flying = false;
    }
    else
    {
        Moveable::flyDown();
    }
}   // flyUp

// ----------------------------------------------------------------------------
/** Starts the engine sound effect. Called once the track intro phase is over.
 */
void Kart::startEngineSFX()
{
    if(!m_engine_sound)
        return;

    // In multiplayer mode, sounds are NOT positional (because we have 
    // multiple listeners) so the engine sounds of all AIs is constantly 
    // heard. So reduce volume of all sounds.
    if (race_manager->getNumLocalPlayers() > 1)
    {
        const int np = race_manager->getNumLocalPlayers();
        const int nai = race_manager->getNumberOfKarts() - np;

        // player karts twice as loud as AIs toghether
        const float players_volume = (np * 2.0f) / (np*2.0f + np);

        if (m_controller->isPlayerController())
            m_engine_sound->volume( players_volume / np );
        else
            m_engine_sound->volume( (1.0f - players_volume) / nai );
    }

    m_engine_sound->speed(0.6f);
    m_engine_sound->setLoop(true);
    m_engine_sound->play();
}   // startEngineSFX

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
void Kart::updateWeight()
{
    float mass = m_kart_properties->getMass() + m_attachment->weightAdjust();

    btVector3 inertia;
    m_kart_chassis.calculateLocalInertia(mass, inertia);
    m_body->setMassProps(mass, inertia);
}   // updateWeight

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
            m_kart_model->setAnimation(KartModel::AF_WIN_START);
        else 
            m_kart_model->setAnimation(KartModel::AF_LOSE_START);
        
        // Not all karts have a camera
        if (m_camera) m_camera->setMode(Camera::CM_FINAL);
        
        RaceGUIBase* m = World::getWorld()->getRaceGUI();
        if(m)
        {
            m->addMessage((getPosition() == 1 ? _("You won the race!") : _("You finished the race!")) ,
                          this, 2.0f);
        }
    }
    else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {        
        // start end animation
        setController(new EndController(this, m_controller->getPlayer()));
        if(m_race_position<=2)
            m_kart_model->setAnimation(KartModel::AF_WIN_START);
        else if(m_race_position>=0.7f*race_manager->getNumberOfKarts())
            m_kart_model->setAnimation(KartModel::AF_LOSE_START);
            
        // Not all karts have a camera
        if (m_camera) m_camera->setMode(Camera::CM_REVERSE);
        
        RaceGUIBase* m = World::getWorld()->getRaceGUI();
        if(m)
        {
            m->addMessage((getPosition() == 2 ? _("You won the race!") : _("You finished the race!")) ,
                          this, 2.0f);
        }
    }
    else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES)
    {
        setController(new EndController(this, m_controller->getPlayer()));
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
void Kart::collectedItem(Item *item, int add_info)
{
    float old_energy          = m_collected_energy;
    const Item::ItemType type = item->getType();

    switch (type)
    {
    case Item::ITEM_BANANA: 
        m_attachment->hitBanana(item, add_info); 
        break;
    case Item::ITEM_NITRO_SMALL: 
        m_collected_energy += m_kart_properties->getNitroSmallContainer();
        break;
    case Item::ITEM_NITRO_BIG:   
        m_collected_energy += m_kart_properties->getNitroBigContainer();
        break;
    case Item::ITEM_BONUS_BOX  :
        {
            m_powerup->hitBonusBox(*item, add_info);
            break;
        }
    case Item::ITEM_BUBBLEGUM:
        // slow down
        //m_body->setLinearVelocity(m_body->getLinearVelocity()*0.3f);
        m_bubblegum_time = 1.0f;
        m_body->setDamping(0.8f, 0.8f);
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
        race_state->itemCollected(getWorldKartId(), item->getItemId());
    }

    if ( m_collected_energy > MAX_NITRO )
        m_collected_energy = MAX_NITRO;
    m_controller->collectedItem(*item, add_info, old_energy);

}   // collectedItem

//-----------------------------------------------------------------------------
/** Simulates gears by adjusting the force of the engine. It also takes the
 *  effect of the zipper into account.
 */
float Kart::getActualWheelForce()
{
    float time_left = 
        m_max_speed->getSpeedIncreaseTimeLeft(MaxSpeed::MS_INCREASE_ZIPPER);
    float zipper_force = time_left>0.0f ? m_kart_properties->getZipperForce()
                                        : 0.0f;
    const std::vector<float>& gear_ratio=m_kart_properties->getGearSwitchRatio();
    for(unsigned int i=0; i<gear_ratio.size(); i++)
    {
        if(m_speed <= m_kart_properties->getMaxSpeed()*gear_ratio[i])
        {
            return m_kart_properties->getMaxPower()
                  *m_kart_properties->getGearPowerIncrease()[i]
                  +zipper_force;
        }
    }
    return m_kart_properties->getMaxPower()+zipper_force;

}   // getActualWheelForce

//-----------------------------------------------------------------------------
/** The kart is on ground if all 4 wheels touch the ground, and if no special
 *  animation (rescue, explosion etc.) is happening).
 */
bool Kart::isOnGround() const
{
    return ((int)m_vehicle->getNumWheelsOnGround() == m_vehicle->getNumWheels()
          && !playingEmergencyAnimation());
}   // isOnGround

//-----------------------------------------------------------------------------
/** The kart is near the ground, but not necessarily on it (small jumps). This
 *  is used to determine when to switch off the upright constraint, so that
 *  explosions can be more violent, while still
*/
bool Kart::isNearGround() const
{
    if(m_terrain_info->getHoT()==Track::NOHIT)
        return false;
    else
        return ((getXYZ().getY() - m_terrain_info->getHoT()) 
                 < stk_config->m_near_ground);
}   // isNearGround

//-----------------------------------------------------------------------------
/** Returns true if the kart is eliminated. 
 */
bool Kart::isEliminated() const 
{
    return m_kart_animation->isEliminated(); 
}   // isEliminated
// ------------------------------------------------------------------------
void Kart::eliminate(bool remove)
{
    m_kart_animation->eliminate(remove); 
}   // eliminate
// ------------------------------------------------------------------------
/** Starts an explosion animation.
 *  \param pos The coordinates of the explosion.
 *  \param direct_hig True if the kart was hit directly --> maximal impact.
 */
void Kart::explode(const Vec3& pos, bool direct_hit)
{
	m_kart_animation->explode(pos, direct_hit);
}   // explode

//-----------------------------------------------------------------------------
/** Sets the mode of the kart to being rescued, attaches the rescue model
 *  and saves the current pitch and roll (for the rescue animation). It
 *  also removes the kart from the physics world.
 */
void Kart::rescue(bool is_auto_rescue)
{
    m_kart_animation->rescue(is_auto_rescue);
}   // rescue

//-----------------------------------------------------------------------------
/** Starts a 'shooting' animation to a specific point.
 *  \param target The coordinates to fly to
 *  \param speed The speed to use when flying.
 */
void Kart::shootTo(const Vec3 &target, float speed)
{
	m_kart_animation->shootTo(target, speed);
}   // shootTo

//-----------------------------------------------------------------------------
/** Updates the kart in each time step. It updates the physics setting,
 *  particle effects, camera position, etc.
 *  \param dt Time step size.
 */
void Kart::update(float dt)
{
    if (m_kart_animation->isEliminated())
    {
        m_kart_animation->update(dt);
        return;
    }

    if(m_squash_time>0)
    {
        m_squash_time-=dt;
        // If squasing time ends, reset the model
        if(m_squash_time<=0)
        {
            m_node->setScale(core::vector3df(1.0f, 1.0f, 1.0f));
            m_max_speed->setSlowdown(MaxSpeed::MS_DECREASE_SQUASH, 
                                    /*slowdown*/1.0f, /*fade in*/0.0f);
        }
    }
    
    if (m_bubblegum_time > 0.0f)
    {
        m_bubblegum_time -= dt;
        if (m_bubblegum_time <= 0.0f)
        {
            // undo bubblegum effect
            m_body->setDamping(m_kart_properties->getChassisLinearDamping(),
                               m_kart_properties->getChassisAngularDamping() );
        }
    }
    
    // Update the position and other data taken from the physics    
    Moveable::update(dt);

    if(!history->replayHistory())
        m_controller->update(dt);
    if(m_camera)
        m_camera->update(dt);

    // if its view is blocked by plunger, decrease remaining time
    if(m_view_blocked_by_plunger > 0) m_view_blocked_by_plunger -= dt;

    // Decrease remaining invulnerability time
    if(m_invulnerable_time>0) m_invulnerable_time-=dt;

    m_slipstream->update(dt);

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
        m_powerup->use() ;
        m_controls.m_fire = false;
    }

    if (!m_flying)
    {
        // When really on air, free fly, when near ground, try to glide / adjust for landing
        // If zipped, be stable, so ramp+zipper can allow nice jumps without scripting the fly
        if(!isNearGround() && 
            m_max_speed->getSpeedIncreaseTimeLeft(MaxSpeed::MS_INCREASE_ZIPPER)<=0.0f )
            m_uprightConstraint->setLimit(M_PI);
        else
            m_uprightConstraint->setLimit(m_kart_properties->getUprightTolerance());
    }

    
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

    //m_wheel_rotation gives the rotation around the X-axis, and since velocity's
    //timeframe is the delta time, we don't have to multiply it with dt.
    m_wheel_rotation += m_speed*dt / m_kart_properties->getWheelRadius();
    m_wheel_rotation=fmodf(m_wheel_rotation, 2*M_PI);

    m_kart_animation->update(dt);

    m_attachment->update(dt);

    //smoke drawing control point
    if (UserConfigParams::m_graphical_effects)
    {
        if (m_rain)
        {
            m_rain->setPosition( getCamera()->getCameraSceneNode()->getPosition() );
            m_rain->update(dt);
        }
    }  // UserConfigParams::m_graphical_effects
    
    m_kart_gfx->update(dt);
    if (m_collision_particles) m_collision_particles->update(dt);
    
    updatePhysics(dt);

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
    if((fabs(getRoll())>60*DEGREE_TO_RAD && fabs(getSpeed())<3.0f) )
    {
        rescue(/*is_auto_rescue*/true);
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
    m_terrain_info->update(pos_plus_epsilon);
    if(m_body->getBroadphaseHandle())
    {
        m_body->getBroadphaseHandle()->m_collisionFilterGroup = old_group;
    }
    handleMaterialGFX();
    const Material* material=m_terrain_info->getMaterial();
    if (!material)   // kart falling off the track
    {
        // let kart fall a bit before rescuing
        const Vec3 *min, *max;
        World::getWorld()->getTrack()->getAABB(&min, &max);
        if(min->getY() - getXYZ().getY() > 17 && !m_flying)
            rescue();
    }
    else
    {
        handleMaterialSFX(material);
        if     (material->isDriveReset() && isOnGround())
            rescue();
        else if(material->isZipper()     && isOnGround())
        {
            handleZipper(material);
            showZipperFire();
        }
        else
        {
            m_max_speed->setSlowdown(MaxSpeed::MS_DECREASE_TERRAIN,
                                     material->getMaxSpeedFraction(), 
                                     material->getSlowDownTime()     );
#ifdef DEBUG
            if(UserConfigParams::m_material_debug)
            {
                printf("%s\tfraction %f\ttime %f.\n",
                       material->getTexFname().c_str(),
                       material->getMaxSpeedFraction(),
                       material->getSlowDownTime()       );
            }
#endif
        }
    }   // if there is material

    // Check if any item was hit.
    item_manager->checkItemHit(this);
    
    static video::SColor pink(255, 255, 133, 253);
    
    // draw skidmarks if relevant (we force pink skidmarks on when hitting a bubblegum)
    if(m_kart_properties->getSkiddingProperties()->hasSkidmarks())
    {
        m_skidmarks->update(dt,
                            m_bubblegum_time > 0,
                            (m_bubblegum_time > 0 ? &pink : NULL) );
    }
    
    const bool emergency = playingEmergencyAnimation();
    
    if (emergency)
    {
        m_view_blocked_by_plunger = 0.0f;
        if (m_flying)
        {
            stopFlying();
            m_flying = false;
        }
    }
    
    // Remove the shadow if the kart is not on the ground (if a kart
    // is rescued isOnGround might still be true, since the kart rigid
    // body was removed from the physics, but still retain the old
    // values for the raycasts).
    if( (!isOnGround() || emergency) && m_shadow_enabled)
    {
        m_shadow_enabled = false;
        m_shadow->disableShadow();
    }
    if(!m_shadow_enabled && isOnGround() && !emergency)
    {
        m_shadow->enableShadow();
        m_shadow_enabled = true;
    }
}   // update

//-----------------------------------------------------------------------------
/** Show fire to go with a zipper.
 */
void Kart::showZipperFire()
{
    m_kart_gfx->setCreationRateAbsolute(KartGFX::KGFX_ZIPPER, 800.0f);
}

//-----------------------------------------------------------------------------
/** Squashes this kart: it will scale the kart in up direction, and causes
 *  a slowdown while this kart is squashed.
 *  \param time How long the kart will be squashed.
 *  \param slowdown Reduction of max speed.
 */
void Kart::setSquash(float time, float slowdown)
{
    if (isInvulnerable()) return;
    
    if(m_attachment->getType()==Attachment::ATTACH_BOMB)
    {
        explode(getXYZ(), /*direct hit*/true);
        return;
    }
    m_node->setScale(core::vector3df(1.0f, 0.5f, 1.0f));
    m_max_speed->setSlowdown(MaxSpeed::MS_DECREASE_SQUASH, slowdown, 0.1f);
    m_squash_time  = time;
}   // setSquash

//-----------------------------------------------------------------------------
/** Plays any terrain specific sound effect.
 */
void Kart::handleMaterialSFX(const Material *material)
{
    // If a terrain specific sfx is already being played, when a new
    // terrain is entered, an old sfx should be finished (once, not
    // looped anymore of course). The m_terrain_sound is then copied
    // to a m_previous_terrain_sound, for which looping is disabled.
    // In case that three sfx needed to be played (i.e. a previous is
    // playing, a current is playing, and a new terrain with sfx is
    // entered), the oldest (previous) sfx is stopped and deleted.
    if(getLastMaterial()!=material)
    {
        // First stop any previously playing terrain sound
        // and remove it, sp that m_previous_terrain_sound
        // can be used again.
        if(m_previous_terrain_sound)
        {
            sfx_manager->deleteSFX(m_previous_terrain_sound);
        }
        m_previous_terrain_sound = m_terrain_sound;
        if(m_previous_terrain_sound)
            m_previous_terrain_sound->setLoop(false);

        const std::string s = material->getSFXName();
        if (s != "")
        {
            m_terrain_sound = sfx_manager->createSoundSource(s);
            
            // in multiplayer mode, sounds are NOT positional (because we have multiple listeners)
            // so the sounds of all AIs would be constantly heard. So silence AI karts.
            if (race_manager->getNumLocalPlayers() > 1)
            {
                if (!m_controller->isPlayerController())
                {
                    m_terrain_sound->volume( 0.0f );
                }
            }
            
            m_terrain_sound->play();
            m_terrain_sound->setLoop(true);
        }
        else
        {
            m_terrain_sound = NULL;
        }
    }
    if(m_previous_terrain_sound && 
        m_previous_terrain_sound->getStatus()==SFXManager::SFX_STOPPED)
    {
        // We don't modify the position of m_previous_terrain_sound 
        // anymore, so that it keeps on playing at the place where the 
        // kart left the material.
        sfx_manager->deleteSFX(m_previous_terrain_sound);
        m_previous_terrain_sound = NULL;
    }
    if(m_terrain_sound) 
    {
        m_terrain_sound->position(getXYZ());
        material->setSFXSpeed(m_terrain_sound, m_speed);
    }

}   // handleMaterialSFX

//-----------------------------------------------------------------------------
/** Handles material specific sfx, mostly particle effects. Particle 
 *  effects can be triggered by two different situations: either
 *  because a kart drives on top of a terrain with a special effect,
 *  or because the kart is driving or falling under a surface (e.g.
 *  water), and the terrain effect is coming from that surface. Those
 *  effects are exclusive - you either get the effect from the terrain
 *  you are driving on, or the effect from a surface the kart is 
 *  (partially) under. The surface effect is triggered, if either the
 *  kart is falling, or if the surface the kart is driving on has
 *  the 'isBelowSurface' property set.
 */
void Kart::handleMaterialGFX()
{
    const Material *material = getMaterial();
    
    // First test: give the terrain effect, if the kart is
    // on top of a surface (i.e. not falling), actually touching
    // something with the wheels, and the material has not the
    // below surface property set.
    if (material && isOnGround() && !material->isBelowSurface() && 
        !m_kart_animation->playingRescueAnimation()&& 
        UserConfigParams::m_graphical_effects)
    {

        // Get the appropriate particle data depending on
        // wether the kart is skidding or driving.
        const ParticleKind* pk = 
            material->getParticlesWhen(m_skidding->isSkidding()
                                                    ? Material::EMIT_ON_SKID
                                                    : Material::EMIT_ON_DRIVE);
        if(!pk) 
        {
            // Disable potentially running particle effects
            m_kart_gfx->setCreationRateAbsolute(KartGFX::KGFX_TERRAIN, 0);
            return;  // no particle effect, return
        }
        m_kart_gfx->updateTerrain(pk);
        return;
    }

    // Now the kart is either falling, or driving on a terrain which
    // has the 'below surface' flag set. Detect if there is a surface 
    // on top of the kart.
    // --------------------------------------------------------------
    if (m_camera && m_camera->getMode() != Camera::CM_FINAL)
    {
        if (material && material->hasFallingEffect() && !m_flying)
        {
            m_camera->setMode(Camera::CM_FALLING);
        }
        else if (m_camera->getMode() != Camera::CM_NORMAL && 
                 m_camera->getMode() != Camera::CM_REVERSE)
        {
            m_camera->setMode(Camera::CM_NORMAL);
        }
    }   // camera != final camera
    
    if (!UserConfigParams::m_graphical_effects)
        return;

    // Use the middle of the contact points of the two rear wheels
    // as the point from which to cast the ray upwards
    const btWheelInfo::RaycastInfo &ri2 = 
        getVehicle()->getWheelInfo(2).m_raycastInfo;
    const btWheelInfo::RaycastInfo &ri3 = 
        getVehicle()->getWheelInfo(3).m_raycastInfo;
    Vec3 from = (ri2.m_contactPointWS + ri3.m_contactPointWS)*0.5f;
    Vec3 xyz;
    const Material *surface_material;
    if(!m_terrain_info->getSurfaceInfo(from, &xyz, &surface_material))
    {
        m_kart_gfx->setCreationRateAbsolute(KartGFX::KGFX_TERRAIN, 0);
        return;
    }
    const ParticleKind *pk = 
        surface_material->getParticlesWhen(Material::EMIT_ON_DRIVE);

    if(!pk || m_flying || m_kart_animation->playingRescueAnimation())
        return;

    // Now the kart is under a surface, and there is a surface effect
    // --------------------------------------------------------------
    m_kart_gfx->setParticleKind(KartGFX::KGFX_TERRAIN, pk);
    m_kart_gfx->setXYZ(KartGFX::KGFX_TERRAIN, xyz);

    const float distance = xyz.distance2(from);
    float ratio;
    if      (distance < 2.0f) ratio = 1.0f;
    else if (distance < 4.0f) ratio = (4.0f-distance)*0.5f;
    else                      ratio = -1.0f;  // No more particles
    m_kart_gfx->setCreationRateRelative(KartGFX::KGFX_TERRAIN, ratio);

    // Play special sound effects for this terrain
    // -------------------------------------------
    const std::string s = surface_material->getSFXName();
    if (s != "" && !m_kart_animation->playingRescueAnimation()&&
        (m_terrain_sound == NULL || m_terrain_sound->getStatus() == SFXManager::SFX_STOPPED))
    {
        if (m_previous_terrain_sound) sfx_manager->deleteSFX(m_previous_terrain_sound);
        m_previous_terrain_sound = m_terrain_sound;
        if(m_previous_terrain_sound)
            m_previous_terrain_sound->setLoop(false);

        m_terrain_sound = sfx_manager->createSoundSource(s);
        m_terrain_sound->play();
        m_terrain_sound->setLoop(false);
    }

}   // handleMaterialGFX

//-----------------------------------------------------------------------------
/** Sets or reset the camera attached to a kart. In profile mode even AI karts
 *  can have a camera attached.
 *  \params camera The camera to attach to this kart (or NULL if no camera
 *          is to be used
 */
void Kart::setCamera(Camera *camera)
{
    m_camera = camera;
    if(!camera)
        return;

#ifdef DEBUG
    m_camera->getCameraSceneNode()
            ->setName((getIdent() + "'s camera").c_str());
#endif
    
    // Handle camera-specific nodes for now if in multiplayer
    if (m_rain && race_manager->getNumLocalPlayers() > 1)
    {
        m_rain->setCamera( camera->getCameraSceneNode() );
    }
}   // setCamera

//-----------------------------------------------------------------------------
/** Sets zipper time, and apply one time additional speed boost. It can be 
 *  used with a specific material, in which case the zipper parmaters are
 *  taken from this material (parameters that are <0 will be using the
 *  kart-specific values from kart-properties.
 *  \param material If not NULL, will be used to determine the zipper
 *                  parameters, otherwise the defaults from kart properties
 *                  will be used.
 * \param play_sound If true this will cause a sfx to be played even if the
 *                  terrain hasn't changed. It is used by the zipper powerup.
 */
void Kart::handleZipper(const Material *material, bool play_sound)
{
    /** The additional speed allowed on top of the kart-specific maximum kart 
     *  speed. */
    float max_speed_increase;

    /**Time the zipper stays activated. */
    float duration;
    /** A one time additional speed gain - the kart will instantly add this 
     *  amount of speed to its current speed. */
    float speed_gain;
    /** Time it takes for the zipper advantage to fade out. */
    float fade_out_time;

    if(material)
    {
        material->getZipperParameter(&max_speed_increase, &duration, 
                                     &speed_gain, &fade_out_time);
        if(max_speed_increase<0) 
            max_speed_increase = m_kart_properties->getZipperMaxSpeedIncrease();
        if(duration<0)
            duration           = m_kart_properties->getZipperTime();
        if(speed_gain<0)
            speed_gain         = m_kart_properties->getZipperSpeedGain();
        if(fade_out_time<0)
            fade_out_time      = m_kart_properties->getZipperFadeOutTime();
    }
    else
    {
        max_speed_increase = m_kart_properties->getZipperMaxSpeedIncrease();
        duration           = m_kart_properties->getZipperTime();
        speed_gain         = m_kart_properties->getZipperSpeedGain();
        fade_out_time      = m_kart_properties->getZipperFadeOutTime();
    }
    // Ignore a zipper that's activated while braking
    if(m_controls.m_brake || m_speed<0) return;

    m_max_speed->instantSpeedIncrease(MaxSpeed::MS_INCREASE_ZIPPER,
                                     max_speed_increase, speed_gain, 
                                     duration, fade_out_time);
    // Play custom character sound (weee!)
    playCustomSFX(SFXManager::CUSTOM_ZIPPER);
    m_controller->handleZipper(play_sound);
}   // handleZipper

// -----------------------------------------------------------------------------
/** Returned an additional engine power boost when using nitro.
 *  \param dt Time step size.
 */
float Kart::handleNitro(float dt)
{
    if(!m_controls.m_nitro || !isOnGround()) return 0.0;
    m_collected_energy -= dt * m_kart_properties->getNitroConsumption();
    if(m_collected_energy<0)
    {
        m_collected_energy = 0;
        return 0.0;
    }
    m_max_speed->increaseMaxSpeed(MaxSpeed::MS_INCREASE_NITRO,
                               m_kart_properties->getNitroMaxSpeedIncrease(),
                               m_kart_properties->getNitroDuration(), 
                               m_kart_properties->getNitroFadeOutTime()      );
    return m_kart_properties->getNitroPowerBoost() 
         * m_kart_properties->getMaxPower();

}   // handleNitro

// -----------------------------------------------------------------------------
/** Activates a slipstream effect */
void Kart::setSlipstreamEffect(float f)
{
    m_kart_gfx->setCreationRateAbsolute(KartGFX::KGFX_ZIPPER, f);
}   // setSlipstreamEffect

// -----------------------------------------------------------------------------
/** Called when the kart crashes against another kart.
 *  \param k The kart that was hit.
 *  \param update_attachments If true the attachment of this kart and the
 *          other kart hit will be updated (e.g. bombs will be moved)
 */
void Kart::crashed(AbstractKart *k, bool update_attachments)
{
    if(update_attachments)
    {
        assert(k);
        getAttachment()->handleCollisionWithKart(k);
    }
	m_controller->crashed(k);
    crashed();
}   // crashed(Kart, update_attachments

// -----------------------------------------------------------------------------
/** Kart hits the track with a given material.
 *  \param m Material hit, can be NULL if no specific material exists.
 */
void Kart::crashed(const Material *m)
{
#ifdef DEBUG
    // Simple debug output for people playing without sound.
    // This makes it easier to see if a kart hit the track (esp.
    // after a jump).
    // FIXME: This should be removed once the physics are fixed.
    if(UserConfigParams::m_physics_debug)
    {
        // Add a counter to make it easier to see if a new line of
        // output was added.
        static int counter=0;
        printf("Kart %s hit track: %d material %s.\n", 
               getIdent().c_str(), counter++,
               m->getTexFname().c_str());
    }
#endif

    // If there is a quad graph, push the kart towards the previous
    // graph node center (we have to use the previous point since the
    // kart might have only now reached the new quad, meaning the kart
    // would be pushed forward).
    // FIXME: or should we try to add a 'proper' reflection, i.e. an
    // impulse depending on the vector and normal at the hit point?
    const LinearWorld *lw = dynamic_cast<LinearWorld*>(World::getWorld());
    if(lw && m_vehicle->getCentralImpulseTime()<=0)
    {
        int sector = lw->getSectorForKart(this);
        if(sector!=QuadGraph::UNKNOWN_SECTOR)
        {
            const GraphNode &gn = QuadGraph::get()->getNode(
                QuadGraph::get()->getNode(sector).getPredecessor());
            Vec3 impulse = gn.getCenter() - getXYZ();
            impulse.setY(0);
            if(impulse.getX() || impulse.getZ())
                impulse.normalize();
            else
                impulse = Vec3(0, 0, -1); // Arbitrary
            impulse *= m_kart_properties->getCollisionTerrainImpulse();
            m_bounce_back_time = 0.2f;
            m_vehicle->setTimedCentralImpulse(0.1f, impulse);
        }

    }
    /** If a kart is crashing against the track, the collision is often
     *  reported more than once, resulting in a machine gun effect, and too
     *  long disabling of the engine. Therefore, this reaction is disabled
     *  for 0.5 seconds after a crash.
     */
    if(m && m->getCollisionReaction() != Material::NORMAL && 
        !playingEmergencyAnimation())
    {
        std::string particles = m->getCrashResetParticles();
        if (particles.size() > 0)
        {
            ParticleKind* kind = 
                ParticleKindManager::get()->getParticles(particles);
            if (kind != NULL)
            {
                if (m_collision_particles == NULL)
                {
                    Vec3 position(-getKartWidth()*0.35f, 0.06f, 
                                  getKartLength()*0.5f);
                    m_collision_particles  = 
                        new ParticleEmitter(kind, position, getNode());
                }
                else
                {
                    m_collision_particles->setParticleType(kind);
                }
            }
            else
            {
                fprintf(stderr, "Unknown particles kind <%s> in material "
                                "crash-reset properties\n", particles.c_str());
            }
        }
        
        if (m->getCollisionReaction() == Material::RESCUE)
        {
            rescue();
        }
        else if (m->getCollisionReaction() == Material::PUSH_BACK)
        {
            if (m_bounce_back_time <= 0.0f)
            {
                btVector3 push = m_body->getLinearVelocity().normalized();
                push[1] = 0.1f;
                m_body->applyCentralImpulse( -4000.0f*push );
                m_bounce_back_time = 2.0f;
            }
        }
    }
	m_controller->crashed(m);
    crashed();
}   // crashed(Material)

// -----------------------------------------------------------------------------
/** Common code used when a kart or a material was hit.
 */
void Kart::crashed()
{
    if(World::getWorld()->getTime()-m_time_last_crash < 0.5f) return;

    m_time_last_crash = World::getWorld()->getTime();
    // After a collision disable the engine for a short time so that karts
    // can 'bounce back' a bit (without this the engine force will prevent
    // karts from bouncing back, they will instead stuck towards the obstable).
    if(m_bounce_back_time<=0.0f)
    {
        if (m_body->getLinearVelocity().length()> 0.555f)
        {
            // In case that the sfx is longer than 0.5 seconds, only play it if
            // it's not already playing.
            if(m_crash_sound->getStatus() != SFXManager::SFX_PLAYING)
                m_crash_sound->play();
        }
        
        m_bounce_back_time = 0.1f;
    }
}   // crashed

// -----------------------------------------------------------------------------
/** Plays a beep sfx.
 */
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
            //printf("Kart SFX: playing %s for %s.\n", 
            //    sfx_manager->getCustomTagName(type), 
            //    m_kart_properties->getIdent().c_str());
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
    // Check if accel is pressed for the first time. The actual timing
    // is done in getStartupBoost - it returns 0 if the start was actually
    // too slow to qualify for a boost.
    if(!m_has_started && m_controls.m_accel)
    {
        m_has_started = true;
        float f       = m_kart_properties->getStartupBoost();
        m_max_speed->instantSpeedIncrease(MaxSpeed::MS_INCREASE_ZIPPER, 0.9f*f,
                                          f, /*duration*/5.0f,
                                          /*fade_out_time*/5.0f);
    }

    m_bounce_back_time-=dt;

    updateEnginePowerAndBrakes(dt);
    
    // apply flying physics if relevant
    if (m_flying)
        updateFlying();    
    
    m_skidding->update(dt, isOnGround(), m_controls.m_steer, 
                       m_controls.m_skid);

    float steering = getMaxSteerAngle() * m_skidding->getSteeringFraction();
    m_vehicle->setSteeringValue(steering, 0);
    m_vehicle->setSteeringValue(steering, 1);

    updateSliding();

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

    // Cap speed if necessary
    m_max_speed->update(dt);

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
    updateEngineSFX();
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
       ,getHeading()
       );
#endif
    
}   // updatePhysics

//-----------------------------------------------------------------------------
/** Adjust the engine sound effect depending on the speed of the kart.
 */
void Kart::updateEngineSFX()
{
    // when going faster, use higher pitch for engine
    if(!m_engine_sound || !sfx_manager->sfxAllowed())
        return;

    if(isOnGround())
    {
        float max_speed = m_max_speed->getCurrentMaxSpeed();
        // Engine noise is based half in total speed, half in fake gears:
        // With a sawtooth graph like /|/|/| we get 3 even spaced gears,
        // ignoring the gear settings from stk_config, but providing a
        // good enough brrrBRRRbrrrBRRR sound effect. Speed factor makes
        // it a "staired sawtooth", so more acoustically rich.
        float f = m_speed/max_speed;
        // Speed at this stage is not yet capped, so it can be > 1, which 
        // results in odd engine sfx.
        if (f>1.0f) f=1.0f;

        float gears = 3.0f * fmod(f, 0.333334f);
        m_engine_sound->speed(0.6f + (f +gears)* 0.35f);
    }
    else
    {
        // When flying, fixed value but not too high pitch
        // This gives some variation (vs previous "on wheels" one)
        m_engine_sound->speed(0.9f);
    }
    m_engine_sound->position(getXYZ());
}   // updateEngineSFX

//-----------------------------------------------------------------------------
/** Sets the engine power. It considers the engine specs, items that influence
 *  the available power, and braking/steering.
 */
void Kart::updateEnginePowerAndBrakes(float dt)
{
    float engine_power = getActualWheelForce() + handleNitro(dt)
                       + m_slipstream->getSlipstreamPower();

    // apply parachute physics if relevant
    if(m_attachment->getType()==Attachment::ATTACH_PARACHUTE) 
        engine_power*=0.2f;

    // apply bubblegum physics if relevant
    if (m_bubblegum_time > 0.0f)
    {
        engine_power = 0.0f;
        m_body->applyTorque(btVector3(0.0, 500.0f, 0.0));
    }
        
    if(m_controls.m_accel)   // accelerating
    {
        // For a short time after a collision disable the engine,
        // so that the karts can bounce back a bit from the obstacle.
        if(m_bounce_back_time>0.0f)
            engine_power = 0.0f;
        // let a player going backwards accelerate quickly (e.g. if a player 
        // hits a wall, he needs to be able to start again quickly after
        // going backwards)
        else if(m_speed < 0.0f)
            engine_power *= 5.0f;

        // Lose some traction when skidding, to balance the advantage
        if(m_controls.m_skid && 
           m_kart_properties->getSkiddingProperties()->getSkidVisualTime()==0)
            engine_power *= 0.5f;

        applyEngineForce(engine_power*m_controls.m_accel);

        // Either all or no brake is set, so test only one to avoid
        // resetting all brakes most of the time.
        if(m_vehicle->getWheelInfo(0).m_brake &&
            !World::getWorld()->isStartPhase())
            m_vehicle->setAllBrakes(0);
    }
    else
    {   // not accelerating
        if(m_controls.m_brake)
        {   // check if the player is currently only slowing down 
            // or moving backwards
            if(m_speed > 0.0f)
            {   // Still going forward while braking
                applyEngineForce(0.f);

                //apply the brakes
                m_vehicle->setAllBrakes(m_kart_properties->getBrakeFactor());
            }
            else   // m_speed < 0
            {
                m_vehicle->setAllBrakes(0);
                // going backward, apply reverse gear ratio (unless he goes 
                // too fast backwards)
                if ( -m_speed <  m_max_speed->getCurrentMaxSpeed()
                                 *m_kart_properties->getMaxSpeedReverseRatio())
                {
                    // The backwards acceleration is artificially increased to
                    // allow players to get "unstuck" quicker if they hit e.g.
                    // a wall. 
                    applyEngineForce(-engine_power*2.5f);
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
            // we are "parking" the kart, so in battle mode we can ambush people
            if(abs(m_speed) < 5.0f) 
                m_vehicle->setAllBrakes(20.0f);
        }   // !m_brake
    }   // not accelerating
}   // updateEnginePowerAndBrakes

// ----------------------------------------------------------------------------
/** Handles sliding, i.e. the kart sliding off terrain that is too steep.
 */
void Kart::updateSliding()
{
    // dynamically determine friction so that the kart looses its traction 
    // when trying to drive on too steep surfaces. Below angles of 0.25 rad, 
    // you have full traction; above 0.5 rad angles you have absolutely none; 
    // inbetween  there is a linear change in friction
    float friction = 1.0f;
    bool enable_sliding = false;
    
    // This way the current handling of sliding can be disabled 
    // for certain material (e.g. the curve in skyline on which otherwise 
    // karts could not drive).
    // We also had a crash reported here, which was caused by not
    // having a material here - no idea how this could have happened,
    // but this problem is now avoided by testing if there is a material
    if (isOnGround() && 
         (!getMaterial() || !getMaterial()->highTireAdhesion()))
    {
        const btMatrix3x3 &m = m_vehicle->getChassisWorldTransform().getBasis();
        // To get the angle between up=(0,1,0), we have to do:
        // m*(0,1,0) to get the up vector of the kart, then the 
        // scalar product between this and (0,1,0) - which is m[1][1]:
        float distanceFromUp = m[1][1];
        
        if (distanceFromUp < 0.85f)
        {
            friction = 0.0f;
            enable_sliding = true;
        }
        else if (distanceFromUp > 0.9f)
        {
            friction = 1.0f;
        }
        else
        {
            friction = (distanceFromUp - 0.85f) / 0.5f;
            enable_sliding = true;
        }
    }
    
    for (unsigned int i=0; i<4; i++)
    {
        btWheelInfo& wheel = m_vehicle->getWheelInfo(i);
        wheel.m_frictionSlip = friction*m_kart_properties->getFrictionSlip();
    }
    
    m_vehicle->setSliding(enable_sliding);
}   // updateSliding

// ----------------------------------------------------------------------------
/** Adjusts kart translation if the kart is flying (in debug mode).
 */
void Kart::updateFlying()
{
    if (m_controls.m_accel)
    {
        float orientation = getHeading();
        m_body->applyCentralImpulse(btVector3(60.0f*sin(orientation), 0.0, 
                                              60.0f*cos(orientation)));
    }
    if (m_controls.m_steer != 0.0f)
    {
        m_body->applyTorque(btVector3(0.0, m_controls.m_steer * 3500.0f, 0.0));
    }
    if (m_controls.m_brake)
    {
        btVector3 velocity = m_body->getLinearVelocity(); 

        const float x = velocity.x();
        if (x > 0.2f)        velocity.setX(x - 0.2f);
        else if (x < -0.2f)  velocity.setX(x + 0.2f);
        else                 velocity.setX(0);

        const float y = velocity.y();
        if (y > 0.2f)        velocity.setY(y - 0.2f);
        else if (y < -0.2f)  velocity.setY(y + 0.2f);
        else                 velocity.setY(0);

        const float z = velocity.z();
        if (z > 0.2f)        velocity.setZ(z - 0.2f);
        else if (z < -0.2f)  velocity.setZ(z + 0.2f);
        else                 velocity.setZ(0);

        m_body->setLinearVelocity(velocity);

    }   // if brake

    // dampen any roll while flying, makes the kart hard to control
    btVector3 velocity = m_body->getAngularVelocity();  
    velocity.setX(0);
    velocity.setZ(0);
    m_body->setAngularVelocity(velocity);

}   // updateFlying

// ----------------------------------------------------------------------------
/** Attaches the right model, creates the physics and loads all special 
 *  effects (particle systems etc.)
 */
void Kart::loadData(RaceManager::KartType type, bool is_first_kart, 
                    bool is_animated_model)
{

    m_node = m_kart_model->attachModel(is_animated_model);
    
#ifdef DEBUG
    m_node->setName( (getIdent()+"(lod-node)").c_str() );
#endif
    
    // Attachment must be created after attachModel, since only then the
    // scene node will exist (to which the attachment is added). But the
    // attachment is needed in createPhysics (which gets the mass, which
    // is dependent on the attachment).
    m_attachment = new Attachment(this);
    createPhysics();

    // Attach Particle System
    
    Track *track = World::getWorld()->getTrack();
    if (type == RaceManager::KT_PLAYER      && 
        UserConfigParams::m_weather_effects &&
        track->getSkyParticles() != NULL)
    {
        track->getSkyParticles()->setBoxSizeXZ(150.0f, 150.0f);
        
        m_sky_particles_emitter = 
            new ParticleEmitter(track->getSkyParticles(),
                                core::vector3df(0.0f, 40.0f, 100.0f),
                                getNode());
        
        // FIXME: in multiplayer mode, this will result in several instances
        //        of the heightmap being calculated and kept in memory
        m_sky_particles_emitter->addHeightMapAffector(track);
    }
    
    if (UserConfigParams::m_weather_effects && track->getWeatherType() == WEATHER_RAIN &&
        type == RaceManager::KT_PLAYER)
    {
        // camera not yet available at this point
        m_rain = new Rain(NULL, NULL, is_first_kart);
    }
        
    Vec3 position(0, getKartHeight()*0.35f, -getKartLength()*0.35f);
        
    m_slipstream = new SlipStream(this);

    if(m_kart_properties->getSkiddingProperties()->hasSkidmarks())
    {
        m_skidmarks = new SkidMarks(*this);
        m_skidmarks->adjustFog( 
            track_manager->getTrack(race_manager->getTrackName())
                         ->isFogEnabled() );
    }
    
    m_shadow = new Shadow(m_kart_properties->getShadowTexture(),
                          m_node,
                          m_kart_properties->getShadowScale(),
                          m_kart_properties->getShadowXOffset(),
                          m_kart_properties->getShadowYOffset());
    
    World::getWorld()->kartAdded(this, m_node);
}   // loadData

// ----------------------------------------------------------------------------
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
void Kart::updateGraphics(float dt, const Vec3& offset_xyz, 
                          const btQuaternion& rotation)
{
    float wheel_up_axis[4];
    for(unsigned int i=0; i<4; i++)
    {
        // Set the suspension length
        wheel_up_axis[i] = m_default_suspension_length[i]
                         - m_vehicle->getWheelInfo(i).m_raycastInfo.m_suspensionLength;
    }
    m_kart_model->update(m_wheel_rotation, getSteerPercent(), wheel_up_axis);

    Vec3        center_shift  = m_kart_properties->getGravityCenterShift();
    float y = m_vehicle->getWheelInfo(0).m_chassisConnectionPointCS.getY()
            - m_default_suspension_length[0]
            - m_vehicle->getWheelInfo(0).m_wheelsRadius
            - (m_kart_model->getWheelGraphicsRadius(0)
               -m_kart_model->getWheelGraphicsPosition(0).getY() );
    center_shift.setY(y);
    
    if (m_controls.m_nitro && isOnGround() &&  m_collected_energy > 0)
    {
        // fabs(speed) is important, otherwise the negative number will
        // become a huge unsigned number in the particle scene node!
        float f = fabsf(getSpeed())/m_kart_properties->getMaxSpeed();
        // The speed of the kart can be higher (due to powerups) than
        // the normal maximum speed of the kart.
        if(f>1.0f) f = 1.0f;
        m_kart_gfx->setCreationRateRelative(KartGFX::KGFX_NITRO, f);
    }
    else
        m_kart_gfx->setCreationRateAbsolute(KartGFX::KGFX_NITRO, 0);
    m_kart_gfx->resizeBox(KartGFX::KGFX_NITRO, getSpeed(), dt);

    m_kart_gfx->resizeBox(KartGFX::KGFX_ZIPPER, getSpeed(), dt);

    Moveable::updateGraphics(dt, center_shift, 
                             btQuaternion(m_skidding->getVisualSkidRotation(), 
                                          0, 0));
    
    /*
    // cheap wheelie effect
    if (m_zipper_fire && m_zipper_fire->getCreationRate() > 0.0f)
    {
        m_node->updateAbsolutePosition();
        m_kart_model->getWheelNodes()[0]->updateAbsolutePosition();
        float wheel_y = m_kart_model->getWheelNodes()[0]->getAbsolutePosition().Y;

        core::vector3df rot = m_node->getRotation();
        
        float ratio = float(m_zipper_fire->getCreationRate())
                   /float(m_zipper_fire->getParticlesInfo()->getMaxRate());
        
        const float a = (13.4f - ratio*13.0f);
        float dst = -45.0f*sin((a*a)/180.f*M_PI);
        
        rot.X = dst;
        m_node->setRotation(rot);
        
        m_node->updateAbsolutePosition();
        m_kart_model->getWheelNodes()[0]->updateAbsolutePosition();
        float wheel_y_after = m_kart_model->getWheelNodes()[0]->getAbsolutePosition().Y;
        
        m_node->setPosition(m_node->getPosition() + core::vector3df(0,wheel_y_after - wheel_y,0));
    }
     */
}   // updateGraphics

// ----------------------------------------------------------------------------
btQuaternion Kart::getVisualRotation() const
{
    return getRotation() 
         * btQuaternion(m_skidding->getVisualSkidRotation(), 0, 0);
}   // getVisualRotation


/* EOF */
