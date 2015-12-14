//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Joerg Henrichs
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
#include <IMeshSceneNode.h>

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/rtts.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "tracks/navmesh.hpp"
#include "utils/log.hpp"

const int BattleGraph::UNKNOWN_POLY  = -1;
BattleGraph * BattleGraph::m_battle_graph = NULL;

/** Constructor, Creates a navmesh, builds a graph from the navmesh and
*    then runs shortest path algorithm to find and store paths to be used
*    by the AI. */
BattleGraph::BattleGraph(const std::string &navmesh_file_name)
{
    m_node                 = NULL;
    m_mesh                 = NULL;
    m_mesh_buffer          = NULL;
    m_new_rtt              = NULL;
    m_items_on_graph.clear();

    NavMesh::create(navmesh_file_name);
    m_navmesh_file = navmesh_file_name;
    buildGraph(NavMesh::get());
    computeFloydWarshall();
} // BattleGraph

// -----------------------------------------------------------------------------
/** Destructor, destroys NavMesh and the debug mesh if it exists */
BattleGraph::~BattleGraph(void)
{
    NavMesh::destroy();

    if(UserConfigParams::m_track_debug)
        cleanupDebugMesh();
    if (m_new_rtt != NULL)
        delete m_new_rtt;
} // ~BattleGraph

// -----------------------------------------------------------------------------
/** Builds a graph from an existing NavMesh. The graph is stored as an adjacency
*    matrix. */
void BattleGraph::buildGraph(NavMesh* navmesh)
{
    unsigned int n_polys = navmesh->getNumberOfPolys();

    m_distance_matrix = std::vector< std::vector<float> > (n_polys, std::vector<float>(n_polys, 9999.9f));
    for(unsigned int i=0; i<n_polys; i++)
    {
        NavPoly currentPoly = navmesh->getNavPoly(i);
        std::vector<int> adjacents = navmesh->getAdjacentPolys(i);
        for(unsigned int j=0; j<adjacents.size(); j++)
        {
            Vec3 adjacentPolyCenter = navmesh->getCenterOfPoly(adjacents[j]);
            float distance = Vec3(adjacentPolyCenter - currentPoly.getCenter()).length_2d();

            m_distance_matrix[i][adjacents[j]] = distance;
            //m_distance_matrix[adjacents[j]][i] = distance;

        }
        m_distance_matrix[i][i] = 0.0f;
    }

}    // buildGraph

// -----------------------------------------------------------------------------
/** computeFloydWarshall() computes the shortest distance between any two nodes.
 *  At the end of the computation, m_distance_matrix[i][j] stores the shortest path
 *  distance from i to j and m_parent_poly[i][j] stores the last vertex visited on the
 *  shortest path from i to j before visiting j. Suppose the shortest path from i to j is
 *  i->......->k->j  then m_parent_poly[i][j] = k
 */
void BattleGraph::computeFloydWarshall()
{
    unsigned int n = getNumNodes();

    // initialize m_parent_poly with unknown_poly so that if no path is found b/w i and j
    // then m_parent_poly[i][j] = -1 (UNKNOWN_POLY)
    // AI must check this
    m_parent_poly = std::vector< std::vector<int> > (n, std::vector<int>(n,BattleGraph::UNKNOWN_POLY));
    for(unsigned int i=0; i<n; i++)
    {
        for(unsigned int j=0; j<n; j++)
        {
            if(i == j || m_distance_matrix[i][j]>=9899.9f) m_parent_poly[i][j]=-1;
            else    m_parent_poly[i][j] = i;
        }
    }

    for(unsigned int k=0; k<n; k++)
    {
        for(unsigned int i=0; i<n; i++)
        {
            for(unsigned int j=0; j<n; j++)
            {
                if( (m_distance_matrix[i][k] + m_distance_matrix[k][j]) < m_distance_matrix[i][j])
                {
                    m_distance_matrix[i][j] = m_distance_matrix[i][k] + m_distance_matrix[k][j];
                    m_parent_poly[i][j] = m_parent_poly[k][j];
                }
            }
        }
    }

}    // computeFloydWarshall

