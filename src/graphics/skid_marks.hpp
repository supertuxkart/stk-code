//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SKID_MARK_HPP
#define HEADER_SKID_MARK_HPP

#include <vector>
#define _WINSOCKAPI_
#include <plib/ssg.h>

#include "utils/vec3.hpp"

class Coord;
class Kart;

/** This class is responsible for drawing skid marks for a kart. */
class SkidMarks
{
private:
                 /** Reference to the kart to which these skidmarks belong. */
    const Kart  &m_kart;
                 /** True if the kart was skidding in the previous frame. */
    bool         m_skid_marking;
                 /** Reduce effect of Z-fighting. */
    float        m_width;
                 /** Index of current (last added) skid mark quad. */
    int          m_current;

                /** Initial alpha value. */
    static const float m_start_alpha;

    class SkidMarkQuads : public ssgVtxTable
    {
        /** Used to move skid marks at the same location slightly on
         *  top of each other to avoid a 'wobbling' effect when sometines
         *  the first and sometimes the 2nd one is drawn on top
         */
        float m_z_offset;
        /** Fade out = alpha value. */
        float m_fade_out;
        /** For culling, we need the overall radius of the skid marks. We
         *  approximate this by maintaining an axis-aligned boundary box. */
        Vec3        m_aabb_min, m_aabb_max;
    public:
            SkidMarkQuads (const Vec3 &left, const Vec3 &right, 
                           ssgSimpleState *state, float z_offset);
        void recalcBSphere();
        void add          (const Vec3 &left,
                           const Vec3 &right);
        void fade         (float f);
    };  // SkidMarkQuads

    /** Two skidmark objects for the left and right wheel. */
    std::vector <SkidMarkQuads *> m_left, m_right;
    /** The state for colour etc. */
    ssgSimpleState               *m_skid_state;
    static float m_avoid_z_fighting;
public:
         SkidMarks(const Kart& kart, float width=0.2f);
        ~SkidMarks();
    void update  (float dt); 
    void reset();
};   // SkidMarks

#endif

/* EOF */
