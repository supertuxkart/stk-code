//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart Team
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

#include "tracks/graph.hpp"

#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/render_target.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "guiengine/engine.hpp"
#include "race/race_manager.hpp"
#include "tracks/arena_node_3d.hpp"
#include "tracks/drive_node_2d.hpp"
#include "tracks/drive_node_3d.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"

#include <ICameraSceneNode.h>
#include <ISceneManager.h>

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#endif

const int Graph::UNKNOWN_SECTOR = -1;
const float Graph::MIN_HEIGHT_TESTING = -1.0f;
const float Graph::MAX_HEIGHT_TESTING = 5.0f;
Graph *Graph::m_graph = NULL;
// -----------------------------------------------------------------------------
Graph::Graph()
{
    m_scaling     = 0;
    m_node        = NULL;
    m_mesh        = NULL;
    m_mesh_buffer = NULL;
    m_render_target = NULL;
    m_bb_min      = Vec3( 99999,  99999,  99999);
    m_bb_max      = Vec3(-99999, -99999, -99999);
    memset(m_bb_nodes, 0, 4 * sizeof(int));
}  // Graph

// -----------------------------------------------------------------------------
Graph::~Graph()
{
    if (UserConfigParams::m_track_debug)
        cleanupDebugMesh();

    for (unsigned int i = 0; i < m_all_nodes.size(); i++)
    {
        delete m_all_nodes[i];
    }
    m_all_nodes.clear();
}  // ~Graph

// -----------------------------------------------------------------------------
/** Creates the debug mesh to display the graph on top of the track
 *  model. */
void Graph::createDebugMesh()
{
    if (getNumNodes() <= 0) return;  // no debug output if not graph

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        createMeshSP(/*show_invisible*/true,
                     /*enable_transparency*/true);
        video::S3DVertexSkinnedMesh *v =
            (video::S3DVertexSkinnedMesh*)m_mesh_buffer->getVertices();
        for (unsigned int i = 0; i < m_mesh_buffer->getVertexCount(); i++)
        {
            // Swap the alpha and back
            v[i].m_color.setAlpha((i%2) ? 64 : 255);
        }
    }
    else
    {
        createMesh(/*show_invisible*/true,
                   /*enable_transparency*/true);
        video::S3DVertex *v = (video::S3DVertex*)m_mesh_buffer->getVertices();
        for (unsigned int i = 0; i < m_mesh_buffer->getVertexCount(); i++)
        {
            // Swap the alpha and back
            v[i].Color.setAlpha((i%2) ? 64 : 255);
        }
    }
#endif

#ifndef SERVER_ONLY
    bool vk = (GE::getVKDriver() != NULL);
    if (vk)
        GE::getGEConfig()->m_convert_irrlicht_mesh = true;
#endif
    m_node = irr_driver->addMesh(m_mesh, "track-debug-mesh");
#ifndef SERVER_ONLY
    if (vk)
        GE::getGEConfig()->m_convert_irrlicht_mesh = false;
#endif

#ifdef DEBUG
    m_node->setName("track-debug-mesh");
#endif

}   // createDebugMesh

// -----------------------------------------------------------------------------
/** Cleans up the debug mesh */
void Graph::cleanupDebugMesh()
{
    if (m_node != NULL)
        irr_driver->removeNode(m_node);

    m_node = NULL;
    // No need to call irr_driber->removeMeshFromCache, since the mesh
    // was manually made and so never added to the mesh cache.
    m_mesh->drop();
    m_mesh = NULL;
}   // cleanupDebugMesh

// -----------------------------------------------------------------------------
/** Creates the actual mesh that is used by createDebugMesh() or makeMiniMap()
 */
