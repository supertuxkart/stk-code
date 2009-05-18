//  $Id: physical_object.cpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "physics/physical_object.hpp"

#include <string>
#include <vector>
#include "irrlicht.h"
using namespace irr;

#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/scene.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/coord.hpp"
#include "utils/string_utils.hpp"

// -----------------------------------------------------------------------------
PhysicalObject::PhysicalObject(const XMLNode *xml_node)
{
    std::string model_name;
    const Track *track=RaceManager::getTrack();
    xml_node->get("model", &model_name);
    std::string full_path = file_manager->getTrackFile(model_name, 
                                                       track->getIdent());
    scene::IAnimatedMesh *obj = irr_driver->getAnimatedMesh(full_path);
    if(!obj)
    {
        // If the model isn't found in the track directory, look 
        // in STK's model directory.
        full_path = file_manager->getModelFile(model_name);
        obj = irr_driver->getAnimatedMesh(full_path);
        if(!obj)
        {
            fprintf(stderr, "Warning: '%s' in '%s' not found and is ignored.\n",
                    xml_node->getName().c_str(), model_name.c_str());
            return;
        }   // if(!obj)
    }
    m_mesh = obj->getMesh(0);
    m_node = irr_driver->addMesh(m_mesh);
    //m_node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    Vec3 xyz(0,0,0);
    int result = xml_node->getXYZ(&xyz);
    if(!XMLNode::hasZ(result))   // needs height
    {
        xyz.setZ(RaceManager::getTrack()->getTerrainHeight(xyz));
    }
    Vec3 hpr(0,0,0);
    result = xml_node->getHPR(&hpr);
    if(!XMLNode::hasP(result) ||
       !XMLNode::hasR(result))   // Needs perhaps pitch and roll
    {
    }
    m_node->setPosition(xyz.toIrrVector());
    m_node->setRotation(hpr.toIrrHPR());
    m_init_pos.setIdentity();
    m_init_pos.setOrigin(xyz);
    m_node->setMaterialFlag(video::EMF_LIGHTING, false);

    m_shape        = NULL;
    m_body         = NULL;
    m_motion_state = NULL;
    m_mass         = 1;

    std::string shape;
    xml_node->get("shape", &shape);
    xml_node->get("mass",  &m_mass);

    m_body_type = MP_NONE;
    if     (shape=="cone"   ) m_body_type = MP_CONE;
    else if(shape=="box"    ) m_body_type = MP_BOX;
    else if(shape=="sphere" ) m_body_type = MP_SPHERE;
}   // PhysicalObject

// -----------------------------------------------------------------------------
PhysicalObject::~PhysicalObject()
{
    RaceManager::getWorld()->getPhysics()->removeBody(m_body);
    delete m_body;
    delete m_motion_state;
    delete m_shape;
}  // ~PhysicalObject

// -----------------------------------------------------------------------------
/** Additional initialisation after loading of the model is finished.
 */
void PhysicalObject::init()
{
    // 1. Determine size of the object
    // -------------------------------
    Vec3 min, max;
    MeshTools::minMax3D(m_mesh, &min, &max);
    Vec3 extend = max-min;
    m_half_height = 0.5f*(extend.getZ());
    switch (m_body_type)
    {
    case MP_CONE:   {
                    float radius = 0.5f*std::max(extend.getX(), extend.getY());
                    m_shape = new btConeShapeZ(radius, extend.getZ());
                    break;
                    }
    case MP_BOX:    m_shape = new btBoxShape(0.5*extend);
                    break;
    case MP_SPHERE: {
                    float radius = std::max(extend.getX(), extend.getY());
                    radius = 0.5f*std::max(radius, extend.getZ());
                    m_shape = new btSphereShape(radius);
                    break;
                    }
    case MP_NONE:   fprintf(stderr, "WARNING: Uninitialised moving shape\n");
                    break;
    }

    // 2. Create the rigid object
    // --------------------------
    Vec3 pos = m_init_pos.getOrigin();
    pos.setZ(m_init_pos.getOrigin().getZ()+m_half_height);
    m_init_pos.setOrigin(pos);
    m_motion_state = new btDefaultMotionState(m_init_pos);
    btVector3 inertia;
    m_shape->calculateLocalInertia(m_mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo info(m_mass, m_motion_state, m_shape, inertia);
    
    // Make sure that the cones stop rolling by defining angular friction != 0.
    info.m_angularDamping = 0.5f;
    m_body = new btRigidBody(info);
    m_user_pointer.set(this);
    m_body->setUserPointer(&m_user_pointer);
    RaceManager::getWorld()->getPhysics()->addBody(m_body);
}   // init

// -----------------------------------------------------------------------------
void PhysicalObject::update(float dt)
{
    btTransform t;
    m_motion_state->getWorldTransform(t);
    Coord c(t);
    if(c.getXYZ().getZ()<-100)
    {
        m_body->setCenterOfMassTransform(m_init_pos);
        c.setXYZ(m_init_pos.getOrigin());
    }
    m_node->setPosition(c.getXYZ().toIrrVector());
    m_node->setRotation(c.getHPR().toIrrHPR());
}   // update

// -----------------------------------------------------------------------------
void PhysicalObject::reset()
{
    m_body->setCenterOfMassTransform(m_init_pos);
    m_body->setAngularVelocity(btVector3(0,0,0));
    m_body->setLinearVelocity(btVector3(0,0,0));
    m_body->activate();
}   // reset 

// -----------------------------------------------------------------------------
void PhysicalObject::handleExplosion(const Vec3& pos, bool direct_hit) {
    if(direct_hit) {
        btVector3 impulse(0.0f, 0.0f, stk_config->m_explosion_impulse_objects);
        m_body->applyCentralImpulse(impulse);
    }
    else  // only affected by a distant explosion
    {
        btTransform t;
        m_motion_state->getWorldTransform(t);
        btVector3 diff=t.getOrigin()-pos;

        float len2=diff.length2();

        // The correct formhale would be to first normalise diff,
        // then apply the impulse (which decreases 1/r^2 depending
        // on the distance r), so:
        // diff/len(diff) * impulseSize/len(diff)^2
        // = diff*impulseSize/len(diff)^3
        // We use diff*impulseSize/len(diff)^2 here, this makes the impulse
        // somewhat larger, which is actually more fun :)
        btVector3 impulse=diff*stk_config->m_explosion_impulse_objects/len2;
        m_body->applyCentralImpulse(impulse);
    }
    m_body->activate();

}   // handleExplosion

// -----------------------------------------------------------------------------
/* EOF */

