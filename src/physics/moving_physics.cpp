//  $Id: moving_physics.cpp 839 2006-10-24 00:01:56Z hiker $
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

#include "physics/moving_physics.hpp"

#include <string>
#include <vector>
#ifdef HAVE_IRRLICHT
#include "irrlicht.h"
using namespace irr;
#endif
#define _WINSOCKAPI_
#include <plib/sg.h>

#include "file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/scene.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/coord.hpp"
#include "utils/ssg_help.hpp"
#include "utils/string_utils.hpp"

// -----------------------------------------------------------------------------
#ifdef HAVE_IRRLICHT
MovingPhysics::MovingPhysics(const XMLNode *xml_node)
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
        }   // if(!obj)
    }
    m_mesh = obj->getMesh(0);
    m_node = irr_driver->addMesh(m_mesh);
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
}   // MovingPhysics
#else
MovingPhysics::MovingPhysics(const std::string data)
             : ssgTransform(), Callback()
{
    m_shape        = NULL;
    m_body         = NULL;
    m_motion_state = NULL;
    m_mass         = 1;
    setUserData(new ssgBase());   // prevent tree optimisations to remove this node

    std::vector<std::string> parameters = StringUtils::split(data, ' ');
    if(parameters.size()<2)
    {
        fprintf(stderr, "Invalid physics specification: '%s'\n",data.c_str());
    }
    parameters.erase(parameters.begin());
    std::string &shape=parameters[0];
    m_body_type = MP_NONE;
    if(shape=="cone"   ) m_body_type = MP_CONE;
    else if(shape=="box"    ) m_body_type = MP_BOX;
    else if(shape=="sphere" ) m_body_type = MP_SPHERE;
    parameters.erase(parameters.begin());

    // Scan for additional parameters, which are in the form of keyword=value
    // (without any spaces). Currently only mass=... is supported.
    while(parameters.size()>0)
    {
        // Split the parameter string by '=' to get the keyword and value
        std::vector<std::string> p=StringUtils::split(parameters[0],'=');
        if(p.size()!=2) 
        {
            fprintf(stderr, "Invalid physics parameter string: '%s'\n",data.c_str());
            break;
        } 
        if(p[0]=="mass") 
        {
            StringUtils::from_string<float>(p[1], m_mass);
        }
        else
        {
            fprintf(stderr, "Invalid physics parameter string: '%s'\n",
                    data.c_str());
            break;
        }

        parameters.erase(parameters.begin());
    }
}   // MovingPhysics
#endif

// -----------------------------------------------------------------------------
MovingPhysics::~MovingPhysics()
{
    RaceManager::getWorld()->getPhysics()->removeBody(m_body);
    delete m_body;
    delete m_motion_state;
    delete m_shape;
#ifdef HAVE_IRRLICHT
#else
    stk_scene->remove(this);
#endif
}  // ~MovingPhysics

// -----------------------------------------------------------------------------
/** Additional initialisation after loading of the model is finished.
 */
/* Main problem is that the loader (see world::loadTrack) adds a 
   ssgTransform->ssgRangeSelector->MovingPhysics, and that this ssgTransform M
   contains the actual position. So, if the physic position P would be set
   immediately, the transform M causes the position the object is drawn to be
   wrong (M*P is drawn instead of P). So, to correct this, the body has to be
   attached to the root of the scene graph. The body is therefore removed from
   its old place, and appended to the root. The original position has to be
   recomputed by going up in the scene graph and multiplying the transforms.  */
