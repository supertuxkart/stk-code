//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#ifndef HEADER_TRACK_OBJECT_HPP
#define HEADER_TRACK_OBJECT_HPP

#include <vector3d.h>

#include "items/item.hpp"
#include "physics/physical_object.hpp"
#include "scriptengine/scriptvec3.hpp"
#include "tracks/track_object_presentation.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"
#include <string>
#include "animations/three_d_animation.hpp"

class ModelDefinitionLoader;
class RenderInfo;
class ThreeDAnimation;
class XMLNode;

/**
 * \ingroup tracks
 *  This is a base object for any separate object on the track, which
 *  might also have a skeletal animation. This is used by objects that
 *  have an IPO animation, as well as physical objects.
 */
class TrackObject : public NoCopy
{
//public:
    // The different type of track objects: physical objects, graphical
    // objects (without a physical representation) - the latter might be
    // eye candy (to reduce work for physics), ...
    //enum TrackObjectType {TO_PHYSICAL, TO_GRAPHICAL};

private:
    /** True if the object is currently being displayed. */
    bool                     m_enabled;

    TrackObjectPresentation* m_presentation;

	std::string m_name;

    std::string m_id;

    RenderInfo*              m_render_info;

protected:

    /** The initial XYZ position of the object. */
    core::vector3df                m_init_xyz;

    /** The initial hpr of the object. */
    core::vector3df                m_init_hpr;

    /** The initial scale of the object. */
    core::vector3df                m_init_scale;

    /** LOD group this object is part of, if it is LOD */
    std::string                    m_lod_group;

    std::string                    m_interaction;

    std::string                    m_type;

    bool                           m_soccer_ball;
    
    /** True if a kart can drive on this object. This will */
    bool                           m_is_driveable;

    PhysicalObject*                m_physical_object;

    ThreeDAnimation*               m_animator;

    TrackObject*                   m_parent_library;
    
    std::vector<TrackObject*>      m_movable_children;
    std::vector<TrackObject*>      m_children;

    bool                           m_initially_visible;

    std::string                     m_visibility_condition;

    void init(const XMLNode &xml_node, scene::ISceneNode* parent,
        ModelDefinitionLoader& model_def_loader,
        TrackObject* parent_library);

public:
                 TrackObject(const XMLNode &xml_node,
                             scene::ISceneNode* parent,
                             ModelDefinitionLoader& model_def_loader,
                             TrackObject* parent_library);

                 TrackObject(const core::vector3df& xyz,
                             const core::vector3df& hpr,
                             const core::vector3df& scale,
                             const char* interaction,
                             TrackObjectPresentation* presentation,
                             bool is_dynamic,
                             const PhysicalObject::Settings* physicsSettings);
    virtual      ~TrackObject();
    virtual void update(float dt);
    void move(const core::vector3df& xyz, const core::vector3df& hpr,
              const core::vector3df& scale, bool updateRigidBody,
              bool isAbsoluteCoord);

    virtual void reset();
    const core::vector3df& getPosition() const;
    const core::vector3df  getAbsolutePosition() const;
    const core::vector3df  getAbsoluteCenterPosition() const;
    const core::vector3df& getRotation() const;
    const core::vector3df& getScale() const;
    bool castRay(const btVector3 &from, 
                 const btVector3 &to, btVector3 *hit_point,
                 const Material **material, btVector3 *normal,
                 bool interpolate_normal) const;

    TrackObject* getParentLibrary()
    {
        return m_parent_library;
    }

    // ------------------------------------------------------------------------
    /** To finish object constructions. Called after the track model
     *  is ready. */
    virtual void onWorldReady();
    // ------------------------------------------------------------------------
    /** Called when an explosion happens. As a default does nothing, will
     *  e.g. be overwritten by physical objects etc. */
    virtual void handleExplosion(const Vec3& pos, bool directHit) {};
    void         setID(std::string obj_id) { m_id = obj_id; }

