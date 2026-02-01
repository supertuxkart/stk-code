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

#ifndef HEADER_PHYSICAL_OBJECT_HPP
#define HEADER_PHYSICAL_OBJECT_HPP

#include <string>

#include "btBulletDynamicsCommon.h"

#include "network/rewinder.hpp"
#include "network/smooth_network_body.hpp"
#include "physics/user_pointer.hpp"
#include "utils/vec3.hpp"

class Material;
class TrackObject;
class XMLNode;

/**
  * \ingroup physics
  */
class PhysicalObject : public Rewinder,
                       public SmoothNetworkBody
{
public:
    /** The supported collision shapes. */
    enum BodyTypes {MP_NONE,
                    MP_CONE_Y, MP_CONE_X, MP_CONE_Z,
                    MP_CYLINDER_Y, MP_CYLINDER_X, MP_CYLINDER_Z,
                    MP_BOX, MP_SPHERE, MP_EXACT};

    class Settings
    {
    public:
        /** ID of the object. */
        std::string               m_id;
        /** Mass of the object. */
        float                     m_mass;
        /** Radius of the object, to overwrite the graphical dimension. */
        float                     m_radius;
        /** Height of an object, to overwrite the graphical dimension. */
        float                     m_height;
        /** Shape of the object. */
        PhysicalObject::BodyTypes m_body_type;
        /** Restitution of the physical object. */
        float                     m_restitution;
        /** Friction for this object. */
        float                     m_friction;
        /** Bullet's linear factor. */
        Vec3                      m_linear_factor;
        /** Bullet angular factor. */
        Vec3                      m_angular_factor;
        /** Bullet's linear damping factor. */
        float                     m_linear_damping;
        /** Bullet's angular damping factor. */
        float                     m_angular_damping;

        /** Trigger a reset in karts touching it? */
        bool                      m_crash_reset;
        /** Knock the kart around. */
        bool                      m_knock_kart;
        /** Flatten the kart when this object is touched. */
        bool                      m_flatten_kart;
        /** Reset the object when it falls under the track (useful
         *  e.g. for a boulder rolling down a hill). */
        bool                      m_reset_when_too_low;
        /** If the item is below that height, it is reset (when
         *  m_reset_when_too_low is true). */
        float                     m_reset_height;
        /** If non-empty, the name of the scripting function to call
          * when a kart collides with this object
          */
        std::string               m_on_kart_collision;
        /** If non-empty, the name of the scripting function to call
        * when a (flyable) item collides with this object
        */
        std::string               m_on_item_collision;
    private:
        void init();
    public:
        Settings(BodyTypes type, float radius, float mass);
        Settings(const XMLNode &xml_node);
    };   // Settings

private:

    /** The initial XYZ position of the object. */
    core::vector3df       m_init_xyz;

    /** The initial hpr of the object. */
    core::vector3df       m_init_hpr;

    /** The initial scale of the object. */
    core::vector3df       m_init_scale;

    TrackObject          *m_object;

    /** The shape of this object. */
    BodyTypes             m_body_type;

    /** The bullet collision shape. */
    btCollisionShape     *m_shape;

    /** ID of the object. */
    std::string           m_id;

    /** The corresponding bullet rigid body. */
    btRigidBody          *m_body;

    /** Bullet's motion state for this object. */
    btDefaultMotionState *m_motion_state;

    /** The mass of this object. */
    float                 m_mass;

    bool                  m_body_added;

    /** The pointer that is stored in the bullet rigid body back to
     *  this object. */
    UserPointer           m_user_pointer;

    /** This is the initial position of the object for the physics. */
    btTransform           m_init_pos;

    /** Save current transform to avoid frequent lookup from 
     * world transform. */
    btTransform           m_current_transform;

    /** The mesh might not have the same center as bullet does. This
     *  offset is used to offset the location of the graphical mesh
     *  so that the graphics are aligned with the bullet collision shape. */
    Vec3                  m_graphical_offset;

    /** Radius of the object - this obviously depends on the actual shape.
     *  As a default the radius is being determined from the shape of the
     *  mesh, but in somce cases that could lead to incorrect results
     *  (if the mesh does not closely resemble a sphere, see init() for
     *  details, but is supposed to be a sphere). In this case the radius
     *  can be set in the scene file. */
    float                 m_radius;

    /** True if a kart colliding with this object should be rescued. */
    bool                  m_crash_reset;

    /** True if kart should "explode" when touching this */
    bool                  m_explode_kart;

    bool                  m_flatten_kart;

    /** True if object should be reset to its initial position if it's
     *  too low (see m_reset_height). */
    bool                  m_reset_when_too_low;

    /** If m_reset_when_too_low this object is set back to its start
     *  position if its height is below this value. */
    float                 m_reset_height;
    /** If non-empty, the name of the scripting function to call
    * when a kart collides with this object
    */
    std::string           m_on_kart_collision;
    /** If non-empty, the name of the scripting function to call
    * when a (flyable) item collides with this object
    */
    std::string           m_on_item_collision;
    /** If this body is a bullet dynamic body, i.e. affected by physics
     *  or not (static (not moving) or kinematic (animated outside
     *  of physics). */
    bool                  m_is_dynamic;

    /** Non-null only if the shape is exact */
    TriangleMesh         *m_triangle_mesh;

    /* Last transform and velocities recieved or saved for networking */
    btTransform           m_last_transform;
    Vec3                  m_last_lv;
    Vec3                  m_last_av;

    /* Used to determine if local state should be used, which is true
     * when the object is not moving */
    bool                  m_no_server_state;

    void copyFromMainProcess(TrackObject* track_obj);
public:
                    PhysicalObject(bool is_dynamic, const Settings& settings,
                                   TrackObject* object);

    static std::shared_ptr<PhysicalObject> fromXML
        (bool is_dynamic, const XMLNode &node, TrackObject* object);

    virtual     ~PhysicalObject ();
    virtual void reset          ();
    virtual void handleExplosion(const Vec3& pos, bool directHit);
    void         update         (float dt);
    void         updateGraphics (float dt);
    void         init           (const Settings &settings);
    void         move           (const Vec3& xyz, const core::vector3df& hpr);
    void         hit            (const Material *m, const Vec3 &normal);
    bool         isSoccerBall   () const;
    bool castRay(const btVector3 &from,
                 const btVector3 &to, btVector3 *hit_point,
                 const Material **material, btVector3 *normal,
                 bool interpolate_normal) const;

    // ------------------------------------------------------------------------
    bool isDynamic() const { return m_is_dynamic; }
    // ------------------------------------------------------------------------
    /** Returns the ID of this physical object. */
    std::string getID()          { return m_id; }
    // ------------------------------------------------------------------------
    btDefaultMotionState* getMotionState() const { return m_motion_state; }
    // ------------------------------------------------------------------------
    /** Returns the rigid body of this physical object. */
    btRigidBody* getBody() const  { return m_body; }
    // ------------------------------------------------------------------------
    /** Returns true if this object should trigger a rescue in a kart that
     *  hits it. */
    bool isCrashReset() const { return m_crash_reset; }
    // ------------------------------------------------------------------------
    /** Returns true if this object should cause an explosion if a kart hits
     *  it. */
    bool isExplodeKartObject () const { return m_explode_kart; }
    // ------------------------------------------------------------------------
    /** Sets the interaction type */
    void setInteraction(std::string interaction);
    // ------------------------------------------------------------------------
    /** Remove body from dynamic world */
    void removeBody();
    // ------------------------------------------------------------------------
    /** Add body to dynamic world */
    void addBody();
    // ------------------------------------------------------------------------
    float getRadius() const { return m_radius; }
    // ------------------------------------------------------------------------
    const std::string& getOnKartCollisionFunction() const { return m_on_kart_collision; }
    // ------------------------------------------------------------------------
    const std::string& getOnItemCollisionFunction() const { return m_on_item_collision; }
    // ------------------------------------------------------------------------
    TrackObject* getTrackObject() { return m_object; }

    // Methods usable by scripts

    /**
    * \addtogroup Scripting
    * @{
    * \addtogroup Scripting_Track Track
    * @{
    * \addtogroup Scripting_PhysicalObject PhysicalObject (script binding)
    * Type returned by trackObject.getPhysicalObject()
    * @{
    */
    /** Returns true if this object should cause a kart that touches it to
    *  be flattened. */
    bool isFlattenKartObject() const { return m_flatten_kart; }
    void disable(/** \cond DOXYGEN_IGNORE */void *memory/** \endcond */)
    {
        ((PhysicalObject*)(memory))->removeBody();
    }

    //enables track object passed from the script
    void enable(/** \cond DOXYGEN_IGNORE */void *memory/** \endcond */)
    {
        ((PhysicalObject*)(memory))->addBody();
    }
    /** @} */
    /** @} */
    /** @} */

    void addForRewind();
    virtual void saveTransform();
    virtual void computeError();
    virtual BareNetworkString* saveState(std::vector<std::string>* ru);
    virtual void undoEvent(BareNetworkString *buffer) {}
    virtual void rewindToEvent(BareNetworkString *buffer) {}
    virtual void restoreState(BareNetworkString *buffer, int count);
    virtual void undoState(BareNetworkString *buffer) {}
    virtual std::function<void()> getLocalStateRestoreFunction();
    bool hasTriangleMesh() const { return m_triangle_mesh != NULL; }
    void joinToMainTrack();
    std::shared_ptr<PhysicalObject> clone(TrackObject* track_obj)
    {
        PhysicalObject* obj = new PhysicalObject(*this);
        obj->copyFromMainProcess(track_obj);
        return std::shared_ptr<PhysicalObject>(obj);
    }
};  // PhysicalObject

#endif
/* EOF */

