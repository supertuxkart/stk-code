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
      
    
    // Eps is used to raise the track debug quads a little bit higher than
    // the ground, so that they are actually visible. 
    core::vector3df eps(0, 0.4f, 0);
    video::SColor     defaultColor(255, 255, 0, 0), c;
        
    // Declare vector to hold new converted vertices, vertices are copied over 
    // for each polygon, although it results in redundant vertex copies in the 
    // final vector, this is the only way I know to make each poly have different color.
     std::vector<video::S3DVertex> new_v;

    // Declare vector to hold indices
    std::vector<irr::u16> ind;
    
    // Now add all polygons
    int i=0;
    for(unsigned int count=0; count<m_graph.size(); count++)
    {
        ///compute colors
        if(!track_color)
        {   
            c.setAlpha(178);
            //c.setRed ((i%2) ? 255 : 0);
            //c.setBlue((i%3) ? 0 : 255);
            c.setRed(i%256);
            c.setBlue((2*i)%256);
            c.setGreen((3*i)%256);
        }
        
        NavPoly poly = NavMesh::get()->getNavPoly(count);

        //std::vector<int> vInd = poly.getVerticesIndex();
        const std::vector<Vec3>& v = poly.getVertices();
       
        // Number of triangles in the triangle fan
        unsigned int numberOfTriangles = v.size() -2 ;

        // Set up the indices for the triangles
        
         for( unsigned int count = 1; count<=numberOfTriangles; count++)
         {
             video::S3DVertex v1,v2,v3;
             v1.Pos=v[0].toIrrVector() + eps;
             v2.Pos=v[count].toIrrVector() + eps;
             v3.Pos=v[count+1].toIrrVector() + eps;
             
             v1.Color = c;
             v2.Color = c;
             v3.Color = c;

             core::triangle3df tri(v1.Pos, v2.Pos, v3.Pos);
             core::vector3df normal = tri.getNormal();
             normal.normalize();
             v1.Normal = normal;
             v2.Normal = normal;
             v3.Normal = normal;
                          
             new_v.push_back(v1);
             new_v.push_back(v2);
             new_v.push_back(v3);
             
             ind.push_back(i++);
             ind.push_back(i++);
             ind.push_back(i++);

            
        }   
               
    }   

    m_mesh_buffer->append(new_v.data(), new_v.size(), ind.data(), ind.size());

    // Instead of setting the bounding boxes, we could just disable culling,
    // since the debug track should always be drawn.
    //m_node->setAutomaticCulling(scene::EAC_OFF);
    m_mesh_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_mesh_buffer->getBoundingBox());

}   // createMesh


/** Creates the debug mesh to display the quad graph on top of the track
 *  model. */

void BattleGraph::createDebugMesh()
{
    if(m_graph.size()<=0) return;  // no debug output if not graph

    createMesh(/*enable_transparency*/true);
    m_node = (scene::ISceneNode*)irr_driver->addMesh(m_mesh);
#ifdef DEBUG
    m_node->setName("track-debug-mesh");
#endif

}   // createDebugMesh
