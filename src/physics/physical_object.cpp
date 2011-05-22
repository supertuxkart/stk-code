//  $Id$
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

using namespace irr;

#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

#include <ISceneManager.h>
#include <IMeshManipulator.h>
#include <IMeshBuffer.h>
#include <SMaterial.h>

// -----------------------------------------------------------------------------
PhysicalObject::PhysicalObject(const XMLNode &xml_node)
              : TrackObject(xml_node)
{

    m_shape        = NULL;
    m_body         = NULL;
    m_motion_state = NULL;
    m_mass         = 1;
    m_radius       = -1;

    std::string shape;
    xml_node.get("mass",   &m_mass  );
    xml_node.get("radius", &m_radius);
    xml_node.get("shape",  &shape   );

    m_body_type = MP_NONE;
    if     (shape=="cone"  ||
            shape=="coneY"    ) m_body_type = MP_CONE_Y;
    else if(shape=="coneX"    ) m_body_type = MP_CONE_X;
    else if(shape=="coneZ"    ) m_body_type = MP_CONE_Z;
    else if(shape=="cylinder"||
            shape=="cylinderY") m_body_type = MP_CYLINDER_Y;
    else if(shape=="cylinderX") m_body_type = MP_CYLINDER_X;
    else if(shape=="cylinderZ") m_body_type = MP_CYLINDER_Z;

    else if(shape=="box"    ) m_body_type = MP_BOX;
    else if(shape=="sphere" ) m_body_type = MP_SPHERE;
    else fprintf(stderr, "Unknown shape type : %s\n", shape.c_str());
    
    m_init_pos.setIdentity();
    Vec3 hpr(m_init_hpr);
    hpr.degreeToRad();
    btQuaternion q;
    q.setEuler(hpr.getX(), hpr.getY(), hpr.getZ());
    m_init_pos.setRotation(q);
    Vec3 init_xyz(m_init_xyz);
    m_init_pos.setOrigin(init_xyz);

}   // PhysicalObject

// -----------------------------------------------------------------------------
PhysicalObject::~PhysicalObject()
{
    World::getWorld()->getPhysics()->removeBody(m_body);
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
    scene::IAnimatedMesh *mesh 
        = ((scene::IAnimatedMeshSceneNode*)m_node)->getMesh();
    MeshTools::minMax3D(mesh, &min, &max);
    Vec3 extend = max-min;
    // Adjust the mesth of the graphical object so that its center is where it
    // is in bullet (usually at (0,0,0)). It can be changed in the case clause
    // if this is not correct for a particular shape.
    Vec3 offset_from_center = -0.5f*(max+min);
    switch (m_body_type)
    {
    case MP_CONE_Y: {
                    if(m_radius<0) m_radius = 0.5f*extend.length_2d();
                    m_shape = new btConeShape(m_radius, extend.getY());
                    break;
                    }
    case MP_CONE_X: {
                    if(m_radius<0) 
                        m_radius = 0.5f*sqrt(extend.getY()*extend.getY() +
                                             extend.getZ()*extend.getZ());
                    m_shape = new btConeShapeX(m_radius, extend.getY());
                    break;
                    }
    case MP_CONE_Z: {
                    if(m_radius<0)
                        m_radius = 0.5f*sqrt(extend.getX()*extend.getX() +
                                             extend.getY()*extend.getY());
                    m_shape = new btConeShapeZ(m_radius, extend.getY());
                    break;
                    }
    case MP_CYLINDER_Y: {
                    if(m_radius<0) m_radius = 0.5f*extend.length_2d();
                    m_shape = new btCylinderShape(0.5f*extend);
                    break;
                    }
    case MP_CYLINDER_X: {
                    if(m_radius<0) 
                        m_radius = 0.5f*sqrt(extend.getY()*extend.getY() +
                                             extend.getZ()*extend.getZ());
                    m_shape = new btCylinderShapeX(0.5f*extend);
                    break;
                    }
    case MP_CYLINDER_Z: {
                    if(m_radius<0)
                        m_radius = 0.5f*sqrt(extend.getX()*extend.getX() +
                                             extend.getY()*extend.getY());
                    m_shape = new btCylinderShapeZ(0.5f*extend);
                    break;
                    }
    case MP_BOX:    m_shape = new btBoxShape(0.5*extend);
                    break;
    case MP_SPHERE: {
                    if(m_radius<0)
                    {
                        m_radius =      std::max(extend.getX(), extend.getY());
                        m_radius = 0.5f*std::max(m_radius,      extend.getZ());
                    }
                    m_shape = new btSphereShape(m_radius);
                    break;
                    }
    case MP_NONE:   fprintf(stderr, "WARNING: Uninitialised moving shape\n");
                    break;
    }

    // 2. Adjust the mesh so that its center is where it is in bullet
    // --------------------------------------------------------------
    // This means that the graphical and physical position are identical
    // which simplifies drawing later on.
    scene::IMeshManipulator *mesh_manipulator = 
        irr_driver->getSceneManager()->getMeshManipulator();
    core::matrix4 transform(core::matrix4::EM4CONST_IDENTITY);  // 
    transform.setTranslation(offset_from_center.toIrrVector());
    mesh_manipulator->transform(mesh, transform);

    // 2. Create the rigid object
    // --------------------------
    // m_init_pos is the point on the track - add the offset
    m_init_pos.setOrigin(m_init_pos.getOrigin()+btVector3(0,extend.getY()*0.5f, 0));
    m_motion_state = new btDefaultMotionState(m_init_pos);
    btVector3 inertia;
    m_shape->calculateLocalInertia(m_mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo info(m_mass, m_motion_state, m_shape, inertia);
    
    // Make sure that the cones stop rolling by defining angular friction != 0.
    info.m_angularDamping = 0.5f;
    m_body = new btRigidBody(info);
    m_user_pointer.set(this);
    m_body->setUserPointer(&m_user_pointer);

    World::getWorld()->getPhysics()->addBody(m_body);
}   // init

// -----------------------------------------------------------------------------
void PhysicalObject::update(float dt)
{
    btTransform t;
    m_motion_state->getWorldTransform(t);

    Vec3 xyz = t.getOrigin();
    if(xyz.getY()<-100)
    {
        m_body->setCenterOfMassTransform(m_init_pos);
        m_body->setLinearVelocity (btVector3(0,0,0));
        m_body->setAngularVelocity(btVector3(0,0,0));
        xyz = Vec3(m_init_pos.getOrigin());
    }
    m_node->setPosition(xyz.toIrrVector());
    Vec3 hpr;
    hpr.setHPR(t.getRotation());
    m_node->setRotation(hpr.toIrrHPR());
    return;
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