// -----------------------------------------------------------------------------
/** Creates the actual mesh that is used by createDebugMesh() or makeMiniMap() */
void BattleGraph::createMesh(bool enable_transparency,
                           const video::SColor *track_color)
{
    // The debug track will not be lighted or culled.
    video::SMaterial m;
    m.BackfaceCulling  = false;
    m.Lighting         = false;
    if(enable_transparency)
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m.setTexture(0, getUnicolorTexture(video::SColor(255, 255, 255, 255)));
    m.setTexture(1, getUnicolorTexture(video::SColor(0, 0, 0, 0)));
    m_mesh             = irr_driver->createQuadMesh(&m);
    m_mesh_buffer      = m_mesh->getMeshBuffer(0);
    assert(m_mesh_buffer->getVertexType()==video::EVT_STANDARD);

    const unsigned int num_nodes = getNumNodes();

    // Four vertices for each of the n-1 remaining quads
    video::S3DVertex *new_v = new video::S3DVertex[4*num_nodes];
    // Each quad consists of 2 triangles with 3 elements, so
    // we need 2*3 indices for each quad.
    irr::u16         *ind   = new irr::u16[6*num_nodes];
    video::SColor     c(255, 255, 0, 0);

    if(track_color)
        c = *track_color;

    // Now add all quads
    int i=0;
    for(unsigned int count=0; count<num_nodes; count++)
    {
        // There should not be a poly which isn't made of 4 vertices
        if((NavMesh::get()->getNavPoly(count).getVerticesIndex()).size() !=4)
        {
            Log::warn("Battle Graph", "There is an invalid poly!");
            continue;
        }
        // Swap the colours from red to blue and back
        if(!track_color)
        {
            c.setRed ((i%2) ? 255 : 0);
            c.setBlue((i%2) ? 0 : 255);
        }
        // Transfer the 4 points of the current quad to the list of vertices
        NavMesh::get()->setVertices(count, new_v+4*i, c);

        // Set up the indices for the triangles
        // (note, afaik with opengl we could use quads directly, but the code
        // would not be portable to directx anymore).
        ind[6*i  ] = 4*i+2;  // First triangle: vertex 0, 1, 2
        ind[6*i+1] = 4*i+1;
        ind[6*i+2] = 4*i;
        ind[6*i+3] = 4*i+3;  // second triangle: vertex 0, 1, 3
        ind[6*i+4] = 4*i+2;
        ind[6*i+5] = 4*i;
        i++;
    }   // for i=1; i<QuadSet::get()

    m_mesh_buffer->append(new_v, num_nodes*4, ind, num_nodes*6);

    // Instead of setting the bounding boxes, we could just disable culling,
    // since the debug track should always be drawn.
    //m_node->setAutomaticCulling(scene::EAC_OFF);
    m_mesh_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_mesh_buffer->getBoundingBox());

    m_mesh_buffer->getMaterial().setTexture(0, irr_driver->getTexture("unlit.png"));

}   // createMesh
// -----------------------------------------------------------------------------
/** Takes a snapshot of the navmesh so they can be used as minimap.
 */
void BattleGraph::makeMiniMap(const core::dimension2du &dimension,
                            const std::string &name,
                            const video::SColor &fill_color,
                            video::ITexture** oldRttMinimap,
                            FrameBuffer** newRttMinimap)
{
    const video::SColor oldClearColor = World::getWorld()->getClearColor();
    World::getWorld()->setClearbackBufferColor(video::SColor(0, 255, 255, 255));
    World::getWorld()->forceFogDisabled(true);
    *oldRttMinimap = NULL;
    *newRttMinimap = NULL;

    RTT* newRttProvider = NULL;
    IrrDriver::RTTProvider* oldRttProvider = NULL;
    if (CVS->isGLSL())
    {
        m_new_rtt = newRttProvider = new RTT(dimension.Width, dimension.Height);
    }
    else
    {
        oldRttProvider = new IrrDriver::RTTProvider(dimension, name, true);
    }

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 255, 255, 255));

    createMesh(/*enable_transparency*/ false,
               /*track_color*/    &fill_color);

    m_node = irr_driver->addMesh(m_mesh, "mini_map");
