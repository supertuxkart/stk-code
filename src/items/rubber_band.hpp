//  $Id: rubber_band.hpp 2458 2008-11-15 02:12:28Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_RUBBER_BAND_HPP
#define HEADER_RUBBER_BAND_HPP

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class Kart;
class Plunger;

/** This class is used together with the pluger to display a rubber band from
 *  the shooting kart to the plunger.
 * \ingroup items
 */
class RubberBand : public NoCopy
{
private:
    enum {RB_TO_PLUNGER,         /**< Rubber band is attached to plunger.    */
          RB_TO_KART,            /**< Rubber band is attached to a kart hit. */
          RB_TO_TRACK}           /**< Rubber band is attached to track.      */
                        m_attached_state;
    /** True if plunger was fired backwards. */
    bool                m_is_backward; 
    /** If rubber band is attached to track, the coordinates. */
    Vec3                m_hit_position;
    /** The plunger the rubber band is attached to. */
    Plunger            *m_plunger;
    /** The kart who shot this plunger. */
    const Kart         &m_owner;

    /** The scene node for the rubber band. */
    scene::ISceneNode  *m_node;
    /** The mesh of the rubber band. */
    scene::IMesh       *m_mesh;
    /** The mesh buffer containing the actual vertices of the rubber band. */
    scene::IMeshBuffer *m_buffer;

    /** The kart a plunger might have hit. */
    Kart               *m_hit_kart;
    /** Stores the end of the rubber band (i.e. the side attached to the 
     *  plunger. */
    Vec3                m_end_position;

    void checkForHit(const Vec3 &k, const Vec3 &p);
    void updatePosition();

public:
         RubberBand(Plunger *plunger, const Kart &kart);
    void update(float dt);
    void removeFromScene();
    void hit(Kart *kart_hit, const Vec3 *track_xyz=NULL);
};   // RubberBand
#endif
