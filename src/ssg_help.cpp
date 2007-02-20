//  $Id: ssg_help.cpp 837 2006-10-23 07:43:05Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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


#include <iostream>
#include <plib/ssg.h>
#include "ssg_help.hpp"
#include "translation.hpp"

// -----------------------------------------------------------------------------
/** Make VtTables use display lists.
 *
 *  Calls recursively 'makeDList' in all ssgVtxTable of the entity.
 *  \param entity Tree in which to create display lists.
 */
void createDisplayLists(ssgEntity* entity)
{
    if (!entity) return;

    ssgVtxTable* table = dynamic_cast<ssgVtxTable*>(entity);
    if(table)
    {
        if(table->getNumTriangles()>1) table->makeDList();
    }
    ssgBranch* branch = dynamic_cast<ssgBranch*>(entity);

    if (branch)
    {
        for(ssgEntity* i = branch->getKid(0); i != NULL;
            i = branch->getNextKid())
        {
            createDisplayLists(i);
        }   // for
    }   // if branch

}  // createDisplayLists

// -----------------------------------------------------------------------------
/** Adds a transform node to the branch.
 *
 *  Creates a new ssgTransform node to which all children of the branch are
 *  added. The new ssgTransform is then set as the only child of the
 *  branch.
 *  \param branch The branch to which a transform node is added.
 */
ssgTransform* add_transform(ssgBranch* branch)
{
    if (!branch) return 0;

    ssgTransform* transform = new ssgTransform;
    transform->ref();
    for(ssgEntity* i = branch->getKid(0); i != NULL; i = branch->getNextKid())
    {
        transform->addKid(i);
    }

    branch->removeAllKids();
    branch->addKid(transform);

    // Set some user data, so that the wheel isn't ssgFlatten()'ed
    branch->setUserData(new ssgBase());
    transform->setUserData(new ssgBase());

    return transform;
}   // add_transform

// -----------------------------------------------------------------------------
/** Recursively prints a model.
 *
 * Recursively prints a model. That function can most likely be removed, the
 * print method of the ssg objects do the same.
 * \param entity The entity ro print (can't be constant because of ssg functions
 *               which are not const correct)
 * \param indent Indentation to use
 * \param maxLevel maximum number of levels to print
 */
void print_model(ssgEntity* entity, const int indent, const int maxLevel)
{
    if(maxLevel <0) return;
    if (entity)
    {
        for(int i = 0; i < indent; ++i)
            std::cout << "  ";

        std::cout << entity->getTypeName() << " " << entity->getType() << " '"
        << entity->getPrintableName()
        << "' '"
        << (entity->getName() ? entity->getName() : "null")
        << "' " << entity << std::endl;

        ssgBranch* branch = dynamic_cast<ssgBranch*>(entity);

        if (branch)
        {
            for(ssgEntity* i = branch->getKid(0); i != NULL;
                i = branch->getNextKid())
            {
                print_model(i, indent + 1, maxLevel-1);
            }
        }   // if branch
    }   // if entity
}   // print_model

// -----------------------------------------------------------------------------
/** Computes the minimum and maximum x/y coordinates for a ssgEntity.
 *
 *  Recursively computes the minimum x and y coordinates of a ssgEntity.
 *  \param p ssgEntity for which t compute the extend (can't be constant because
 *  of plib not const correct)
 *  \param x_min minimum x value
 *  \param x_max maximum x value
 *  \param y_min minimum y value
 *  \param y_max maximum y value
 *  \param z_min minimum z value, optional parameter!
 *  \param z_max minimum z value, optional parameter!
 */
void MinMax(ssgEntity* p, float *x_min, float *x_max,
            float *y_min, float *y_max,
            float *z_min, float *z_max)
{
    sgMat4 mat;
    sgMakeIdentMat4(mat);
    *x_min = *y_min =  10000.0f; if(z_min) *z_min =  10000.0f;
    *x_max = *y_max = -10000.0f; if(z_max) *z_max = -10000.0f;
    MinMax(p, mat, x_min, x_max, y_min, y_max, z_min, z_max);

}   // MinMax

/** Internal function used by MinMax. */
void MinMax(ssgEntity* p, sgMat4 m, float* x_min, float* x_max,
            float* y_min, float* y_max,
            float* z_min, float* z_max)
{
    if(p->isAKindOf(ssgTypeLeaf()))
    {
        ssgLeaf* l=(ssgLeaf*)p;
        for(int i=0; i<l->getNumTriangles(); i++)
        {
            short v1,v2,v3;
            sgVec3 vv1, vv2, vv3;

            l->getTriangle(i, &v1, &v2, &v3);

            sgXformPnt3 ( vv1, l->getVertex(v1), m );
            sgXformPnt3 ( vv2, l->getVertex(v2), m );
            sgXformPnt3 ( vv3, l->getVertex(v3), m );
            *x_min = std::min(*x_min, vv1[0]); *x_max = std::max(*x_max, vv1[0]);
            *x_min = std::min(*x_min, vv2[0]); *x_max = std::max(*x_max, vv2[0]);
            *x_min = std::min(*x_min, vv3[0]); *x_max = std::max(*x_max, vv3[0]);
            *y_min = std::min(*y_min, vv1[1]); *y_max = std::max(*y_max, vv1[1]);
            *y_min = std::min(*y_min, vv2[1]); *y_max = std::max(*y_max, vv2[1]);
            *y_min = std::min(*y_min, vv3[1]); *y_max = std::max(*y_max, vv3[1]);
            if(z_min)
            {
                *z_min = std::min(*z_min, vv1[2]);
                *z_min = std::min(*z_min, vv2[2]);
                *z_min = std::min(*z_min, vv3[2]);
            }
            if(z_max)
            {
                *z_max = std::max(*z_max, vv1[2]);
                *z_max = std::max(*z_max, vv2[2]);
                *z_max = std::max(*z_max, vv3[2]);
            }

        }   // for i<p->getNumTriangles
    }
    else if (p->isAKindOf(ssgTypeTransform()))
    {
        ssgBaseTransform* t=(ssgBaseTransform*)p;

        sgMat4 tmpT, tmpM;
        t->getTransform(tmpT);
        sgCopyMat4(tmpM, m);
        sgPreMultMat4(tmpM,tmpT);

        for(ssgEntity* e=t->getKid(0); e!=NULL; e=t->getNextKid())
        {
            MinMax(e, tmpM, x_min, x_max, y_min, y_max, z_min, z_max);
        }   // for i<getNumKids

    }
    else if (p->isAKindOf(ssgTypeBranch()))
    {
        ssgBranch* b =(ssgBranch*)p;
        for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid())
        {
            MinMax(e, m, x_min, x_max, y_min, y_max, z_min, z_max);
        }   // for i<getNumKids
    }
    else
    {
        printf(_("StaticSSG::MinMax: unkown type\n"));
        p->print(stdout, 0, 0);
    }
}   // MinMax

// -----------------------------------------------------------------------------

/* EOF */
