//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "config/stk_config.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

#include <ISceneManager.h>
#include <IMeshSceneNode.h>
#include <ITexture.h>
using namespace irr;

#include <memory>
#include <string>
#include <vector>

/** Creates a physical Settings object with the given type, radius and mass.
 */
PhysicalObject::Settings::Settings(BodyTypes type, float radius, float mass)
{
    init();
    m_body_type  = type;
    m_mass       = mass;
    m_radius     = radius;
}   // Settings

// ----------------------------------------------------------------------------
/** Reads the physical settings values from a given XML node.
 */
PhysicalObject::Settings::Settings(const XMLNode &xml_node)
{
    init();
    std::string shape;
    xml_node.get("id",                &m_id               );
    xml_node.get("mass",              &m_mass             );
    xml_node.get("radius",            &m_radius           );
    xml_node.get("height",            &m_height           );
    xml_node.get("friction",          &m_friction         );
    xml_node.get("restitution",       &m_restitution      );
    xml_node.get("linear-factor",     &m_linear_factor    );
    xml_node.get("angular-factor",    &m_angular_factor   );
    xml_node.get("linear-damping",    &m_linear_damping   );
    xml_node.get("angular-damping",   &m_angular_damping  );

    xml_node.get("shape",             &shape              );
    xml_node.get("reset",             &m_crash_reset      );
    xml_node.get("explode",           &m_knock_kart       );
    xml_node.get("flatten",           &m_flatten_kart     );
    xml_node.get("on-kart-collision", &m_on_kart_collision);
    xml_node.get("on-item-collision", &m_on_item_collision);
    m_reset_when_too_low =
        xml_node.get("reset-when-below", &m_reset_height) == 1;

    m_body_type = MP_NONE;
    if     (shape=="cone"  ||
            shape=="coneY"    ) m_body_type = MP_CONE_Y;
    else if(shape=="coneX"    ) m_body_type = MP_CONE_X;
    else if(shape=="coneZ"    ) m_body_type = MP_CONE_Z;
    else if(shape=="cylinder"||
            shape=="cylinderY") m_body_type = MP_CYLINDER_Y;
    else if(shape=="cylinderX") m_body_type = MP_CYLINDER_X;
    else if(shape=="cylinderZ") m_body_type = MP_CYLINDER_Z;
    else if(shape=="box"      ) m_body_type = MP_BOX;
    else if(shape=="sphere"   ) m_body_type = MP_SPHERE;
    else if(shape=="exact"    ) m_body_type = MP_EXACT;

    else
        Log::error("PhysicalObject", "Unknown shape type : %s.",
                   shape.c_str());
}   // Settings(XMLNode)

// ----------------------------------------------------------------------------
/** Initialises a Settings object.
 */
void PhysicalObject::Settings::init()
{
    m_body_type          = PhysicalObject::MP_NONE;
    m_crash_reset        = false;
    m_knock_kart         = false;
    m_mass               = 0.0f;
    m_radius             = -1.0f;
    m_height             = -1.0f;
    m_friction           = 0.5f;   // default bullet value
    m_restitution        = 0.0f;
    m_linear_factor      = Vec3(1.0f, 1.0f, 1.0f);
    m_angular_factor     = Vec3(1.0f, 1.0f, 1.0f); 
    m_linear_damping     = 0.0f;
    // Make sure that the cones stop rolling by defining angular friction != 0.
    m_angular_damping    = 0.5f;
    m_reset_when_too_low = false;
    m_flatten_kart       = false;
}   // Settings

// ============================================================================
PhysicalObject* PhysicalObject::fromXML(bool is_dynamic,
                                        const XMLNode &xml_node,
                                        TrackObject* object)
{
    PhysicalObject::Settings settings(xml_node);
    return new PhysicalObject(is_dynamic, settings, object);
}   // fromXML

// ----------------------------------------------------------------------------

