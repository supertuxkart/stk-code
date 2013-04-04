//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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
#include <IAnimatedMeshSceneNode.h>
namespace irr
{
    namespace scene { class IAnimatedMesh; class ISceneNode; }
}
using namespace irr;

#include "items/item.hpp"
#include "utils/cpp2011.h"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"
#include <string>
#include "graphics/lod_node.hpp"

class XMLNode;
class SFXBase;
class ParticleEmitter;
class PhysicalObject;
class ThreeDAnimation;

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

class TrackObjectPresentationEmpty : public TrackObjectPresentationSceneNode
{
public:

    TrackObjectPresentationEmpty(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationEmpty();
};

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
    
public:
    TrackObjectPresentationMesh(const XMLNode& xml_node, bool enabled);
    virtual ~TrackObjectPresentationMesh();
    
    virtual void reset() OVERRIDE;
    
    
    /** 2-step construction */
    void setNode(scene::ISceneNode* node)
    {
        //assert(m_node == NULL);
        if (m_node != NULL)
        {
            m_node->remove();
        }
        
        m_node = node;
        
        if (m_node->getType() == irr::scene::ESNT_LOD_NODE)
        {
            ((LODNode*)m_node)->setNodesPosition(m_init_xyz);
            ((LODNode*)m_node)->setNodesRotation(m_init_hpr);
            ((LODNode*)m_node)->setNodesScale(m_init_scale);
        }
        else
        {
            m_node->setPosition(m_init_xyz);
            m_node->setRotation(m_init_hpr);
            m_node->setScale(m_init_scale);
        }
    }

};

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


class TrackObjectPresentationParticles : public TrackObjectPresentation
{
private:
    ParticleEmitter* m_emitter;
    LODNode* m_lod_emitter_node;
    std::string m_trigger_condition;
    
public:
    TrackObjectPresentationParticles(const XMLNode& xml_node);
    virtual ~TrackObjectPresentationParticles();
    
    virtual void update(float dt) OVERRIDE;
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
                      const core::vector3df& scale) OVERRIDE;
    std::string& getTriggerCondition() { return m_trigger_condition; }
    
    void triggerParticles();
};

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
    
    PhysicalObject*                m_rigid_body;
    
    ThreeDAnimation*               m_animator;
    
    
public:
                 TrackObject(const XMLNode &xml_node);
                 TrackObject();
                 
                 /*
                 TrackObject(const core::vector3df& pos, const core::vector3df& hpr,
                             const core::vector3df& scale, const std::string& model);
                */
                ~TrackObject();
    virtual void update(float dt);
    virtual void reset();
    /** To finish object constructions. Called after the track model 
     *  is ready. */
    virtual void init() {};
    /** Called when an explosion happens. As a default does nothing, will
     *  e.g. be overwritten by physical objects etc. */
    virtual void handleExplosion(const Vec3& pos, bool directHit) {};
    void         setEnable(bool mode);
    
    const std::string& getLodGroup() const { return m_lod_group; }
    
    const std::string& getType() const { return m_type; }
    
    bool isSoccerBall() const { return m_soccer_ball; }

    const PhysicalObject* getPhysics() const { return m_rigid_body; }
    PhysicalObject* getPhysics() { return m_rigid_body; }

    const core::vector3df getInitXYZ() const { return m_init_xyz; }
    const core::vector3df getInitRotation() const { return m_init_hpr; }
    const core::vector3df getInitScale() const { return m_init_scale; }
    
    void move(const core::vector3df& xyz, const core::vector3df& hpr, const core::vector3df& scale);

    template<typename T>
    T* getPresentation() { return dynamic_cast<T*>(m_presentation); }

    template<typename T>
    const T* getPresentation() const { return dynamic_cast<T*>(m_presentation); }
    
    ThreeDAnimation* getAnimator() { return m_animator; }
    const ThreeDAnimation* getAnimator() const { return m_animator; }
    
    const core::vector3df& getPosition() const;
    const core::vector3df& getRotation() const;
    const core::vector3df& getScale() const;
};   // TrackObject

#endif
