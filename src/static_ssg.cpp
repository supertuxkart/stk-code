//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//        Using the plib functions for InfoTriangle::collision and
//        InfoTriangle::hot, (C) Steve Baker
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
#include <algorithm>
#include "static_ssg.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "ssg_help.hpp"

#ifndef round
# define round(x)  (floor(x+0.5f))
#endif

StaticSSG::StaticSSG(ssgEntity* start_, int nSize)
{
    m_start  = start_;
    m_x_min   = m_y_min =  1E10;
    m_x_max   = m_y_max = -1E10;
    m_test_number = 0;
    Setup(nSize);
}   // StaticSSG

//-----------------------------------------------------------------------------
void StaticSSG::Setup(int nSize)
{
    // First compute the minimum and maximum x/y values of all leaves
    MinMax(m_start, &m_x_min, &m_x_max, &m_y_min, &m_y_max);

    const float RATIO = (m_x_max-m_x_min)/(m_y_max-m_y_min);

    /* The m_n, m_m computed here is the maximum value which can be used,
       so the actual range is 0-m_n and 0-m_m, or m_n+1 and m_m+1 'buckets' */
    m_n = (int)round(sqrt(nSize/RATIO));
    m_m = (int)round(nSize/m_n);

    /* Make sure that we don't use more than requested.
       This is done by reducing the larger value by 1.  */
    while(m_n*m_m>nSize)
    {
        if(m_n>m_m) m_n--; else m_m--;
    }

    /* If 'm_m' would be used instead of 'm_m-1', the hash value for the column
       can be any number between 0 and m_m, i.e. m_m+1 different values!        */
    m_invdx   = (float)(m_m-1)/(m_x_max-m_x_min);
    m_invdy   = (float)(m_n-1)/(m_y_max-m_y_min);
    //JH  printf("hash data: m_m=%d, m_n=%d, m_x_max=%f, m_x_min=%f, m_y_max=%f, m_y_min=%f, m_invdx=%f, m_invdy=%f\m_n",
    //  m_m,m_n,m_x_max,m_x_min,m_y_max,m_y_min,m_invdx,m_invdy);
    m_buckets   = new allBucketsType(m_n*m_m);

    sgMat4 mat;
    sgMakeIdentMat4(mat);
    Distribute(m_start, mat);
}   // Setup

//-----------------------------------------------------------------------------
void StaticSSG::Draw(ssgBranch* b)
{
    for(int j=0; j<m_n; j++)
    {
        for(int i=0; i<m_m; i++)
        {
            ssgVertexArray* a = new ssgVertexArray();
            sgVec3 v;
            float h=0.2f;
            v[0]=m_x_min+1/m_invdx*i;    v[1]= m_y_min+1/m_invdy*j;    v[2]=h; a->add(v);
            v[0]=m_x_min+1/m_invdx*(i+1);v[1]= m_y_min+1/m_invdy*j;    v[2]=h; a->add(v);
            v[0]=m_x_min+1/m_invdx*(i+1);v[1]= m_y_min+1/m_invdy*(j+1);v[2]=h; a->add(v);
            v[0]=m_x_min+1/m_invdx*i;    v[1]= m_y_min+1/m_invdy*(j+1);v[2]=h; a->add(v);
            ssgColourArray* c = new ssgColourArray();
            v[0]=float(j)/float(m_n);v[1]=float(i)/float(m_m);v[2]=0;
            c->add(v);c->add(v);c->add(v);c->add(v);
            ssgVtxTable*    l = new ssgVtxTable(GL_POLYGON, a,
                                                (ssgNormalArray*)NULL,
                                                (ssgTexCoordArray*)NULL,
                                                c);
            //       (ssgColourArray*)NULL);
            b->addKid(l);
        }   // for i
    }   //   for j
}   // Draw

