//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2013-2015 Joerg Henrichs, Marianne Gagnon
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


#ifndef HEADER_TRACK_OBJECT_PRESENTATION_HPP
#define HEADER_TRACK_OBJECT_PRESENTATION_HPP

#include "graphics/lod_node.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"
#include "utils/log.hpp"
#include "utils/leak_check.hpp"
#include "utils/time.hpp"
#include "utils/vec3.hpp"

#include <vector3d.h>

#include <memory>
#include <limits>
#include <string>

class SFXBase;
class ParticleEmitter;
class PhysicalObject;
class ThreeDAnimation;
class ModelDefinitionLoader;
namespace GE { class GERenderInfo; }
class STKInstancedSceneNode;
class XMLNode;
class TrackObject;

namespace irr
{
    namespace scene { class IAnimatedMesh; class IMesh; class IMeshSceneNode; class ISceneNode; }
}
using namespace irr;

/** \ingroup tracks
 *  Base class for all track object presentation classes.
 */
class TrackObjectPresentation
{
protected:
    /** The initial XYZ position of the object. */
    core::vector3df                m_init_xyz;

    /** The initial hpr of the object. */
    core::vector3df                m_init_hpr;

    /** The initial scale of the object. */
    core::vector3df                m_init_scale;


public:

    TrackObjectPresentation(const XMLNode& xml_node);

    TrackObjectPresentation(const core::vector3df& xyz,
                            const core::vector3df& hpr = core::vector3df(0,0,0),
                            const core::vector3df& scale = core::vector3df(0,0,0))
    {
        m_init_xyz = xyz;
        m_init_hpr = hpr;
        m_init_scale = scale;
    }   // TrackObjectPresentation

    // ------------------------------------------------------------------------
    virtual ~TrackObjectPresentation() {}
    // ------------------------------------------------------------------------

    virtual void reset() {}
    virtual void setEnable(bool enabled)
    {
        Log::warn("TrackObjectPresentation", "setEnable unimplemented for this presentation type");
    }
    virtual void updateGraphics(float dt) {}
    virtual void update(float dt) {}
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
        const core::vector3df& scale, bool isAbsoluteCoord) {}

    // ------------------------------------------------------------------------
    /** Returns the position of this TrackObjectPresentation. */
    virtual const core::vector3df& getPosition() const { return m_init_xyz; }
    // ------------------------------------------------------------------------
    /** Returns a copy of the initial position. Note this function does not
     *  return a const reference, since some classes overwrite it this way. */
    virtual const core::vector3df getAbsolutePosition() const
    { 
        return m_init_xyz;
    }   // getAbsolutePosition
    // ------------------------------------------------------------------------
    virtual const core::vector3df getAbsoluteCenterPosition() const
    {
        return m_init_xyz;
    }
    // ------------------------------------------------------------------------
    /** Returns the initial rotation. */
    virtual const core::vector3df& getRotation() const { return m_init_hpr; }
    // ------------------------------------------------------------------------
    /** Returns the initial scale. */
    virtual const core::vector3df& getScale() const { return m_init_scale; }

    LEAK_CHECK()
};

// ============================================================================
/** \ingroup tracks
 *  Base class for all track object presentation classes using a scene node
 *  as presentation
 */
class TrackObjectPresentationSceneNode : public TrackObjectPresentation
{
protected:
    /** A pointer to the scene node of this object. */
    scene::ISceneNode* m_node;

    bool m_force_always_hidden;
public:

    /** Constructor based on data from xml. */
    TrackObjectPresentationSceneNode(const XMLNode& xml_node) :
        TrackObjectPresentation(xml_node)
    {
        m_node = NULL;
        m_force_always_hidden = false;
    }   // TrackObjectPresentationSceneNode
    // ------------------------------------------------------------------------
    /** Constructor based on a transform. */
    TrackObjectPresentationSceneNode(const core::vector3df& xyz,
                                     const core::vector3df& hpr,
                                     const core::vector3df& scale,
                                     scene::ISceneNode* node = NULL) :
        TrackObjectPresentation(xyz, hpr, scale)
    {
        m_node = node;
        m_force_always_hidden = false;
    }   // TrackObjectPresentationSceneNode

