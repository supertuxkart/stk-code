//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013 Joerg Henrichs
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

#include "graphics/show_curve.hpp"

#include "graphics/irr_driver.hpp"
#include "utils/vec3.hpp"

#include <IMeshSceneNode.h>
#include <IMesh.h>


/** ShowCurve constructor. It just creates an empty scene node.
 */
ShowCurve::ShowCurve(float width, float height,
                     const irr::video::SColor &color)
         : m_width(width), m_height(height)
{
    m_color = color;
    addEmptyMesh();
    m_scene_node      = irr_driver->addMesh(m_mesh, "showcurve");
    // After addMesh ref count is 1 (for the attachment to the
    // scene). We keep a copy here, so increase the ref count.
    m_scene_node->grab();
#ifdef DEBUG
    std::string debug_name = " (show-curve)";
    m_scene_node->setName(debug_name.c_str());
#endif

}   // ShowCurve

// ----------------------------------------------------------------------------
/** The destructor removes this scene node and frees the mesh. */
ShowCurve::~ShowCurve()
{
    m_mesh->drop();
    m_scene_node->drop();
    irr_driver->removeNode(m_scene_node);
}   // ShowCurve

// ----------------------------------------------------------------------------
void ShowCurve::addEmptyMesh()
{
    video::SMaterial m;
    m.AmbientColor    = m_color;
    m.DiffuseColor    = m_color;
    m.EmissiveColor   = m_color;
    m.BackfaceCulling = false;
    m_mesh            = irr_driver->createQuadMesh(&m,
                                                   /*create_one_quad*/ false);
    m_buffer          = m_mesh->getMeshBuffer(0);
    assert(m_buffer->getVertexType()==video::EVT_STANDARD);
}   // addEmptyMesh

// ----------------------------------------------------------------------------
/** Adds a point to the curve ('tunnel'). This adds 4 vertices, and it creates
 *  the triangles to connect it to the previous point.
 *  \param pnt The new point to add.
 */
void ShowCurve::addPoint(const Vec3 &pnt)
{
    // Check (again) that buffer is indeed the right type, otherwise the
    // cast to vertices would be incorrect.
    assert(m_buffer->getVertexType()==video::EVT_STANDARD);
    video::S3DVertex vertices[4];

    // 1. Append the four new vertices
    // -------------------------------
    const float w2 = 0.5f*m_width, h2 = 0.5f*m_height;
    vertices[0].Pos = Vec3(pnt + Vec3(-w2, -h2, 0)).toIrrVector();
    vertices[1].Pos = Vec3(pnt + Vec3( w2, -h2, 0)).toIrrVector();
    vertices[2].Pos = Vec3(pnt + Vec3( w2,  h2, 0)).toIrrVector();
    vertices[3].Pos = Vec3(pnt + Vec3(-w2,  h2, 0)).toIrrVector();
    vertices[0].Normal = core::vector3df(-w2, -h2, 0).normalize();
    vertices[1].Normal = core::vector3df( w2, -h2, 0).normalize();
    vertices[2].Normal = core::vector3df( w2,  h2, 0).normalize();
    vertices[3].Normal = core::vector3df(-w2,  h2, 0).normalize();

    // 2. Handle the first point, which does not define any triangles.
    // ----------------------------------------------------------------

    // The first point does not create any triangles.
    unsigned int n = m_buffer->getVertexCount();
    if(n==0)
    {
        m_buffer->append(vertices, 4, NULL, 0);
        m_mesh->setDirty();
        return;
    }

    // 3. Connect to previous indices
    // ------------------------------
    // Appending the vertices to an existing mesh buffer is complicated by the
    // fact that irrlicht adjust the indices by the number of current vertices
    // (which means that the data added can not use any previous vertex).
    // So we first add empty indices (so that irrlicht reallocates the
    // index data), then we set the actual index data.

    // Create space for the indices: we need 2 triangles for each of the
    // 4 sides of the 'tunnel', so all in all 24 indices.
    irr::u16 *indices = new irr::u16[24];
    m_buffer->append(vertices, 4, indices, 24);
    delete[] indices;
    indices = m_buffer->getIndices();

    // index = first newly added index
    unsigned int index = m_buffer->getIndexCount()-24;
    for(unsigned int i=0; i<4; i++)
    {
        indices[index  ] = n-4 + i;
        indices[index+1] = n   + i;
        indices[index+2] = n   + (i+1)%4;
        indices[index+3] = n-4 + i;
        indices[index+4] = n   + (i+1)%4;
        indices[index+5] = n-4 + (i+1)%4;
        index += 6;
    }
    m_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_buffer->getBoundingBox());
    m_mesh->setDirty();
}   // addPoint

// ----------------------------------------------------------------------------
void ShowCurve::clear()
{
    m_scene_node->setMesh(NULL);
    m_mesh->drop();
    addEmptyMesh();
    m_scene_node->setMesh(m_mesh);
    m_mesh->setDirty();
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
        Vec3 xyz(radius*sin(x), 0.2f, radius*cos(x));
        addPoint(xyz+center);
    }
}   // makeCircle

// ----------------------------------------------------------------------------
/** Sets the heading for the curve.
 *  \param heading The heading (in rad).
 */
void ShowCurve::setHeading(float heading)
{
    core::vector3df rotation(0, heading*180.0f/3.1415f, 0);
    m_scene_node->setRotation(rotation);
}   // setHeading

// ----------------------------------------------------------------------------
/** Makes this scene node visible or not.
 */
void ShowCurve::setVisible(bool isVisible)
{
    m_scene_node->setVisible(isVisible);
}   // setVisible

// ----------------------------------------------------------------------------
bool ShowCurve::isVisible() const
{
    return m_scene_node->isVisible();
}   // isVisible

// ----------------------------------------------------------------------------
/** Sets the origin of this scene node.
 */
void ShowCurve::setPosition(const Vec3 &xyz)
{
    m_scene_node->setPosition(xyz.toIrrVector());
}   // setPosition
// ----------------------------------------------------------------------------
