//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include <cinttypes>
#include <memory>

namespace SP
{
    class SPDynamicDrawCall;
}

namespace irr
{
    namespace scene
    {
        class IMeshSceneNode;
    }
}

class AbstractKart;
class Plunger;

/** This class is used together with the pluger to display a rubber band from
 *  the shooting kart to the plunger.
 * \ingroup items
 */
class RubberBand : public NoCopy
{
public:
enum RubberBandTo
{
    RB_TO_PLUNGER = 0, /**< Rubber band is attached to plunger.    */
    RB_TO_KART,        /**< Rubber band is attached to a kart hit. */
    RB_TO_TRACK        /**< Rubber band is attached to track.      */
};
private:
    /** If rubber band is attached to track, the coordinates. */
    Vec3                m_hit_position;
    /** The plunger the rubber band is attached to. */
    Plunger            *m_plunger;
    /** The kart who shot this plunger. */
    AbstractKart       *m_owner;

    RubberBandTo        m_attached_state;

    /** The dynamic draw call of the rubber band. */
    std::shared_ptr<SP::SPDynamicDrawCall> m_dy_dc;

    irr::scene::IMeshSceneNode* m_node;

    /** The kart a plunger might have hit. */
    AbstractKart       *m_hit_kart;
    /** Stores the end of the rubber band (i.e. the side attached to the
     *  plunger. */
    Vec3                m_end_position;

    void checkForHit(const Vec3 &k, const Vec3 &p);
    void updatePosition();

public:
         RubberBand(Plunger *plunger, AbstractKart *kart);
        ~RubberBand();
    void reset();
    void updateGraphics(float dt);
    void update(int ticks);
    void hit(AbstractKart *kart_hit, const Vec3 *track_xyz=NULL);
    uint8_t get8BitState() const;
    void set8BitState(uint8_t bit_state);
    void remove();
};   // RubberBand
#endif