PhysicalObject::PhysicalObject(bool is_dynamic,
                               const PhysicalObject::Settings& settings,
                               TrackObject* object)
{
    m_shape              = NULL;
    m_body               = NULL;
    m_motion_state       = NULL;
    m_reset_when_too_low = false;
    m_reset_height       = 0;
    m_mass               = 1;
    m_radius             = -1;
    m_crash_reset        = false;
    m_explode_kart       = false;
    m_flatten_kart       = false;
    m_triangle_mesh      = NULL;

    m_object             = object;
    m_init_xyz           = object->getAbsoluteCenterPosition();
    m_init_hpr           = object->getRotation();
    m_init_scale         = object->getScale();

    m_id                 = settings.m_id;
    m_mass               = settings.m_mass;
    m_radius             = settings.m_radius;
    m_body_type          = settings.m_body_type;
    m_crash_reset        = settings.m_crash_reset;
    m_explode_kart       = settings.m_knock_kart;
    m_flatten_kart       = settings.m_flatten_kart;
    m_reset_when_too_low = settings.m_reset_when_too_low;
    m_reset_height       = settings.m_reset_height;
    m_on_kart_collision  = settings.m_on_kart_collision;
    m_on_item_collision  = settings.m_on_item_collision;
    m_body_added = false;

    m_init_pos.setIdentity();
    Vec3 radHpr(m_init_hpr);
    radHpr.degreeToRad();
    btQuaternion q;
    q.setEuler(radHpr.getY(),radHpr.getX(), radHpr.getZ());
    m_init_pos.setRotation(q);
    Vec3 init_xyz(m_init_xyz);
    m_init_pos.setOrigin(init_xyz);

    m_is_dynamic = is_dynamic;

    init(settings);
}   // PhysicalObject

// ----------------------------------------------------------------------------
PhysicalObject::~PhysicalObject()
{
    Physics::getInstance()->removeBody(m_body);
    delete m_body;
    delete m_motion_state;

    // If an exact shape was used, the collision shape pointer
    // here is a copy of the collision shape pointer in the
    // triangle mesh. In order to avoid double-freeing this
    // pointer, we don't free the pointer in this case.
    if (!m_triangle_mesh)
        delete m_shape;

    if (m_triangle_mesh)
    {
        delete m_triangle_mesh;
    }
}  // ~PhysicalObject

// ----------------------------------------------------------------------------

void PhysicalObject::move(const Vec3& xyz, const core::vector3df& hpr)
{
    Vec3 hpr2(hpr);
    hpr2.degreeToRad();
    btQuaternion q;

    core::matrix4 mat;
    mat.setRotationDegrees(hpr);

    irr::core::quaternion tempQuat(mat);
    q = btQuaternion(tempQuat.X, tempQuat.Y, tempQuat.Z, tempQuat.W);

    btTransform trans(q, xyz-quatRotate(q,m_graphical_offset));
    m_motion_state->setWorldTransform(trans);
}   // move

// ----------------------------------------------------------------------------
/** Additional initialisation after loading of the model is finished.
 */