void Graph::createMesh(bool show_invisible, bool enable_transparency,
                       const video::SColor *track_color, bool invert_x_z,
                       bool flatten)
{
#ifndef SERVER_ONLY
    // The debug track will not be lighted or culled.
    video::SMaterial m;
    m.BackfaceCulling  = false;
    m.Lighting         = false;
    if (enable_transparency)
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    m_mesh             = irr_driver->createQuadMesh(&m);
    m_mesh_buffer      = m_mesh->getMeshBuffer(0);
    assert(m_mesh_buffer->getVertexType()==video::EVT_STANDARD);

    unsigned int n = 0;
    const unsigned int total_nodes = getNumNodes();

    // Count the number of quads to display (some quads might be invisible)
    for (unsigned int i = 0; i < total_nodes; i++)
    {
        if (show_invisible || !m_all_nodes[i]->isInvisible())
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
        if (!show_invisible && m_all_nodes[count]->isInvisible())
            continue;

        // Swap the colours from red to blue and back
        if (!track_color)
        {
            c.setRed ((i%2) ? 255 : 0);
            c.setBlue((i%2) ? 0 : 255);
        }

        video::SColor this_color = c;
        differentNodeColor(count, &this_color);
        // Transfer the 4 points of the current quad to the list of vertices
        m_all_nodes[count]->getVertices(new_v+4*i, this_color);
        auto* vptr = new_v + 4 * i;
        if (invert_x_z)
        {
            vptr[0].Pos.X = -vptr[0].Pos.X;
            vptr[0].Pos.Z = -vptr[0].Pos.Z;
            vptr[1].Pos.X = -vptr[1].Pos.X;
            vptr[1].Pos.Z = -vptr[1].Pos.Z;
            vptr[2].Pos.X = -vptr[2].Pos.X;
            vptr[2].Pos.Z = -vptr[2].Pos.Z;
            vptr[3].Pos.X = -vptr[3].Pos.X;
            vptr[3].Pos.Z = -vptr[3].Pos.Z;
        }
        if (flatten)
        {
            // Vulkan driver needs 0.1 instead of 0
            vptr[0].Pos.Y = 0.1f;
            vptr[1].Pos.Y = 0.1f;
            vptr[2].Pos.Y = 0.1f;
            vptr[3].Pos.Y = 0.1f;
        }

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
        m_all_nodes[0]->getVertices(lap_v, lap_color);
        if (flatten)
        {
            lap_v[0].Pos.Y = 0.1f;
            lap_v[1].Pos.Y = 0.1f;
            lap_v[2].Pos.Y = 0.1f;
            lap_v[3].Pos.Y = 0.1f;
        }

        // Now scale the length (distance between vertix 0 and 3
        // and between 1 and 2) to be 'length':
        // Length of the lap line about 3% of the 'height'
        // of the track.
        const float length = (m_bb_max.getZ()-m_bb_min.getZ())*0.03f;

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
        m_mesh_buffer->append(lap_v, 4, lap_ind, 6);
    }

    // Instead of setting the bounding boxes, we could just disable culling,
    // since the debug track should always be drawn.
    //m_node->setAutomaticCulling(scene::EAC_OFF);
    m_mesh_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_mesh_buffer->getBoundingBox());

    m_mesh_buffer->getMaterial().setTexture(0, irr_driver
        ->getTexture("unlit.png"));

    delete[] ind;
    delete[] new_v;
#endif
}   // createMesh

// -----------------------------------------------------------------------------
/** Creates the actual mesh that is used by createDebugMesh() or makeMiniMap()
 */
