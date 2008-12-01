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

#include "scene.hpp"
#include "skid_mark.hpp"

float SkidMark::m_global_track_offset = 0.005f;

SkidMark::SkidMark(float angle_sign)
{
    m_skid_state = new ssgSimpleState();
    m_angle_sign = angle_sign;
    m_skid_state->ref();
    m_skid_state -> enable (GL_BLEND);
    //This is just for the skidmarks, so the ones drawn when the kart is in
    //reverse get drawn
    m_skid_state -> disable (GL_CULL_FACE);
    m_skid_marking = false;
}   // SkidMark

//-----------------------------------------------------------------------------
SkidMark::~SkidMark()
{
    if(!m_skid_marks.empty())
    {
        const unsigned int SIZE = (unsigned int)m_skid_marks.size() -1;
        for(unsigned int i = 0; i < SIZE; ++i)
        {
            ssgDeRefDelete(m_skid_marks[i]);
        }   // for
    }   // if !empty
    ssgDeRefDelete(m_skid_state);
}   // ~SkidMark

//-----------------------------------------------------------------------------
void SkidMark::add(const Coord& coord, float angle, float length)
{
    if(m_skid_marking)
    {
        const int CURRENT = (int)m_skid_marks.size() - 1;
        m_skid_marks[CURRENT]->add(coord, m_angle_sign*angle, length);
    }
    else
    {
        addBreak(coord, angle, length);
    }
}   // add

//-----------------------------------------------------------------------------
void SkidMark::addBreak(const Coord& kart_coord, float angle, float length)
{
    Coord coord(kart_coord);
    float rad_angle = DEGREE_TO_RAD(m_angle_sign*angle);
    Vec3 add( length * sin(coord.getHeading() + rad_angle), 
             -length * cos(coord.getHeading() + rad_angle),
             0);
    coord.setXYZ(coord.getXYZ()+add);

    const unsigned int CURRENT = (unsigned int)m_skid_marks.size() - 1;
    if(m_skid_marking)
        m_skid_marks[CURRENT]->addEnd(coord);
    else
    {
        m_global_track_offset += 0.005f;
        if(m_global_track_offset > 0.05f) m_global_track_offset = 0.01f;

        // Width of the skidmark
        const float WIDTH = 0.1f;

        sgVec3 pos;
        sgSetVec3(pos,
                  coord.getX() + sin(coord.getHeading()-M_PI*0.5f) * WIDTH,
                  coord.getY() - cos(coord.getHeading()-M_PI*0.5f) * WIDTH,
                  coord.getZ() + m_global_track_offset);
        ssgVertexArray* SkidMarkVertices = new ssgVertexArray;
        SkidMarkVertices->add(pos);

        sgSetVec3(pos,
                  coord.getX() + sin(coord.getHeading()+M_PI*0.5f) * WIDTH,
                  coord.getY() - cos(coord.getHeading()+M_PI*0.5f) * WIDTH,
                  coord.getZ() + m_global_track_offset);
        SkidMarkVertices->add(pos);

        sgVec3 norm;
        sgSetVec3(norm, 0, 0, 1);

        ssgNormalArray* SkidMarkNormals = new ssgNormalArray;
        SkidMarkNormals->add(norm);
        SkidMarkNormals->add(norm);

        sgVec4 color;
        sgSetVec4(color, 0, 0, 0, 1);

        ssgColourArray* SkidMarkColors = new ssgColourArray;
        SkidMarkColors->add(color);
        SkidMarkColors->add(color);

        SkidMarkPos* new_skid_mark = new SkidMarkPos( SkidMarkVertices,
                                   SkidMarkNormals,
                                   SkidMarkColors,
                                   m_global_track_offset);
        new_skid_mark->ref();
        scene->add(new_skid_mark);
        new_skid_mark-> setState (m_skid_state);

        m_skid_marks.push_back(new_skid_mark);
    }   // if m_skid_marking

    m_skid_marking = !m_skid_marking;
}   // addBreak