//-----------------------------------------------------------------------------
void StaticSSG::Distribute(ssgEntity* p, sgMat4 m)
{
    if(p->isAKindOf(ssgTypeLeaf()))
    {
        ssgLeaf* l=(ssgLeaf*)p;
        if (material_manager->getMaterial(l)->isIgnore())return;
        for(int i=0; i<l->getNumTriangles(); i++)
        {
            short v1,v2,v3;
            sgVec3 vv1, vv2, vv3;

            l->getTriangle(i, &v1, &v2, &v3);

            sgXformPnt3 ( vv1, l->getVertex(v1), m );
            sgXformPnt3 ( vv2, l->getVertex(v2), m );
            sgXformPnt3 ( vv3, l->getVertex(v3), m );
            StoreTriangle(l, i, vv1, vv2, vv3);
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
            Distribute(e, tmpM);
        }   // for i<getNumKids

    }
    else if (p->isAKindOf(ssgTypeBranch()))
    {
        ssgBranch* b =(ssgBranch*)p;
        for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid())
        {
            Distribute(e, m);
        }   // for i<getNumKids
    }
    else
    {
        printf("StaticSSG::Distribute: unkown type\n");
        p->print(stdout, 0, 0);
    }
}   // Distribute
//-----------------------------------------------------------------------------
void StaticSSG::StoreTriangle(ssgLeaf* l, int indx, sgVec3 vv1,
                              sgVec3 vv2, sgVec3 vv3           )
{
    InfoTriangle* t;
    t = new InfoTriangle(l, indx, vv1, vv2, vv3);
    t->m_x_min = std::min(std::min(vv1[0],vv2[0]), vv3[0]);
    t->m_x_max = std::max(std::max(vv1[0],vv2[0]), vv3[0]);
    t->m_y_min = std::min(std::min(vv1[1],vv2[1]), vv3[1]);
    t->m_y_max = std::max(std::max(vv1[1],vv2[1]), vv3[1]);
    sgMakePlane(t->m_plane, vv1,vv2,vv3);
    int nMin, mMin, nMax, mMax;
    GetRowCol(t->m_x_min, t->m_y_min, &mMin, &nMin);
    GetRowCol(t->m_x_max, t->m_y_max, &mMax, &nMax);
    for(int j=nMin; j<=nMax; j++)
    {
        for(int i=mMin; i<=mMax; i++)
        {
            int nHash = GetHash(i, j);
            (*m_buckets)[nHash].push_back(t);
        }   // for i<=mMax
    }   // for j<=nMax

}   // StoreTriangle