    // ------------------------------------------------------------------------
    virtual const core::vector3df& getPosition() const OVERRIDE;
    virtual const core::vector3df  getAbsolutePosition() const OVERRIDE;
    virtual const core::vector3df getAbsoluteCenterPosition() const OVERRIDE;
    virtual const core::vector3df& getRotation() const OVERRIDE;
    virtual const core::vector3df& getScale() const OVERRIDE;
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
        const core::vector3df& scale, bool isAbsoluteCoord) OVERRIDE;
    virtual void setEnable(bool enabled) OVERRIDE;
    virtual void reset() OVERRIDE;

    // ------------------------------------------------------------------------
    /** Returns a pointer to the scene node. */
    scene::ISceneNode* getNode() { return m_node; }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the scene node, const version. */
    const scene::ISceneNode* getNode() const { return m_node; }
    // ------------------------------------------------------------------------
    bool isAlwaysHidden() const { return m_force_always_hidden; }
};   // class TrackObjectPresentationSceneNode

// ============================================================================
/** \ingroup tracks
 *  A track object representation that is invisible and only consists of a
 *  location, rotation and scale.
 */
class TrackObjectPresentationEmpty : public TrackObjectPresentationSceneNode
{
public:
    TrackObjectPresentationEmpty(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationEmpty();
};   // class TrackObjectPresentationEmpty

// ============================================================================
/** \ingroup tracks
 *  A track object representation that is a library node
 */
class TrackObjectPresentationLibraryNode : public TrackObjectPresentationSceneNode
{
    TrackObject* m_parent;
    using TrackObjectPresentationSceneNode::move;
    std::string m_name;
    bool m_start_executed, m_reset_executed;
public:
    TrackObjectPresentationLibraryNode(TrackObject* parent,
        const XMLNode& xml_node,
        ModelDefinitionLoader& model_def_loader);
    virtual ~TrackObjectPresentationLibraryNode();
    virtual void update(float dt) OVERRIDE;
    virtual void reset() OVERRIDE
    {
        m_reset_executed = false;
        TrackObjectPresentationSceneNode::reset();
    }
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
        const core::vector3df& scale, bool isAbsoluteCoord) OVERRIDE;
};   // TrackObjectPresentationLibraryNode

// ============================================================================
/** \ingroup tracks
 *  A track object representation that consists of a level-of-detail scene node
 */
class TrackObjectPresentationLOD : public TrackObjectPresentationSceneNode
{
public:

    TrackObjectPresentationLOD(const XMLNode& xml_node,
                               scene::ISceneNode* parent,
                               ModelDefinitionLoader& model_def_loader,
                               std::shared_ptr<GE::GERenderInfo> ri);
    virtual ~TrackObjectPresentationLOD();
    virtual void reset() OVERRIDE;
};

// ============================================================================
/** \ingroup tracks
 *  A track object representation that consists of a mesh scene node.
 */
class TrackObjectPresentationMesh : public TrackObjectPresentationSceneNode
{
private:
    /** The mesh used here. It needs to be stored so that it can be
     *  removed from irrlicht's mesh cache when it is deleted. */
    scene::IMesh                  *m_mesh;

    /** True if it is a looped animation. */
    bool                    m_is_looped;

    /** True if the object is in the skybox */
    bool                    m_is_in_skybox;

    std::string             m_model_file;

    std::shared_ptr<GE::GERenderInfo> m_render_info;

    void init(const XMLNode* xml_node, scene::ISceneNode* parent, bool enabled);

public:
    TrackObjectPresentationMesh(const XMLNode& xml_node, bool enabled,
                                scene::ISceneNode* parent,
                                std::shared_ptr<GE::GERenderInfo> render_info);

    TrackObjectPresentationMesh(const std::string& model_file,
                                const core::vector3df& xyz,
                                const core::vector3df& hpr, 
                                const core::vector3df& scale);
    TrackObjectPresentationMesh(scene::IAnimatedMesh* mesh, 
                                const core::vector3df& xyz,
                                const core::vector3df& hpr,
                                const core::vector3df& scale);
    virtual ~TrackObjectPresentationMesh();
    virtual void reset() OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns the mode file name. */
    const std::string& getModelFile() const { return m_model_file; }
};   // class TrackObjectPresentationMesh

// ============================================================================
/** \ingroup tracks
 *  A track object representation that consists of a sound emitter
 */
class TrackObjectPresentationSound : public TrackObjectPresentation
{
private:

    /** If a sound is attached to this object and/or this is a sound emitter
     *  object */
    SFXBase* m_sound;

    /** Currently used for sound effects only, in cutscenes only atm */
    std::string  m_trigger_condition;

    core::vector3df m_xyz;

    bool m_enabled;

public:

