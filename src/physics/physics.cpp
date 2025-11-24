//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "physics/physics.hpp"

#include "animations/three_d_animation.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/user_config.hpp"
#include "karts/kart.hpp"
#include "karts/kart_utils.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stars.hpp"
#include "items/flyable.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "modes/soccer_world.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "physics/btKart.hpp"
#include "physics/irr_debug_drawer.hpp"
#include "physics/physical_object.hpp"
#include "physics/stk_dynamics_world.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "scriptengine/script_engine.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "utils/profiler.hpp"
#include "utils/stk_process.hpp"

#include <IVideoDriver.h>

//=============================================================================
Physics* g_physics[PT_COUNT];
// ----------------------------------------------------------------------------
Physics* Physics::get()
{
    ProcessType type = STKProcess::getType();
    return g_physics[type];
}   // get

// ----------------------------------------------------------------------------
void Physics::create()
{
    ProcessType type = STKProcess::getType();
    g_physics[type] = new Physics();
}   // create

// ----------------------------------------------------------------------------
void Physics::destroy()
{
    ProcessType type = STKProcess::getType();
    delete g_physics[type];
    g_physics[type] = NULL;
}   // destroy

// ----------------------------------------------------------------------------
/** Initialise physics.
 *  Create the bullet dynamics world.
 */
Physics::Physics() : btSequentialImpulseConstraintSolver()
{
    m_collision_conf      = new btDefaultCollisionConfiguration();
    m_dispatcher          = new btCollisionDispatcher(m_collision_conf);
}   // Physics

//-----------------------------------------------------------------------------
/** The actual initialisation of the physics, which is called after the track
 *  model is loaded. This allows the physics to use the actual track dimension
 *  for the axis sweep.
 */
void Physics::init(const Vec3 &world_min, const Vec3 &world_max)
{
    m_physics_loop_active = false;
    m_axis_sweep          = new btAxisSweep3(world_min, world_max);
    m_dynamics_world      = new STKDynamicsWorld(m_dispatcher,
                                                 m_axis_sweep,
                                                 this,
                                                 m_collision_conf);
    m_karts_to_delete.clear();
    m_dynamics_world->setGravity(
        btVector3(0.0f,
                  -Track::getCurrentTrack()->getGravity(),
                  0.0f));
    m_debug_drawer = new IrrDebugDrawer();
    m_dynamics_world->setDebugDrawer(m_debug_drawer);

    // Get the solver settings from the config file
    btContactSolverInfo& info = m_dynamics_world->getSolverInfo();
    info.m_numIterations = stk_config->m_solver_iterations;
    info.m_splitImpulse  = stk_config->m_solver_split_impulse;
    info.m_splitImpulsePenetrationThreshold =
        stk_config->m_solver_split_impulse_thresh;

    // Modify the mode according to the bits of the solver mode:
    info.m_solverMode = (info.m_solverMode & (~stk_config->m_solver_reset_flags))
                      | stk_config->m_solver_set_flags;
}   // init

//-----------------------------------------------------------------------------
Physics::~Physics()
{
    delete m_debug_drawer;
    delete m_dynamics_world;
    delete m_axis_sweep;
    delete m_dispatcher;
    delete m_collision_conf;
}   // ~Physics

// ----------------------------------------------------------------------------
/** Adds a kart to the physics engine.
 *  This adds the rigid body and the vehicle but only if the kart is not
 *  already in the physics world.
 *  \param kart The kart to add.
 *  \param vehicle The raycast vehicle object.
 */
void Physics::addKart(const Kart *kart)
{
    const btCollisionObjectArray &all_objs =
        m_dynamics_world->getCollisionObjectArray();
    for(unsigned int i=0; i<(unsigned int)all_objs.size(); i++)
    {
        if(btRigidBody::upcast(all_objs[i])== kart->getBody())
            return;
    }
    m_dynamics_world->addRigidBody(kart->getBody());
    m_dynamics_world->addVehicle(kart->getVehicle());
}   // addKart