//-----------------------------------------------------------------------------
float StaticSSG::hot(sgVec3 start, sgVec3 end, ssgLeaf** leaf, sgVec4** nrm)
{

    float hot      = NOINTERSECT;
    int N_HASH_START = GetHash(start[0], start[1]);
    int nTriangles = (*m_buckets)[N_HASH_START].size();
    *leaf          = NULL;
    for(int i=0; i<nTriangles; i++)
    {
        InfoTriangle *t = (*m_buckets)[N_HASH_START][i];
        const float HOT_NEW = t->hot(start);
        if(HOT_NEW>hot)
        {
            hot   = HOT_NEW;
            *leaf = t->m_leaf;
            *nrm  = &t->m_plane;
        }
    }   // for i <nTriangles

    if(start[0]==end[0] && start[1]==end[1]) return hot;

    const int HASH_END = GetHash(end[0], end[1]);
    nTriangles   = (*m_buckets)[HASH_END].size();
    for(int i=0; i<nTriangles; i++)
    {
        InfoTriangle *t = (*m_buckets)[HASH_END][i];
        const float HOT_NEW = t->hot(end);
        if(HOT_NEW>hot)
        {
            hot=HOT_NEW;
            *leaf = t->m_leaf;
        }
    }   // for i <nTriangles
    return hot;
}   // StaticSSG::hot
//-----------------------------------------------------------------------------
int StaticSSG::collision(sgSphere *s, AllHits *allHits)
{

    sgVec3 center; sgCopyVec3(center,s->getCenter());
    float  RADIUS  = s->getRadius();

    /* m_test_number is used to tag each triangle tested during this call to
       collision. This way we make sure that each triangle is tested at 
       most once, even if it belongs to different buckets. To remove the 
       need to reset the m_test_nr value, we increase the testnumber at each test, 
       and only reset this flag if the m_test_number wraps around again to 
       zero ... which is unlikely to happen :)                              */
    m_test_number++;
    if(m_test_number==0)
    {
        /* wrap around of the test number, reset all m_test_nr values to zero,
           and set m_test_number to 1 to have a clean start                      */
        for(unsigned int i=0; i<m_all_leaves.size(); i++)
        {
            m_all_leaves[i]->m_test_nr=0;
        }
        m_test_number=1;
    }   // if m_test_number==0

    int nMin, nMax, mMin, mMax;
    GetRowCol(center[0]-RADIUS, center[1]-RADIUS, &mMin, &nMin);
    GetRowCol(center[0]+RADIUS, center[1]+RADIUS, &mMax, &nMax);
    int count=0;
    for(int j=nMin; j<=nMax; j++)
    {
        for(int i=mMin; i<=mMax; i++)
        {
            int N_HASH  = GetHash(i, j);
            if(N_HASH<0 || N_HASH>=m_n*m_m)
            {   // that should be a car off track
                continue;                   // rescue should take care of this
            }
            int nCount = (*m_buckets)[N_HASH].size();
            for(int k=0; k<nCount; k++)
            {
                InfoTriangle *t = (*m_buckets)[N_HASH][k];
                // Make sure that each triangle is tested only once
                if(t->m_test_nr!=m_test_number)
                {
                    t->m_test_nr=m_test_number;
                    if(t->collision(s))
                    {
                        allHits->push_back(t);
                        count++;
                    }   // if hit
                }   // if t->m_test_nr!=m_test_number
            }   // for k
        }   // for i<=mMax
    }   // for j<=nMax

    return count;
}   // StaticSSG::collision

//=============================================================================
float InfoTriangle::hot(sgVec3 s)
{
    /*
      Does the X/Y coordinate lie outside the triangle's bbox, or
      does the Z coordinate lie beneath the bbox ?
    */
    if ( ( s[0] < m_vv1[0] && s[0] < m_vv2[0] && s[0] < m_vv3[0] ) ||
         ( s[1] < m_vv1[1] && s[1] < m_vv2[1] && s[1] < m_vv3[1] ) ||
         ( s[0] > m_vv1[0] && s[0] > m_vv2[0] && s[0] > m_vv3[0] ) ||
         ( s[1] > m_vv1[1] && s[1] > m_vv2[1] && s[1] > m_vv3[1] ) ||
         ( s[2] < m_vv1[2] && s[2] < m_vv2[2] && s[2] < m_vv3[2] ) ) return NOINTERSECT;


    /* No HOT from upside-down or vertical triangles */
    if ( m_leaf->getCullFace() && m_plane [ 2 ] <= 0 ) return NOINTERSECT;

    /* Find the point vertically below the text point
       as it crosses the plane of the polygon */
    const float Z = sgHeightOfPlaneVec2 ( m_plane, s );

    /* No HOT from below the triangle */
    if ( Z > s[2] ) return NOINTERSECT;

    /* Outside the vertical extent of the triangle? */
    if ( ( Z < m_vv1[2] && Z < m_vv2[2] && Z < m_vv3[2] ) ||
         ( Z > m_vv1[2] && Z > m_vv2[2] && Z > m_vv3[2] ) ) return NOINTERSECT;

    /*
      Now it gets messy - the isect point is inside
      the bbox of the triangle - but that's not enough.
      Is it inside the triangle itself?
    */
    const float  E1 =  s [0] * m_vv1[1] -  s [1] * m_vv1[0] ;
    const float  E2 =  s [0] * m_vv2[1] -  s [1] * m_vv2[0] ;
    const float  E3 =  s [0] * m_vv3[1] -  s [1] * m_vv3[0] ;
    const float EP1 = m_vv1[0] * m_vv2[1] - m_vv1[1] * m_vv2[0] ;
    const float EP2 = m_vv2[0] * m_vv3[1] - m_vv2[1] * m_vv3[0] ;
    const float EP3 = m_vv3[0] * m_vv1[1] - m_vv3[1] * m_vv1[0] ;

    const float AP = (float) fabs ( EP1 + EP2 + EP3 ) ;
    const float AI = (float) ( fabs ( E1 + EP1 - E2 ) +
                         fabs ( E2 + EP2 - E3 ) +
                         fabs ( E3 + EP3 - E1 ) ) ;

    if ( AI > AP * 1.01 ) return NOINTERSECT;

    return Z;
}   // InfoTriangle::hot

