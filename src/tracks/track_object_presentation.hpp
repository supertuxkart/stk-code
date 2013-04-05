#ifndef HEADER_TRACK_OBJECT_PRESENTATION_HPP
#define HEADER_TRACK_OBJECT_PRESENTATION_HPP

#include <vector3d.h>
#include <IAnimatedMeshSceneNode.h>
namespace irr
{
    namespace scene { class IAnimatedMesh; class ISceneNode; }
}
using namespace irr;

#include "graphics/lod_node.hpp"
#include "items/item.hpp"
#include "utils/cpp2011.h"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"
#include <string>

class XMLNode;
class SFXBase;
class ParticleEmitter;
class PhysicalObject;
class ThreeDAnimation;

/**
 * \ingroup tracks
 * Base class for all track object presentation classes
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
    
    TrackObjectPresentation(
        const core::vector3df& xyz,
        const core::vector3df& hpr,
        const core::vector3df& scale)
    {
        m_init_xyz = xyz;
        m_init_hpr = hpr;
        m_init_scale = scale;
    }
    
    virtual ~TrackObjectPresentation() {}
    
    virtual void reset() {}
    virtual void setEnable(bool enabled) {}
    virtual void update(float dt) {}
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
                      const core::vector3df& scale) {}
    
    virtual const core::vector3df& getPosition() const { return m_init_xyz; }
    virtual const core::vector3df& getRotation() const { return m_init_hpr; }
    virtual const core::vector3df& getScale() const { return m_init_scale; }
    
    LEAK_CHECK()
};

/**
 * \ingroup tracks
 * Base class for all track object presentation classes using a scene node
 * as presentation
 */
class TrackObjectPresentationSceneNode : public TrackObjectPresentation
{
protected:
    scene::ISceneNode* m_node;
public:

    TrackObjectPresentationSceneNode(const XMLNode& xml_node) :
        TrackObjectPresentation(xml_node)
    {
        m_node = NULL;
    }

    TrackObjectPresentationSceneNode(
        const core::vector3df& xyz,
        const core::vector3df& hpr,
        const core::vector3df& scale) :
        TrackObjectPresentation(xyz, hpr, scale)
    {
        m_node = NULL;
    }
    
    virtual const core::vector3df& getPosition() const OVERRIDE;
    virtual const core::vector3df& getRotation() const OVERRIDE;
    virtual const core::vector3df& getScale() const OVERRIDE;
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
                      const core::vector3df& scale) OVERRIDE;
    virtual void setEnable(bool enabled) OVERRIDE;
    virtual void reset() OVERRIDE;
    
    scene::ISceneNode* getNode() { return m_node; }
    const scene::ISceneNode* getNode() const { return m_node; }
};

/**
 * \ingroup tracks
 * A track object representation that is invisible and only consists of a
 * location, rotation and scale.
 */
class TrackObjectPresentationEmpty : public TrackObjectPresentationSceneNode
{
public:

    TrackObjectPresentationEmpty(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationEmpty();
};


/**
 * \ingroup tracks
 * A track object representation that consists of a level-of-detail scene node
 */
class TrackObjectPresentationLOD : public TrackObjectPresentationSceneNode
{
public:

    TrackObjectPresentationLOD(const XMLNode& xml_node, LODNode* lod_node);
    virtual ~TrackObjectPresentationLOD();
};

/**
 * \ingroup tracks
 * A track object representation that consists of a mesh scene node.
 */
class TrackObjectPresentationMesh : public TrackObjectPresentationSceneNode
{
private:
    /** The mesh used here. It needs to be stored so that it can be 
     *  removed from irrlicht's mesh cache when it is deleted. */
    scene::IMesh                  *m_mesh;
    
    /** True if it is a looped animation. */
    bool                    m_is_looped;

    /** Start frame of the animation to be played. */
    unsigned int            m_frame_start;

    /** End frame of the animation to be played. */
    unsigned int            m_frame_end;
    
    void init(const XMLNode* xml_node, bool enabled);
    
public:
    TrackObjectPresentationMesh(const XMLNode& xml_node, bool enabled);
    
    TrackObjectPresentationMesh(
        const std::string& model_file, const core::vector3df& xyz,
        const core::vector3df& hpr, const core::vector3df& scale);

    virtual ~TrackObjectPresentationMesh();
    
    virtual void reset() OVERRIDE;
};

/**
 * \ingroup tracks
 * A track object representation that consists of a sound emitter
 */
class TrackObjectPresentationSound : public TrackObjectPresentation,
                                     public TriggerItemListener
{
private:

    /** If a sound is attached to this object and/or this is a sound emitter object */
    SFXBase* m_sound;

    /** Currently used for sound effects only, in cutscenes only atm */
    std::string  m_trigger_condition;
    
    core::vector3df m_xyz;
    
public:

    TrackObjectPresentationSound(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationSound();
    virtual void onTriggerItemApproached(Item* who) OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    void triggerSound(bool loop);
    void stopSound();
    
    /** Currently used for sound effects only, in cutscenes only atm */
    const std::string& getTriggerCondition() const { return m_trigger_condition; }
    
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
                      const core::vector3df& scale) OVERRIDE;
};

/**
 * \ingroup tracks
 * A track object representation that consists of a billboard scene node.
 */
class TrackObjectPresentationBillboard : public TrackObjectPresentationSceneNode
{
    /** To make the billboard disappear when close to the camera. Useful for light halos :
     *  instead of "colliding" with the camera and suddenly disappearing when clipped by
     *  frustum culling, it will gently fade out.
     */
    bool m_fade_out_when_close;
    float m_fade_out_start;
    float m_fade_out_end;
public:
    TrackObjectPresentationBillboard(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationBillboard();
    virtual void update(float dt) OVERRIDE;
};


/**
 * \ingroup tracks
 * A track object representation that consists of a particle emitter
 */
class TrackObjectPresentationParticles : public TrackObjectPresentationSceneNode
{
private:
    ParticleEmitter* m_emitter;
    LODNode* m_lod_emitter_node;
    std::string m_trigger_condition;
    
public:
    TrackObjectPresentationParticles(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationParticles();
    
    virtual void update(float dt) OVERRIDE;

    std::string& getTriggerCondition() { return m_trigger_condition; }
    
    void triggerParticles();
};

/**
 * \ingroup tracks
 * A track object representation that consists of an action trigger
 */
class TrackObjectPresentationActionTrigger : public TrackObjectPresentation,
                                             public TriggerItemListener
{
private:

    /** For action trigger objects */
    std::string m_action;
    
public:

    
    TrackObjectPresentationActionTrigger(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationActionTrigger() {}
    
    virtual void onTriggerItemApproached(Item* who) OVERRIDE;
};


#endif // TRACKOBJECTPRESENTATION_HPP
