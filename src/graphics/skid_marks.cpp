//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2013-2015 Joerg Henrichs
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

#include "graphics/skid_marks.hpp"

#include "config/stk_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_per_object_uniform.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"
#include "physics/btKart.hpp"
#include "utils/mini_glm.hpp"

#ifndef SERVER_ONLY

float     SkidMarks::m_avoid_z_fighting  = 0.005f;
const int SkidMarks::m_start_alpha       = 200;
const int SkidMarks::m_start_grey        = 32;

/** Initialises empty skid marks. */
SkidMarks::SkidMarks(const AbstractKart& kart, float width) : m_kart(kart)
{
    m_width = width;
    m_material = material_manager->getMaterialSPM("skidmarks.png", "",
        "alphablend");
    m_shader = SP::SPShaderManager::get()->getSPShader("alphablend");
    assert(m_shader);
    auto texture = SP::SPTextureManager::get()->getTexture(
        m_material->getSamplerPath(0), m_material,
        m_shader->isSrgbForTextureLayer(0), m_material->getContainerId());
    m_skid_marking = false;
}   // SkidMark

//-----------------------------------------------------------------------------
/** Removes all skid marks from the scene graph and frees the state. */
SkidMarks::~SkidMarks()
{
    reset();  // remove all skid marks
}   // ~SkidMarks

//-----------------------------------------------------------------------------
/** Removes all skid marks, called when a race is restarted.
 */
void SkidMarks::reset()
{
    m_left.clear();
    m_right.clear();
    m_skid_marking = false;
}   // reset

//-----------------------------------------------------------------------------
/** Either adds to an existing skid mark quad, or (if the kart is skidding)
 *  starts a new skid mark quad.
 *  \param dt Time step.
 */
void SkidMarks::update(float dt, bool force_skid_marks,
                       video::SColor* custom_color)
{
    //if the kart is gnu, then don't skid because he floats!
    if (m_kart.isWheeless())
        return;

    float f = dt / stk_config->m_skid_fadeout_time;
    auto it = m_left.begin();
    // Don't clean the current skidmarking
    while (it != m_left.end())
    {
        if ((it + 1 != m_left.end() || !m_skid_marking)
            && (*it)->fade(f))
        {
            it = m_left.erase(it);
            continue;
        }
        it++;
    }
    it = m_right.begin();
    while (it != m_right.end())
    {
        if ((it + 1 != m_right.end() || !m_skid_marking)
            && (*it)->fade(f))
        {
            it = m_right.erase(it);
            continue;
        }
        it++;
    }

    // Get raycast information
    // -----------------------
    btKart *vehicle = m_kart.getVehicle();

    Vec3 raycast_right;
    Vec3 raycast_left;
    vehicle->getVisualContactPoint(m_kart.getSmoothedTrans(), &raycast_left,
        &raycast_right);

    btTransform smoothed_inv = m_kart.getSmoothedTrans().inverse();
    Vec3 lc_l = smoothed_inv(raycast_left);
    Vec3 lc_r = smoothed_inv(raycast_right);
    btTransform skidding_rotation = m_kart.getSmoothedTrans();
    skidding_rotation.setRotation(m_kart.getSmoothedTrans().getRotation() *
        btQuaternion(m_kart.getSkidding()->getVisualSkidRotation(), 0.0f, 0.0f));
    raycast_left = skidding_rotation(lc_l);
    raycast_right = skidding_rotation(lc_r);

    Vec3 delta = raycast_right - raycast_left;

    // The kart is making skid marks when it's:
    // - forced to leave skid marks, or all of:
    // - in accumulating skidding mode
    // - not doing the grphical jump
    // - wheels are in contact with floor, which includes a special case:
    //   the physics force both wheels on one axis to touch the ground or not.
    //   If only one wheel touches the ground, the 2nd one gets the same
    //   raycast result --> delta is 0, which is considered to be not skidding.
    const Skidding *skid = m_kart.getSkidding();
    bool is_skidding = vehicle->visualWheelsTouchGround() &&
               ( force_skid_marks ||
                 (    (skid->getSkidState()==Skidding::SKID_ACCUMULATE_LEFT||
                       skid->getSkidState()==Skidding::SKID_ACCUMULATE_RIGHT )
                    && !skid->isJumping()
                    && delta.length2()>=0.0001f                           ) );

    if(m_skid_marking)
    {
        assert(!m_left.empty());
        assert(!m_right.empty());
        if (!is_skidding)   // end skid marking
        {
            m_skid_marking = false;
            return;
        }

        // We are still skid marking, so add the latest quad
        // -------------------------------------------------

        delta.normalize();
        delta *= m_width*0.5f;

        Vec3 start = m_left.back()->getCenterStart();
        Vec3 newPoint = (raycast_left + raycast_right)/2;
        // this linear distance does not account for the kart turning, it's true,
        // but it produces good enough results
        float distance = (newPoint - start).length();

        const Vec3 up_offset = (m_kart.getNormal() * 0.05f);
        m_left.back()->add(raycast_left - delta + up_offset,
            raycast_left + delta + up_offset, m_kart.getNormal(), distance);
        m_right.back()->add(raycast_right - delta + up_offset,
            raycast_right + delta + up_offset, m_kart.getNormal(), distance);
        return;
    }

    // Currently no skid marking
    // -------------------------
    if (!is_skidding) return;

    // Start new skid marks
    // --------------------
    // No skidmarking if wheels don't have contact
    if(!vehicle->visualWheelsTouchGround()) return;
    if(delta.length2()<0.0001) return;

    delta.normalize();
    delta *= m_width*0.5f;

    const int cleaning_threshold =
        core::clamp(int(World::getWorld()->getNumKarts()), 5, 15);
    while ((int)m_left.size() >=
        stk_config->m_max_skidmarks / cleaning_threshold)
    {
        m_left.erase(m_left.begin());
    }
    while ((int)m_right.size() >=
        stk_config->m_max_skidmarks / cleaning_threshold)
    {
        m_right.erase(m_right.begin());
    }

    m_left.emplace_back(
        new SkidMarkQuads(raycast_left - delta, raycast_left + delta,
        m_kart.getNormal(), m_material, m_shader, m_avoid_z_fighting,
        custom_color));

    m_right.emplace_back(
        new SkidMarkQuads(raycast_right - delta, raycast_right + delta,
        m_kart.getNormal(), m_material, m_shader, m_avoid_z_fighting,
        custom_color));

    m_skid_marking = true;
}   // update

