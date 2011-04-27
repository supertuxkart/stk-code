//  $Id$
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

#include "irrlicht.h"
using namespace irr;

#include "utils/vec3.hpp"


class XMLNode;

/**
  * \ingroup tracks
  */
class TrackObject : public scene::IAnimationEndCallBack
{
//public:
    // The different type of track objects: physical objects, graphical 
    // objects (without a physical representation) - the latter might be
    // eye candy (to reduce work for physics), ...
    //enum TrackObjectType {TO_PHYSICAL, TO_GRAPHICAL};

private:
    /** True if the object is currently being displayed. */
    bool                    m_enabled;

    /** >0 if it is active for a timer based object. */
    float                   m_timer;

    /** True if it is a looped animation. */
    bool                    m_is_looped;

    /** Start frame of the animation to be played. */
    unsigned int            m_frame_start;

    /** End frame of the animation to be played. */
    unsigned int            m_frame_end;

    virtual void OnAnimationEnd(scene::IAnimatedMeshSceneNode* node);

protected:
    /** The irrlicht scene node this object is attached to. */
    scene::ISceneNode             *m_node;

    /** The mesh used here. It needs to be stored so that it can be 
     *  removed from irrlicht's mesh cache when it is deleted. */
    scene::IAnimatedMesh          *m_mesh;

    /** The initial XYZ position of the object. */
    core::vector3df                m_init_xyz;

    /** The initial hpr of the object. */
    core::vector3df                m_init_hpr;

    /** The initial scale of the object. */
    core::vector3df                m_init_scale;

public:
                 TrackObject(const XMLNode &xml_node);
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
};   // TrackObject

#endif
