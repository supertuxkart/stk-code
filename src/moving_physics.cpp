//  $Id: moving_physics.cpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <string>
#include <vector>
#include <plib/sg.h>
#include "moving_physics.hpp"
#include "string_utils.hpp"
#include "world.hpp"
#include "ssg_help.hpp"

#ifdef BULLET
// -----------------------------------------------------------------------------
MovingPhysics::MovingPhysics(const std::string data)
             : ssgTransform(), Callback()
{
    m_shape        = NULL;
    m_body         = NULL;
    m_motion_state = NULL;
    m_mass         = 1;
    setUserData(new ssgBase());
    ref();

    std::vector<std::string> parameters = StringUtils::split(data, ' ');
    if(parameters.size()<2)
    {
        fprintf(stderr, "Invalid physics specification: '%s'\n",data.c_str());
    }
    parameters.erase(parameters.begin());
    std::string &shape=parameters[0];
    m_body_type = BODY_NONE;
    if(shape=="cone") m_body_type = BODY_CONE;
    if(shape=="box" ) m_body_type = BODY_BOX;
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
        else
        {
            if(p[0]=="mass") 
            {
                StringUtils::from_string<float>(p[1], m_mass);
            }
            else
            {
                fprintf(stderr, "Invalid physics parameter string: '%s'\n",data.c_str());
                break;
            }
        }
        parameters.erase(parameters.begin());
    }
}   // MovingPhysics

// -----------------------------------------------------------------------------
MovingPhysics::~MovingPhysics()
{
    world->getPhysics()->getPhysicsWorld()->removeRigidBody(m_body);
    deRef();
    delete m_shape;
    delete m_motion_state;
    delete m_body;
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

    // 1. Remove the object from the graph and attach it to the root
    // -------------------------------------------------------------
    if(getNumParents()>1) 
    {
        fprintf(stderr, "WARNING: physical object with more than one parent!!\n");
        return;
    }
    ssgBranch *parent = getParent(0);

    parent->removeKid(this);
    world->addToScene(this);

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

    //    printf("matrix: ");
    //    for(int i=0; i<4; i++) 
    //        for(int j=0; j<4; j++)
    //            printf("%f ",pos[i][j]);
    //    printf("\n");

    // 3. Determine size of the object
    // -------------------------------
    float x_min, x_max, y_min, y_max, z_min, z_max, radius;
    MinMax(this, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
    m_half_height = 0.5*(z_max-z_min);
    switch (m_body_type)
    {
    case BODY_CONE: radius = 0.5*std::max(x_max-x_min, y_max-y_min);
                    m_shape = new btConeShape(radius, z_max-z_min);
                    setName("cone");
                    break;
    case BODY_BOX:  m_shape = new btBoxShape(btVector3(0.5*(x_max-x_min),
                                                       0.5*(y_max-y_min),
                                                       0.5*(z_max-z_min) ) );
                    setName("box");
                    break;
    case BODY_NONE: fprintf(stderr, "WARNING: Uninitialised moving shape\n");
        break;
    }

    // 4. Create the rigid object
    // --------------------------
    btTransform trans;
    trans.setIdentity();
    //    trans.setOrigin(btVector3(0, 8, 5+rand()%10));
    trans.setOrigin(btVector3(pos[3][0],pos[3][1],pos[3][2]+m_half_height));
    m_motion_state = new btDefaultMotionState(trans);
    float mass     = 10;
    btVector3 inertia;
    m_shape->calculateLocalInertia(mass, inertia);
    m_body = new btRigidBody(mass, m_motion_state, m_shape, inertia);
    world->getPhysics()->getPhysicsWorld()->addRigidBody(m_body);

}   // init

// -----------------------------------------------------------------------------
void MovingPhysics::update(float dt)
{
    btTransform t;
    m_motion_state->getWorldTransform(t);
    float m[4][4];
    t.getOpenGLMatrix((float*)&m);

    //    printf("%lx is %f %f %f\n",this, t.getOrigin().x(),t.getOrigin().y(),t.getOrigin().z());
    // Transfer the new position and hpr to m_curr_pos
    sgCoord m_curr_pos;
    sgSetCoord(&m_curr_pos, m);
    setTransform(&m_curr_pos);
}   // update
// -----------------------------------------------------------------------------
#endif

/* EOF */

