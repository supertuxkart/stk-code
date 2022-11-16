//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Joerg Henrichs
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
#include "graphics/central_settings.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "modes/world.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"
#include "tracks/quad.hpp"
#include "utils/constants.hpp"
#include "mini_glm.hpp"

#include <IMeshCache.h>
#include <ISceneManager.h>
#include <SMeshBuffer.h>
#include <SMesh.h>

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#include <ge_render_info.hpp>
#endif

//-----------------------------------------------------------------------------
const char* g_slipstream_textures[3] =
{
    "slipstream.png", "slipstream2.png", "slipstream_bonus.png"
};
//-----------------------------------------------------------------------------
/** Creates the slip stream object
 *  \param kart Pointer to the kart to which the slip stream
 *              belongs to.
 */
SlipStream::SlipStream(AbstractKart* kart)
{
    m_speed_increase_ticks = m_speed_increase_duration = -1;
    m_kart = kart;
    m_moving = NULL;
    m_moving_fast = NULL;
    m_moving_bonus = NULL;
    m_node = NULL;
    m_node_fast = NULL;
    m_bonus_node = NULL;
    m_length = 0.0f;

#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
    {
        m_moving = new MovingTexture(0.0f, 0.0f);

        scene::IAnimatedMesh* mesh = NULL;
        if (CVS->isGLSL())
            mesh = createMeshSP(0, false);
        else
            mesh = createMesh(0, false);
        m_node = irr_driver->addMesh(mesh, "slipstream");
        mesh->drop();
        std::string debug_name = m_kart->getIdent()+" (slip-stream)";
        m_node->setName(debug_name.c_str());
        m_node->setPosition(core::vector3df(0, 0 * 0.25f + 2.5f,
            m_kart->getKartLength()));
        m_node->setVisible(false);
        SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(m_node);
        if (spmn)
            m_moving->setSPTM(spmn->getTextureMatrix(0).data());
        else
        {
            m_moving->setTextureMatrix(&(m_node->getMaterial(0)
                .getTextureMatrix(0)));
            m_node->getMaterial(0).getRenderInfo() =
                std::make_shared<GE::GERenderInfo>();
        }

        m_moving_fast = new MovingTexture(0.0f, 0.0f);

        if (CVS->isGLSL())
            mesh = createMeshSP(1, false);
        else
            mesh = createMesh(1, false);
        m_node_fast = irr_driver->addMesh(mesh, "slipstream2");
        mesh->drop();
        debug_name = m_kart->getIdent()+" (slip-stream2)";
        m_node_fast->setName(debug_name.c_str());
        m_node_fast->setPosition(core::vector3df(0, 0 * 0.25f + 2.5f,
            m_kart->getKartLength()));
        m_node_fast->setVisible(false);
        spmn = dynamic_cast<SP::SPMeshNode*>(m_node_fast);
        if (spmn)
            m_moving_fast->setSPTM(spmn->getTextureMatrix(0).data());
        else
        {
            m_moving_fast->setTextureMatrix(&(m_node_fast->getMaterial(0)
                .getTextureMatrix(0)));
            m_node_fast->getMaterial(0).getRenderInfo() =
                std::make_shared<GE::GERenderInfo>();
        }

        m_moving_bonus = new MovingTexture(0.0f, 0.0f);

        if (CVS->isGLSL())
            mesh = createMeshSP(2, true);
        else
            mesh = createMesh(2, true);
        m_bonus_node = irr_driver->addMesh(mesh, "slipstream-bonus");
        mesh->drop();
        debug_name = m_kart->getIdent()+" (slip-stream-bonus)";
        m_bonus_node->setName(debug_name.c_str());
        m_bonus_node->setPosition(core::vector3df(0, 0 * 0.25f + 2.5f,
            m_kart->getKartLength()));
        m_bonus_node->setVisible(true);
        spmn = dynamic_cast<SP::SPMeshNode*>(m_bonus_node);
        if (spmn)
            m_moving_bonus->setSPTM(spmn->getTextureMatrix(0).data());
        else
        {
            m_moving_bonus->setTextureMatrix(&(m_bonus_node->getMaterial(0)
                .getTextureMatrix(0)));
            m_bonus_node->getMaterial(0).getRenderInfo() =
                std::make_shared<GE::GERenderInfo>();
        }
    }
#endif

    m_slipstream_time      = 0.0f;
    m_bonus_time           = 0.0f;
    m_bonus_active         = false;
    m_current_target_id    = -1;//should not match a real possible kart ID
    m_previous_target_id   = -1;

    //The kart starts at 0 speed anyway
    float length = 0.0f;
    float kw     = m_kart->getKartWidth();
    float ew     = 0.0f;
    float kl     = m_kart->getKartLength();

    //making the slipstream quad start at the kart front
    //allows better results when the kart turns
    Vec3 p[4];
    p[0]=Vec3(-kw*0.5f, 0, kl*0.5f );
    p[1]=Vec3(-ew*0.5f, 0, -kl*0.5f-length);
    p[2]=Vec3( ew*0.5f, 0, -kl*0.5f-length);
    p[3]=Vec3( kw*0.5f, 0, kl*0.5f );

    m_slipstream_quad          = new Quad(p[0], p[1], p[2], p[3]);
    //The position will be corrected in the update anyway
    m_slipstream_inner_quad    = new Quad(p[0], p[1], p[2], p[3]);
    //The position will be corrected in the update anyway
    m_slipstream_outer_quad    = new Quad(p[0], p[1], p[2], p[3]);
#ifndef SERVER_ONLY
    if (UserConfigParams::m_slipstream_debug && CVS->isGLSL())
    {
        m_debug_dc = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP,
            SP::SPShaderManager::get()->getSPShader("additive"),
            material_manager->getDefaultSPMaterial("additive"));
        m_debug_dc->getVerticesVector().resize(4);
        video::S3DVertexSkinnedMesh* v =
            m_debug_dc->getVerticesVector().data();
        video::SColor red(128, 255, 0, 0);
        unsigned idx[] = { 0, 3, 1, 2 };
        for (unsigned i = 0; i < 4; i++)
        {
            v[i].m_position = p[idx[i]].toIrrVector();
            v[i].m_normal = 0x1FF << 10;
            v[i].m_color = red;
        }
        m_debug_dc->recalculateBoundingBox();
        m_debug_dc->setParent(m_kart->getNode());
        SP::addDynamicDrawCall(m_debug_dc);

        m_debug_dc2 = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP,
            SP::SPShaderManager::get()->getSPShader("additive"),
            material_manager->getDefaultSPMaterial("additive"));
        m_debug_dc2->getVerticesVector().resize(4);
        v = m_debug_dc2->getVerticesVector().data();
        for (unsigned i = 0; i < 4; i++)
        {
            v[i].m_position = p[idx[i]].toIrrVector();
            v[i].m_normal = 0x1FF << 10;
            v[i].m_color = red;
        }
        m_debug_dc2->recalculateBoundingBox();
        m_debug_dc2->setParent(m_kart->getNode());
        SP::addDynamicDrawCall(m_debug_dc2);
    }