#ifdef DEBUG
    m_node->setName("minimap-mesh");
#endif

    m_node->setAutomaticCulling(0);
    m_node->setMaterialFlag(video::EMF_LIGHTING, false);

    // Add the camera:
    // ---------------
    scene::ICameraSceneNode *camera = irr_driver->addCameraSceneNode();
    Vec3 bb_min, bb_max;
    NavMesh::get()->getBoundingBox(&bb_min, &bb_max);
    Vec3 center = (bb_max+bb_min)*0.5f;

    float dx = bb_max.getX()-bb_min.getX();
    float dz = bb_max.getZ()-bb_min.getZ();

    // Set the scaling correctly. Also the center point (which is used
    // as the camera position) needs to be adjusted: the track must
    // be aligned to the left/top of the texture which is used (otherwise
    // mapPoint2MiniMap doesn't work), so adjust the camera position
    // that the track is properly aligned (view from the side):
    //          c        camera
    //         / \       .
    //        /   \     <--- camera angle
    //       /     \     .
    //      {  [-]  }   <--- track flat (viewed from the side)
    // If [-] is the shorter side of the track, then the camera will
    // actually render the area in { } - which is the length of the
    // longer side of the track.
    // To align the [-] side to the left, the camera must be moved
    // the distance betwwen '{' and '[' to the right. This distance
    // is exacly (longer_side - shorter_side) / 2.
    // So, adjust the center point by this amount:
    if(dz > dx)
    {
        center.setX(center.getX() + (dz-dx)*0.5f);
        m_scaling = dimension.Width / dz;
    }
    else
    {
        center.setZ(center.getZ() + (dx-dz)*0.5f);
        m_scaling = dimension.Width / dx;
    }

    float range = (dx>dz) ? dx : dz;

    core::matrix4 projection;
    projection.buildProjectionMatrixOrthoLH(range /* width */,
                                            range /* height */,
                                            -1, bb_max.getY()-bb_min.getY()+1);
    camera->setProjectionMatrix(projection, true);

    irr_driver->suppressSkyBox();
    irr_driver->clearLights();

    // Adjust Y position by +1 for max, -1 for min - this helps in case that
    // the maximum Y coordinate is negative (otherwise the minimap is mirrored)
    // and avoids problems for tracks which have a flat (max Y = min Y) minimap.
    camera->setPosition(core::vector3df(center.getX(), bb_min.getY() + 1.0f, center.getZ()));
    //camera->setPosition(core::vector3df(center.getX() - 5.0f, bb_min.getY() - 1 - 5.0f, center.getZ() - 15.0f));
    camera->setUpVector(core::vector3df(0, 0, 1));
    camera->setTarget(core::vector3df(center.getX(),bb_min.getY()-1,center.getZ()));
    //camera->setAspectRatio(1.0f);
    camera->updateAbsolutePosition();

    video::ITexture* texture = NULL;
    FrameBuffer* frame_buffer = NULL;

    if (CVS->isGLSL())
    {
        frame_buffer = newRttProvider->render(camera, GUIEngine::getLatestDt());
    }
    else
    {
        texture = oldRttProvider->renderToTexture();
        delete oldRttProvider;
    }

    cleanupDebugMesh();
    irr_driver->removeCameraSceneNode(camera);
    m_min_coord = bb_min;


    if (texture == NULL && frame_buffer == NULL)
    {
        Log::error("BattleGraph", "[makeMiniMap] WARNING: RTT does not appear to work,"
                                  "mini-map will not be available.");
    }

    *oldRttMinimap = texture;
    *newRttMinimap = frame_buffer;
    World::getWorld()->setClearbackBufferColor(oldClearColor);
    World::getWorld()->forceFogDisabled(false);

    irr_driver->getSceneManager()->clear();
    VAOManager::kill();
    irr_driver->clearGlowingNodes();
    irr_driver->clearLights();
    irr_driver->clearForcedBloom();
    irr_driver->clearBackgroundNodes();
}   // makeMiniMap