void PhysicalObject::init(const PhysicalObject::Settings& settings)
{
    // 1. Determine size of the object
    // -------------------------------
    Vec3 min, max;

    TrackObjectPresentationSceneNode* presentation =
        m_object->getPresentation<TrackObjectPresentationSceneNode>();

    if (presentation->getNode()->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMesh *mesh
            = ((scene::IAnimatedMeshSceneNode*)presentation->getNode())->getMesh();

        MeshTools::minMax3D(mesh, &min, &max);
    }
    else if (presentation->getNode()->getType()==scene::ESNT_MESH)
    {
        scene::IMesh *mesh
            = ((scene::IMeshSceneNode*)presentation->getNode())->getMesh();

        MeshTools::minMax3D(mesh, &min, &max);
    }
    else if (presentation->getNode()->getType()==scene::ESNT_LOD_NODE)
    {
        scene::ISceneNode* node =
            ((LODNode*)presentation->getNode())->getAllNodes()[0];
        if (node->getType() == scene::ESNT_ANIMATED_MESH)
        {
            scene::IAnimatedMesh *mesh
                = ((scene::IAnimatedMeshSceneNode*)node)->getMesh();

            MeshTools::minMax3D(mesh, &min, &max);
        }
        else if (node->getType()==scene::ESNT_MESH)
        {
            scene::IMesh *mesh
                = ((scene::IMeshSceneNode*)node)->getMesh();

            MeshTools::minMax3D(mesh, &min, &max);
        }
        else
        {
            Log::fatal("PhysicalObject", "Unknown node type");
        }
    }
    else
    {
        Log::fatal("PhysicalObject", "Unknown node type");
    }

    Vec3 parent_scale(1.0f, 1.0f, 1.0f);
    if (m_object->getParentLibrary() != NULL)
    {
        parent_scale = m_object->getParentLibrary()->getScale();
    }

    max = max * (Vec3(m_init_scale) * parent_scale);
    min = min * (Vec3(m_init_scale) * parent_scale);

    Vec3 extend = max-min;
    // Adjust the mesth of the graphical object so that its center is where it
    // is in bullet (usually at (0,0,0)). It can be changed in the case clause
    // if this is not correct for a particular shape.
    m_graphical_offset = -0.5f*(max+min);
    switch (m_body_type)
    {
    case MP_CONE_Y:
    {
        if (m_radius < 0) m_radius = 0.5f*extend.length_2d();
        m_shape = new btConeShape(m_radius, extend.getY());
        break;
    }
    case MP_CONE_X:
    {
        if (m_radius < 0)
            m_radius = 0.5f*sqrt(extend.getY()*extend.getY() +
                                 extend.getZ()*extend.getZ());
        m_shape = new btConeShapeX(m_radius, extend.getY());
        break;
    }
    case MP_CONE_Z:
    {
        if (m_radius < 0)
            m_radius = 0.5f*sqrt(extend.getX()*extend.getX() +
                                 extend.getY()*extend.getY());
        m_shape = new btConeShapeZ(m_radius, extend.getY());
        break;
    }
    case MP_CYLINDER_Y:
    {
        if(settings.m_height > 0)
            extend.setY(settings.m_height);
        if (m_radius < 0) m_radius = 0.5f*extend.length_2d();
        m_shape = new btCylinderShape(0.5f*extend);
        break;
    }
    case MP_CYLINDER_X:
    {
        if (m_radius < 0)
            m_radius = 0.5f*sqrt(extend.getY()*extend.getY() +
                                 extend.getZ()*extend.getZ());
        m_shape = new btCylinderShapeX(0.5f*extend);
        break;
    }
    case MP_CYLINDER_Z:
    {
        if (m_radius < 0)
            m_radius = 0.5f*sqrt(extend.getX()*extend.getX() +
                                 extend.getY()*extend.getY());
        m_shape = new btCylinderShapeZ(0.5f*extend);
        break;
    }
    case MP_SPHERE:
    {
        if(m_radius<0)
        {
            m_radius =      std::max(extend.getX(), extend.getY());
            m_radius = 0.5f*std::max(m_radius,      extend.getZ());
        }
        m_shape = new btSphereShape(m_radius);
        break;
    }
    case MP_EXACT:
    {
        extend.setY(0);

        // In case of readonly materials we have to get the material from
        // the mesh, otherwise from the node. This is esp. important for
        // water nodes, which only have the material defined in the node,
        // but not in the mesh at all!
        bool is_readonly_material = false;

        scene::IMesh* mesh = NULL;
        switch (presentation->getNode()->getType())
        {
            case scene::ESNT_MESH          :
            case scene::ESNT_WATER_SURFACE :
            case scene::ESNT_OCTREE        :
                {
                    scene::IMeshSceneNode *node =
                        (scene::IMeshSceneNode*)presentation->getNode();
                    mesh = node->getMesh();
                    is_readonly_material = node->isReadOnlyMaterials();
                    break;
                }
            case scene::ESNT_ANIMATED_MESH :
                {
                    // for now just use frame 0
                    scene::IAnimatedMeshSceneNode *node =
                      (scene::IAnimatedMeshSceneNode*)presentation->getNode();
                    mesh = node->getMesh()->getMesh(0);
                    is_readonly_material = node->isReadOnlyMaterials();
                    break;
                }
            default:
                Log::warn("PhysicalObject", "Unknown object type, "
                                        "cannot create exact collision body!");
                return;
        }   // switch node->getType()

        std::unique_ptr<TriangleMesh> 
                   triangle_mesh(new TriangleMesh(/*can_be_transformed*/true));

        for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++)
        {
            scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
            // FIXME: take translation/rotation into account
            if (mb->getVertexType() != video::EVT_STANDARD &&
                mb->getVertexType() != video::EVT_2TCOORDS &&
                mb->getVertexType() != video::EVT_TANGENTS)
            {
                Log::warn("PhysicalObject",
                          "createPhysicsBody: Ignoring type '%d'!",
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
                : presentation->getNode()->getMaterial(i);
            video::ITexture* t=irrMaterial.getTexture(0);

            const Material* material=0;
            if(t)
            {
                std::string image =
                              std::string(core::stringc(t->getName()).c_str());
                material = material_manager
                         ->getMaterial(StringUtils::getBasename(image));
                if(material->isIgnore())
                    continue;
            }

            u16 *mbIndices = mb->getIndices();
            Vec3 vertices[3];
            Vec3 normals[3];

            if (mb->getVertexType() == video::EVT_STANDARD)
            {
                irr::video::S3DVertex* mbVertices =
                                          (video::S3DVertex*)mb->getVertices();
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
                    triangle_mesh->addTriangle(vertices[0], vertices[1],
                                               vertices[2], normals[0],
                                               normals[1],  normals[2],
                                               material                 );
                }   // for j
            }
            else if (mb->getVertexType() == video::EVT_2TCOORDS)
            {
                irr::video::S3DVertex2TCoords* mbVertices =
                    (video::S3DVertex2TCoords*)mb->getVertices();
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
                    triangle_mesh->addTriangle(vertices[0], vertices[1],
                                               vertices[2], normals[0],
                                               normals[1],  normals[2],
                                               material                 );
                }   // for j
            }
            else if (mb->getVertexType() == video::EVT_TANGENTS)
            {
                irr::video::S3DVertexTangents* mbVertices =
                    (video::S3DVertexTangents*)mb->getVertices();
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
                    triangle_mesh->addTriangle(vertices[0], vertices[1],
                                               vertices[2], normals[0],
                                               normals[1],  normals[2],
                                               material                 );
                }   // for j
            }

        }   // for i<getMeshBufferCount
        triangle_mesh->createCollisionShape();
        m_shape = &triangle_mesh->getCollisionShape();
        m_triangle_mesh = triangle_mesh.release();
        m_init_pos.setOrigin(m_init_pos.getOrigin() + m_graphical_offset);
        // m_graphical_offset = Vec3(0,0,0);
        break;
    }
    case MP_NONE:
    default:
        Log::warn("PhysicalObject", "Uninitialised moving shape");
        // intended fall-through
    case MP_BOX:
    {
        m_shape = new btBoxShape(0.5*extend);
        break;
    }
    }

    // 2. Create the rigid object
    // --------------------------
    // m_init_pos is the point on the track - add the offset
    if (m_is_dynamic)
    {
        m_init_pos.setOrigin(m_init_pos.getOrigin() +
                             btVector3(0, extend.getY()*0.5f, 0));
    }


    // If this object has a parent, apply the parent's rotation
    if (m_object->getParentLibrary() != NULL)
    {
        core::vector3df parent_rot_hpr = m_object->getParentLibrary()->getInitRotation();
        core::matrix4 parent_rot_matrix;
        parent_rot_matrix.setRotationDegrees(parent_rot_hpr);

        btQuaternion child_rot_quat = m_init_pos.getRotation();
        core::matrix4 child_rot_matrix;
        Vec3 axis = child_rot_quat.getAxis();
        child_rot_matrix.setRotationAxisRadians(child_rot_quat.getAngle(), axis.toIrrVector());

        irr::core::quaternion tempQuat(parent_rot_matrix * child_rot_matrix);
        btQuaternion q(tempQuat.X, tempQuat.Y, tempQuat.Z, tempQuat.W);

        m_init_pos.setRotation(q);
    }

    m_motion_state = new btDefaultMotionState(m_init_pos);
    btVector3 inertia(1,1,1);
    if (m_body_type != MP_EXACT)
        m_shape->calculateLocalInertia(m_mass, inertia);
    else
    {
        if (m_mass == 0)
            inertia.setValue(0, 0, 0);
    }
    btRigidBody::btRigidBodyConstructionInfo info(m_mass, m_motion_state,
                                                  m_shape, inertia);

    if(m_triangle_mesh)
        m_body = new TriangleMesh::RigidBodyTriangleMesh(m_triangle_mesh, info);
    else
        m_body = new btRigidBody(info);
    m_user_pointer.set(this);
    m_body->setUserPointer(&m_user_pointer);

    m_body->setFriction(settings.m_friction);
    m_body->setRestitution(settings.m_restitution);
    m_body->setLinearFactor(settings.m_linear_factor);
    m_body->setAngularFactor(settings.m_angular_factor);
    m_body->setDamping(settings.m_linear_damping,settings.m_angular_damping);

    if (!m_is_dynamic)
    {
        m_body->setCollisionFlags(   m_body->getCollisionFlags()
                                   | btCollisionObject::CF_KINEMATIC_OBJECT);
        m_body->setActivationState(DISABLE_DEACTIVATION);
    }

    Physics::getInstance()->addBody(m_body);
    m_body_added = true;
    if(m_triangle_mesh)
        m_triangle_mesh->setBody(m_body);
}   // init