void MovingPhysics::init()
{
#ifdef HAVE_IRRLICHT
#else
    // 1. Remove the object from the graph and attach it to the root
    // -------------------------------------------------------------
    if(getNumParents()>1) 
    {
        fprintf(stderr, "WARNING: physical object with more than one parent!!\n");
        return;
    }
    ssgBranch *parent = getParent(0);

    stk_scene->add(this);
    parent->removeKid(this);

    // 2. Determine the original position of the object
    // ------------------------------------------------

    ssgEntity *p=parent;
    sgMat4    pos;
    sgMakeIdentMat4(pos);
    while(p->getNumParents())
    {
        if(p->getNumParents()!=1)
        {
            // FIXME: Not sure if this is needed: if one object has more than
            //        one parent (--> this object appears more than once in the
            //        scene), we have to follow all possible ways up to the
            //        root, and for each way add one instance to the root.
            //        For now this is unsupported, and we abort here.
            fprintf(stderr, "MovingPhysics: init: %d parents found, ignored.\n",
                    p->getNumParents());
            return;
        }   // if numparents!=1
        if(p->isAKindOf(ssgTypeTransform()))
        {
            ssgBaseTransform *trans=(ssgBaseTransform*)p;
            sgMat4 change_position;
            trans->getTransform(change_position);
            sgPostMultMat4(pos, change_position);
        }
        if(p->getNumKids()==0)
        {
            ssgBranch *new_parent=p->getParent(0);
            new_parent->removeKid(p);
            p = new_parent;
        }
        else
        {
            p=p->getParent(0);
        }
    }   // while
#endif

    // 3. Determine size of the object
    // -------------------------------
    Vec3 min, max;
#ifdef HAVE_IRRLICHT
    MeshTools::minMax3D(m_mesh, &min, &max);
#else
    SSGHelp::MinMax(this, &min, &max);
#endif
    Vec3 extend = max-min;
    m_half_height = 0.5f*(extend.getZ());
    switch (m_body_type)
    {
    case MP_CONE:   {
                    float radius = 0.5f*std::max(extend.getX(), extend.getY());
                    m_shape = new btConeShapeZ(radius, extend.getZ());
#ifdef HAVE_IRRLICHT
                    
#else
                    setName("cone");
#endif
                    break;
                    }
    case MP_BOX:    m_shape = new btBoxShape(0.5*extend);
#ifndef HAVE_IRRLICHT
                    setName("box");
#endif
                    break;
    case MP_SPHERE: {
                    float radius = std::max(extend.getX(), extend.getY());
                    radius = 0.5f*std::max(radius, extend.getZ());
                    m_shape = new btSphereShape(radius);
#ifndef HAVE_IRRLICHT
                    setName("sphere");
#endif
                    break;
                    }
    case MP_NONE:   fprintf(stderr, "WARNING: Uninitialised moving shape\n");
        break;
    }

    // 4. Create the rigid object
    // --------------------------
#ifdef HAVE_IRRLICHT
    Vec3 pos = m_init_pos.getOrigin();
    pos.setZ(m_init_pos.getOrigin().getZ()+m_half_height);
    m_init_pos.setOrigin(pos);
#else
    m_init_pos.setIdentity();
    m_init_pos.setOrigin(btVector3(pos[3][0],pos[3][1],pos[3][2]+m_half_height));
#endif
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
void MovingPhysics::update(float dt)
{
    btTransform t;
    m_motion_state->getWorldTransform(t);
    Coord c(t);
    if(c.getXYZ().getZ()<-100)
    {
        m_body->setCenterOfMassTransform(m_init_pos);
        c.setXYZ(m_init_pos.getOrigin());
    }
#ifdef HAVE_IRRLICHT
    m_node->setPosition(c.getXYZ().toIrrVector());
    m_node->setRotation(c.getHPR().toIrrHPR());
#else
    setTransform(const_cast<sgCoord*>(&c.toSgCoord()));
#endif

}   // update
// -----------------------------------------------------------------------------
void MovingPhysics::reset()
{
    m_body->setCenterOfMassTransform(m_init_pos);
    m_body->setAngularVelocity(btVector3(0,0,0));
    m_body->setLinearVelocity(btVector3(0,0,0));
    m_body->activate();
}   // reset 

// -----------------------------------------------------------------------------
void MovingPhysics::handleExplosion(const Vec3& pos, bool direct_hit) {
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

