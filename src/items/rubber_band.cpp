//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include "items/rubber_band.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "guiengine/engine.hpp"
#include "items/plunger.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "race/race_manager.hpp"
#include "utils/string_utils.hpp"

#include <array>
#include <ge_vulkan_dynamic_spm_buffer.hpp>
#include <IMeshSceneNode.h>
#include <IVideoDriver.h>
#include <SMesh.h>
#include <SMeshBuffer.h>

/** RubberBand constructor. It creates a simple quad and attaches it to the
 *  root(!) of the graph. It's easier this way to get the right coordinates
 *  than attaching it to the plunger or kart, and trying to find the other
 *  coordinate.
 *  \param plunger Pointer to the plunger (non const, since the rubber band
 *                 can trigger an explosion)
 *  \param kart    Reference to the kart.
 */
RubberBand::RubberBand(Plunger *plunger, AbstractKart *kart)
          : m_plunger(plunger), m_owner(kart), m_node(NULL)
{
    m_hit_kart = NULL;
    m_attached_state = RB_TO_PLUNGER;

#ifndef SERVER_ONLY
    if (GUIEngine::isNoGraphics())
        return;

    video::SColor color(255, 179, 0, 0);
    if (CVS->isGLSL())
    {
        if (CVS->isDeferredEnabled())
        {
            color.setRed(SP::srgb255ToLinear(color.getRed()));
            color.setGreen(SP::srgb255ToLinear(color.getGreen()));
            color.setBlue(SP::srgb255ToLinear(color.getBlue()));
        }
        m_dy_dc = std::make_shared<SP::SPDynamicDrawCall>
            (scene::EPT_TRIANGLE_STRIP, SP::SPShaderManager::get()->getSPShader
            ("unlit"), material_manager->getDefaultSPMaterial("unlit"));
        m_dy_dc->getVerticesVector().resize(4);
        // Set the vertex colors properly, as the new pipeline doesn't use the
        // old light values
        for (unsigned i = 0; i < 4; i++)
        {
            m_dy_dc->getSPMVertex()[i].m_color = color;
        }
        SP::addDynamicDrawCall(m_dy_dc);
    }
    else
    {
        std::array<uint16_t, 6> indices = {{ 0, 1, 2, 0, 2, 3 }};
        scene::IMeshBuffer* buffer = NULL;
        if (irr_driver->getVideoDriver()->getDriverType() == video::EDT_VULKAN)
        {
            buffer = new GE::GEVulkanDynamicSPMBuffer();
            video::S3DVertexSkinnedMesh v;
            v.m_normal = 0x1FF << 10;
            v.m_color = color;
            std::array<video::S3DVertexSkinnedMesh, 4> vertices =
                {{ v, v, v, v }};
            buffer->append(vertices.data(), vertices.size(), indices.data(),
                indices.size());
        }
        else
        {
            buffer = new scene::SMeshBuffer();
            video::S3DVertex v;
            v.Normal = core::vector3df(0, 1, 0);
            v.Color = color;
            std::array<video::S3DVertex, 4> vertices = {{ v, v, v, v }};
            buffer->append(vertices.data(), vertices.size(), indices.data(),
                indices.size());
        }
        buffer->getMaterial().AmbientColor = color;
        buffer->getMaterial().DiffuseColor = color;
        buffer->getMaterial().EmissiveColor = color;
        buffer->setHardwareMappingHint(scene::EHM_STREAM);
        buffer->recalculateBoundingBox();
        scene::SMesh* mesh = new scene::SMesh();
        mesh->addMeshBuffer(buffer);
        mesh->setBoundingBox(buffer->getBoundingBox());
        buffer->drop();
        std::string debug_name = m_owner->getIdent() + " (rubber-band)";
        m_node = static_cast<scene::IMeshSceneNode*>(
            irr_driver->addMesh(mesh, debug_name));
        mesh->drop();
    }
#endif
}   // RubberBand

// ----------------------------------------------------------------------------
RubberBand::~RubberBand()
{
    remove();
}   // RubberBand

// ----------------------------------------------------------------------------
void RubberBand::reset()
{
    m_hit_kart = NULL;
    m_attached_state = RB_TO_PLUNGER;
    updatePosition();
}   // reset

// ----------------------------------------------------------------------------
/** Updates the position of the rubber band. It especially sets the
 *  end position of the rubber band, i.e. the side attached to the plunger,
 *  track, or kart hit.
 */
void RubberBand::updatePosition()
{
    const Vec3 &k = m_owner->getXYZ();

    // Get the position to which the band is attached
    // ----------------------------------------------
    switch(m_attached_state)
    {
    case RB_TO_KART:    m_end_position = m_hit_kart->getXYZ(); break;
    case RB_TO_TRACK:   m_end_position = m_hit_position;       break;
    case RB_TO_PLUNGER: m_end_position = m_plunger->getXYZ();
                        checkForHit(k, m_end_position);        break;
    }   // switch(m_attached_state);
}   // updatePosition

