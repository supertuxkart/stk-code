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

#include "items/item.hpp"
#include "physics/physical_object.hpp"
#include "tracks/track_object_presentation.hpp"
#include "utils/cpp2011.h"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"
#include <string>

class XMLNode;
class ThreeDAnimation;


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

    void init(const XMLNode &xml_node, LODNode* lodNode);

public:
                 TrackObject(const XMLNode &xml_node, LODNode* lodNode=NULL);

                 TrackObject(const core::vector3df& xyz,
                             const core::vector3df& hpr,
                             const core::vector3df& scale,
                             const char* interaction,
                             TrackObjectPresentation* presentation,
                             bool is_dynamic,
                             const PhysicalObject::Settings* physicsSettings);
    virtual      ~TrackObject();
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

    void move(const core::vector3df& xyz, const core::vector3df& hpr,
              const core::vector3df& scale, bool updateRigidBody);

    template<typename T>
    T* getPresentation() { return dynamic_cast<T*>(m_presentation); }

    template<typename T>
    const T* getPresentation() const { return dynamic_cast<T*>(m_presentation); }

    ThreeDAnimation* getAnimator() { return m_animator; }
    const ThreeDAnimation* getAnimator() const { return m_animator; }

    const core::vector3df& getPosition() const;
    const core::vector3df& getRotation() const;
    const core::vector3df& getScale() const;

    LEAK_CHECK()
};   // TrackObject

#endif