// ----------------------------------------------------------------------------
void PhysicalObject::update(float dt)
{
    if (!m_is_dynamic) return;

    btTransform t;
    m_motion_state->getWorldTransform(t);

    Vec3 xyz = t.getOrigin();
    if(m_reset_when_too_low && xyz.getY()<m_reset_height)
    {
        m_body->setCenterOfMassTransform(m_init_pos);
        m_body->setLinearVelocity (btVector3(0,0,0));
        m_body->setAngularVelocity(btVector3(0,0,0));
        xyz = Vec3(m_init_pos.getOrigin());
    }
    // Offset the graphical position correctly:
    xyz += t.getBasis()*m_graphical_offset;

    //m_node->setPosition(xyz.toIrrVector());
    Vec3 hpr;
    hpr.setHPR(t.getRotation());
    //m_node->setRotation(hpr.toIrrHPR());

    m_object->move(xyz.toIrrVector(), hpr.toIrrVector()*RAD_TO_DEGREE,
                   m_init_scale, false, true /* isAbsoluteCoord */);
}   // update

// ----------------------------------------------------------------------------
/** Does a raycast against this physical object. The physical object must
 *  have an 'exact' shape, i.e. be a triangle mesh (for other physical objects
 *  no material information would be available).
 *  \param from/to The from and to position for the raycast.
 *  \param xyz The position in world where the ray hit.
 *  \param material The material of the mesh that was hit.
 *  \param normal The intrapolated normal at that position.
 *  \param interpolate_normal If true, the returned normal is the interpolated
 *         based on the three normals of the triangle and the location of the
 *         hit point (which is more compute intensive, but results in much
 *         smoother results).
 *  \return True if a triangle was hit, false otherwise (and no output
 *          variable will be set.
 */