    TrackObjectPresentationSound(const XMLNode& xml_node,
                                 scene::ISceneNode* parent,
                                 bool disable_for_multiplayer);
    virtual ~TrackObjectPresentationSound();
    void onTriggerItemApproached(int kart_id);
    virtual void updateGraphics(float dt) OVERRIDE;
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
        const core::vector3df& scale, bool isAbsoluteCoord) OVERRIDE;
    void triggerSound(bool loop);
    void stopSound();

    virtual void setEnable(bool enabled) OVERRIDE;

    // ------------------------------------------------------------------------
    /** Currently used for sound effects only, in cutscenes only atm */
    const std::string& getTriggerCondition() const { return m_trigger_condition; }
};   // TrackObjectPresentationSound

// ============================================================================
/** \ingroup tracks
 *  A track object representation that consists of a billboard scene node.
 */
class TrackObjectPresentationBillboard : public TrackObjectPresentationSceneNode
{
    /** To make the billboard disappear when close to the camera. Useful for
     *  light halos: instead of "colliding" with the camera and suddenly
     *  disappearing when clipped by frustum culling, it will gently fade out.
     */
    bool m_fade_out_when_close;
    float m_fade_out_start;
    float m_fade_out_end;
public:
    TrackObjectPresentationBillboard(const XMLNode& xml_node,
                                     scene::ISceneNode* parent);
    virtual ~TrackObjectPresentationBillboard();
    virtual void updateGraphics(float dt) OVERRIDE;
};   // TrackObjectPresentationBillboard


// ============================================================================
/** \ingroup tracks
 *  A track object representation that consists of a particle emitter
 */
class TrackObjectPresentationParticles : public TrackObjectPresentationSceneNode
{
private:
    ParticleEmitter* m_emitter;
    LODNode* m_lod_emitter_node;
    std::string m_trigger_condition;
    bool m_delayed_stop;
    double m_delayed_stop_time;

public:
    TrackObjectPresentationParticles(const XMLNode& xml_node,
                                     scene::ISceneNode* parent);
    virtual ~TrackObjectPresentationParticles();

    virtual void updateGraphics(float dt) OVERRIDE;
    void triggerParticles();
    void stop();
    void stopIn(double delay);
    void setRate(float rate);
    // ------------------------------------------------------------------------
    /** Returns the trigger condition for this object. */
    std::string& getTriggerCondition() { return m_trigger_condition; }
};   // TrackObjectPresentationParticles

// ============================================================================
/** \ingroup tracks
 *  A track object representation that consists of a light emitter
 */
class TrackObjectPresentationLight : public TrackObjectPresentationSceneNode
{
private:
    video::SColor m_color;
    float m_distance;
    float m_energy;
public:
    TrackObjectPresentationLight(const XMLNode& xml_node,
                                 scene::ISceneNode* parent);
    virtual ~TrackObjectPresentationLight();
    float getEnergy() const { return m_energy; }
    virtual void setEnable(bool enabled) OVERRIDE;
    void setEnergy(float energy);
};   // TrackObjectPresentationLight

// ============================================================================

enum ActionTriggerType
{
    TRIGGER_TYPE_POINT = 0,
    TRIGGER_TYPE_CYLINDER = 1
};

/** \ingroup tracks
 *  A track object representation that consists of an action trigger
 */
class TrackObjectPresentationActionTrigger : public TrackObjectPresentation
{
private:
    /** For action trigger objects */
    std::string m_action, m_library_id, m_triggered_object, m_library_name;

    float m_xml_reenable_timeout;

    uint64_t m_reenable_timeout;

    ActionTriggerType m_type;

public:
    TrackObjectPresentationActionTrigger(const XMLNode& xml_node,
                                         TrackObject* parent);
    TrackObjectPresentationActionTrigger(const core::vector3df& xyz,
                                         const std::string& scriptname,
                                         float distance);

    virtual ~TrackObjectPresentationActionTrigger() {}

    void onTriggerItemApproached(int kart_id);
    // ------------------------------------------------------------------------
    /** Reset the trigger (i.e. sets it to active again). */
    virtual void reset() OVERRIDE 
                             { m_reenable_timeout = StkTime::getMonoTimeMs(); }
    // ------------------------------------------------------------------------
    /** Sets the trigger to be enabled or disabled. getMonoTimeMs is used to
     *  to avoid called update which duplicated in network rewinding. */
    virtual void setEnable(bool status) OVERRIDE
    {
        m_reenable_timeout = status ? StkTime::getMonoTimeMs() :
            std::numeric_limits<uint64_t>::max();
    }
    // ------------------------------------------------------------------------
    void setReenableTimeout(float time)
    {
        m_reenable_timeout =
            StkTime::getMonoTimeMs() + (uint64_t)(time * 1000.0f);
    }
};   // class TrackObjectPresentationActionTrigger


#endif // TRACKOBJECTPRESENTATION_HPP