void Graph::createMeshSP(bool show_invisible, bool enable_transparency,
                         const video::SColor *track_color, bool invert_x_z)
{
#ifndef SERVER_ONLY

    SP::SPMesh* spm = new SP::SPMesh();
    SP::SPMeshBuffer* spmb = new SP::SPMeshBuffer();
    m_mesh = spm;
    m_mesh_buffer = spmb;

    unsigned int n = 0;
    const unsigned int total_nodes = getNumNodes();

    // Count the number of quads to display (some quads might be invisible)
    for (unsigned int i = 0; i < total_nodes; i++)
    {
        if (show_invisible || !m_all_nodes[i]->isInvisible())
            n++;
    }

    // Four vertices for each of the n-1 remaining quads
    std::vector<video::S3DVertexSkinnedMesh> vertices;
    vertices.resize(4 * n);
    // Each quad consists of 2 triangles with 3 elements, so
    // we need 2*3 indices for each quad.
    std::vector<uint16_t> indices;
    indices.resize(6 * n);
    video::SColor c(255, 255, 0, 0);

    if (track_color)
        c = *track_color;

    // Now add all quads
    int i = 0;
    for (unsigned int count = 0; count < total_nodes; count++)
    {
        // Ignore invisible quads
        if (!show_invisible && m_all_nodes[count]->isInvisible())
            continue;

        // Swap the colours from red to blue and back
        if (!track_color)
        {
            c.setRed ((i % 2) ? 255 : 0);
            c.setBlue((i % 2) ? 0 : 255);
        }

        video::SColor this_color = c;
        differentNodeColor(count, &this_color);
        // Transfer the 4 points of the current quad to the list of vertices
        m_all_nodes[count]->getSPMVertices(vertices.data() + (4 * i), this_color);
        if (invert_x_z)
        {
            auto* vptr = vertices.data() + (4 * i);
            vptr[0].m_position.X = -vptr[0].m_position.X;
            vptr[0].m_position.Z = -vptr[0].m_position.Z;
            vptr[1].m_position.X = -vptr[1].m_position.X;
            vptr[1].m_position.Z = -vptr[1].m_position.Z;
            vptr[2].m_position.X = -vptr[2].m_position.X;
            vptr[2].m_position.Z = -vptr[2].m_position.Z;
            vptr[3].m_position.X = -vptr[3].m_position.X;
            vptr[3].m_position.Z = -vptr[3].m_position.Z;
        }

        // Set up the indices for the triangles
        indices[6 * i] = 4 * i + 2;  // First triangle: vertex 0, 1, 2
        indices[6 * i + 1] = 4 * i + 1;
        indices[6 * i + 2] = 4 * i;
        indices[6 * i + 3] = 4 * i + 3;  // second triangle: vertex 0, 1, 3
        indices[6 * i + 4] = 4 * i + 2;
        indices[6 * i + 5] = 4 * i;
        i++;
    }

    if (hasLapLine())
    {
        video::S3DVertexSkinnedMesh lap_v[4];
        uint16_t lap_ind[6];
        video::SColor lap_color(128, 255, 0, 0);
        m_all_nodes[0]->getSPMVertices(lap_v, lap_color);

        // Now scale the length (distance between vertix 0 and 3
        // and between 1 and 2) to be 'length':
        // Length of the lap line about 3% of the 'height'
        // of the track.
        const float length = (m_bb_max.getZ() - m_bb_min.getZ()) * 0.03f;

        core::vector3df dl = lap_v[3].m_position-lap_v[0].m_position;
        float ll2 = dl.getLengthSQ();
        if (ll2 < 0.001)
            lap_v[3].m_position = lap_v[0].m_position + core::vector3df(0, 0, 1);
        else
            lap_v[3].m_position = lap_v[0].m_position + dl * length / sqrt(ll2);

        core::vector3df dr = lap_v[2].m_position - lap_v[1].m_position;
        float lr2 = dr.getLengthSQ();
        if (lr2 < 0.001)
            lap_v[2].m_position = lap_v[1].m_position + core::vector3df(0, 0, 1);
        else
            lap_v[2].m_position = lap_v[1].m_position + dr * length / sqrt(lr2);
        lap_ind[0] = 4 * n + 2;
        lap_ind[1] = 4 * n + 1;
        lap_ind[2] = 4 * n;
        lap_ind[3] = 4 * n + 3;
        lap_ind[4] = 4 * n + 2;
        lap_ind[5] = 4 * n;
        // Set it a bit higher to avoid issued with z fighting,
        // i.e. part of the lap line might not be visible.
        for (unsigned int i = 0; i < 4; i++)
            lap_v[i].m_position.Y += 0.1f;
        std::copy(lap_v, lap_v + 4, std::back_inserter(vertices));
        std::copy(lap_ind, lap_ind +6, std::back_inserter(indices));
    }

    spmb->setSPMVertices(vertices);
    spmb->setIndices(indices);
    spmb->recalculateBoundingBox();
    std::string shader_name = enable_transparency ? "alphablend" : "unlit";
#ifndef SERVER_ONLY
    if (!CVS->isDeferredEnabled())
    {
        shader_name = "solid";
    }
#endif
    spmb->setSTKMaterial(material_manager->getDefaultSPMaterial(shader_name));
    spm->addSPMeshBuffer(spmb);
    spm->setBoundingBox(spmb->getBoundingBox());
#endif
}   // createMeshSP