//-----------------------------------------------------------------------------
/** Removes a kart from the physics engine. This is used when rescuing a kart
 *  (and during cleanup).
 *  \param kart The kart to remove.
 */
void Physics::removeKart(const Kart *kart)
{
    // We can't simply remove a kart from the physics world when currently
    // loops over all kart objects are active. This can happen in collision
    // handling, where a collision of a kart with a cake etc. removes
    // a kart from the physics. In this case save pointers to the kart
    // to be removed, and remove them once the physics processing is done.
    if(m_physics_loop_active)
    {
        // Make sure to remove each kart only once.
        if(std::find(m_karts_to_delete.begin(), m_karts_to_delete.end(), kart)
                     == m_karts_to_delete.end())
        {
            m_karts_to_delete.push_back(kart);
        }
    }
    else
    {
        m_dynamics_world->removeRigidBody(kart->getBody());
        m_dynamics_world->removeVehicle(kart->getVehicle());
    }
}   // removeKart

//-----------------------------------------------------------------------------
/** Updates the physics simulation and handles all collisions.
 *  \param ticks Number of physics steps to simulate.
 */
void Physics::update(int ticks)
{
    PROFILER_PUSH_CPU_MARKER("Physics", 0, 0, 0);

    m_physics_loop_active = true;
    // Bullet can report the same collision more than once (up to 4
    // contact points per collision). Additionally, more than one internal
    // substep might be taken, resulting in potentially even more
    // duplicates. To handle this, all collisions (i.e. pair of objects)
    // are stored in a vector, but only one entry per collision pair
    // of objects.
    m_all_collisions.clear();

    // Since the world update (which calls physics update) is called at the
    // fixed frequency necessary for the physics update, we need to do exactly
    // one physic step only.
    double start;
    bool const physics_debug = UserConfigParams::m_physics_debug; // Used to silence a warning
    if (physics_debug) start = StkTime::getRealTime();

    m_dynamics_world->stepSimulation(stk_config->ticks2Time(1), 1,
                                     stk_config->ticks2Time(1)      );
    if (physics_debug)
    {
        Log::verbose("Physics", "At %d physics duration %12.8f",
                     World::getWorld()->getTicksSinceStart(),
                     StkTime::getRealTime() - start);
    }

    // Now handle the actual collision. Note: flyables can not be removed
    // inside of this loop, since the same flyables might hit more than one
    // other object. So only a flag is set in the flyables, the actual
    // clean up is then done later in the projectile manager.
    std::vector<CollisionPair>::iterator p;
    // Child process currently has no scripting engine
    bool is_child = STKProcess::getType() == PT_CHILD;
    for(p=m_all_collisions.begin(); p!=m_all_collisions.end(); ++p)
    {
        // Kart-kart collision
        // --------------------
        if(p->getUserPointer(0)->is(UserPointer::UP_KART))
        {
            KartKartCollision(p->getUserPointer(0)->getPointerKart(),
                              p->getContactPointCS(0),
                              p->getUserPointer(1)->getPointerKart(),
                              p->getContactPointCS(1)                );
            if (!is_child)
            {
                Scripting::ScriptEngine* script_engine =
                                                Scripting::ScriptEngine::getInstance();
                int kartid1 = p->getUserPointer(0)->getPointerKart()->getWorldKartId();
                int kartid2 = p->getUserPointer(1)->getPointerKart()->getWorldKartId();
                script_engine->runFunction(false, "void onKartKartCollision(int, int)",
                    [=](asIScriptContext* ctx) {
                        ctx->SetArgDWord(0, kartid1);
                        ctx->SetArgDWord(1, kartid2);
                    });
            }
            continue;
        }  // if kart-kart collision

        if(p->getUserPointer(0)->is(UserPointer::UP_PHYSICAL_OBJECT))
        {
            // Kart hits physical object
            // -------------------------
            Kart *kart = p->getUserPointer(1)->getPointerKart();
            int kartId = kart->getWorldKartId();
            PhysicalObject* obj = p->getUserPointer(0)->getPointerPhysicalObject();
            std::string obj_id = obj->getID();
            std::string scripting_function = obj->getOnKartCollisionFunction();

            TrackObject* to = obj->getTrackObject();
            TrackObject* library = to->getParentLibrary();
            std::string lib_id;
            std::string* lib_id_ptr = NULL;
            if (library != NULL)
                lib_id = library->getID();
            lib_id_ptr = &lib_id;

            if (!is_child && !scripting_function.empty())
            {
                Scripting::ScriptEngine* script_engine = Scripting::ScriptEngine::getInstance();
                script_engine->runFunction(true, "void " + scripting_function + "(int, const string, const string)",
                    [&](asIScriptContext* ctx) {
                        ctx->SetArgDWord(0, kartId);
                        ctx->SetArgObject(1, lib_id_ptr);
                        ctx->SetArgObject(2, &obj_id);
                    });
            }
            if (obj->isCrashReset())
            {
                kart->applyRescue(/* auto-rescue */ false);
            }
            else if (obj->isExplodeKartObject())
            {
                KartUtils::createExplosion(kart);
            }
            else if (obj->isFlattenKartObject())
            {
                const KartProperties *kp = kart->getKartProperties();
                // Count squash only once from original state
                bool was_squashed = kart->isSquashed();
                if (kart->setSquash(kp->getSwatterSquashDuration(),
                    kp->getSwatterSquashSlowdown()) && !was_squashed)
                {
                    World::getWorld()->kartHit(kart->getWorldKartId());
                }
            }
            else if(obj->isSoccerBall() && 
                    RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
            {
                SoccerWorld* soccerWorld = (SoccerWorld*)World::getWorld();
                soccerWorld->setBallHitter(kartId);
            }
            continue;
        }

        if (p->getUserPointer(0)->is(UserPointer::UP_ANIMATION))
        {
            // Kart hits animation
            ThreeDAnimation *anim=p->getUserPointer(0)->getPointerAnimation();
            if(anim->isCrashReset())
            {
                Kart *kart = p->getUserPointer(1)->getPointerKart();
                kart->applyRescue(/* auto-rescue */ false);
            }
            else if (anim->isExplodeKartObject())
            {
                Kart *kart = p->getUserPointer(1)->getPointerKart();
                KartUtils::createExplosion(kart);
            }
            else if (anim->isFlattenKartObject())
            {
                Kart *kart = p->getUserPointer(1)->getPointerKart();
                const KartProperties *kp = kart->getKartProperties();

                // Count squash only once from original state
                bool was_squashed = kart->isSquashed();
                if (kart->setSquash(kp->getSwatterSquashDuration(),
                    kp->getSwatterSquashSlowdown()) && !was_squashed)
                {
                    World::getWorld()->kartHit(kart->getWorldKartId());
                }

            }
            continue;

        }
        // now the first object must be a projectile
        // =========================================
        if(p->getUserPointer(1)->is(UserPointer::UP_TRACK))
        {
            // Projectile hits track
            // ---------------------
            p->getUserPointer(0)->getPointerFlyable()->hitTrack();
        }
        else if(p->getUserPointer(1)->is(UserPointer::UP_PHYSICAL_OBJECT))
        {
            // Projectile hits physical object
            // -------------------------------
            Flyable* flyable = p->getUserPointer(0)->getPointerFlyable();
            PhysicalObject* obj = p->getUserPointer(1)->getPointerPhysicalObject();
            std::string obj_id = obj->getID();
            std::string scripting_function = obj->getOnItemCollisionFunction();
            if (!is_child && !scripting_function.empty())
            {
                Scripting::ScriptEngine* script_engine = Scripting::ScriptEngine::getInstance();
                script_engine->runFunction(true, "void " + scripting_function + "(int, int, const string)",
                        [&](asIScriptContext* ctx) {
                        ctx->SetArgDWord(0, (int)flyable->getType());
                        ctx->SetArgDWord(1, flyable->getOwnerId());
                        ctx->SetArgObject(2, &obj_id);
                    });
            }
            flyable->hit(NULL, obj);

            if (obj->isSoccerBall() && 
                RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
            {
                int kartId = p->getUserPointer(0)->getPointerFlyable()->getOwnerId();
                SoccerWorld* soccerWorld = (SoccerWorld*)World::getWorld();
                soccerWorld->setBallHitter(kartId);
            }

        }
        else if(p->getUserPointer(1)->is(UserPointer::UP_KART))
        {
            // Projectile hits kart
            // --------------------
            // Only explode a bowling ball if the target is
            // not invulnerable
            Kart* target_kart = p->getUserPointer(1)->getPointerKart();
            PowerupManager::PowerupType type = p->getUserPointer(0)->getPointerFlyable()->getType();
            if(type != PowerupManager::POWERUP_BOWLING || !target_kart->isInvulnerable())
            {
                Flyable *f = p->getUserPointer(0)->getPointerFlyable();
                f->hit(target_kart);

                // Check for achievements
                Kart * kart = World::getWorld()->getKart(f->getOwnerId());
                LocalPlayerController *lpc =
                    dynamic_cast<LocalPlayerController*>(kart->getController());

                // Check that it's not a kart hitting itself (this can
                // happen at the time a flyable is shot - release too close
                // to the kart, and it's the current player. At this stage
                // only the current player can get achievements.
                if (target_kart != kart && lpc && lpc->canGetAchievements())
                {
                    if (type == PowerupManager::POWERUP_BOWLING)
                    {
                        PlayerManager::increaseAchievement(AchievementsStatus::BOWLING_HIT, 1);
                        if (RaceManager::get()->isLinearRaceMode())
                            PlayerManager::increaseAchievement(AchievementsStatus::BOWLING_HIT_1RACE, 1);
                    }   // is bowling ball
                }   // if target_kart != kart && is a player kart and is current player
            }

        }
        else
        {
            // Projectile hits projectile
            // --------------------------
            p->getUserPointer(0)->getPointerFlyable()->hit(NULL);
            p->getUserPointer(1)->getPointerFlyable()->hit(NULL);
        }
    }  // for all p in m_all_collisions

    m_physics_loop_active = false;
    // Now remove the karts that were removed while the above loop
    // was active. Now we can safely call removeKart, since the loop
    // is finished and m_physics_world_active is not set anymore.
    for(unsigned int i=0; i<m_karts_to_delete.size(); i++)
        removeKart(m_karts_to_delete[i]);
    m_karts_to_delete.clear();

    PROFILER_POP_CPU_MARKER();
}   // update

//-----------------------------------------------------------------------------
/** Handles the special case of two karts colliding with each other, which
 *  means that bombs must be passed on. If both karts have a bomb, they'll
 *  explode immediately. This function is called from physics::update() on the
 *  server and if no networking is used, and from race_state on the client to
 *  replay what happened on the server.
 *  \param kart_a First kart involved in the collision.
 *  \param contact_point_a Location of collision at first kart (in kart
 *         coordinates).
 *  \param kart_b Second kart involved in the collision.
 *  \param contact_point_b Location of collision at second kart (in kart
 *         coordinates).
 */
void Physics::KartKartCollision(Kart *kart_a, const Vec3 &contact_point_a,
                                Kart *kart_b, const Vec3 &contact_point_b)
{
    // Only one kart needs to handle the attachments, it will
    // fix the attachments for the other kart.
    kart_a->crashed(kart_b, /*handle_attachments*/true);
    kart_b->crashed(kart_a, /*handle_attachments*/false);

    Kart *left_kart, *right_kart;
    float left_kart_contact_Z, right_kart_contact_Z;

    // Determine which kart is pushed to the left, and which one to the
    // right. Ideally the sign of the X coordinate of the local contact point
    // could decide the direction (negative X --> was hit on left side, gets
    // push to right), but that can lead to both karts being pushed in the
    // same direction (front left of kart hits rear left).
    // So we just use a simple test (which does the right thing in ideal
    // crashes, but avoids pushing both karts in corner cases - pun intended ;) ).

    if(contact_point_a.getX() < contact_point_b.getX())
    {
        left_kart  = kart_b;
        right_kart = kart_a;
        left_kart_contact_Z  = (kart_b->getKartLength()/2 + contact_point_b.getZ()) / kart_b->getKartLength();
        right_kart_contact_Z = (kart_a->getKartLength()/2 + contact_point_a.getZ()) / kart_a->getKartLength();
    }
    else
    {
        left_kart  = kart_a;
        right_kart = kart_b;
        left_kart_contact_Z  = (kart_a->getKartLength()/2 + contact_point_a.getZ()) / kart_a->getKartLength();
        right_kart_contact_Z = (kart_b->getKartLength()/2 + contact_point_b.getZ()) / kart_b->getKartLength();
    }

    // If both karts have an active impulse, we can return immediately
    if(right_kart->getVehicle()->getCentralImpulseTicks() > 0 &&
       left_kart->getVehicle()->getCentralImpulseTicks()  > 0)
        return;

    // Add a scaling factor depending on the mass (avoid div by zero).
    // The value of f_right is applied to the right kart, and f_left
    // to the left kart. f_left = 1 / f_right
    // The max value based on mass with standard settings is 350/210 = 1.6667
    float f_right =  right_kart->getKartProperties()->getMass() > 0
                     ? left_kart->getKartProperties()->getMass()
                       / right_kart->getKartProperties()->getMass()
                     : 4.0f;
    // The impulse being X times stronger for one kart means it will
    // be 1/X times stronger for the other kart, leading to a X² ratio.
    // To keep impulse strength ratio and mass ratio equal, we should
    // take the square root.
    // However, with the current kart classes, that would make differences
    // feel too weak, and changing masses would require a lot of adjustments.
    // We currently take a middle ground approach for gameplay reasons.
    float sqrt_right = sqrtf(f_right);
    f_right = sqrt_right * sqrtf(sqrt_right);
    f_right = std::min(2.0f, std::max(0.5f, f_right));

    // Add a scaling factor depending on speed, making the impulse
    // weaker for the kart going faster and stronger for the kart going slower.
    // Negative speeds (going backwards) count as 0 speed.
    // We compute the speed difference and clamp it to [-21, 21].
    // We then obtain the strength of the effect in the [1, 2] range,
    // and invert the result if the right kart is the faster one.
    // The final result is in the [0.5, 2] range.
    float left_ref_speed  = std::max(0.0f, left_kart->getSpeed());
    float right_ref_speed = std::max(0.0f, right_kart->getSpeed());
    float speed_diff = std::min(21.0f, std::max(-21.0f, (left_ref_speed - right_ref_speed)));
    float speed_impulse_factor = fabsf(speed_diff / 21.0f) + 1.0f;
    if (speed_diff < 0)
        speed_impulse_factor = 1 / speed_impulse_factor;

    f_right *= speed_impulse_factor;

    // Check if a kart is more 'actively' trying to push another kart
    // To do this we check two things:
    // - The position of the contact point on the kart body.
    //   The active kart is touching with the front of the kart
    // - The angle between the two karts. A higher angle indicates
    //   ramming.

    // The "kart_contact_Z" are between 0.0f (kart touching at the very back)
    // and 1.0f (kart touching at the very front)
    // Note that the wheels are usually not at the very front
    bool left_kart_ramming = false, right_kart_ramming = false;
    if (left_kart_contact_Z > 0.7f && 
        left_kart_contact_Z > right_kart_contact_Z + 0.15f)
        left_kart_ramming = true;
    else if (right_kart_contact_Z > 0.7f &&
             right_kart_contact_Z > left_kart_contact_Z + 0.15f)
        right_kart_ramming = true;

    // Kart heading is in the range [-pi, pi]. It depends on the direction
    // the kart is pointing at.
    // We can assume that both karts are more or less in the same plane
    // when they collide, in which case the heading difference indicates
    // the angle at which they collided in this plane
    // We use this difference to compute a "ramming factor"
    float heading_difference = left_kart->getHeading() - right_kart->getHeading();

    // The difference is in the range [-2pi, 2pi]
    // First, we must normalize it
    // The small inaccuracies in approximating pi are irrelevant here
    // Ramming is maximized if one kart has a perpendicular direction to the other
    if (heading_difference < 0.0f)
        heading_difference += 6.2831854f;
    if (heading_difference > 3.1415927f)
        heading_difference -= 3.1415927f;
    if (heading_difference > 1.57079635f)
        heading_difference = 3.1415927f - heading_difference;

    // Now heading difference is in the range [0, pi/2] (approximately)
    // With 0 indicating parallel karts and pi/2 perpendicular karts
    // We set ramming_factor in the [1, 1+pi/2] range
    float ramming_factor = 1.0f + (heading_difference);

    if (left_kart_ramming)
        f_right = f_right * ramming_factor;
    else if (right_kart_ramming)
        f_right = f_right / ramming_factor;

    // Because the speed, mass and ramming factors are
    // each in an interval of the form [1/x; x] (with x > 1),
    // f_left is capped in the same interval as f_right
    float f_left = 1/f_right;

    // Reduce the bouncing if relative velocity is low
    // Relative velocity accounts for both speed and direction
    const Vec3 right_velocity = right_kart->getVelocity();
    const Vec3 left_velocity = left_kart->getVelocity();
    const Vec3 velocity_difference = right_velocity - left_velocity;
    const float velocity_value = velocity_difference.length();

    float impulse_time_factor = 1.0f;

    if (velocity_value < 8.0f) // velocity_value is never negative
    {
        f_right *= 0.375f + velocity_value / 12.8f;
        f_left  *= 0.375f + velocity_value / 12.8f;
        impulse_time_factor = 0.6875f + velocity_value / 25.6f;
    }

    float lean_factor = std::min(1.25f, std::max(f_right, f_left)) * 0.8f;

    // First push one kart to the right (if there is not already
    // an impulse happening - one collision might cause more
    // than one impulse otherwise)
    if(right_kart->getVehicle()->getCentralImpulseTicks()<=0)
        applyKartCollisionImpulse(right_kart, true, f_right, lean_factor, impulse_time_factor);
 
    // Then push the other kart to the left (if there is no impulse happening atm).
    // The force is made negative to obtain the opposite impulse direction.
    if(left_kart->getVehicle()->getCentralImpulseTicks()<=0)
        applyKartCollisionImpulse(left_kart, false, -f_left, lean_factor, impulse_time_factor);
}   // KartKartCollision

//-----------------------------------------------------------------------------
/** This function applies the impulse for a collision between karts
 * */
void Physics::applyKartCollisionImpulse(Kart *kart, bool right, float force,
                                        float lean_factor, float time_factor)
{
    const KartProperties *kp = kart->getKartProperties();
    Vec3 impulse(kp->getCollisionImpulse() * force, 0, 0);
    impulse = kart->getTrans().getBasis() * impulse;
    uint16_t duration = (uint16_t)stk_config->time2Ticks(kp->getCollisionImpulseTime() * time_factor);
    kart->getVehicle()->setTimedCentralImpulse(duration, impulse);
    kart->getVehicle()->setCollisionLean(/* towards the right*/ right);
    kart->getVehicle()->setCollisionLeanFactor(lean_factor);
    // The kart rotation will be prevented as the impulse is applied
}   // applyKartCollisionImpulse

//-----------------------------------------------------------------------------
/** This function is called at each internal bullet timestep. It is used
 *  here to do the collision handling: using the contact manifolds after a
 *  physics time step might miss some collisions (when more than one internal
 *  time step was done, and the collision is added and removed). So this
 *  function stores all collisions in a list, which is then handled after the
 *  actual physics timestep. This list only stores a collision if it's not
 *  already in the list, so a collisions which is reported more than once is
 *  nevertheless only handled once.
 *  The list of collision
 *  Parameters: see bullet documentation for details.
 */
btScalar Physics::solveGroup(btCollisionObject** bodies, int numBodies,
                             btPersistentManifold** manifold,int numManifolds,
                             btTypedConstraint** constraints,
                             int numConstraints,
                             const btContactSolverInfo& info,
                             btIDebugDraw* debugDrawer,
                             btStackAlloc* stackAlloc,
                             btDispatcher* dispatcher)
{
    btScalar returnValue=
        btSequentialImpulseConstraintSolver::solveGroup(bodies, numBodies,
                                                        manifold, numManifolds,
                                                        constraints,
                                                        numConstraints, info,
                                                        debugDrawer,
                                                        stackAlloc,
                                                        dispatcher);
    int currentNumManifolds = m_dispatcher->getNumManifolds();
    // We can't explode a rocket in a loop, since a rocket might collide with
    // more than one object, and/or more than once with each object (if there
    // is more than one collision point). So keep a list of rockets that will
    // be exploded after the collisions
    for(int i=0; i<currentNumManifolds; i++)
    {
        btPersistentManifold* contact_manifold =
            m_dynamics_world->getDispatcher()->getManifoldByIndexInternal(i);

        const btCollisionObject* objA =
            static_cast<const btCollisionObject*>(contact_manifold->getBody0());
        const btCollisionObject* objB =
            static_cast<const btCollisionObject*>(contact_manifold->getBody1());

        unsigned int num_contacts = contact_manifold->getNumContacts();
        if(!num_contacts) continue;   // no real collision

        const UserPointer *upA = (UserPointer*)(objA->getUserPointer());
        const UserPointer *upB = (UserPointer*)(objB->getUserPointer());

        if(!upA || !upB) continue;

        // 1) object A is a track
        // =======================
        if(upA->is(UserPointer::UP_TRACK))
        {
            if(upB->is(UserPointer::UP_FLYABLE))   // 1.1 projectile hits track
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_KART))
            {
                Kart *kart=upB->getPointerKart();
                int n = contact_manifold->getContactPoint(0).m_index0;
                const Material *m
                    = n>=0 ? upA->getPointerTriangleMesh()->getMaterial(n)
                           : NULL;
                // I assume that the normal needs to be flipped in this case,
                // but  I can't verify this since it appears that bullet
                // always has the kart as object A, not B.
                const btVector3 &normal = -contact_manifold->getContactPoint(0)
                                                            .m_normalWorldOnB;
                kart->crashed(m, normal);
            }
            else if(upB->is(UserPointer::UP_PHYSICAL_OBJECT))
            {
                std::vector<int> used;
                for(int i=0; i< contact_manifold->getNumContacts(); i++)
                {
                    int n = contact_manifold->getContactPoint(i).m_index0;
                    // Make sure to call the callback function only once
                    // per triangle.
                    if(std::find(used.begin(), used.end(), n)!=used.end())
                        continue;
                    used.push_back(n);
                    const Material *m
                        = n >= 0 ? upB->getPointerTriangleMesh()->getMaterial(n)
                        : NULL;
                    const btVector3 &normal = contact_manifold->getContactPoint(i)
                        .m_normalWorldOnB;
                    upA->getPointerPhysicalObject()->hit(m, normal);
                }   // for i in getNumContacts()
            }   // upB is physical object
        }   // upA is track
        // 2) object a is a kart
        // =====================
        else if(upA->is(UserPointer::UP_KART))
        {
            if(upB->is(UserPointer::UP_TRACK))
            {
                Kart *kart = upA->getPointerKart();
                int n = contact_manifold->getContactPoint(0).m_index1;
                const Material *m
                    = n>=0 ? upB->getPointerTriangleMesh()->getMaterial(n)
                           : NULL;
                const btVector3 &normal = contact_manifold->getContactPoint(0)
                                                           .m_normalWorldOnB;
                kart->crashed(m, normal);   // Kart hit track
            }
            else if(upB->is(UserPointer::UP_FLYABLE))
                // 2.1 projectile hits kart
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_KART))
                // 2.2 kart hits kart
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
            else if(upB->is(UserPointer::UP_PHYSICAL_OBJECT))
            {
                // 2.3 kart hits physical object
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
                // If the object is a statical object (e.g. a door in
                // overworld) add a push back to avoid that karts get stuck
                if (objB->isStaticObject())
                {
                    Kart *kart = upA->getPointerKart();
                    const btVector3 &normal = contact_manifold->getContactPoint(0)
                        .m_normalWorldOnB;
                    kart->crashed((Material*)NULL, normal);
                }   // isStatiObject
            }
            else if(upB->is(UserPointer::UP_ANIMATION))
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
        }
        // 3) object is a projectile
        // =========================
        else if(upA->is(UserPointer::UP_FLYABLE))
        {
            // 3.1) projectile hits track
            // 3.2) projectile hits projectile
            // 3.3) projectile hits physical object
            // 3.4) projectile hits kart
            if(upB->is(UserPointer::UP_TRACK          ) ||
               upB->is(UserPointer::UP_FLYABLE        ) ||
               upB->is(UserPointer::UP_PHYSICAL_OBJECT) ||
               upB->is(UserPointer::UP_KART           )   )
            {
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
            }
        }
        // Object is a physical object
        // ===========================
        else if(upA->is(UserPointer::UP_PHYSICAL_OBJECT))
        {
            if(upB->is(UserPointer::UP_FLYABLE))
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_KART))
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
            else if(upB->is(UserPointer::UP_TRACK))
            {
                std::vector<int> used;
                for(int i=0; i< contact_manifold->getNumContacts(); i++)
                {
                    int n = contact_manifold->getContactPoint(i).m_index1;
                    // Make sure to call the callback function only once
                    // per triangle.
                    if(std::find(used.begin(), used.end(), n)!=used.end())
                        continue;
                    used.push_back(n);
                    const Material *m
                        = n >= 0 ? upB->getPointerTriangleMesh()->getMaterial(n)
                        : NULL;
                    const btVector3 &normal = contact_manifold->getContactPoint(i)
                                             .m_normalWorldOnB;
                    upA->getPointerPhysicalObject()->hit(m, normal);
                }   // for i in getNumContacts()
            }   // upB is track
        }   // upA is physical object
        else if (upA->is(UserPointer::UP_ANIMATION))
        {
            if(upB->is(UserPointer::UP_KART))
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
        }
        else
            assert("Unknown user pointer");           // 4) Should never happen
    }   // for i<numManifolds

    return returnValue;
}   // solveGroup

// ----------------------------------------------------------------------------
/** A debug draw function to show the track and all karts.
 */
void Physics::draw()
{
    if(!m_debug_drawer->debugEnabled() ||
        !World::getWorld()->isRacePhase()) return;

    video::SColor color(77,179,0,0);
    video::SMaterial material;
    material.Thickness = 2;
    material.AmbientColor = color;
    material.DiffuseColor = color;
    material.EmissiveColor= color;
    material.BackfaceCulling = false;
    material.setFlag(video::EMF_LIGHTING, false);
    irr_driver->getVideoDriver()->setMaterial(material);
    irr_driver->getVideoDriver()->setTransform(video::ETS_WORLD,
                                               core::IdentityMatrix);
    m_dynamics_world->debugDrawWorld();
    return;
}   // draw

// ----------------------------------------------------------------------------

/* EOF */

