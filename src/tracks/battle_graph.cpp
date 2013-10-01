//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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
//  Foundation, Inc., 59 Temple Place - Suite 330, B

#include "tracks/battle_graph.hpp"


#include <IMesh.h>
#include <ICameraSceneNode.h>

#include "graphics/irr_driver.hpp"
#include "tracks/navmesh.hpp"
#include "utils/vec3.hpp"


BattleGraph * BattleGraph::m_battle_graph = NULL;

BattleGraph::BattleGraph(const std::string &navmesh_file_name)
{
    NavMesh::create(navmesh_file_name);
    m_navmesh_file = navmesh_file_name;
    buildGraph(NavMesh::get());
}

void BattleGraph::buildGraph(NavMesh* navmesh)
{
    unsigned int n_polys = navmesh->getNumberOfPolys();
    m_graph.resize(n_polys);
    for(int i=0; i<n_polys; i++)
    {
        NavPoly currentPoly = navmesh->getNavPoly(i);
        std::vector<int> adjacents = navmesh->getAdjacentPolys(i);
        for(unsigned int j=0; j<adjacents.size(); j++)
        {
            Vec3 adjacentPolyCenter = navmesh->getCenterOfPoly(adjacents[j]);
            float distance = Vec3(adjacentPolyCenter - currentPoly.getCenter()).length_2d();
            m_graph[i].push_back(std::make_pair(adjacents[i], distance));

        }
    }

}
BattleGraph::~BattleGraph(void)
{

}

void BattleGraph::createMesh(bool enable_transparency,
                           const video::SColor *track_color)
{
    // The debug track will not be lighted or culled.
    video::SMaterial m;
    m.BackfaceCulling  = false;
    m.Lighting         = false;
    if(enable_transparency)
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m_mesh             = irr_driver->createQuadMesh(&m);
    m_mesh_buffer      = m_mesh->getMeshBuffer(0);
    assert(m_mesh_buffer->getVertexType()==video::EVT_STANDARD);

   
    
    // Fetch the number of vertices
    unsigned int n = NavMesh::get()->getNumberOfVerts();
    
    // Get the reference to a vector of vertices from NavMesh
    const std::vector<Vec3>& v = NavMesh::get()->getAllVertices();
 
    // Declare array to hold new converted vertices
    video::S3DVertex *new_v = new video::S3DVertex[n];
    
    // Eps is used to raise the track debug quads a little bit higher than
    // the ground, so that they are actually visible. 
    core::vector3df eps(0, 0.4f, 0);
    video::SColor     defaultColor(255, 255, 0, 0), c;
    
    for( unsigned count=0; count<n; count++)
    {
        new_v[count].Pos = v[count].toIrrVector() + eps;
        new_v[count].Color = defaultColor;
    }
    
    // Each quad consists of 2 triangles with 3 elements, so
    // we need 2*3 indices for each quad.
    std::vector<irr::u16> ind;
    
    // Now add all quads
    int i=0;
    for(unsigned int count=0; count<m_graph.size(); count++)
    {
        ///from red to blue and back
        if(!track_color)
        {   
            c.setAlpha(178);
            //c.setRed ((i%2) ? 255 : 0);
            //c.setBlue((i%3) ? 0 : 255);
            c.setRed((3*i)%256);
            c.setBlue((5*i)%256);
            c.setGreen((7*i)%256);
        }
        
        NavPoly poly = NavMesh::get()->getNavPoly(count);

        std::vector<int> vInd = poly.getVerticesIndex();

        // Number of triangles in the triangle fan
        unsigned int numberOfTriangles = vInd.size() -2 ;

        // Set up the indices for the triangles
        
         for( unsigned int count = 1; count<=numberOfTriangles; count++)
         {
             ind.push_back(vInd[0]);
             ind.push_back(vInd[count]);
             ind.push_back(vInd[count+1]);

             if(new_v[vInd[0]].Color==defaultColor) new_v[vInd[0]].Color = c;
             if(new_v[vInd[count]].Color==defaultColor) new_v[vInd[count]].Color = c;
             if(new_v[vInd[count+1]].Color==defaultColor)new_v[vInd[count+1]].Color = c;

            core::triangle3df tri(v[vInd[0]].toIrrVector(), 
                                v[vInd[count]].toIrrVector(),
                                v[vInd[count+1]].toIrrVector());
            core::vector3df normal = tri.getNormal();
            normal.normalize();
            new_v[vInd[0]].Normal = normal;
            new_v[vInd[count]].Normal = normal;
            new_v[vInd[count+1]].Normal = normal;

        }   
        
                 
        //// (note, afaik with opengl we could use quads directly, but the code
        //// would not be portable to directx anymore).
        //ind[6*i  ] = 4*i;  // First triangle: vertex 0, 1, 2
        //ind[6*i+1] = 4*i+1;
        //ind[6*i+2] = 4*i+2;
        //ind[6*i+3] = 4*i;  // second triangle: vertex 0, 1, 3
        //ind[6*i+4] = 4*i+2;
        //ind[6*i+5] = 4*i+3;
        //i++;
    }   // for i=1; i<QuadSet::get()

    m_mesh_buffer->append(new_v, n, ind.data(), ind.size());

    // Instead of setting the bounding boxes, we could just disable culling,
    // since the debug track should always be drawn.
    //m_node->setAutomaticCulling(scene::EAC_OFF);
    m_mesh_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_mesh_buffer->getBoundingBox());

    delete[] new_v;
}   // createMesh


/** Creates the debug mesh to display the quad graph on top of the track
 *  model. */
void BattleGraph::createDebugMesh()
{
    if(m_graph.size()<=0) return;  // no debug output if not graph

    createMesh(/*enable_transparency*/true);

    //// Now colour the quads red/blue/red ...
    //video::SColor     c( 128, 255, 0, 0);
    //video::S3DVertex *v = (video::S3DVertex*)m_mesh_buffer->getVertices();
    //for(unsigned int i=0; i<m_mesh_buffer->getVertexCount(); i++)
    //{
    //    // Swap the colours from red to blue and back
    //    c.setRed ((i%2) ? 255 : 0);
    //    c.setBlue((i%2) ? 0 : 255);
    //    v[i].Color = c;
    //}
    m_node = (scene::ISceneNode*)irr_driver->addMesh(m_mesh);
#ifdef DEBUG
    m_node->setName("track-debug-mesh");
#endif

}   // createDebugMesh