// ----------------------------------------------------------------------------
void RubberBand::updateGraphics(float dt)
{
#ifndef SERVER_ONLY
    if (m_node)
    {
        scene::IMesh* mesh = m_node->getMesh();
        scene::IMeshBuffer* buffer = mesh->getMeshBuffer(0);
        // Update the rubber band positions
        // --------------------------------
        // Todo: make height dependent on length (i.e. rubber band gets
        // thinner). And call explosion if the band is too long.
        const Vec3& k = m_owner->getXYZ();
        const float hh = .1f;  // half height of the band
        const Vec3& p = m_end_position;  // for shorter typing
        core::vector3df& v0 = buffer->getPosition(0);
        core::vector3df& v1 = buffer->getPosition(1);
        core::vector3df& v2 = buffer->getPosition(2);
        core::vector3df& v3 = buffer->getPosition(3);
        v0.X = p.getX() - hh; v0.Y = p.getY(); v0.Z = p.getZ() - hh;
        v1.X = p.getX() + hh; v1.Y = p.getY(); v1.Z = p.getZ() + hh;
        v2.X = k.getX() + hh; v2.Y = k.getY(); v2.Z = k.getZ() + hh;
        v3.X = k.getX() - hh; v3.Y = k.getY(); v3.Z = k.getZ() - hh;
        core::vector3df normal = (v1 - v0).crossProduct(v2 - v0);
        core::vector3df kart_pos = Vec3(m_owner->getTrans()(
            Vec3(0, 5.0f, -2.0f))).toIrrVector();
        float dot_product = (v0 - kart_pos).dotProduct(normal);
        // For avoiding cull face
        if (dot_product >= 0.0f)
        {
            buffer->getIndices()[0] = 2;
            buffer->getIndices()[1] = 1;
            buffer->getIndices()[2] = 0;
            buffer->getIndices()[3] = 3;
            buffer->getIndices()[4] = 2;
            buffer->getIndices()[5] = 0;
        }
        else
        {
            buffer->getIndices()[0] = 0;
            buffer->getIndices()[1] = 1;
            buffer->getIndices()[2] = 2;
            buffer->getIndices()[3] = 0;
            buffer->getIndices()[4] = 2;
            buffer->getIndices()[5] = 3;
        }
        buffer->recalculateBoundingBox();
        buffer->setDirtyOffset(0, irr::scene::EBT_VERTEX_AND_INDEX);
        mesh->setBoundingBox(buffer->getBoundingBox());
    }
    else if (m_dy_dc)
    {
        // Update the rubber band positions
        // --------------------------------
        // Todo: make height dependent on length (i.e. rubber band gets
        // thinner). And call explosion if the band is too long.
        const Vec3 &k = m_owner->getXYZ();
        const float hh=.1f;  // half height of the band
        const Vec3 &p=m_end_position;  // for shorter typing
        auto& v = m_dy_dc->getVerticesVector();
        v[0].m_position.X = p.getX()-hh; v[0].m_position.Y=p.getY(); v[0].m_position.Z = p.getZ()-hh;
        v[1].m_position.X = p.getX()+hh; v[1].m_position.Y=p.getY(); v[1].m_position.Z = p.getZ()+hh;
        v[2].m_position.X = k.getX()-hh; v[2].m_position.Y=k.getY(); v[2].m_position.Z = k.getZ()-hh;
        v[3].m_position.X = k.getX()+hh; v[3].m_position.Y=k.getY(); v[3].m_position.Z = k.getZ()+hh;
        v[0].m_normal = 0x1FF << 10;
        v[1].m_normal = 0x1FF << 10;
        v[2].m_normal = 0x1FF << 10;
        v[3].m_normal = 0x1FF << 10;
        core::vector3df normal = (v[1].m_position - v[0].m_position)
            .crossProduct(v[2].m_position - v[0].m_position);
        core::vector3df kart_pos = Vec3(m_owner->getTrans()(Vec3(0, 5.0f, -2.0f))).toIrrVector();
        float dot_product = (v[0].m_position - kart_pos).dotProduct(normal);
        // For avoiding cull face
        if (dot_product >= 0.0f)
        {
            video::S3DVertexSkinnedMesh tmp = v[1];
            v[1] = v[2];
            v[2] = tmp;
        }
        m_dy_dc->setUpdateOffset(0);
        m_dy_dc->recalculateBoundingBox();
    }
#endif
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Updates the rubber band. It takes the new position of the kart and the
 *  plunger, and sets the quad representing the rubber band appropriately.
 *  It then casts a ray along the rubber band to detect if anything is hit. If
 *  so, an explosion is triggered.
 *  \param dt: Time step size.
 */
void RubberBand::update(int ticks)
{
    const KartProperties *kp = m_owner->getKartProperties();

    if(m_owner->isEliminated())
    {
        // Rubber band snaps
        m_plunger->hit(NULL);
        // This causes the plunger to be removed at the next update
        m_plunger->setKeepAlive(0);
        return;
    }

    updatePosition();
    const Vec3 &k = m_owner->getXYZ();

    // Check for rubber band snapping
    // ------------------------------
    float l = (m_end_position-k).length2();
    float max_len = kp->getPlungerBandMaxLength();
    if(l>max_len*max_len)
    {
        // Rubber band snaps
        m_plunger->hit(NULL);
        // This causes the plunger to be removed at the next update
        m_plunger->setKeepAlive(0);
    }

    // Apply forces (if applicable)
    // ----------------------------
    if(m_attached_state!=RB_TO_PLUNGER)
    {
        float force = kp->getPlungerBandForce();
        Vec3 diff   = m_end_position-k;

        // detach rubber band if kart gets very close to hit point
        if(m_attached_state==RB_TO_TRACK && diff.length2() < 10*10)
        {
            // Rubber band snaps
            m_plunger->hit(NULL);
            // This causes the plunger to be removed at the next update
            m_plunger->setKeepAlive(0);
            return;
        }
        // diff can be zero if the rubber band hits its owner
        if (diff.x() != 0.0f && diff.y() != 0.0f && diff.z() != 0.0f)
            diff.normalize();
        m_owner->getBody()->applyCentralForce(diff*force);
        m_owner->increaseMaxSpeed(MaxSpeed::MS_INCREASE_RUBBER,
            kp->getPlungerBandSpeedIncrease(),
            /*engine_force*/ 0.0f,
            /*duration*/stk_config->time2Ticks(0.1f),
            stk_config->time2Ticks(kp->getPlungerBandFadeOutTime()));
        if(m_attached_state==RB_TO_KART)
            m_hit_kart->getBody()->applyCentralForce(diff*(-force));
    }
}   // update

// ----------------------------------------------------------------------------
/** Uses a raycast to see if anything has hit the rubber band.
 *  \param k Position of the kart = one end of the rubber band
 *  \param p Position of the plunger = other end of the rubber band.
 */
void RubberBand::checkForHit(const Vec3 &k, const Vec3 &p)
{
    btCollisionWorld::ClosestRayResultCallback ray_callback(k, p);
    // Disable raycast collision detection for this plunger and this kart!
    short int old_plunger_group = m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup;
    short int old_kart_group=0;

    // If the owner is being rescued, the broadphase handle does not exist!
    if(m_owner->getBody()->getBroadphaseHandle())
        old_kart_group = m_owner->getBody()->getBroadphaseHandle()->m_collisionFilterGroup;
    m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = 0;
    if(m_owner->getBody()->getBroadphaseHandle())
        m_owner->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = 0;

    // Do the raycast
    Physics::get()->getPhysicsWorld()->rayTest(k, p, ray_callback);
    // Reset collision groups
    m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = old_plunger_group;
    if(m_owner->getBody()->getBroadphaseHandle())
        m_owner->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = old_kart_group;
    if(ray_callback.hasHit())
    {
        Vec3 pos(ray_callback.m_hitPointWorld);
        UserPointer *up = (UserPointer*)ray_callback.m_collisionObject->getUserPointer();
        if(up && up->is(UserPointer::UP_KART))
            hit(up->getPointerKart(), &pos);
        else
            hit(NULL, &pos);
    }  // if raycast hast hit

}   // checkForHit

// ----------------------------------------------------------------------------
/** The plunger hit a kart or the track.
 *  \param kart_hit The kart hit, or NULL if the track was hit.
 *  \param track _xyz The coordinated where the track was hit (NULL if a kart
 *                    was hit.
 */
void RubberBand::hit(AbstractKart *kart_hit, const Vec3 *track_xyz)
{
    // More than one report of a hit. This can happen if the raycast detects
    // a hit as well as the bullet physics.
    if(m_attached_state!=RB_TO_PLUNGER) return;


    // A kart was hit
    // ==============
    if(kart_hit)
    {
        if(kart_hit->isShielded())
        {
            kart_hit->decreaseShieldTime();
            m_plunger->setKeepAlive(0);

            return;
        }

        m_hit_kart       = kart_hit;
        m_attached_state = RB_TO_KART;
        return;
    }

    // The track was hit
    // =================
    m_hit_position   = *track_xyz;
    m_attached_state = RB_TO_TRACK;
    m_hit_kart       = NULL;
}   // hit

// ----------------------------------------------------------------------------
void RubberBand::remove()
{
#ifndef SERVER_ONLY
    if (m_dy_dc)
    {
        m_dy_dc->removeFromSP();
        m_dy_dc = nullptr;
    }
    else if (m_node)
    {
        irr_driver->removeNode(m_node);
        m_node = NULL;
    }
#endif
}   // remove

// ----------------------------------------------------------------------------
uint8_t RubberBand::get8BitState() const
{
    uint8_t state = (uint8_t)(m_attached_state & 3);
    state |= m_attached_state == RB_TO_KART && m_hit_kart ?
        (m_hit_kart->getWorldKartId() << 3) : 0;
    return state;
}   // get8BitState

// ----------------------------------------------------------------------------
void RubberBand::set8BitState(uint8_t bit_state)
{
    m_hit_kart = NULL;
    m_attached_state = (RubberBandTo)(bit_state & 3);
    if (m_attached_state == RB_TO_KART)
    {
        unsigned kart = bit_state >> 3;
        m_hit_kart = World::getWorld()->getKart(kart);
    }
}   // set8BitState