#endif
}   // SlipStream

//-----------------------------------------------------------------------------
/** Removes the node from the scene graph.
 */
SlipStream::~SlipStream()
{
    if (m_node)
    {
        irr_driver->removeNode(m_node);
    }
    if (m_node_fast)
    {
        irr_driver->removeNode(m_node_fast);
    }
    if (m_bonus_node)
    {
        irr_driver->removeNode(m_bonus_node);
    }
    if (m_debug_dc)
    {
        m_debug_dc->removeFromSP();
    }
    if (m_debug_dc2)
    {
        m_debug_dc2->removeFromSP();
    }
    delete m_slipstream_quad;
    delete m_slipstream_inner_quad;
    delete m_slipstream_outer_quad;
    delete m_moving;
    delete m_moving_fast;
    delete m_moving_bonus;
#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics() && !CVS->isGLSL())
    {
        scene::IMeshCache* mc = irr_driver->getSceneManager()->getMeshCache();
        for (const char* texture : g_slipstream_textures)
        {
            scene::IAnimatedMesh* amesh = mc->getMeshByName(texture);
            if (amesh && amesh->getReferenceCount() == 1)
                mc->removeMesh(amesh);
        }
    }
#endif
}   // ~SlipStream

//-----------------------------------------------------------------------------
/** Called at re-start of a race. */
void SlipStream::reset()
{
    m_slipstream_mode = SS_NONE;
    m_slipstream_time = 0;
    m_bonus_time      = 0;
    m_speed_increase_ticks = m_speed_increase_duration = -1;
    // Reset a potential max speed increase
    m_kart->increaseMaxSpeed(MaxSpeed::MS_INCREASE_SLIPSTREAM, 0, 0, 0, 0);
    hideAllNodes();
}   // reset

//-----------------------------------------------------------------------------
/** Creates the mesh for the slipstream effect. This function creates a
 *  first a series of circles (with a certain number of vertices each and
 *  distance from each other. Then it will create the triangles and add
 *  texture coordniates.
 *  \param material_id  The material to use.
 */