//-----------------------------------------------------------------------------
int InfoTriangle::collision(sgSphere *s)
{

    const sgVec3 * const CENTER = (sgVec3*)s->getCenter();
    const float R = s->getRadius();

    /* First test: see if the sphere is outside the 2d bounding box
       of the triangle - a quite fast and easy test                 */
    if((*CENTER)[0]+R<m_x_min || (*CENTER)[0]-R>m_x_max ||
       (*CENTER)[1]+R<m_y_min || (*CENTER)[1]-R>m_y_max)
    {
        return 0;
    }

    m_dist = (float)fabs( sgDistToPlaneVec3(m_plane, s->getCenter()) );

    if ( m_dist > R ) return 0;

    /*
      The BSphere touches the plane containing
      the triangle - but does it actually touch
      the triangle itself?  Let's erect some
      vertical walls around the triangle.
    */

    /*
      Construct a 'wall' as a plane through
      two vertices and a third vertex made
      by adding the surface normal to the
      first of those two vertices.
    */

    sgVec3 vvX;
    sgVec4 planeX;

    sgAddVec3 ( vvX, m_plane, m_vv1 );
    sgMakePlane ( planeX, m_vv1, m_vv2, vvX );
    const float DP1 = sgDistToPlaneVec3 ( planeX, s->getCenter() );

    if ( DP1 > s->getRadius() ) return 0;

    sgAddVec3 ( vvX, m_plane, m_vv2 );
    sgMakePlane ( planeX, m_vv2, m_vv3, vvX );
    const float DP2 = sgDistToPlaneVec3 ( planeX, s->getCenter() );

    if ( DP2 > s->getRadius() ) return 0;

    sgAddVec3 ( vvX, m_plane, m_vv3 );
    sgMakePlane ( planeX, m_vv3, m_vv1, vvX );
    const float DP3 = sgDistToPlaneVec3 ( planeX, s->getCenter() );

    if ( DP3 > s->getRadius() ) return 0;

    /*
      OK, so we now know that the sphere
      intersects the plane of the triangle
      and is not more than one radius outside
      the walls. However, you can still get
      close enough to the wall and to the
      triangle itself and *still* not
      intersect the triangle itself.

      However, if the center is inside the
      triangle then we don't need that
      costly test.
    */

    if ( DP1 <= 0 && DP2 <= 0 && DP3 <= 0 ) return 1;

    /*
      <sigh> ...now we really need that costly set of tests...

      If the sphere penetrates the plane of the triangle
      and the plane of the wall, then we can use pythagoras
      to determine if the sphere actually intersects that
      edge between the wall and the triangle.

      if ( dp_sqd + dp1_sqd > radius_sqd ) ...in! else ...out!
    */

    const float R2 = s->getRadius() * s->getRadius() - m_dist * m_dist ;

    if ( DP1 * DP1 <= R2 || DP2 * DP2 <= R2 || DP3 * DP3 <= R2 )
    {
        return 1;
    }
    return 0;
}   // InfoTriangle::collision
