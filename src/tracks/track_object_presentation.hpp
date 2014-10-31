//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2013-2013 Joerg Henrichs, Marianne Gagnon
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

#include <vector3d.h>
#include <IAnimatedMeshSceneNode.h>
namespace irr
{
    namespace scene { class IAnimatedMesh; class IMeshSceneNode; class ISceneNode; }
}
using namespace irr;

#include "graphics/lod_node.hpp"
#include "items/item.hpp"
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"
#include <string>

class XMLNode;
class SFXBase;
class ParticleEmitter;
class PhysicalObject;
class ThreeDAnimation;
class ModelDefinitionLoader;
class STKInstancedSceneNode;

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
    TrackObjectPresentation(const core::vector3df& xyz)
    {
        m_init_xyz = xyz;
    }

    virtual ~TrackObjectPresentation() {}

    virtual void reset() {}
    virtual void setEnable(bool enabled) {}
    virtual void update(float dt) {}
    virtual void move(const core::vector3df& xyz, const core::vector3df& hpr,
                      const core::vector3df& scale) {}

    virtual const core::vector3df& getPosition() const { return m_init_xyz; }
    virtual const core::vector3df  getAbsolutePosition() const { return m_init_xyz; }
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

    TrackObjectPresentationSceneNode(
        scene::ISceneNode* node,
        const core::vector3df& xyz,
        const core::vector3df& hpr,
        const core::vector3df& scale) :
        TrackObjectPresentation(xyz, hpr, scale)
    {
        m_node = node;
    }

    virtual const core::vector3df& getPosition() const OVERRIDE;
    virtual const core::vector3df  getAbsolutePosition() const OVERRIDE;
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
* A track object representation that is a library node
*/
class TrackObjectPresentationLibraryNode : public TrackObjectPresentationSceneNode
{
public:

    TrackObjectPresentationLibraryNode(const XMLNode& xml_node,
        ModelDefinitionLoader& model_def_loader);
    virtual ~TrackObjectPresentationLibraryNode();
};

/**
 * \ingroup tracks
 * A track object representation that consists of a level-of-detail scene node
 */
class TrackObjectPresentationLOD : public TrackObjectPresentationSceneNode
{
public:

    TrackObjectPresentationLOD(const XMLNode& xml_node,
                               scene::ISceneNode* parent,
                               ModelDefinitionLoader& model_def_loader);
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

    /** True if the object is in the skybox */
    bool                    m_is_in_skybox;

    /** Start frame of the animation to be played. */
    unsigned int            m_frame_start;

    /** End frame of the animation to be played. */
    unsigned int            m_frame_end;

    std::string             m_model_file;

    void init(const XMLNode* xml_node, scene::ISceneNode* parent, bool enabled);

public:
    TrackObjectPresentationMesh(const XMLNode& xml_node, bool enabled, scene::ISceneNode* parent);

    TrackObjectPresentationMesh(
        const std::string& model_file, const core::vector3df& xyz,
        const core::vector3df& hpr, const core::vector3df& scale);
    TrackObjectPresentationMesh(
        scene::IAnimatedMesh* mesh, const core::vector3df& xyz,
        const core::vector3df& hpr, const core::vector3df& scale);

    void setLoop(int start, int end); //set custom loops, as well as pause by scripts

    void setCurrentFrame(int frame);

    int getCurrentFrame();

    virtual ~TrackObjectPresentationMesh();

    virtual void reset() OVERRIDE;

    const std::string& getModelFile() const { return m_model_file; }
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

    TrackObjectPresentationSound(const XMLNode& xml_node, scene::ISceneNode* parent);
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
    TrackObjectPresentationBillboard(const XMLNode& xml_node, scene::ISceneNode* parent);
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
    TrackObjectPresentationParticles(const XMLNode& xml_node, scene::ISceneNode* parent);
    virtual ~TrackObjectPresentationParticles();

    virtual void update(float dt) OVERRIDE;

    std::string& getTriggerCondition() { return m_trigger_condition; }

    void triggerParticles();
};

/**
* \ingroup tracks
* A track object representation that consists of a light emitter
*/
class TrackObjectPresentationLight : public TrackObjectPresentationSceneNode
{
private:
    video::SColor m_color;
    float m_distance;
    float m_energy;

public:
    TrackObjectPresentationLight(const XMLNode& xml_node, scene::ISceneNode* parent);
    virtual ~TrackObjectPresentationLight();
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

    bool m_action_active;


public:


    TrackObjectPresentationActionTrigger(const XMLNode& xml_node);
    TrackObjectPresentationActionTrigger(const core::vector3df& xyz,std::string scriptname, float distance);

    virtual ~TrackObjectPresentationActionTrigger() {}

    virtual void onTriggerItemApproached(Item* who) OVERRIDE;

    virtual void reset() OVERRIDE { m_action_active = true; }

    virtual void setEnable(bool status) OVERRIDE{ m_action_active = status; }
};


#endif // TRACKOBJECTPRESENTATION_HPP