// -----------------------------------------------------------------------------
    /** Returns the 2d coordinates of a point when drawn on the mini map
     *  texture.
     *  \param xyz Coordinates of the point to map.
     *  \param draw_at The coordinates in pixel on the mini map of the point,
     *         only the first two coordinates will be used.
     */
void BattleGraph::mapPoint2MiniMap(const Vec3 &xyz,Vec3 *draw_at) const
{
    draw_at->setX((xyz.getX()-m_min_coord.getX())*m_scaling);
    draw_at->setY((xyz.getZ()-m_min_coord.getZ())*m_scaling);

}   // mapPoint

// -----------------------------------------------------------------------------
/** Creates the debug mesh to display the quad graph on top of the track
 *  model. */
void BattleGraph::createDebugMesh()
{
    if(getNumNodes()<=0) return;  // no debug output if not graph

    createMesh(/*enable_transparency*/true);

    // Now colour the quads red/blue/red ...
    video::SColor     c( 128, 255, 0, 0);
    video::S3DVertex *v = (video::S3DVertex*)m_mesh_buffer->getVertices();
    for(unsigned int i=0; i<m_mesh_buffer->getVertexCount(); i++)
    {
        // Swap the colours from red to blue and back
        c.setRed ((i%2) ? 255 : 0);
        c.setBlue((i%2) ? 0 : 255);
        v[i].Color = c;
    }
    m_node = irr_driver->addMesh(m_mesh, "track-debug-mesh");
#ifdef DEBUG
    m_node->setName("track-debug-mesh");
#endif

}   // createDebugMesh

// -----------------------------------------------------------------------------
/** Cleans up the debug mesh */
void BattleGraph::cleanupDebugMesh()
{
    if(m_node != NULL)
        irr_driver->removeNode(m_node);

    m_node = NULL;
    // No need to call irr_driber->removeMeshFromCache, since the mesh
    // was manually made and so never added to the mesh cache.
    m_mesh->drop();
    m_mesh = NULL;
}

// -----------------------------------------------------------------------------
/** Maps items on battle graph */
void BattleGraph::findItemsOnGraphNodes()
{
    const ItemManager* item_manager = ItemManager::get();
    unsigned int item_count = item_manager->getNumberOfItems();

    for (unsigned int i = 0; i < item_count; ++i)
    {
        const Item* item = item_manager->getItem(i);
        Vec3 xyz = item->getXYZ();
        int polygon = BattleGraph::UNKNOWN_POLY;

        for (unsigned int j = 0; j < this->getNumNodes(); ++j)
        {
            if (NavMesh::get()->getNavPoly(j).pointInPoly(xyz))
            {
                float dist = xyz.getY() - NavMesh::get()->getCenterOfPoly(j).getY();
                if (fabsf(dist) < 1.0f )
                    polygon = j;
            }
        }

        if (polygon != BattleGraph::UNKNOWN_POLY)
        {
            m_items_on_graph.push_back(std::make_pair(item, polygon));
            Log::debug("BattleGraph","item number %d is on polygon %d", i, polygon);
        }
        else
            Log::debug("BattleGraph","Can't map item number %d with a suitable polygon", i);
    }
}

// -----------------------------------------------------------------------------

const int & BattleGraph::getNextShortestPathPoly(int i, int j) const
{
    if (i == BattleGraph::UNKNOWN_POLY || j == BattleGraph::UNKNOWN_POLY)
        return BattleGraph::UNKNOWN_POLY;
    return m_parent_poly[j][i];
}