// -----------------------------------------------------------------------------
/** Takes a snapshot of the graph so they can be used as minimap.
 */
RenderTarget* Graph::makeMiniMap(const core::dimension2du &dimension,
                                 const std::string &name,
                                 const video::SColor &fill_color,
                                 bool invert_x_z)
{
    // Skip minimap when profiling
#ifdef SERVER_ONLY
    return NULL;
#else
    if (GUIEngine::isNoGraphics()) return NULL;
#endif

    const video::SColor oldClearColor = irr_driver->getClearColor();
    irr_driver->setClearbackBufferColor(video::SColor(0, 255, 255, 255));
    Track::getCurrentTrack()->forceFogDisabled(true);
#ifndef SERVER_ONLY
    m_render_target = irr_driver->createRenderTarget(dimension, name);
#endif
    irr_driver->getSceneManager()
        ->setAmbientLight(video::SColor(255, 255, 255, 255));

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        createMeshSP(/*show_invisible part of the track*/ false,
            /*enable_transparency*/ false, /*track_color*/&fill_color,
            invert_x_z);
    }
    else
    {
        createMesh(/*show_invisible part of the track*/ false,
            /*enable_transparency*/ false, /*track_color*/&fill_color,
            invert_x_z, true/*flatten*/);
    }
#endif

    // Adjust bounding boxes for flags in CTF
    if (Track::getCurrentTrack()->isCTF() &&
        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
    {
        Vec3 red_flag = Track::getCurrentTrack()->getRedFlag().getOrigin();
        Vec3 blue_flag = Track::getCurrentTrack()->getBlueFlag().getOrigin();
        // In case the flag is placed outside of the graph, we scale it a bit
        red_flag *= 1.1f;
        blue_flag *= 1.1f;
        m_bb_max.max(red_flag);
        m_bb_max.max(blue_flag);
        m_bb_min.min(red_flag);
        m_bb_min.min(blue_flag);
    }

    Vec3 bb_min = m_bb_min;
    Vec3 bb_max = m_bb_max;
#ifndef SERVER_ONLY
    if (!CVS->isGLSL())
    {
        // Flatten the minimap for DirectX 9 driver, otherwise some vertices
        // too far from camera will not be rendered
        bb_min.setY(0);
        bb_max.setY(0);
    }
#endif

#ifndef SERVER_ONLY
    bool vk = (GE::getVKDriver() != NULL);
    if (vk)
        GE::getGEConfig()->m_convert_irrlicht_mesh = true;
#endif
    m_node = irr_driver->addMesh(m_mesh, "mini_map");
#ifndef SERVER_ONLY
    if (vk)
        GE::getGEConfig()->m_convert_irrlicht_mesh = false;
#endif

#ifdef DEBUG
    m_node->setName("minimap-mesh");
#endif

    m_node->setAutomaticCulling(0);
    m_node->setMaterialFlag(video::EMF_LIGHTING, false);

    // Add the camera:
    // ---------------
    scene::ICameraSceneNode *camera = irr_driver->addCameraSceneNode();
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
    projection.buildProjectionMatrixOrthoLH
        (range /* width */, range /* height */, -1,
        bb_max.getY()-bb_min.getY()+1);
    camera->setProjectionMatrix(projection, true);

    irr_driver->suppressSkyBox();
    irr_driver->clearLights();

    // Adjust Y position by +1 for max, -1 for min - this helps in case that
    // the maximum Y coordinate is negative (otherwise the minimap is mirrored)
    // and avoids problems for tracks which have a flat (max Y=min Y) minimap.
    camera->setPosition(core::vector3df(center.getX(), bb_min.getY() + 1.0f,
        center.getZ()));
    //camera->setPosition(core::vector3df(center.getX() - 5.0f,
    //    bb_min.getY() - 1 - 5.0f, center.getZ() - 15.0f));
    camera->setUpVector(core::vector3df(0, 0, 1));
    camera->setTarget(core::vector3df(center.getX(), bb_min.getY() - 1,
        center.getZ()));
    //camera->setAspectRatio(1.0f);
    camera->updateAbsolutePosition();

    m_render_target->renderToTexture(camera, GUIEngine::getLatestDt());

    cleanupDebugMesh();
    irr_driver->removeCameraSceneNode(camera);

    irr_driver->setClearbackBufferColor(oldClearColor);
    Track::getCurrentTrack()->forceFogDisabled(false);

    irr_driver->getSceneManager()->clear();
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
void Graph::mapPoint2MiniMap(const Vec3 &xyz,Vec3 *draw_at) const
{
    draw_at->setX((xyz.getX()-m_bb_min.getX())*m_scaling);
    draw_at->setY((xyz.getZ()-m_bb_min.getZ())*m_scaling);

}   // mapPoint

// -----------------------------------------------------------------------------
void Graph::createQuad(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2,
                       const Vec3 &p3, unsigned int node_index,
                       bool invisible, bool ai_ignore, bool is_arena,
                       bool ignored)
{
    // Find the normal of this quad by computing the normal of two triangles
    // and taking their average.
    core::triangle3df tri1(p0.toIrrVector(), p1.toIrrVector(),
        p2.toIrrVector());
    core::triangle3df tri2(p0.toIrrVector(), p2.toIrrVector(),
        p3.toIrrVector());
    Vec3 normal1 = tri1.getNormal();
    Vec3 normal2 = tri2.getNormal();
    Vec3 normal = -0.5f * (normal1 + normal2);
    normal.normalize();

    // Use the angle between the normal and an up vector to choose 3d/2d quad
    const float angle = normal.angle(Vec3(0, 1, 0));

    Quad* q = NULL;
    if (angle > 0.5f)
    {
        Log::debug("Graph", "3d node created, normal: %f, %f, %f",
            normal.x(), normal.y(), normal.z());
        if (is_arena)
        {
            q = new ArenaNode3D(p0, p1, p2, p3, normal, node_index);
        }
        else
        {
            q = new DriveNode3D(p0, p1, p2, p3, normal, node_index, invisible,
                ai_ignore, ignored);
        }
    }
    else
    {
#ifndef SERVER_ONLY
        Log::debug("Graph", "2d node created, normal: %f, %f, %f",
            normal.x(), normal.y(), normal.z());
#endif
        if (is_arena)
        {
            q = new ArenaNode(p0, p1, p2, p3, normal, node_index);
        }
        else
        {
            q = new DriveNode2D(p0, p1, p2, p3, normal, node_index, invisible,
                ai_ignore, ignored);
        }
    }
    m_all_nodes.push_back(q);

    m_bb_max.max(p0); m_bb_max.max(p1); m_bb_max.max(p2); m_bb_max.max(p3);
    m_bb_min.min(p0); m_bb_min.min(p1); m_bb_min.min(p2); m_bb_min.min(p3);

}   // createQuad

//-----------------------------------------------------------------------------
/** findRoadSector returns in which sector on the road the position
 *  xyz is. If xyz is not on top of the road, it sets UNKNOWN_SECTOR as sector.
 *
 *  \param xyz Position for which the segment should be determined.
 *  \param sector Contains the previous sector (as a shortcut, since usually
 *         the sector is the same as the last one), and on return the result
 *  \param all_sectors If this is not NULL, it is a list of all sectors to
 *         test. This is used by the AI to make sure that it ends up on the
 *         selected way in case of a branch, and also to make sure that it
 *         doesn't skip e.g. a loop (see explanation below for details).
 */
void Graph::findRoadSector(const Vec3& xyz, int *sector,
                           std::vector<int> *all_sectors,
                           bool ignore_vertical) const
{
    // Most likely the kart will still be on the sector it was before,
    // so this simple case is tested first.
    if (*sector!=UNKNOWN_SECTOR &&
        getQuad(*sector)->pointInside(xyz, ignore_vertical))
    {
        return;
    }   // if still on same quad

    // Now we search through all quads, starting with
    // the current one
    int indx       = *sector;

    // If a current sector is given, and max_lookahead is specify, only test
    // the next max_lookahead quads instead of testing the whole graph.
    // This is necessary for the AI: if the track contains a loop, e.g.:
    // -A--+---B---+----F--------
    //     E       C
    //     +---D---+
    // and the track is supposed to be driven: ABCDEBF, the AI might find
    // the quad on F, and then keep on going straight ahead instead of
    // using the loop at all.
    unsigned int max_count  = (*sector!=UNKNOWN_SECTOR && all_sectors!=NULL)
                            ? (unsigned int)all_sectors->size()
                            : (unsigned int)m_all_nodes.size();
    *sector = UNKNOWN_SECTOR;
    for(unsigned int i=0; i<max_count; i++)
    {
        if(all_sectors)
            indx = (*all_sectors)[i];
        else
            indx = indx<(int)m_all_nodes.size()-1 ? indx +1 : 0;
        const Quad* q = getQuad(indx);
        if (q->pointInside(xyz, ignore_vertical))
        {
            *sector  = indx;
            return;
        }
    }   // for i<m_all_nodes.size()

    return;
}   // findRoadSector

//-----------------------------------------------------------------------------
/** findOutOfRoadSector finds the sector where XYZ is, but as it name
    implies, it is more accurate for the outside of the track than the
    inside, and for STK's needs the accuracy on top of the track is
    unacceptable; but if this was a 2D function, the accuracy for out
    of road sectors would be perfect.

    To find the sector we look for the closest line segment from the
    right and left drivelines, and the number of that segment will be
    the sector.

    The SIDE argument is used to speed up the function only; if we know
    that XYZ is on the left or right side of the track, we know that
    the closest driveline must be the one that matches that condition.
    In reality, the side used in STK is the one from the previous frame,
    but in order to move from one side to another a point would go
    through the middle, that is handled by findRoadSector() which doesn't
    has speed ups based on the side.

    NOTE: This method of finding the sector outside of the road is *not*
    perfect: if two line segments have a similar altitude (but enough to
    let a kart get through) and they are very close on a 2D system,
    if a kart is on the air it could be closer to the top line segment
    even if it is supposed to be on the sector of the lower line segment.
    Probably the best solution would be to construct a quad that reaches
    until the next higher overlapping line segment, and find the closest
    one to XYZ.
 */
int Graph::findOutOfRoadSector(const Vec3& xyz, const int curr_sector,
                               std::vector<int> *all_sectors,
                               bool ignore_vertical) const
{
    int count = (all_sectors!=NULL) ? (int)all_sectors->size() : getNumNodes();
    int current_sector = 0;
    if(curr_sector != UNKNOWN_SECTOR && !all_sectors)
    {
        // We have to test all quads here: reason is that on track with
        // shortcuts the n quads of the main drivelines is followed by
        // the quads of the shortcuts. So after quad n-1 (the last one
        // before the lap counting line) quad n will not be 0 (the first
        // quad after the lap counting line), but one of the quads on a
        // shortcut. If we only tested a limited number of quads to
        // improve the performance the crossing of a lap might not be
        // detected (because quad 0 is not tested, only quads on the
        // shortcuts are tested). If this should become a performance
        // bottleneck, we need to set up a graph of 'next' quads for each
        // quad (similar to what the AI does), and only test the quads
        // in this graph.
        const int LIMIT = getNumNodes();
        count           = LIMIT;
        // Start 10 quads before the current quad, so the quads closest
        // to the current position are tested first.
        current_sector  = curr_sector -10;
        if(current_sector<0) current_sector += getNumNodes();
    }

    int   min_sector = UNKNOWN_SECTOR;
    float min_dist_2 = 999999.0f*999999.0f;

    // If a kart is falling and in between (or too far below)
    // a driveline point it might not fulfill
    // the height condition. So we run the test twice: first with height
    // condition, then again without the height condition - just to make sure
    // it always comes back with some kind of quad.
    for(int phase=0; phase<2; phase++)
    {
        for(int j=0; j<count; j++)
        {
            int next_sector;
            if(all_sectors)
                next_sector = (*all_sectors)[j];
            else
                next_sector  = current_sector+1 == (int)getNumNodes()
                ? 0
                : current_sector+1;

            const Quad* q = getQuad(next_sector);
            if (!q->isIgnored())
            {
                // A first simple test uses the 2d distance to the center of the
                // quad.
                float dist_2 =
                    m_all_nodes[next_sector]->getDistance2FromPoint(xyz);
                if (dist_2 < min_dist_2)
                {
                    float dist = xyz.getY() - q->getMinHeight();
                    // While negative distances are unlikely, we allow some small
                    // negative numbers in case that the kart is partly in the
                    // track. Only do the height test in phase==0, in phase==1
                    // accept any point, independent of height, or this node is 3d
                    // which already takes height into account
                    if (phase == 1 || (dist < 5.0f && dist>-1.0f) ||
                        q->is3DQuad() || ignore_vertical)
                    {
                        min_dist_2 = dist_2;
                        min_sector = next_sector;
                    }
                }
            }
            current_sector = next_sector;
        }   // for j
        // If any sector was found after a phase, return it.
        if(min_sector!=UNKNOWN_SECTOR)
            return min_sector;
    }   // phase
    
    // We can only reach this point if min_sector==UNKNOWN_SECTOR
    Log::warn("Graph", "unknown sector found.");
    return 0;
}   // findOutOfRoadSector

//-----------------------------------------------------------------------------
void Graph::loadBoundingBoxNodes()
{
    m_bb_nodes[0] = findOutOfRoadSector(Vec3(m_bb_min.x(), 0, m_bb_min.z()),
        -1/*curr_sector*/, NULL/*all_sectors*/, true/*ignore_vertical*/);
    m_bb_nodes[1] = findOutOfRoadSector(Vec3(m_bb_min.x(), 0, m_bb_max.z()),
        -1/*curr_sector*/, NULL/*all_sectors*/, true/*ignore_vertical*/);
    m_bb_nodes[2] = findOutOfRoadSector(Vec3(m_bb_max.x(), 0, m_bb_min.z()),
        -1/*curr_sector*/, NULL/*all_sectors*/, true/*ignore_vertical*/);
    m_bb_nodes[3] = findOutOfRoadSector(Vec3(m_bb_max.x(), 0, m_bb_max.z()),
        -1/*curr_sector*/, NULL/*all_sectors*/, true/*ignore_vertical*/);
}   // loadBoundingBoxNodes