scene::IAnimatedMesh* SlipStream::createMesh(unsigned material_id,
                                             bool bonus_mesh)
{
    // All radius, starting with the one closest to the kart (and
    // widest) to the one furthest away. A 0 indicates the end of the list

    std::vector<float> radius = {1.5f, 1.0f, 0.5f, 0.0f};

    if (bonus_mesh)
    {
        radius = {0.9f,0.6f,0.3f,0.0f};
    }

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
    std::vector<float> distance = {2.0f, 6.0f, 10.0f };

    if (bonus_mesh)
    {
        distance = {0.4f, 0.8f, 1.2f };
    }

    // The alpha values for the rings, no 'end of list' entry required.
    int alphas[] = {0, 255, 0};

    // Loop through all given radius to determine the number
    // of segments to create.
    unsigned int num_circles=0;
    while(radius[num_circles]>0.0f) num_circles++;

    assert(num_circles > 0);

    // Length is distance of last circle to distance of first circle:
    float length = distance[num_circles-1] - distance[0];

    if (!bonus_mesh)
        m_length = length;

    const io::path cache_key = g_slipstream_textures[material_id];
    scene::IMeshCache* mc = irr_driver->getSceneManager()->getMeshCache();
    scene::IAnimatedMesh* amesh = mc->getMeshByName(cache_key);
    if (amesh)
    {
        amesh->grab();
        return amesh;
    }

    // The number of points for each circle. Since part of the slip stream
    // might be under the ground (esp. first and last segment), specify
    // which one is the first and last to be actually drawn.
    const unsigned int  num_segments   = 15;
    const unsigned int  first_segment  = 0;
    const unsigned int  last_segment   = 14;
    const float         f              = 2*M_PI/float(num_segments);
    scene::SMeshBuffer* buffer         = new scene::SMeshBuffer();
    Material* material                 = material_manager->getMaterialSPM(
                                         cache_key.c_str(), "");
    buffer->getMaterial().TextureLayer[0].Texture = material->getTexture();
    for(unsigned int j=0; j<num_circles; j++)
    {
        float curr_distance = distance[j]-distance[0];
        // Create the vertices for each of the circle
        for(unsigned int i=first_segment; i<=last_segment; i++)
        {
            video::S3DVertex v;
            // Offset every 2nd circle by one half segment to increase
            // the number of planes so it looks better.
            v.Pos.X =  sinf((i+(j%2)*0.5f)*f)*radius[j];
            v.Pos.Y = -cosf((i+(j%2)*0.5f)*f)*radius[j];
            v.Pos.Z = distance[j];
            v.Normal = core::vector3df(0.0f,1.0f,0.0f);
            v.Color = video::SColor(alphas[j],255,255,255);
            v.TCoords.X = curr_distance/length;
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

    material->setMaterialProperties(&buffer->getMaterial(), buffer);
    buffer->Material.setFlag(video::EMF_BACK_FACE_CULLING, false);
    buffer->Material.setFlag(video::EMF_COLOR_MATERIAL, true);
    buffer->Material.ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;

    scene::SMesh* mesh = new scene::SMesh();
    mesh->addMeshBuffer(buffer);
    mesh->recalculateBoundingBox();

    buffer->drop();
#ifndef SERVER_ONLY
    if (GE::getDriver()->getDriverType() == video::EDT_VULKAN)
    {
        amesh = GE::convertIrrlichtMeshToSPM(mesh);
        mesh->drop();
        mc->addMesh(cache_key, amesh);
        return amesh;
    }
    else
#endif
        mc->addMesh(cache_key, mesh);

    return mesh;
}   // createMesh

//-----------------------------------------------------------------------------
/** Creates the mesh for the slipstream effect. This function creates a
 *  first a series of circles (with a certain number of vertices each and
 *  distance from each other. Then it will create the triangles and add
 *  texture coordniates.
 *  \param material_id  The material to use.
 */
SP::SPMesh* SlipStream::createMeshSP(unsigned material_id, bool bonus_mesh)
{
    SP::SPMesh* spm = NULL;
#ifndef SERVER_ONLY
    // All radius, starting with the one closest to the kart (and
    // widest) to the one furthest away. A 0 indicates the end of the list

    std::vector<float> radius = {1.5f, 1.0f, 0.5f, 0.0f};

    if (bonus_mesh)
    {
        radius = {0.9f,0.6f,0.3f,0.0f};
    }

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
    std::vector<float> distance = {2.0f, 6.0f, 10.0f };

    if (bonus_mesh)
    {
        distance = {0.4f, 0.8f, 1.2f };
    }

    // The alpha values for the rings, no 'end of list' entry required.
    int alphas[]     = {0, 255, 0};

    // Loop through all given radius to determine the number
    // of segments to create.
    unsigned int num_circles=0;
    while(radius[num_circles]>0.0f) num_circles++;

    assert(num_circles > 0);

    // Length is distance of last circle to distance of first circle:
    float length = distance[num_circles-1] - distance[0];

    if (!bonus_mesh)
        m_length = length;

    // The number of points for each circle. Since part of the slip stream
    // might be under the ground (esp. first and last segment), specify
    // which one is the first and last to be actually drawn.
    const unsigned int  num_segments   = 15;
    const unsigned int  first_segment  = 0;
    const unsigned int  last_segment   = 14;
    const float         f              = 2*M_PI/float(num_segments);
    SP::SPMeshBuffer* buffer           = new SP::SPMeshBuffer();

    if(!bonus_mesh)
    {
        static_cast<SP::SPPerObjectUniform*>(buffer)->addAssignerFunction
            ("custom_alpha", [this](SP::SPUniformAssigner* ua)->void
            {
                // In sp shader it's assigned reverse by 1.0 - custom_alpha
                ua->setValue(1.0f - m_slipstream_time);
            });
    }
    else
    {
        static_cast<SP::SPPerObjectUniform*>(buffer)->addAssignerFunction
            ("custom_alpha", [this](SP::SPUniformAssigner* ua)->void
            {
                // In sp shader it's assigned reverse by 1.0 - custom_alpha
                ua->setValue(1.0f - m_bonus_time);
            });
    }

    std::vector<uint16_t> indices;
    std::vector<video::S3DVertexSkinnedMesh> vertices;
    for(unsigned int j=0; j<num_circles; j++)
    {
        float curr_distance = distance[j]-distance[0];
        // Create the vertices for each of the circle
        for(unsigned int i=first_segment; i<=last_segment; i++)
        {
            video::S3DVertexSkinnedMesh v;
            // Offset every 2nd circle by one half segment to increase
            // the number of planes so it looks better.
            v.m_position.X =  sinf((i+(j%2)*0.5f)*f)*radius[j];
            v.m_position.Y = -cosf((i+(j%2)*0.5f)*f)*radius[j];
            v.m_position.Z = distance[j];
            // Enable texture matrix and dummy normal for visualization
            v.m_normal = 0x1FF << 10 | 1 << 30;
            v.m_color = video::SColor(alphas[j], 255, 255, 255);
            v.m_all_uvs[0] = MiniGLM::toFloat16(curr_distance/length);
            v.m_all_uvs[1] = MiniGLM::toFloat16(
                (float)(i-first_segment)/(last_segment-first_segment)
                + (j%2)*(.5f/num_segments));
            vertices.push_back(v);
        }   // for i<num_segments
    }   // while radius[num_circles]!=0

    // Now create the triangles from circle j to j+1 (so the loop
    // only goes to num_circles-1).
    const int diff_segments = last_segment-first_segment+1;
    for(unsigned int j=0; j<num_circles-1; j++)
    {
        for(unsigned int i=first_segment; i<last_segment; i++)
        {
            indices.push_back(uint16_t( j   *diff_segments+i  ));
            indices.push_back(uint16_t((j+1)*diff_segments+i  ));
            indices.push_back(uint16_t( j   *diff_segments+i+1));
            indices.push_back(uint16_t( j   *diff_segments+i+1));
            indices.push_back(uint16_t((j+1)*diff_segments+i  ));
            indices.push_back(uint16_t((j+1)*diff_segments+i+1));
        }
    }   // for j<num_circles-1
    buffer->setSPMVertices(vertices);
    buffer->setIndices(indices);
    Material* material = material_manager->getMaterialSPM(
        g_slipstream_textures[material_id], "");
    buffer->setSTKMaterial(material);
    buffer->uploadGLMesh();

    spm = new SP::SPMesh();
    spm->addSPMeshBuffer(buffer);
    spm->updateBoundingBox();
#endif
    return spm;
}   // createMeshSP

//----------------------------------------------------------------------------- */
void SlipStream::updateSlipstreamingTextures(float f, const AbstractKart *kart)
{
    if (!kart || kart->isEliminated() || !m_node || !m_node_fast)
    {
        if (m_node)
        {
            m_node->setVisible(false);
        }
        if (m_node_fast)
        {
            m_node_fast->setVisible(false);
        }
        return;
    }

    float ktf = m_kart->getKartProperties()->getSlipstreamMinCollectTime();

    const float above_terrain = 0.2f;
    core::vector3df my_pos = m_kart->getNode()->getPosition();
    my_pos.Y = m_kart->getHoT()+above_terrain;
    core::vector3df other_pos = kart->getNode()->getPosition();
    other_pos.Y = kart->getHoT()+above_terrain;
    core::vector3df diff =   other_pos - my_pos;
    core::vector3df rotation = diff.getHorizontalAngle();
    float fs = diff.getLength()/m_length;

    m_node->setPosition(my_pos);
    m_node->setRotation(rotation);
    m_node->setScale(core::vector3df(1, 1, fs));

    m_node_fast->setPosition(my_pos);
    m_node_fast->setRotation(rotation);
    m_node_fast->setScale(core::vector3df(1, 1, fs));

    m_node->setVisible(f>0.0f && f<ktf);
    m_node_fast->setVisible(f>=ktf);
#ifndef SERVER_ONLY
    if (!CVS->isGLSL())
    {
        float a = m_slipstream_time * 255.0f;
        if (a > 255.0f)
            a = 255.0f;
        m_node->getMaterial(0).getRenderInfo()->getVertexColor().setAlpha(a);
        m_node_fast->getMaterial(0).getRenderInfo()->getVertexColor()
            .setAlpha(a);
    }
#endif

    //specify the texture speed movement
    float max_f = m_kart->getKartProperties()->getSlipstreamMaxCollectTime();

    if (f > max_f) f = max_f;
    f = f/2;

    m_moving->setSpeed(f, 0);
    m_moving_fast->setSpeed(f, 0);

    return;
    // For debugging: make the slip stream effect visible all the time
    m_node->setVisible(true);
    m_moving->setSpeed(1.0f, 0.0f);
}   // updateSlipstreamingTextures

//----------------------------------------------------------------------------- */
void SlipStream::updateBonusTexture()
{
    if (!m_bonus_node)
    {
        return;
    }

    const float above_terrain = 0.2f;
    core::vector3df my_pos = m_kart->getNode()->getPosition();
    my_pos.Y += above_terrain;

    core::vector3df previous_pos = m_kart->getRecentPreviousXYZ().toIrrVector();
    core::vector3df diff = my_pos - previous_pos;
    core::vector3df rotation = diff.getHorizontalAngle();

    m_bonus_node->setPosition(my_pos);
    m_bonus_node->setRotation(rotation);

    m_bonus_node->setVisible(m_bonus_time > 0.0f && m_kart->getSpeed() > 2.0f);
#ifndef SERVER_ONLY
    if (!CVS->isGLSL())
    {
        float a = m_bonus_time * 255.0f;
        if (a > 255.0f)
            a = 255.0f;
        m_bonus_node->getMaterial(0).getRenderInfo()->getVertexColor()
            .setAlpha(a);
    }
#endif

    float bonus_speed = 1.0f + std::max(m_bonus_time/1.5f,0.0f);
    m_moving_bonus->setSpeed(bonus_speed, 0);
} //updateBonusTexture

//-----------------------------------------------------------------------------
/** Returns true if enough slipstream credits have been accumulated
*  to get a boost when leaving the slipstream area.
*/
bool SlipStream::isSlipstreamReady() const
{
    return m_slipstream_time>
        m_kart->getKartProperties()->getSlipstreamMinCollectTime();
}   // isSlipstreamReady

//-----------------------------------------------------------------------------
/** Sets the color of the debug mesh (which shows the area in which slipstream
 *  can be accumulated).
 *  Color codes:
 *  black:  kart too slow
 *  red:    not inside of slipstream area
 *  green:  slipstream is being accumulated.
 *  \param inner : bool to know if we apply the color to the inner quad or to the main one
 */
void SlipStream::setDebugColor(const video::SColor &color, bool inner)
{
    if (!inner)
    {
        if (!m_debug_dc)
        {
            return;
        }

        video::S3DVertexSkinnedMesh* v = m_debug_dc->getVerticesVector().data();
        for (unsigned i = 0; i < 4; i++)
        {
            v[i].m_color = color;
        }
        m_debug_dc->setUpdateOffset(0);
    }
    else
    {
        if (!m_debug_dc2)
        {
            return;
        }

        video::S3DVertexSkinnedMesh* v = m_debug_dc2->getVerticesVector().data();
        for (unsigned i = 0; i < 4; i++)
        {
            v[i].m_color = color;
        }
        m_debug_dc2->setUpdateOffset(0);
    }
}   // setDebugColor

//-----------------------------------------------------------------------------
/** UpdateQuad
 */
void SlipStream::updateQuad()
{

    //Change the quad form to counteract the mismatch between
    //kart orientation and real direction

    //Computations are contrieved by the fact we have several
    //different 3D vector, one for each library.
    Vec3 moving_xyz = m_kart->getPreviousXYZ() - m_kart->getXYZ();

    //retrieve a vector rotated to kart direction
    btScalar bx,by,bz;//a btScalar is a float or a double
    bx = 1.0f;
    by = bz = 0.0f;
    btVector3 rotated_base;
    rotated_base.setValue(bx,by,bz);
    btQuaternion rotation = m_kart->getRotation();
    rotated_base = quatRotate(rotation,rotated_base);
    Vec3 direction_vector;
    //Z and X need to be inverted and X multiplied by -1 to match moving_xyz
    direction_vector = Vec3(rotated_base.getZ(), rotated_base.getY(), -rotated_base.getX());

    //normalize the moving vector
    float vec_length = moving_xyz.x()*moving_xyz.x()
                     + moving_xyz.y()*moving_xyz.y()
                     + moving_xyz.z()*moving_xyz.z();
	if (vec_length != 0)
    {
    	vec_length = core::reciprocal_squareroot(vec_length);
        float x,y,z;
        x = moving_xyz.x() * vec_length;
        y = moving_xyz.y() * vec_length;
        z = moving_xyz.z() * vec_length;
        moving_xyz = Vec3(x,y,z);
    }

    //This vector gives us the change to apply in absolute coordinates
    Vec3 noffset = moving_xyz - direction_vector;

    //But the quad position is in the kart coordinate space
    //So we rotate it back
    rotated_base.setValue(-noffset.z(),noffset.y(),noffset.x());
    rotation = rotation.inverse();
    rotated_base = quatRotate(rotation,rotated_base);
    noffset = Vec3(rotated_base.getZ(), rotated_base.getY(), -rotated_base.getX());

    float speed_factor = m_kart->getSpeed()/m_kart->getKartProperties()->getSlipstreamBaseSpeed();
    float length = m_kart->getKartProperties()->getSlipstreamLength()*speed_factor;
    float kw     = m_kart->getKartWidth();
    float ew     = m_kart->getKartProperties()->getSlipstreamWidth()*speed_factor;
    float kl     = m_kart->getKartLength();
    float offx   = (kl*0.5f+length)*noffset.x();
    float offz   = (kl*0.5f+length)*noffset.z();

    //making the slipstream quad start at the kart front
    //allows better results when the kart turns
    Vec3 p[4];
    p[0]=Vec3(-kw*0.5f, 0, kl*0.5f );
    p[1]=Vec3(-ew*0.5f+offx, 0, -kl*0.5f-length+offz);
    p[2]=Vec3( ew*0.5f+offx, 0, -kl*0.5f-length+offz);
    p[3]=Vec3( kw*0.5f, 0, kl*0.5f );

    //Update the slipstreaming quad
    m_slipstream_quad->setQuad(p[0], p[1], p[2], p[3]);

    p[1]=Vec3((-ew*0.5f+offx)*1.1f, 0, -kl*0.5f-(length+offz)*1.1f);
    p[2]=Vec3((ew*0.5f+offx)*1.1f, 0, -kl*0.5f-(length+offz)*1.1f);

    //Update the slipstreaming outer quad
    m_slipstream_outer_quad->setQuad(p[0], p[1], p[2], p[3]);

#ifndef SERVER_ONLY
    //recalculate quad position for debug drawing
    if (UserConfigParams::m_slipstream_debug && CVS->isGLSL())
    {
        video::S3DVertexSkinnedMesh* v =
            m_debug_dc->getVerticesVector().data();
        unsigned idx[] = { 0, 3, 1, 2 };
        for (unsigned i = 0; i < 4; i++)
        {
            v[i].m_position = p[idx[i]].toIrrVector();
        }
        m_debug_dc->recalculateBoundingBox();
        m_debug_dc->setParent(m_kart->getNode());
        SP::addDynamicDrawCall(m_debug_dc);
    }
#endif
    float inner_factor = m_kart->getKartProperties()->getSlipstreamInnerFactor()*sqrt(speed_factor);
    length = length*inner_factor;
    ew = ew*inner_factor;
    if (ew > 0.5f) ew -= 0.5f;
    else ew = 0;

    offx   = (kl*0.5f+length)*noffset.x();
    offz   = (kl*0.5f+length)*noffset.z();

    p[1]=Vec3(-ew*0.5f+offx, 0, -kl*0.5f-length+offz);
    p[2]=Vec3( ew*0.5f+offx, 0, -kl*0.5f-length+offz);

    //Update the slipstreaming inner quad
    m_slipstream_inner_quad->setQuad(p[0], p[1], p[2], p[3]);

#ifndef SERVER_ONLY
    //recalculate inner quad position for debug drawing
    if (UserConfigParams::m_slipstream_debug && CVS->isGLSL())
    {
        video::S3DVertexSkinnedMesh* v =
            m_debug_dc2->getVerticesVector().data();
        unsigned idx[] = { 0, 3, 1, 2 };
        for (unsigned i = 0; i < 4; i++)
        {
            v[i].m_position = p[idx[i]].toIrrVector();
        }
        m_debug_dc2->recalculateBoundingBox();
        m_debug_dc2->setParent(m_kart->getNode());
        SP::addDynamicDrawCall(m_debug_dc2);
    }
#endif

}   // updateQuad

//-----------------------------------------------------------------------------
void SlipStream::hideAllNodes()
{
#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
    {
        if (m_node && m_node->isVisible())
            m_node->setVisible(false);
        if (m_node_fast && m_node_fast->isVisible())
            m_node_fast->setVisible(false);
        if (m_bonus_node && m_bonus_node->isVisible())
            m_bonus_node->setVisible(false);
    }
#endif
}   // hideAllNodes

//-----------------------------------------------------------------------------
/** Update, called once per timestep.
 *  \param dt Time step size.
 */
void SlipStream::update(int ticks)
{
    const KartProperties *kp = m_kart->getKartProperties();

    // Low level AIs and ghost karts should not do any slipstreaming.
    if (m_kart->getController()->disableSlipstreamBonus()
        || m_kart->isGhostKart())
    {
        hideAllNodes();
        return;
    }

    //there is no slipstreaming at low speed
    //and the quad may do weird things if going in reverse
    if(m_kart->getSpeed() > 1.0f)
    {
        updateQuad();
    }

    float dt = stk_config->ticks2Time(ticks);
#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
    {
        m_moving->update(dt);
        m_moving_fast->update(dt);
        m_moving_bonus->update(dt);
    }
#endif

    m_bonus_time -= dt;
    if (m_bonus_time <= 0) m_bonus_active = false;

    // If this kart is too slow for slipstreaming taking effect, do nothing
    // Use a margin because what really matters is the target's speed
    // If this kart is much slower than the minSpeed, then either its
    // targets are slower too, or it won't stay long enough behind them
    // --------------------------------------------------------------------
    // Define this to get slipstream effect shown even when the karts are
    // not moving. This is useful for debugging the graphics of SS-ing.
//#define DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
#ifndef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
    if(m_kart->getSpeed() < kp->getSlipstreamMinSpeed() - 2.0f)
    {
#ifndef SERVER_ONLY
        if (!GUIEngine::isNoGraphics())
        {
            updateSlipstreamingTextures(0,NULL);
            updateBonusTexture();
        }
#endif
        m_slipstream_mode = SS_NONE;
        if(UserConfigParams::m_slipstream_debug)
        {
            setDebugColor(video::SColor(255, 0, 0, 0),false);
            setDebugColor(video::SColor(255, 0, 0, 0),true);            
        }
        return;
    }
#endif

    // Then test if this kart is in the slipstream range of another kart:
    // ------------------------------------------------------------------
    World *world             = World::getWorld();
    unsigned int num_karts   = world->getNumKarts();
    bool is_sstreaming       = false;
    bool is_inner_sstreaming = false;
    bool is_outer_sstreaming = false;
    m_target_kart            = NULL;
    std::vector<float> target_value;

    // Note that this loop can not be simply replaced with a shorter loop
    // using only the karts with a better position - since a kart might
    // be a lap behind
    for(unsigned int i=0; i<num_karts; i++)
    {
        m_target_kart= world->getKart(i);
        target_value.push_back(0);

        // Don't test for slipstream with itself, a kart that is being
        // rescued or exploding, a ghost kart or an eliminated kart
        if(m_target_kart==m_kart               ||
            m_target_kart->getKartAnimation()  ||
            m_target_kart->isGhostKart()       ||
            m_target_kart->isEliminated()        )
        {
            if (m_previous_target_id >= 0 && (int) i==m_previous_target_id)
                m_previous_target_id = -1;
            continue;
        }

        const KartProperties *kp_target = m_target_kart->getKartProperties();

        // Transform this kart location into target kart point of view
        Vec3 lc = m_target_kart->getTrans().inverse()(m_kart->getXYZ());

        // If the kart is 'on top' of this kart (e.g. up on a bridge),
        // don't consider it for slipstreaming.
        if (fabsf(lc.y()) > 6.0f) continue;

        // If the kart we are testing against is too slow, no need to test
        // slipstreaming.
#ifndef DISPLAY_SLIPSTREAM_WITH_0_SPEED_FOR_DEBUGGING
        if (m_target_kart->getSpeed() < kp_target->getSlipstreamMinSpeed())
        {
            if(UserConfigParams::m_slipstream_debug &&
                m_kart->getController()->isLocalPlayerController())
            {
                m_target_kart->getSlipstream()
                              ->setDebugColor(video::SColor(255, 0, 0, 0), false);
                m_target_kart->getSlipstream()
                              ->setDebugColor(video::SColor(255, 0, 0, 0), true);
            }
            if (m_previous_target_id >= 0 && (int) i==m_previous_target_id)
                m_previous_target_id = -1;
            continue;
        }
#endif
        // Quick test: the kart must be not more than
        // slipstream length+0.5*kart_length()+target_kart_length
        // away from the other kart
        // (additional target_kart_length because that kart's center
        // is not the center of rotation of the slipstreaming quad)
        Vec3 delta = m_kart->getXYZ() - m_target_kart->getXYZ();
        float l    = kp_target->getSlipstreamLength()*1.1f;//Outer quad margin
        float speed_factor = m_target_kart->getSpeed()
                            /kp_target->getSlipstreamBaseSpeed();
        l = l*speed_factor + m_target_kart->getKartLength()
                           + 0.5f*m_kart->getKartLength();
        if(delta.length2() > l*l)
        {
            if (m_previous_target_id >= 0 && (int) i==m_previous_target_id)
                m_previous_target_id = -1;
            continue;
        }
        // Real test 1: if in inner slipstream quad of other kart
        if(m_target_kart->getSlipstream()->m_slipstream_inner_quad
                                         ->pointInside(lc))
        {
            is_inner_sstreaming = true;
            is_sstreaming       = true;
            target_value[i]     = 2000.0f - delta.length2();
            continue;
        }
        if(UserConfigParams::m_slipstream_debug &&
            m_kart->getController()->isLocalPlayerController())
            m_target_kart->getSlipstream()
                         ->setDebugColor(video::SColor(255, 0, 0, 255),true);

        // Real test2: if in slipstream quad of other kart
        if(m_target_kart->getSlipstream()->m_slipstream_quad
                                         ->pointInside(lc))
        {
            is_sstreaming     = true;
            target_value[i]     = 1000.0f - delta.length2();
            continue;
        }
        else if (m_previous_target_id >= 0 && (int) i==m_previous_target_id)
        {
            m_previous_target_id = -1;
        }
        if(UserConfigParams::m_slipstream_debug &&
            m_kart->getController()->isLocalPlayerController())
            m_target_kart->getSlipstream()
                         ->setDebugColor(video::SColor(255, 0, 0, 255),false);

        // Real test3: if in outer slipstream quad of other kart
        if(m_target_kart->getSlipstream()->m_slipstream_outer_quad
                                         ->pointInside(lc))
        {
            is_outer_sstreaming     = true;
            continue;
        }
    }   // for i < num_karts

    int best_target=-1;
    float best_target_value=0.0f;
    
    //Select the best target
    for(unsigned int i=0; i<num_karts; i++)
    {
        if (target_value[i] > best_target_value)
        {
            best_target_value = target_value[i];
            best_target=i;
        }
    }   // for i < num_karts

    if (best_target >= 0)
    {
        m_target_kart = world->getKart(best_target);
    }

    //When changing slipstream target (including no good target)
    if (best_target!=m_current_target_id)
    {
        m_previous_target_id = m_current_target_id;
        m_current_target_id = best_target;
    }

    if(isSlipstreamReady() && (m_current_target_id < 0
                               || (m_previous_target_id >= 0
                                   && target_value[m_previous_target_id] == 0.0f)))
    {
        // The first time slipstream is ready after collecting, and
        // you are leaving the slipstream area, the bonus is activated
        float additional_time = m_slipstream_time*kp->getSlipstreamDurationFactor();
        if (m_bonus_time <= 0.0f)
            m_bonus_time = additional_time;
        else
            m_bonus_time += additional_time;

        m_slipstream_time = 0.0f;
        m_bonus_active = true;
        m_speed_increase_duration = stk_config->time2Ticks(m_bonus_time);
        m_speed_increase_ticks = World::getWorld()->getTicksSinceStart();
    }

    if(!is_sstreaming)
    {
        if(UserConfigParams::m_slipstream_debug &&
            m_kart->getController()->isLocalPlayerController())
        {
            m_target_kart->getSlipstream()
                         ->setDebugColor(video::SColor(255, 255, 0, 0),false);

            m_target_kart->getSlipstream()
                         ->setDebugColor(video::SColor(255, 0, 255, 0),true);

        }
        //Reduces the easiness of reusing most of the accumulated time with another kart
        if(is_outer_sstreaming)
            m_slipstream_time -=dt;
        else
            m_slipstream_time -=3*dt;
        if(m_slipstream_time<0) m_slipstream_mode = SS_NONE;
#ifndef SERVER_ONLY
        if (!GUIEngine::isNoGraphics())
        {
            updateSlipstreamingTextures(0,NULL);
            updateBonusTexture();
        }
#endif
        return;
    }   // if !is_sstreaming

    if(UserConfigParams::m_slipstream_debug &&
        m_kart->getController()->isLocalPlayerController())
        m_target_kart->getSlipstream()->setDebugColor(video::SColor(255, 128, 255, 0),false);

    // Accumulate slipstream credits now
    //Twice as fast in the inner quad
    if (is_inner_sstreaming)
    {
        m_slipstream_time = m_slipstream_mode==SS_NONE ? 2*dt : m_slipstream_time+2*dt;

        if(UserConfigParams::m_slipstream_debug &&
        m_kart->getController()->isLocalPlayerController())
        m_target_kart->getSlipstream()->setDebugColor(video::SColor(255, 0, 255, 128),true);
    }
    else
    {
        m_slipstream_time = m_slipstream_mode==SS_NONE ? dt : m_slipstream_time+dt;
    }

    //Cap the possible credits
    if (m_slipstream_time > m_kart->getKartProperties()->getSlipstreamMaxCollectTime())
        m_slipstream_time = m_kart->getKartProperties()->getSlipstreamMaxCollectTime();

    if(isSlipstreamReady())
        m_kart->setSlipstreamEffect(9.0f);
#ifndef SERVER_ONLY
    if (!GUIEngine::isNoGraphics())
    {
        updateSlipstreamingTextures(m_slipstream_time, m_target_kart);
        updateBonusTexture();
    }
#endif
    m_slipstream_mode = SS_COLLECT;
}   // update

// ----------------------------------------------------------------------------
void SlipStream::updateSpeedIncrease()
{
    if (m_speed_increase_ticks == World::getWorld()->getTicksSinceStart())
    {
        const KartProperties* kp = m_kart->getKartProperties();
        float speed_increase = kp->getSlipstreamMaxSpeedIncrease();
        float add_power = kp->getSlipstreamAddPower();
        int fade_out = stk_config->time2Ticks(kp->getSlipstreamFadeOutTime());
        m_kart->instantSpeedIncrease(
            MaxSpeed::MS_INCREASE_SLIPSTREAM, speed_increase,
            speed_increase, add_power, m_speed_increase_duration, fade_out);
    }
}   // updateSpeedIncrease
