//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013  Joerg Henrichs
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

#include "graphics/shadow.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/skidding.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "physics/btKart.hpp"
#include "utils/mini_glm.hpp"
#include "utils/vec3.hpp"

#ifndef SERVER_ONLY

Shadow::Shadow(Material* shadow_mat, const AbstractKart& kart)
      : m_dy_dc(NULL), m_shadow_enabled(false), m_kart(kart)
{
    m_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
        (scene::EPT_TRIANGLE_STRIP,
        SP::SPShaderManager::get()->getSPShader("alphablend"), shadow_mat);

    m_dy_dc->getVerticesVector().resize(4);
    video::S3DVertexSkinnedMesh* v = m_dy_dc->getVerticesVector().data();
    v[0].m_all_uvs[0] = 0;
    v[0].m_all_uvs[1] = 0;
    v[1].m_all_uvs[0] = 15360;
    v[1].m_all_uvs[1] = 0;
    v[3].m_all_uvs[0] = 15360;
    v[3].m_all_uvs[1] = 15360;
    v[2].m_all_uvs[0] = 0;
    v[2].m_all_uvs[1] = 15360;

    m_dy_dc->setVisible(false);
    SP::addDynamicDrawCall(m_dy_dc);
}   // Shadow

// ----------------------------------------------------------------------------
Shadow::~Shadow()
{
    m_dy_dc->removeFromSP();
}   // ~Shadow

// ----------------------------------------------------------------------------
/** Updates the simulated shadow. It takes the 4 suspension lengths of vehicle
 *  from ground to position the shadow quad exactly on the ground.
 *  It also disables the shadow if requested (e.g. if the kart is in the air).
 *  \param enabled If the shadow should be shown or not.
 */
void Shadow::update(bool enabled)
{
    if (enabled != m_shadow_enabled)
    {
        m_shadow_enabled = enabled;
        if (m_shadow_enabled)
        {
            m_dy_dc->setVisible(true);
        }
        else
        {
            m_dy_dc->setVisible(false);
        }
    }
    if (m_shadow_enabled)
    {
        video::S3DVertexSkinnedMesh* v = m_dy_dc->getVerticesVector().data();
        v[0].m_position.X = -1; v[0].m_position.Z =  1; v[0].m_position.Y = 0;
        v[1].m_position.X =  1; v[1].m_position.Z =  1; v[1].m_position.Y = 0;
        v[2].m_position.X = -1; v[2].m_position.Z = -1; v[2].m_position.Y = 0;
        v[3].m_position.X =  1; v[3].m_position.Z = -1; v[3].m_position.Y = 0;
        btTransform kart_trans = m_kart.getSmoothedTrans();
        btTransform skidding_rotation;
        skidding_rotation.setOrigin(Vec3(0, 0, 0));
        skidding_rotation.setRotation
            (btQuaternion(m_kart.getSkidding()->getVisualSkidRotation(), 0, 0));
        kart_trans *= skidding_rotation;
        for (unsigned i = 0; i < 4; i++)
        {
            const btWheelInfo& wi = m_kart.getVehicle()->getWheelInfo(i);
            Vec3 up_vector = kart_trans.getBasis().getColumn(1);
            up_vector = up_vector * (wi.m_raycastInfo.m_suspensionLength - 0.02f);
            Vec3 pos = kart_trans(Vec3(v[i].m_position)) - up_vector;
            v[i].m_position = pos.toIrrVector();
            v[i].m_normal = MiniGLM::compressVector3
                (Vec3(wi.m_raycastInfo.m_contactNormalWS).toIrrVector());
        }
        m_dy_dc->recalculateBoundingBox();
        m_dy_dc->setUpdateOffset(0);
    }
}   // update

#endif
