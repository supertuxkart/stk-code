//  $Id: slip_stream.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

#include "graphics/slip_stream.hpp"

#include "graphics/material_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "utils/constants.hpp"

/** Creates the slip stream object using a moving texture.
 *  \param kart Pointer to the kart to which the slip stream
 *              belongs to.
 */
SlipStream::SlipStream(Kart* kart) : m_kart(kart), MovingTexture(0, 0)
{
    video::SMaterial m;
    Material *material = material_manager->getMaterial("slipstream.png");
    m.setTexture(0, material->getTexture());
    m.setFlag(video::EMF_BACK_FACE_CULLING, false);
    m.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
    createMesh(m);
    m_node = irr_driver->addMesh(m_mesh);
    m_node->setParent(m_kart->getNode());
    m_node->setPosition(core::vector3df(0, 
                                        0*0.25f,
                                        m_kart->getKartLength()) );
    setTextureMatrix(&(m_node->getMaterial(0).getTextureMatrix(0)));
}   // SlipStream

//-----------------------------------------------------------------------------
/** Removes the node from the scene graph.
 */
SlipStream::~SlipStream()
{
    irr_driver->removeNode(m_node);
}   // ~SlipStream

//-----------------------------------------------------------------------------
/** Creates the mesh for the slipstream effect. This function creates a
 *  first a series of circles (with a certain number of vertices each and
 *  distance from each other. Then it will create the triangles and add
 *  texture coordniates.
 *  \param material  The material to use.
 */
void SlipStream::createMesh(const video::SMaterial &material)
{
    // All radius, starting with the one closest to the kart (and
    // widest) to the one furthest away. A 0 indicates the end of the list
    float radius[] = {3.0f, 2.0f, 1.0f, 0.0f};

    // The distance of each of the circle from the kart. The number of
    // entries in this array must be the same as the number of non-zero 
    // entries in the radius[] array above. No 'end of list' entry required.
    float distance[] = {0.0f, 2.0f, 4.0f };

    // The alpha values for the rings, no 'end of list' entry required.
    int alphas[]     = {0, 255, 0};

    // Loop through all given radius to determine the number 
    // of segments to create.
    unsigned int num_circles;
    for(num_circles=0; radius[num_circles]!=0; num_circles++) ;

    // Length is distance of last circle to distance of first circle:
    float length = distance[num_circles-1] - distance[0];

    // The number of points for each circle.
    const int   num_segments   = 15;
    const float f              = 2*M_PI/float(num_segments);
    scene::SMeshBuffer *buffer = new scene::SMeshBuffer();
    buffer->Material           = material;
    for(unsigned int j=0; j<num_circles; j++)
    {
        float curr_distance = distance[j]-distance[0];
        // Create the vertices for each of the circle
        for(unsigned int i=0; i<num_segments; i++)
        {
            video::S3DVertex v;                  
            v.Pos.X =  sin(i*f)*radius[j];
            v.Pos.Y = -cos(i*f)*radius[j];
            v.Pos.Z = distance[j];
            v.Color = video::SColor(alphas[j], 255, 0, 0);
            v.TCoords.X = curr_distance/length;
            v.TCoords.Y = (float)i/(num_segments-1);
            buffer->Vertices.push_back(v);
        }   // for i<num_segments
    }   // while radius[num_circles]!=0

    // Now create the triangles. 
    for(unsigned int j=0; j<num_circles-1; j++)
    {
        for(unsigned int i=0; i<num_segments-1; i++)
        {
            buffer->Indices.push_back( j   *num_segments+i  );
            buffer->Indices.push_back((j+1)*num_segments+i  );
            buffer->Indices.push_back( j   *num_segments+i+1);
            buffer->Indices.push_back( j   *num_segments+i+1);
            buffer->Indices.push_back((j+1)*num_segments+i  );
            buffer->Indices.push_back((j+1)*num_segments+i+1);
        }
    }   // for j<num_circles-1

    scene::SMesh *mesh = new scene::SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();

    buffer->drop();
    m_mesh = mesh;
}   // createMesh
 
//-----------------------------------------------------------------------------
/** Sets the animation intensity (or speed).
 *  \param f Intensity: 0 = no slip stream,
 *                      1 = collecting
 *                      2 = using slip stream bonus
 */
void SlipStream::setIntensity(float f)
{
    // For now: disable them permanently
    m_node->setVisible(false);
    return;

    // For real testing in game: this needs some tuning!
    //m_node->setVisible(f!=0);
    //MovingTexture::setSpeed(f*0.1f, 0);

    // For debugging: make the slip stream effect visible all the time
    m_node->setVisible(true);
    MovingTexture::setSpeed(1.0f, 0.0f);
}   // setIntensity

