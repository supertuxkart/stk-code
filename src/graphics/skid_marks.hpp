//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2013-2015 Joerg Henrichs
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

#include <aabbox3d.h>
#include <SMeshBuffer.h>
namespace irr
{
    namespace video { class SMaterial;      }
    namespace scene { class IMeshSceneNode; }
}
using namespace irr;

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

class AbstractKart;

/** \brief This class is responsible for drawing skid marks for a kart.
  * \ingroup graphics
  */
class SkidMarks : public NoCopy
{
private:
    /** Reference to the kart to which these skidmarks belong. */
    const AbstractKart &m_kart;

    /** True if the kart was skidding in the previous frame. */
    bool               m_skid_marking;

    /** Reduce effect of Z-fighting. */
    float              m_width;

    /** Index of current (last added) skid mark quad. */
    int                m_current;

    /** Initial alpha value. */
    static const int   m_start_alpha;

    /** Initial grey value, same for the 3 channels. */
    static const int   m_start_grey;

    /** Material to use for the skid marks. */
    video::SMaterial  *m_material;

    // ------------------------------------------------------------------------
    class SkidMarkQuads : public scene::SMeshBuffer, public NoCopy
    {
        /** Used to move skid marks at the same location slightly on
         *  top of each other to avoid a 'wobbling' effect when sometines
         *  the first and sometimes the 2nd one is drawn on top. */
        float m_z_offset;

        /** Fade out = alpha value. */
        float m_fade_out;

        /** For culling, we need the overall radius of the skid marks. We
         *  approximate this by maintaining an axis-aligned boundary box. */
        core::aabbox3df m_aabb;

        video::SColor   m_start_color;

        /** Vector marking the start of the skidmarks (located between left and right wheel) */
        Vec3 m_center_start;

    public:
            SkidMarkQuads (const Vec3 &left, const Vec3 &right,
                           const Vec3 &normal, video::SMaterial *material,
                           float z_offset, video::SColor* custom_color = NULL);
        void add          (const Vec3 &left,
                           const Vec3 &right,
                           const Vec3 &normal,
                           float distance);
        void fade         (float f);
        /** Returns the aabb of this skid mark quads. */
        const core::aabbox3df &getAABB() { return m_aabb; }
        const Vec3& getCenterStart() const { return m_center_start; }
    };  // SkidMarkQuads

    // ------------------------------------------------------------------------
    /** Two skidmark objects for the left and right wheel. */
    std::vector<SkidMarkQuads *>     m_left, m_right;

    /** The nodes where each left/right pair is attached to. */
    std::vector<scene::IMeshSceneNode *> m_nodes;

    /** Shared static so that consecutive skidmarks are at a slightly
     *  different height. */
    static float                  m_avoid_z_fighting;

public:
         SkidMarks(const AbstractKart& kart, float width=0.32f);
        ~SkidMarks();
    void update (float dt, bool force_skid_marks=false,
                 video::SColor* custom_color = NULL);
    void reset();

    void adjustFog(bool enabled);

};   // SkidMarks

#endif

/* EOF */