//-----------------------------------------------------------------------------
bool SkidMark::wasSkidMarking() const
{
    return m_skid_marking;
}   // wasSkidMarking


//=============================================================================
SkidMark::SkidMarkPos::SkidMarkPos() : ssgVtxTable ( GL_QUAD_STRIP,
                new ssgVertexArray,
                new ssgNormalArray,
                new ssgTexCoordArray,
                new ssgColourArray )
{}   // SkidMarkPos

//-----------------------------------------------------------------------------
SkidMark::SkidMarkPos::SkidMarkPos( ssgVertexArray* vertices,
                                    ssgNormalArray* normals,
                                    ssgColourArray* colors,
                                    float global_track_offset) :
        ssgVtxTable ( GL_QUAD_STRIP,
                      vertices,
                      normals,
                      new ssgTexCoordArray,
                      colors )
{
    m_track_offset = global_track_offset;
}   // SkidMarkPos

//-----------------------------------------------------------------------------
SkidMark::SkidMarkPos::~SkidMarkPos()
{}   // ~SkidMarkPos

//-----------------------------------------------------------------------------
void SkidMark::SkidMarkPos::recalcBSphere()
{
    bsphere . setRadius ( 1000.0f ) ;
    bsphere . setCenter ( 0, 0, 0 ) ;
}   // recalcBSphere

//-----------------------------------------------------------------------------
void SkidMark::SkidMarkPos::add(const Coord& kart_coord, float angle, float length)
{
    Coord coord(kart_coord);

    Vec3 add( length * sin(kart_coord.getHPR().getHeading() + angle),
             -length * cos(kart_coord.getHPR().getHeading() + angle),
             0);
    coord.setXYZ(coord.getXYZ()+add);

    // Width of the skidmark
    const float WIDTH = 0.1f;

    static float a = 0.5f;
    sgVec3 pos;
    sgSetVec3(pos,
              coord.getX() + sgSin(coord.getHeading()+a*M_PI) * WIDTH,
              coord.getY() - sgCos(coord.getHeading()+a*M_PI) * WIDTH,
              coord.getZ() + m_track_offset);
    vertices->add(pos);

    sgSetVec3(pos,
              coord.getX() + sin(coord.getHeading()-a*M_PI) * WIDTH,
              coord.getY() - cos(coord.getHeading()-a*M_PI) * WIDTH,
              coord.getZ() + m_track_offset);
    vertices->add(pos);
    a = (a > 0.0f ? -0.5f : 0.5f);

    sgVec3 norm;
    sgSetVec3(norm, 0, 0, 1);
    normals->add(norm); normals->add(norm);

    sgVec4 color;
    sgSetVec4(color, 0.07f, 0.07f, 0.07f, 0.8f);
    colours->add(color); colours->add(color);
    this->dirtyBSphere();
}   // add

//-----------------------------------------------------------------------------
void SkidMark::SkidMarkPos::addEnd(const Coord& coord)
{
    // Width of the skidmark
    const float width = 0.1f;

    sgVec3 pos;
    sgSetVec3(pos,
              coord.getX() + sin(coord.getHeading()-M_PI*0.5f) * width,
              coord.getY() - cos(coord.getHeading()-M_PI*0.5f) * width,
              coord.getZ() + m_track_offset);
    vertices->add(pos);

    sgSetVec3(pos,
              coord.getX() + sin(coord.getHeading()+M_PI*0.5f) * width,
              coord.getY() - cos(coord.getHeading()+M_PI*0.5f) * width,
              coord.getZ() + m_track_offset);
    vertices->add(pos);

    sgVec3 norm;
    sgSetVec3(norm, 0, 0, 1);
    normals->add(norm); normals->add(norm);

    sgVec4 color;
    sgSetVec4(color, 0.15f, 0.15f, 0.15f, 0.0f);
    colours->add(color); colours->add(color);
    makeDList();
}   // addEnd

/* EOF */
