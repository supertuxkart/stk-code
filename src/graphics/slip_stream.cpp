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
SlipStream::SlipStream(Kart* kart) : MovingTexture(0, 0), m_kart(kart)
{
    video::SMaterial m;
    Material *material = material_manager->getMaterial("slipstream.png");
    m.setTexture(0, material->getTexture());
    m.setFlag(video::EMF_BACK_FACE_CULLING, false);
    m.setFlag(video::EMF_COLOR_MATERIAL, true);

    m.ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
    
    //m.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
    //m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m.MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;

    createMesh(m);
    m_node = irr_driver->addMesh(m_mesh);
#ifdef DEBUG
    std::string debug_name = m_kart->getIdent()+" (slip-stream)";
    m_node->setName(debug_name.c_str());
#endif
    //m_node->setParent(m_kart->getNode());
    m_node->setPosition(core::vector3df(0, 
                                        0*0.25f+2.5,
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
    float radius[] = {1.5f, 1.0f, 0.5f, 0.0f};

    // The distance of each of the circle from the kart. The number of
    // entries in this array must be the same as the number of non-zero 
    // entries in the radius[] array above. No 'end of list' entry required.
    // Note also that in order to avoid a 'bent' in the texture the
    // difference between the distances must be linearly correlated to the
    // difference in the corresponding radius, see the following sideview:
    //            +
    //       +    |
    //  +    |    |    three radius
    //  |    |    |
    //  +----+----+
    //  0    1    2
    //  distances
    // (radius1-radius0)/(distance1-distance0) = (radius2-radius1)/(distnace2-distance0)
    // This way the line connecting the upper "+" is a straight line,
    // and so the 3d cone shape will not be disturbed.
    float distance[] = {2.0f, 6.0f, 10.0f };

    // The alpha values for the rings, no 'end of list' entry required.
    int alphas[]     = {0, 255, 0};

    // Loop through all given radius to determine the number 
    // of segments to create.
    unsigned int num_circles=0;
    while(radius[num_circles]) num_circles++;

    // Length is distance of last circle to distance of first circle:
    m_length = distance[num_circles-1] - distance[0];

    // The number of points for each circle. Since part of the slip stream
    // might be under the ground (esp. first and last segment), specify
    // which one is the first and last to be actually drawn.
    const unsigned int  num_segments   = 15;
    const unsigned int  first_segment  = 0;
    const unsigned int  last_segment   = 14;
    const float         f              = 2*M_PI/float(num_segments);
    scene::SMeshBuffer *buffer         = new scene::SMeshBuffer();
    buffer->Material                   = material;
    for(unsigned int j=0; j<num_circles; j++)
    {
        float curr_distance = distance[j]-distance[0];
        // Create the vertices for each of the circle
        for(unsigned int i=first_segment; i<=last_segment; i++)
        {
            video::S3DVertex v;
            // Offset every 2nd circle by one half segment to increase
            // the number of planes so it looks better.
            v.Pos.X =  sin((i+(j%2)*0.5f)*f)*radius[j];
            v.Pos.Y = -cos((i+(j%2)*0.5f)*f)*radius[j];
            v.Pos.Z = distance[j];
            v.Color = video::SColor(alphas[j], alphas[j], alphas[j], alphas[j]);
            v.TCoords.X = curr_distance/m_length;
            v.TCoords.Y = (float)(i-first_segment)/(last_segment-first_segment)
                          + (j%2)*(.5f/num_segments);
            buffer->Vertices.push_back(v);
        }   // for i<num_segments
    }   // while radius[num_circles]!=0

    // Now create the triangles from circle j to j+1 (so the loop
    // only goes to num_circles-1).
    const int diff_segments = last_segment-first_segment+1;
    for(unsigned int j=0; j<num_circles-1; j++)
    {
        for(unsigned int i=first_segment; i<last_segment; i++)
        {
            buffer->Indices.push_back( j   *diff_segments+i  );
            buffer->Indices.push_back((j+1)*diff_segments+i  );
            buffer->Indices.push_back( j   *diff_segments+i+1);
            buffer->Indices.push_back( j   *diff_segments+i+1);
            buffer->Indices.push_back((j+1)*diff_segments+i  );
            buffer->Indices.push_back((j+1)*diff_segments+i+1);
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
void SlipStream::setIntensity(float f, const Kart *kart)
{
    if(!kart)
    {
        m_node->setVisible(false);
        return;
    }

    const float above_terrain = 0.2f;
    core::vector3df my_pos = m_kart->getNode()->getPosition();
    my_pos.Y = m_kart->getHoT()+above_terrain;
    m_node->setPosition(my_pos);
    
    core::vector3df other_pos = kart->getNode()->getPosition();
    other_pos.Y = kart->getHoT()+above_terrain;
    core::vector3df diff =   other_pos - my_pos;
    core::vector3df rotation = diff.getHorizontalAngle();
    m_node->setRotation(rotation);
    float fs = diff.getLength()/m_length;
    m_node->setScale(core::vector3df(1, 1, fs));

    // For real testing in game: this needs some tuning!
    m_node->setVisible(f!=0);
    MovingTexture::setSpeed(f, 0);
    return;
    // For debugging: make the slip stream effect visible all the time
    m_node->setVisible(true);
    MovingTexture::setSpeed(1.0f, 0.0f);
}   // setIntensity

//-----------------------------------------------------------------------------
/** Update, called once per timestep.
 *  \param dt Time step size.
 */
void SlipStream::update(float dt)
{
    MovingTexture::update(dt);
return;
    core::vector3df pos = m_kart->getNode()->getPosition();
    pos.Y = m_kart->getHoT()+0.2f;
    m_node->setPosition(pos);
    core::vector3df f = core::vector3df(0, 0, 10) - f;
    core::vector3df r = f.getHorizontalAngle();
    m_node->setRotation(r);
    return;

    const core::quaternion new_rot(m_kart->getNode()->getRotation());
    const core::quaternion old_rot(m_node->getRotation()            );

    core::quaternion interpo;
    core::vector3df interp;
    new_rot.toEuler(interp);
    m_node->setRotation(interp);
}   // update
