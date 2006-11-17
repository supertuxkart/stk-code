//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distribhuted in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef _HEADER_STATICSSG_
#define _HEADER_STATICSSG_

#include <vector>
#include <plib/ssg.h>

#define NOINTERSECT -999999.9f

class InfoTriangle
{
public:
    ssgLeaf*     m_leaf;
    int          m_indx;
    sgVec3       m_vv1,m_vv2,m_vv3;
    unsigned int m_test_nr;
    sgVec4       m_plane;
    float        m_x_min, m_x_max, m_y_min, m_y_max;
    // distance to plane in case of a hit, undefined otherwise
    float        m_dist;
public:
    InfoTriangle() {};
    InfoTriangle(ssgLeaf* l,int i,sgVec3 v1,sgVec3 v2,sgVec3 v3)
    {
        m_leaf = l; m_indx=i;
        sgCopyVec3(m_vv1,v1);
        sgCopyVec3(m_vv2,v2);
        sgCopyVec3(m_vv3,v3);
        m_test_nr=0;
        l->ref();
    }
    ~InfoTriangle()                           { ssgDeRefDelete(m_leaf); }
    float hot(sgVec3 p);
    int   collision(sgSphere *s);
}
;   // InfoTriangle

// =============================================================================
typedef std::vector<InfoTriangle*> AllHits;

// =============================================================================
class StaticSSG
{
private:
    /* allLeavesType is a list storing all triangles which
       touch a certain hash bucket.                        */
    typedef std::vector<InfoTriangle*>  allLeavesType;

    /* This is the list of buckets, each of which stores
       a list of leaves at that hash value/bucket          */
    typedef std::vector<allLeavesType> allBucketsType;

    allLeavesType   m_all_leaves;
    allBucketsType* m_buckets;
    float           m_x_min, m_x_max, m_y_min, m_y_max;
    int             m_n, m_m;
    unsigned int    m_test_number;
    float           m_invdx, m_invdy;
    ssgEntity*      m_start;

    void Setup        (int n);
    void Distribute   (ssgEntity* start, sgMat4 m);
    void StoreTriangle(ssgLeaf* l, int indx, sgVec3 vv1, sgVec3 vv2, sgVec3 vv3);
    int  GetHash  (float x, float y)
    {
        int m1; int n1;
        GetRowCol(x,y,&m1,&n1);
        return GetHash(m1, n1);
    }
    int  GetHash  (int m1,  int n1 ) { return m1+n1*m_m;             }
    void GetRowCol(float x, float y, int *m1, int *n1)
    {
        *m1=(int)((x-m_x_min)*m_invdx);
        *n1=(int)((y-m_y_min)*m_invdy);
    }
public:
    StaticSSG(ssgEntity* start, int n);
    void Draw(ssgBranch* b);
    float hot(sgVec3 start, sgVec3 end, ssgLeaf** l, sgVec4** nrm);
    int collision(sgSphere *s, AllHits *allHits);
};
#endif
