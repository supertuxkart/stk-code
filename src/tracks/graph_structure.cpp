//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015  SuperTuxKart Team
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

#include "tracks/graph_structure.hpp"

#include <ICameraSceneNode.h>
#include <IMesh.h>
#include <IMeshSceneNode.h>
#include <ISceneManager.h>

#include "graphics/irr_driver.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/rtts.hpp"
#include "modes/world.hpp"
#include "modes/profile_world.hpp"
#include "utils/log.hpp"

// -----------------------------------------------------------------------------

GraphStructure::GraphStructure()
{
    m_min_coord     = 0;
    m_scaling       = 0;
    m_node          = NULL;
    m_mesh          = NULL;
    m_mesh_buffer   = NULL;
    m_render_target = NULL;
}  // GraphStructure

// -----------------------------------------------------------------------------
/** Cleans up the debug mesh */
void GraphStructure::cleanupDebugMesh()
{
    if (m_node != NULL)
        irr_driver->removeNode(m_node);

    m_node = NULL;
    // No need to call irr_driber->removeMeshFromCache, since the mesh
    // was manually made and so never added to the mesh cache.
    m_mesh->drop();
    m_mesh = NULL;
}

// -----------------------------------------------------------------------------
/** Creates the debug mesh to display the graph on top of the track
 *  model. */
void GraphStructure::createDebugMesh()
{
    if (getNumNodes() <= 0) return;  // no debug output if not graph

    createMesh(/*show_invisible*/true,
               /*enable_transparency*/true);

    // Now colour the quads red/blue/red ...
    video::SColor     c( 128, 255, 0, 0);
    video::S3DVertex *v = (video::S3DVertex*)m_mesh_buffer->getVertices();
    for (unsigned int i = 0; i < m_mesh_buffer->getVertexCount(); i++)
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
/** Creates the actual mesh that is used by createDebugMesh() or makeMiniMap() */
void GraphStructure::createMesh(bool show_invisible,
                                bool enable_transparency,
                                const video::SColor *track_color)
{
    // The debug track will not be lighted or culled.
    video::SMaterial m;
    m.BackfaceCulling  = false;
    m.Lighting         = false;
    if (enable_transparency)
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m.setTexture(0, getUnicolorTexture(video::SColor(255, 255, 255, 255)));
    m.setTexture(1, getUnicolorTexture(video::SColor(0, 0, 0, 0)));
    m.setTexture(7, getUnicolorTexture(video::SColor(0, 0, 0, 0)));
    m_mesh             = irr_driver->createQuadMesh(&m);
    m_mesh_buffer      = m_mesh->getMeshBuffer(0);
    assert(m_mesh_buffer->getVertexType()==video::EVT_STANDARD);

    unsigned int n = 0;
    const unsigned int total_nodes = getNumNodes();

    // Count the number of quads to display (some quads might be invisible)
    for (unsigned int i = 0; i < total_nodes; i++)
    {
        if (show_invisible || !isNodeInvisible(i))
            n++;
    }

    // Four vertices for each of the n-1 remaining quads
    video::S3DVertex *new_v = new video::S3DVertex[4*n];
    // Each quad consists of 2 triangles with 3 elements, so
    // we need 2*3 indices for each quad.
    irr::u16         *ind   = new irr::u16[6*n];
    video::SColor     c(255, 255, 0, 0);

    if (track_color)
        c = *track_color;

    // Now add all quads
    int i = 0;
    for (unsigned int count = 0; count < total_nodes; count++)
    {
        // Ignore invisible quads
        if (!show_invisible && isNodeInvisible(count))
            continue;

        // Swap the colours from red to blue and back
        if (!track_color)
        {
            c.setRed ((i%2) ? 255 : 0);
            c.setBlue((i%2) ? 0 : 255);
        }

        NodeColor nc = COLOR_RED;
        const bool different_color = differentNodeColor(count, &nc);
        // Transfer the 4 points of the current quad to the list of vertices
        set3DVerticesOfGraph(count, new_v+4*i, (different_color ?
            (nc == COLOR_RED ? video::SColor(255, 255, 0, 0) :
            video::SColor(255, 0, 0, 255)) : c));

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
    }

    m_mesh_buffer->append(new_v, n*4, ind, n*6);

    if (hasLapLine())
    {
        video::S3DVertex lap_v[4];
        irr::u16         lap_ind[6];
        video::SColor    lap_color(128, 255, 0, 0);
        set3DVerticesOfGraph(0, lap_v, lap_color);

        // Now scale the length (distance between vertix 0 and 3
        // and between 1 and 2) to be 'length':
        Vec3 bb_min, bb_max;
        getGraphBoundingBox(&bb_min, &bb_max);
        // Length of the lap line about 3% of the 'height'
        // of the track.
        const float length = (bb_max.getZ()-bb_min.getZ())*0.03f;

        core::vector3df dl = lap_v[3].Pos-lap_v[0].Pos;
        float ll2 = dl.getLengthSQ();
        if (ll2 < 0.001)
            lap_v[3].Pos = lap_v[0].Pos+core::vector3df(0, 0, 1);
        else
            lap_v[3].Pos = lap_v[0].Pos+dl*length/sqrt(ll2);

        core::vector3df dr = lap_v[2].Pos-lap_v[1].Pos;
        float lr2 = dr.getLengthSQ();
        if (lr2 < 0.001)
            lap_v[2].Pos = lap_v[1].Pos+core::vector3df(0, 0, 1);
        else
            lap_v[2].Pos = lap_v[1].Pos+dr*length/sqrt(lr2);
        lap_ind[0] = 2;
        lap_ind[1] = 1;
        lap_ind[2] = 0;
        lap_ind[3] = 3;
        lap_ind[4] = 2;
        lap_ind[5] = 0;
        // Set it a bit higher to avoid issued with z fighting,
        // i.e. part of the lap line might not be visible.
        for (unsigned int i = 0; i < 4; i++)
            lap_v[i].Pos.Y += 0.1f;
#ifndef USE_TEXTURED_LINE
        m_mesh_buffer->append(lap_v, 4, lap_ind, 6);
#else
        lap_v[0].TCoords = core::vector2df(0,0);
        lap_v[1].TCoords = core::vector2df(3,0);
        lap_v[2].TCoords = core::vector2df(3,1);
        lap_v[3].TCoords = core::vector2df(0,1);
        m_mesh_buffer->append(lap_v, 4, lap_ind, 6);
        video::SMaterial &m = m_mesh_buffer->getMaterial();
        video::ITexture *t = irr_driver->getTexture("chess.png");
        m.setTexture(0, t);
#endif
    }

    // Instead of setting the bounding boxes, we could just disable culling,
    // since the debug track should always be drawn.
    //m_node->setAutomaticCulling(scene::EAC_OFF);
    m_mesh_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_mesh_buffer->getBoundingBox());

    m_mesh_buffer->getMaterial().setTexture(0, irr_driver->getTexture("unlit.png"));

    delete[] ind;
    delete[] new_v;
}   // createMesh

