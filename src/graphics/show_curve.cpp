//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef SERVER_ONLY

#include "graphics/show_curve.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "mini_glm.hpp"
#include "utils/vec3.hpp"

/** ShowCurve constructor. It just creates an empty dynamic draw call.
 */
ShowCurve::ShowCurve(float width, float height,
                     const irr::video::SColor &color)
         : m_width(width), m_height(height), m_color(color),
           m_first_vertices(true), m_visible(true)
{
    m_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
        (irr::scene::EPT_TRIANGLES,
        SP::SPShaderManager::get()->getSPShader("alphablend"),
        material_manager->getDefaultSPMaterial("alphablend"));
    SP::addDynamicDrawCall(m_dy_dc);
}   // ShowCurve

// ----------------------------------------------------------------------------
/** The destructor removes this scene node and frees the mesh. */
ShowCurve::~ShowCurve()
{
    m_dy_dc->removeFromSP();
}   // ShowCurve

// ----------------------------------------------------------------------------
/** Adds a point to the curve ('tunnel'). This adds 4 vertices, and it creates
 *  the triangles to connect it to the previous point.
 *  \param pnt The new point to add.
 */
void ShowCurve::addPoint(const Vec3 &pnt)
{
    if (!m_first_vertices)
    {
        m_previous_vertices = m_current_vertices;
    }
    // 1. Append the four new vertices
    // -------------------------------
    const float w2 = 0.5f*m_width, h2 = 0.5f*m_height;
    m_current_vertices[0].m_position = Vec3(pnt + Vec3(-w2, -h2, 0)).toIrrVector();
    m_current_vertices[1].m_position = Vec3(pnt + Vec3( w2, -h2, 0)).toIrrVector();
    m_current_vertices[2].m_position = Vec3(pnt + Vec3( w2,  h2, 0)).toIrrVector();
    m_current_vertices[3].m_position = Vec3(pnt + Vec3(-w2,  h2, 0)).toIrrVector();
    m_current_vertices[0].m_normal =
        MiniGLM::compressVector3(core::vector3df(-w2, -h2, 0).normalize());
    m_current_vertices[1].m_normal =
        MiniGLM::compressVector3(core::vector3df( w2, -h2, 0).normalize());
    m_current_vertices[2].m_normal =
        MiniGLM::compressVector3(core::vector3df( w2,  h2, 0).normalize());
    m_current_vertices[3].m_normal =
        MiniGLM::compressVector3(core::vector3df(-w2,  h2, 0).normalize());
    m_current_vertices[0].m_color = m_color;
    m_current_vertices[1].m_color = m_color;
    m_current_vertices[2].m_color = m_color;
    m_current_vertices[3].m_color = m_color;

    // 2. Handle the first point, which does not define any triangles.
    // ----------------------------------------------------------------

    // The first point does not create any triangles.
    if (m_first_vertices)
    {
        m_first_vertices = false;
        return;
    }

    // 3. Connect to previous vertices
    // ------------------------------
    // Create space for the vertices: we need 2 triangles for each of the
    // 4 sides of the 'tunnel', so all in all 24 vertices.
    for (unsigned int i = 0; i < 4; i++)
    {
        m_dy_dc->addSPMVertex(m_previous_vertices[i]);
        m_dy_dc->addSPMVertex(m_current_vertices[i]);
        m_dy_dc->addSPMVertex(m_current_vertices[(i + 1) % 4]);
        m_dy_dc->addSPMVertex(m_previous_vertices[i]);
        m_dy_dc->addSPMVertex(m_current_vertices[(i + 1) % 4]);
        m_dy_dc->addSPMVertex(m_previous_vertices[(i + 1) % 4]);
    }
    // addPoint can be called more than once per frame, so reload it from the
    // beginning every time
    m_dy_dc->setUpdateOffset(0);
    m_dy_dc->recalculateBoundingBox();
}   // addPoint

// ----------------------------------------------------------------------------
void ShowCurve::clear()
{
    m_first_vertices = true;
    m_dy_dc->setUpdateOffset(-1);
    m_dy_dc->getVerticesVector().clear();
}   // clear

// ----------------------------------------------------------------------------
/** Makes this curve to show a circle with given center point and radius.
 *  \param center Center point of the circle.
 *  \param radius Radius of the circle.
 */
void ShowCurve::makeCircle(const Vec3 &center, float radius)
{
    clear();

    const float num_segs = 40.0f;
    float dx = 2.0f*M_PI/num_segs;
    for(float x = 0; x <=2.0f*M_PI; x+=dx)
    {
        Vec3 xyz(radius*sinf(x), 0.2f, radius*cosf(x));
        addPoint(xyz+center);
    }
}   // makeCircle

// ----------------------------------------------------------------------------
/** Sets the heading for the curve.
 *  \param heading The heading (in rad).
 */
void ShowCurve::setHeading(float heading)
{
    m_dy_dc->setRotationRadians(core::vector3df(0, heading, 0));
}   // setHeading

// ----------------------------------------------------------------------------
/** Makes this scene node visible or not.
 */
void ShowCurve::setVisible(bool isVisible)
{
    m_visible = isVisible;
    m_dy_dc->setVisible(m_visible);
}   // setVisible

// ----------------------------------------------------------------------------
bool ShowCurve::isVisible() const
{
    return m_visible;
}   // isVisible

// ----------------------------------------------------------------------------
/** Sets the origin of this scene node.
 */
void ShowCurve::setPosition(const Vec3 &xyz)
{
    m_dy_dc->setPosition(xyz.toIrrVector());
}   // setPosition
// ----------------------------------------------------------------------------

#endif   // !SERVER_ONLY

