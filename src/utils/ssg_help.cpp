//  $Id: ssg_help.cpp 837 2006-10-23 07:43:05Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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

#include "utils/ssg_help.hpp"

#include <iostream>

#if 0

#include <plib/ssg.h>

namespace SSGHelp
{
    // ------------------------------------------------------------------------
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

    // ------------------------------------------------------------------------
    /** Recursively prints a model.
     *
     * Recursively prints a model. That function can most likely be removed, the
     * print method of the ssg objects do the same.
     * \param entity The entity ro print (can't be constant because of ssg 
     *               functions which are not const correct)
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

            std::cout << entity->getTypeName() << " " << entity->getType() 
                << " '"
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

    // ------------------------------------------------------------------------
    /** MinMax helper function which uses a transform and then computes the
     *  minimum/maximum for this subtree.
     *  \param p Subtree.
     *  \param m Transformation to apply.
     *  \param min On return contains the minimum.
     *  \param max On return contains the maximum.
     */
    static void MinMaxInternal(const ssgEntity* p, sgMat4 m,  Vec3 *min, Vec3 *max)
    {
        if(const_cast<ssgEntity*>(p)->isAKindOf(ssgTypeLeaf()))
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
                Vec3 vec3vv1(vv1);
                Vec3 vec3vv2(vv2);
                Vec3 vec3vv3(vv3);
                min->min(vec3vv1);
                min->min(vec3vv2);
                min->min(vec3vv3);
                max->max(vec3vv1);
                max->max(vec3vv2);
                max->max(vec3vv3);
            }   // for i<p->getNumTriangles
        }
        else if (const_cast<ssgEntity*>(p)->isAKindOf(ssgTypeTransform()))
        {
            ssgBaseTransform* t=(ssgBaseTransform*)p;

            sgMat4 tmpT, tmpM;
            t->getTransform(tmpT);
            sgCopyMat4(tmpM, m);
            sgPreMultMat4(tmpM,tmpT);

            for(ssgEntity* e=t->getKid(0); e!=NULL; e=t->getNextKid())
            {
                MinMaxInternal(e, tmpM, min, max);
            }   // for i<getNumKids

        }
        else if (const_cast<ssgEntity*>(p)->isAKindOf(ssgTypeBranch()))
        {
            ssgBranch* b =(ssgBranch*)p;
            for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid())
            {
                MinMaxInternal(e, m, min, max);
            }   // for i<getNumKids
        }
        else
        {
            printf("StaticSSG::MinMax: unkown type\n");
            const_cast<ssgEntity*>(p)->print(stdout, 0, 0);
        }
    }   // MinMaxInternal
    // ------------------------------------------------------------------------
    /** Computes the minimum and maximum x/y coordinates for a ssgEntity.
     *
     *  Recursively computes the minimum x and y coordinates of a ssgEntity.
     *  \param p ssgEntity for which t compute the extend.
     *  \param min Minimum values in all three dimensions.
     *  \param max Maximum values in all three dimensions.
     */
    void MinMax(const ssgEntity* p, Vec3 *min, Vec3 *max)
    {
        sgMat4 mat;
        sgMakeIdentMat4(mat);
        *min = Vec3(10000.0f);
        *max = Vec3(-10000.0f);
        MinMaxInternal(p, mat,min, max);
    }   // Minmax

}   // namespace SSGHelp
#endif
/* EOF */