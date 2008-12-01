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

#include <plib/ssg.h>

class Coord;

class SkidMark
{
private:
    float m_angle_sign;
public:
    SkidMark(float angle_sign);
    ~SkidMark();
    void add           (const Coord& coord,  // Add a position where the skidmark is
                        float angle,
                        float length); 
    void addBreak      (const Coord& coord, //Begin or finish an skidmark
                        float angle, 
                        float length);  
    bool wasSkidMarking() const;

private:

class SkidMarkPos : public ssgVtxTable
    {
    public:
        SkidMarkPos   ();
        SkidMarkPos  (ssgVertexArray* vertices,
                      ssgNormalArray* normals,
                      ssgColourArray* colors,
                      float global_track_offset);
        ~SkidMarkPos ();
        void recalcBSphere();
        void add          (const Coord& coord,  // Add a position where the skidmark is
                           float angle,
                           float length);
        void addEnd       (const Coord& coord);
    private:
        float m_track_offset;                 // Amount of which the skidmark is lifted
        // above the track to avoid z-buffer errors
    }
    ;  // SkidMarkPos

    bool m_skid_marking;
    static float m_global_track_offset;       // This is to make Z-fighting between
    // skidmarks less likely, specially the
    // skidmarks made by the AI

    std::vector <SkidMarkPos *> m_skid_marks;
    ssgSimpleState *m_skid_state;
};

#endif

/* EOF */