// -----------------------------------------------------------------------------
/** Takes a snapshot of the graph so they can be used as minimap.
 */
RenderTarget* GraphStructure::makeMiniMap(const core::dimension2du &dimension,
                                          const std::string &name,
                                          const video::SColor &fill_color)
{
    // Skip minimap when profiling
    if (ProfileWorld::isNoGraphics()) return NULL;

    const video::SColor oldClearColor = World::getWorld()->getClearColor();
    World::getWorld()->setClearbackBufferColor(video::SColor(0, 255, 255, 255));
    World::getWorld()->forceFogDisabled(true);

    m_render_target = irr_driver->createRenderTarget(dimension, name);

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 255, 255, 255));

    createMesh(/*show_invisible part of the track*/ false,
               /*enable_transparency*/ false,
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
    getGraphBoundingBox(&bb_min, &bb_max);
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
    if (dz > dx)
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

    m_render_target->renderToTexture(camera, GUIEngine::getLatestDt());

    cleanupDebugMesh();
    irr_driver->removeCameraSceneNode(camera);
    m_min_coord = bb_min;

    World::getWorld()->setClearbackBufferColor(oldClearColor);
    World::getWorld()->forceFogDisabled(false);

    irr_driver->getSceneManager()->clear();
    VAOManager::kill();
    irr_driver->clearGlowingNodes();
    irr_driver->clearLights();
    irr_driver->clearForcedBloom();
    irr_driver->clearBackgroundNodes();
    
    return m_render_target.get();
}   // makeMiniMap

// -----------------------------------------------------------------------------
/** Returns the 2d coordinates of a point when drawn on the mini map
 *  texture.
 *  \param xyz Coordinates of the point to map.
 *  \param draw_at The coordinates in pixel on the mini map of the point,
 *         only the first two coordinates will be used.
 */
void GraphStructure::mapPoint2MiniMap(const Vec3 &xyz,Vec3 *draw_at) const
{
    draw_at->setX((xyz.getX()-m_min_coord.getX())*m_scaling);
    draw_at->setY((xyz.getZ()-m_min_coord.getZ())*m_scaling);

}   // mapPoint