    // ------------------------------------------------------------------------
    const std::string& getLodGroup() const { return m_lod_group; }
    // ------------------------------------------------------------------------
    const std::string& getType() const { return m_type; }
    // ------------------------------------------------------------------------
	const std::string getName() const { return m_name; }
    // ------------------------------------------------------------------------
    const std::string getID() const { return m_id; }
    // ------------------------------------------------------------------------
    const std::string getInteraction() const { return m_interaction; }
    // ------------------------------------------------------------------------
	bool isEnabled() const { return m_enabled; }
    // ------------------------------------------------------------------------
    bool isSoccerBall() const { return m_soccer_ball; }
    // ------------------------------------------------------------------------
    const PhysicalObject* getPhysicalObject() const { return m_physical_object; }
    // ------------------------------------------------------------------------
    PhysicalObject* getPhysicalObject() { return m_physical_object; }
    // ------------------------------------------------------------------------
    const core::vector3df getInitXYZ() const { return m_init_xyz; }
    // ------------------------------------------------------------------------
    const core::vector3df getInitRotation() const { return m_init_hpr; }
    // ------------------------------------------------------------------------
    const core::vector3df getInitScale() const { return m_init_scale; }
    // ------------------------------------------------------------------------
    template<typename T>
    T* getPresentation() { return dynamic_cast<T*>(m_presentation); }
    // ------------------------------------------------------------------------
    template<typename T>
    const T* getPresentation() const { return dynamic_cast<T*>(m_presentation); }
    // ------------------------------------------------------------------------
    // Methods usable by scripts
    /**
    * \addtogroup Scripting
    * @{
    * \addtogroup Scripting_Track Track
    * @{
    * \addtogroup Scripting_TrackObject TrackObject (script binding)
    * @{
    */
    /** Should only be used on mesh track objects.
    * On the script side, the returned object is of type : @ref Scripting_Mesh
    */
    scene::IAnimatedMeshSceneNode* getMesh();
    /** Should only be used on particle emitter track objects.
    * On the script side, the returned object is of type : @ref Scripting_ParticleEmitter
    */
    TrackObjectPresentationParticles* getParticleEmitter() { return getPresentation<TrackObjectPresentationParticles>(); }
    /** Should only be used on sound emitter track objects.
      * On the script side, the returned object is of type : @ref Scripting_SoundEmitter
      */
    TrackObjectPresentationSound* getSoundEmitter(){ return getPresentation<TrackObjectPresentationSound>(); }
    /** Should only be used on sound emitter track objects.
    * On the script side, the returned object is of type : @ref Scripting_Light
    */
    TrackObjectPresentationLight* getLight() { return getPresentation<TrackObjectPresentationLight>(); }
    // For angelscript. Needs to be named something different than getAnimator since it's overloaded.
    /** Should only be used on TrackObjects that use curve-based animation.
      * On the script side, the returned object is of type : @ref Scripting_Animator
      */
    ThreeDAnimation* getIPOAnimator() { return m_animator; }
    // For angelscript. Needs to be named something different than getPhysicalObject since it's overloaded.
    /** Get the physics representation of an object.
      * On the script side, the returned object is of type : @ref Scripting_PhysicalObject
      */
    PhysicalObject* getPhysics() { return m_physical_object; }
    /** Hide or show the object */
    void setEnabled(bool mode);

    void moveTo(const Scripting::SimpleVec3* pos, bool isAbsoluteCoord);
    /* @} */
    /* @} */
    /* @} */

    void resetEnabled();
    // ------------------------------------------------------------------------
    ThreeDAnimation* getAnimator() { return m_animator; }
    // ------------------------------------------------------------------------
    const ThreeDAnimation* getAnimator() const { return m_animator; }
    // ------------------------------------------------------------------------
    void setPaused(bool mode){ m_animator->setPaused(mode); }
    // ------------------------------------------------------------------------
    /** Returns if a kart can drive on this object. */
    bool isDriveable() const { return m_is_driveable; }
    // ------------------------------------------------------------------------
    /** Used along the "extract movable nodes out of library objects" hack, used
      * to still preserve the parent-child relationship
      */
    void addMovableChild(TrackObject* child);
    // ------------------------------------------------------------------------
    void addChild(TrackObject* child);
    // ------------------------------------------------------------------------
    std::vector<TrackObject*>& getMovableChildren() { return m_movable_children; }
    // ------------------------------------------------------------------------
    std::vector<TrackObject*>& getChildren() { return m_children; }
    // ------------------------------------------------------------------------
    void movePhysicalBodyToGraphicalNode(const core::vector3df& xyz, const core::vector3df& hpr);
    LEAK_CHECK()
};   // TrackObject

#endif