bool PhysicalObject::castRay(const btVector3 &from, const btVector3 &to, 
                             btVector3 *hit_point, const Material **material, 
                             btVector3 *normal, bool interpolate_normal) const
{
    if(m_body_type!=MP_EXACT)
    {
        Log::warn("PhysicalObject", "Can only raycast against 'exact' meshes.");
        return false;
    }
    bool result = m_triangle_mesh->castRay(from, to, hit_point, 
                                           material, normal, 
                                           interpolate_normal);
    return result;
}   // castRay

// ----------------------------------------------------------------------------
void PhysicalObject::reset()
{
    m_body->setCenterOfMassTransform(m_init_pos);
    m_body->setAngularVelocity(btVector3(0,0,0));
    m_body->setLinearVelocity(btVector3(0,0,0));
    m_body->activate();
}   // reset

// ----------------------------------------------------------------------------
void PhysicalObject::handleExplosion(const Vec3& pos, bool direct_hit)
{

    if(direct_hit)
    {
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

// ----------------------------------------------------------------------------
/** Returns true if this object is a soccer ball.
 */
bool PhysicalObject::isSoccerBall() const
{
    return m_object->isSoccerBall();
}   // is SoccerBall

// ----------------------------------------------------------------------------
/** Sets interaction type for object*/
void PhysicalObject::setInteraction(std::string interaction){
    if ( interaction == "flatten") m_flatten_kart = true;
    if ( interaction == "reset") m_crash_reset = true;
    if ( interaction == "explode") m_explode_kart = true;
    if ( interaction == "none" )
    {
        m_flatten_kart = false;
        m_crash_reset = false;
        m_explode_kart = false;
    }
}   // set interaction

// ----------------------------------------------------------------------------
/** Remove body from physics dynamic world interaction type for object*/
void PhysicalObject::removeBody()
{
    if (m_body_added)
    {
        Physics::getInstance()->removeBody(m_body);
        m_body_added = false;
    }
}   // Remove body

// ----------------------------------------------------------------------------
/** Add body to physics dynamic world */
void PhysicalObject::addBody()
{
    if (!m_body_added)
    {
        m_body_added = true;
        Physics::getInstance()->addBody(m_body);
    }
}   // Add body

// ----------------------------------------------------------------------------
/** Called when a physical object hits the track. Atm only used to push a
 *  soccer ball away from the edge of the field.
 *  \param m Material which was hit.
 *  \param normal Normal of the track at the hit point.
 */
void PhysicalObject::hit(const Material *m, const Vec3 &normal)
{
    if(isSoccerBall() && m != NULL &&
       m->getCollisionReaction() == Material::PUSH_SOCCER_BALL)
    {
        m_body->applyCentralImpulse(normal * m_mass * 5.0f);
    }
}   // hit

// ----------------------------------------------------------------------------
/* EOF */

