//  $Id: three_d_animation.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#include "animations/three_d_animation.hpp"

#include <stdio.h>

#include "animations/ipo.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "physics/kart_motion_state.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/bezier_curve.hpp"
#include "utils/constants.hpp"

#include <IMeshSceneNode.h>

ThreeDAnimation::ThreeDAnimation(const XMLNode &node)
               : AnimationBase(node)
{
    m_crash_reset   = false;
    node.get("reset", &m_crash_reset);
    m_triangle_mesh = NULL;
    
    /** Save the initial position and rotation in the base animation object. */
    setInitialTransform(AnimationBase::m_node->getPosition(),
                        AnimationBase::m_node->getRotation() );

    m_body            = NULL;
    m_motion_state    = NULL;
    m_collision_shape = NULL;
    m_hpr = AnimationBase::m_node->getRotation();
    std::string shape;
    node.get("shape", &shape);
    if(shape!="")
    {
        createPhysicsBody(shape);
    }
}   // ThreeDAnimation

// ----------------------------------------------------------------------------
/** Creates a bullet rigid body for this animated model. */
void ThreeDAnimation::createPhysicsBody(const std::string &shape)
{
    // 1. Determine size of the object
    // -------------------------------
    Vec3 min, max;
    MeshTools::minMax3D(m_mesh, &min, &max);
    Vec3 extend = max-min;
    if(shape=="box")
    {
        m_collision_shape = new btBoxShape(0.5*extend);
    }
    else if(shape=="coneX")
    {
        float radius = 0.5f*std::max(extend.getY(), extend.getZ());
        m_collision_shape = new btConeShapeX(radius, extend.getX());
    }
    else if(shape=="coneY" || shape=="cone")
    {
        float radius = 0.5f*std::max(extend.getX(), extend.getZ());
        m_collision_shape = new btConeShape(radius, extend.getY());
    }
    else if(shape=="coneZ")
    {
        // Note that the b3d model and therefore the extend has the
        // irrlicht axis, i.e. Y and Z swapped. Also we need to
        // convert 
        float radius = 0.5f*std::max(extend.getX(), extend.getY());
        m_collision_shape = new btConeShapeZ(radius, extend.getZ());
    }
    else if(shape=="cylinderX")
    {
        m_collision_shape = new btCylinderShapeX(0.5f*extend);
    }
    else if(shape=="cylinderY")
    {
        m_collision_shape = new btCylinderShape(0.5f*extend);
    }
    else if(shape=="cylinderZ")
    {
        m_collision_shape = new btCylinderShapeZ(0.5f*extend);
    }
    
    else if(shape=="exact")
    {
        TriangleMesh* triangle_mesh = new TriangleMesh();
        
        // In case of readonly materials we have to get the material from
        // the mesh, otherwise from the node. This is esp. important for
        // water nodes, which only have the material defined in the node,
        // but not in the mesh at all!
        bool is_readonly_material = false;
        
        scene::IMesh* mesh = NULL;
        switch (m_node->getType())
        {
            case scene::ESNT_MESH          :
            case scene::ESNT_WATER_SURFACE :
            case scene::ESNT_OCTREE        :
                mesh = ((scene::IMeshSceneNode*)m_node)->getMesh();
                is_readonly_material = 
                    ((scene::IMeshSceneNode*)m_node)->isReadOnlyMaterials();
                break;
            case scene::ESNT_ANIMATED_MESH :
                // for now just use frame 0
                mesh = ((scene::IAnimatedMeshSceneNode*)m_node)->getMesh()->getMesh(0);
                is_readonly_material = 
                    ((scene::IAnimatedMeshSceneNode*)m_node)->isReadOnlyMaterials();
                break;
            default:
                fprintf(stderr, "[3DAnimation] Unknown object type, cannot create exact collision body!\n");
                return;
        }   // switch node->getType()
        
        
        //core::matrix4 mat;
        //mat.setRotationDegrees(hpr);
        //mat.setTranslation(pos);
        //core::matrix4 mat_scale;
        
        // Note that we can't simply call mat.setScale, since this would
        // overwrite the elements on the diagonal, making any rotation incorrect.
        //mat_scale.setScale(scale);
        //mat *= mat_scale;
        
        for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++)
        {
            scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
            // FIXME: take translation/rotation into account
            if (mb->getVertexType() != video::EVT_STANDARD &&
                mb->getVertexType() != video::EVT_2TCOORDS)
            {
                fprintf(stderr, "WARNING: Physics::convertTrack: Ignoring type '%d'!", 
                        mb->getVertexType());
                continue;
            }
            
            // Handle readonly materials correctly: mb->getMaterial can return 
            // NULL if the node is not using readonly materials. E.g. in case
            // of a water scene node, the mesh (which is the animated copy of
            // the original mesh) does not contain any material information,
            // the material is only available in the node.
            const video::SMaterial &irrMaterial = 
                is_readonly_material ? mb->getMaterial()
                : m_node->getMaterial(i);
            video::ITexture* t=irrMaterial.getTexture(0);
            
            const Material* material=0;
            TriangleMesh *tmesh = triangle_mesh;
            if(t)
            {
                std::string image = std::string(core::stringc(t->getName()).c_str());
                material=material_manager->getMaterial(StringUtils::getBasename(image));
                if(material->isIgnore()) 
                    continue;
            } 
            
            u16 *mbIndices = mb->getIndices();
            Vec3 vertices[3];
            Vec3 normals[3];
            
            if (mb->getVertexType() == video::EVT_STANDARD)
            {
                irr::video::S3DVertex* mbVertices=(video::S3DVertex*)mb->getVertices();
                for(unsigned int j=0; j<mb->getIndexCount(); j+=3)
                {
                    for(unsigned int k=0; k<3; k++)
                    {
                        int indx=mbIndices[j+k];
                        core::vector3df v = mbVertices[indx].Pos;
                        //mat.transformVect(v);
                        vertices[k]=v;
                        normals[k]=mbVertices[indx].Normal;
                    }   // for k
                    if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                                 vertices[2], normals[0],
                                                 normals[1],  normals[2],
                                                 material                 );
                }   // for j
            }
            else
            {
                if (mb->getVertexType() == video::EVT_2TCOORDS)
                {
                    irr::video::S3DVertex2TCoords* mbVertices = (video::S3DVertex2TCoords*)mb->getVertices();
                    for(unsigned int j=0; j<mb->getIndexCount(); j+=3)
                    {
                        for(unsigned int k=0; k<3; k++)
                        {
                            int indx=mbIndices[j+k];
                            core::vector3df v = mbVertices[indx].Pos;
                            //mat.transformVect(v);
                            vertices[k]=v;
                            normals[k]=mbVertices[indx].Normal;
                        }   // for k
                        if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                                     vertices[2], normals[0],
                                                     normals[1],  normals[2],
                                                     material                 );
                    }   // for j
                    
                }
            }
        }   // for i<getMeshBufferCount
        triangle_mesh->createPhysicalBody();
        m_collision_shape = &triangle_mesh->getCollisionShape();
        m_triangle_mesh = triangle_mesh;
    }
    else
    {
        fprintf(stderr, "[3DAnimation] WARNING: Shape '%s' is not supported, ignored.\n", shape.c_str());
        return;
    }
    const core::vector3df &hpr=m_node->getRotation()*DEGREE_TO_RAD;
    btQuaternion q(hpr.X, hpr.Y, hpr.Z);
    const core::vector3df &xyz=m_node->getPosition();
    Vec3 p(xyz);
    btTransform trans(q,p);
    m_motion_state = new KartMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo info(0, m_motion_state, 
                                                  m_collision_shape);

    m_body = new btRigidBody(info);
    m_user_pointer.set(this);
    m_body->setUserPointer(&m_user_pointer);
    World::getWorld()->getPhysics()->addBody(m_body);
    m_body->setCollisionFlags( m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    m_body->setActivationState(DISABLE_DEACTIVATION);
}   // createPhysicsBody