//=============================================================================
SkidMarks::SkidMarkQuads::SkidMarkQuads(const Vec3 &left,
                                        const Vec3 &right,
                                        const Vec3 &normal,
                                        Material* material,
                                        std::shared_ptr<SP::SPShader> shader,
                                        float z_offset,
                                        video::SColor* custom_color)
{
    m_center_start = (left + right)/2;
    m_z_offset = z_offset;
    m_fade_out = 0.0f;
    m_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
        (scene::EPT_TRIANGLE_STRIP, shader, material);
    static_cast<SP::SPPerObjectUniform*>(m_dy_dc.get())->addAssignerFunction
        ("custom_alpha", [this](SP::SPUniformAssigner* ua)->void
        {
            // SP custom_alpha is assigned 1 - x, so this is correct
            ua->setValue(m_fade_out);
        });
    SP::addDynamicDrawCall(m_dy_dc);
    m_start_color = (custom_color != NULL ? *custom_color :
        video::SColor(255, SkidMarks::m_start_grey, SkidMarks::m_start_grey,
        SkidMarks::m_start_grey));

    if (CVS->isDeferredEnabled())
    {
        m_start_color.setRed(SP::srgb255ToLinear(m_start_color.getRed()));
        m_start_color.setGreen(SP::srgb255ToLinear(m_start_color.getGreen()));
        m_start_color.setBlue(SP::srgb255ToLinear(m_start_color.getBlue()));
    }

    add(left, right, normal, 0.0f);
}   // SkidMarkQuads

//-----------------------------------------------------------------------------
SkidMarks::SkidMarkQuads::~SkidMarkQuads()
{
    m_dy_dc->removeFromSP();
}   // ~SkidMarkQuads

//-----------------------------------------------------------------------------
/** Adds the two points to this SkidMarkQuads.
 *  \param left,right Left and right coordinates.
 */
void SkidMarks::SkidMarkQuads::add(const Vec3 &left,
                                   const Vec3 &right,
                                   const Vec3 &normal,
                                   float distance)
{
    // The skid marks must be raised slightly higher, otherwise it blends
    // too much with the track.
    int n = m_dy_dc->getVertexCount();

    video::S3DVertexSkinnedMesh v;
    v.m_color = m_start_color;
    v.m_color.setAlpha(0); // initially create all vertices at alpha=0...

    // then when adding a new set of vertices, make the previous 2 opaque.
    // this ensures that the last two vertices are always at alpha=0,
    // producing a fade-out effect
    if (n > 3)
    {
        m_dy_dc->getSPMVertex()[n - 1].m_color.setAlpha(m_start_alpha);
        m_dy_dc->getSPMVertex()[n - 2].m_color.setAlpha(m_start_alpha);
    }

    v.m_position = Vec3(right + normal * m_z_offset).toIrrVector();
    v.m_normal = MiniGLM::compressVector3(normal.toIrrVector());
    short half_float_1 = 15360;
    v.m_all_uvs[0] = half_float_1;
    v.m_all_uvs[1] = MiniGLM::toFloat16(distance * 0.5f);
    m_dy_dc->addSPMVertex(v);

    v.m_position = Vec3(left + normal * m_z_offset).toIrrVector();
    v.m_all_uvs[0] = 0;
    v.m_all_uvs[1] = MiniGLM::toFloat16(distance * 0.5f);
    m_dy_dc->addSPMVertex(v);
    m_dy_dc->setUpdateOffset(n > 3 ? n - 2 : n);
    m_dy_dc->recalculateBoundingBox();

}   // add

// ----------------------------------------------------------------------------
/** Fades the current skid marks.
 *  \param f fade factor.
 *  \return true if this skid mark can be deleted (alpha == zero)
 */
bool SkidMarks::SkidMarkQuads::fade(float f)
{
    m_fade_out += f;
    if (m_fade_out >= 1.0f)
    {
        return true;
    }
    return false;
}   // fade

#endif
