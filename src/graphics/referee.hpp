//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Joerg Henrichs
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

#ifndef HEADER_REFEREE_HPP
#define HEADER_REFEREE_HPP

#include "irrlicht.h"
using namespace irr;

#include "utils/vec3.hpp"

class Kart;

/**
  * \ingroup graphics
  *  This implements the referee, a character that is displayed at the start
  *  of the race holding a 'ready-set-go' traffic light (or so)
  */
class Referee
{
private:
    /** The static mesh, which is loaded from a static function and shared
     *  between all instances. */
    static scene::IAnimatedMesh *m_st_referee_mesh;

    /** The three textures to use for ready, set, go. */
    static video::ITexture *m_st_traffic_lights[3];

    /** Which mesh buffer to use to show the traffic light texture. */
    static int m_st_traffic_buffer;

    /** Start frame of start animation. */
    static int m_st_first_start_frame;

    /** End frame of start animation. */
    static int m_st_last_start_frame;

    /** Start frame of rescue animation. */
    static int m_st_first_rescue_frame;

    /** End frame of rescue animation. */
    static int m_st_last_rescue_frame;

    /** The position the referee should be shown relative to the kart
     *  when starting the race. */
    static Vec3 m_st_start_offset;

    /** Scaling to be applied to the referee. */
    static Vec3 m_st_scale;

    /** A rotation to be applied to the referee before displaying it. */
    static Vec3 m_st_start_rotation;

    /** The scene node for an instance of the referee. */
    scene::IAnimatedMeshSceneNode *m_scene_node;

public:
                Referee();
               ~Referee();
    void        selectReadySetGo(int rsg);
    void        attachToSceneNode();
    static void init();
    static void cleanup();
    void        removeFromSceneGraph();
    // ------------------------------------------------------------------------
    /** Moves the referee to the specified position. */
    void        setPosition(const Vec3 &xyz) 
                {m_scene_node->setPosition(xyz.toIrrVector()); }
    // ------------------------------------------------------------------------
    /** Returns true if this referee is attached to the scene graph. */
    bool        isAttached() const {return m_scene_node->getParent()!=NULL;}
    // ------------------------------------------------------------------------
    /** Returns the graphical offset the referee should be drawn at at the
     *  start of a race. */
    static const Vec3& getStartOffset() {return m_st_start_offset; }
};   // Referee

#endif

/* EOF */