// ----------------------------------------------------------------------------
/** Destructor. */
ThreeDAnimation::~ThreeDAnimation()
{
    if(m_body)
    {
        World::getWorld()->getPhysics()->removeBody(m_body);
        delete m_body;
        delete m_motion_state;
        delete m_collision_shape;
    }
    
    if (m_triangle_mesh)
    {
        delete m_triangle_mesh;
    }
}   // ~ThreeDAnimation

// ----------------------------------------------------------------------------
/** Updates position and rotation of this model. Called once per time step.
 *  \param dt Time since last call.
 */
void ThreeDAnimation::update(float dt)
{
    if ( UserConfigParams::m_graphical_effects )
    {
        core::vector3df xyz   = m_node->getPosition();
        core::vector3df scale = m_node->getScale();
        AnimationBase::update(dt, &xyz, &m_hpr, &scale);     //updates all IPOs
        m_node->setPosition(xyz);
        m_node->setScale(scale);
        // Note that the rotation order of irrlicht is different from the one
        // in blender. So in order to reproduce the blender IPO rotations 
        // correctly, we have to get the rotations around each axis and combine
        // them in the right order for irrlicht
        core::matrix4 m;
        m.makeIdentity();
        core::matrix4 mx;
        mx.setRotationDegrees(core::vector3df(m_hpr.X, 0, 0));
        core::matrix4 my;
        my.setRotationDegrees(core::vector3df(0, m_hpr.Y, 0));
        core::matrix4 mz;
        mz.setRotationDegrees(core::vector3df(0, 0, m_hpr.Z));
        m = my*mz*mx;
        core::vector3df hpr = m.getRotationDegrees();
        m_node->setRotation(hpr);

        // Now update the position of the bullet body if there is one:
        if(m_body)
        {
            hpr = DEGREE_TO_RAD*hpr;
            btQuaternion q(hpr.X, hpr.Y, hpr.Z);
            Vec3 p(xyz);
            btTransform trans(q,p);
            m_motion_state->setWorldTransform(trans);
        }
    }
}   // update
