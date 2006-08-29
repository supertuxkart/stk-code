//  $Id: static_ssg.cpp,v 1.8 2005/07/19 08:23:40 joh Exp $
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

#include "static_ssg.hpp"
#include "material_manager.hpp"
#include "material.hpp"

StaticSSG::StaticSSG(ssgEntity* start_, int nSize) {
  start  = start_;
  xMin   = yMin =  1E10;
  xMax   = yMax = -1E10;
  Setup(nSize);
}   // StaticSSG

// -----------------------------------------------------------------------------

void StaticSSG::Setup(int nSize) {
  // First compute the minimum and maximum x/y values of all leaves
  sgMat4 mat;
  sgMakeIdentMat4(mat);
  MinMax(start, mat);

  float ratio = (xMax-xMin)/(yMax-yMin);

  /* The n, m computed here is the maximum value which can be used,
     so the actual range is 0-n and 0-m, or n+1 and m+1 'buckets' */
  n = (int)round(sqrt(nSize/ratio));
  m = (int)round(nSize/n);

  /* Make sure that we don't use more than requested. 
     This is done by reducing the larger value by 1.  */
  while(n*m>nSize) {
    if(n>m) n--; else m--;
  }

  /* If 'm' would be used instead of 'm-1', the hash value for the column
     can be any number between 0 and m, i.e. m+1 different values!        */
  invdx   = (float)(m-1)/(xMax-xMin);
  invdy   = (float)(n-1)/(yMax-yMin);
  //JH  printf("hash data: m=%d, n=%d, xMax=%f, xMin=%f, yMax=%f, yMin=%f, invdx=%f, invdy=%f\n",
  //	 m,n,xMax,xMin,yMax,yMin,invdx,invdy);
  buckets   = new allBucketsType(n*m);
  Distribute(start, mat);
}   // Setup

// -----------------------------------------------------------------------------
void StaticSSG::Draw(ssgBranch* b) {
  for(int j=0; j<n; j++) {
    for(int i=0; i<m; i++) {
      ssgVertexArray* a = new ssgVertexArray();
      sgVec3 v;
      float h=0.2;
      v[0]=xMin+1/invdx*i;    v[1]= yMin+1/invdy*j;    v[2]=h; a->add(v);
      v[0]=xMin+1/invdx*(i+1);v[1]= yMin+1/invdy*j;    v[2]=h; a->add(v);
      v[0]=xMin+1/invdx*(i+1);v[1]= yMin+1/invdy*(j+1);v[2]=h; a->add(v);
      v[0]=xMin+1/invdx*i;    v[1]= yMin+1/invdy*(j+1);v[2]=h; a->add(v);
      ssgColourArray* c = new ssgColourArray();
      v[0]=float(j)/float(n);v[1]=float(i)/float(m);v[2]=0;
      c->add(v);c->add(v);c->add(v);c->add(v);
      ssgVtxTable*    l = new ssgVtxTable(GL_POLYGON, a,
					  (ssgNormalArray*)NULL,
					  (ssgTexCoordArray*)NULL,
					  c);
      //					  (ssgColourArray*)NULL);
      b->addKid(l);
    }   // for i
  }   //   for j
}   // Draw

// -----------------------------------------------------------------------------
void StaticSSG::MinMax(ssgEntity* p, sgMat4 m) {
  if(p->isAKindOf(ssgTypeLeaf())) {
    ssgLeaf* l=(ssgLeaf*)p;
    for(int i=0; i<l->getNumTriangles(); i++) {
      short v1,v2,v3;
      sgVec3 vv1, vv2, vv3;

      l->getTriangle(i, &v1, &v2, &v3);

      sgXformPnt3 ( vv1, l->getVertex(v1), m );
      sgXformPnt3 ( vv2, l->getVertex(v2), m );
      sgXformPnt3 ( vv3, l->getVertex(v3), m );
      xMin = std::min(xMin, vv1[0]); xMax = std::max(xMax, vv1[0]);
      xMin = std::min(xMin, vv2[0]); xMax = std::max(xMax, vv2[0]);
      xMin = std::min(xMin, vv3[0]); xMax = std::max(xMax, vv3[0]);
      yMin = std::min(yMin, vv1[1]); yMax = std::max(yMax, vv1[1]);
      yMin = std::min(yMin, vv2[1]); yMax = std::max(yMax, vv2[1]);
      yMin = std::min(yMin, vv3[1]); yMax = std::max(yMax, vv3[1]);
    }   // for i<p->getNumTriangles
  } else if (p->isAKindOf(ssgTypeTransform())) {
    ssgBaseTransform* t=(ssgBaseTransform*)p;

    sgMat4 tmpT, tmpM;
    t->getTransform(tmpT);
    sgCopyMat4(tmpM, m);
    sgPreMultMat4(tmpM,tmpT);

    for(ssgEntity* e=t->getKid(0); e!=NULL; e=t->getNextKid()) {
      MinMax(e, tmpM);
    }   // for i<getNumKids

  } else if (p->isAKindOf(ssgTypeBranch())) {
    ssgBranch* b =(ssgBranch*)p;
    for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid()) {
      MinMax(e, m);
    }   // for i<getNumKids
  } else {
    printf("StaticSSG::MinMax: unkown type\n");
    p->print(stdout, 0, 0);
  }
}   // MinMax
// -----------------------------------------------------------------------------
void StaticSSG::Distribute(ssgEntity* p, sgMat4 m) {
  if(p->isAKindOf(ssgTypeLeaf())) {
    ssgLeaf* l=(ssgLeaf*)p;
    if (material_manager->getMaterial(l)->isIgnore())return;
    for(int i=0; i<l->getNumTriangles(); i++) {
      short v1,v2,v3;
      sgVec3 vv1, vv2, vv3;

      l->getTriangle(i, &v1, &v2, &v3);

      sgXformPnt3 ( vv1, l->getVertex(v1), m );
      sgXformPnt3 ( vv2, l->getVertex(v2), m );
      sgXformPnt3 ( vv3, l->getVertex(v3), m );
      StoreTriangle(l, i, vv1, vv2, vv3);
    }   // for i<p->getNumTriangles
  } else if (p->isAKindOf(ssgTypeTransform())) {
    ssgBaseTransform* t=(ssgBaseTransform*)p;

    sgMat4 tmpT, tmpM;
    t->getTransform(tmpT);
    sgCopyMat4(tmpM, m);
    sgPreMultMat4(tmpM,tmpT);

    for(ssgEntity* e=t->getKid(0); e!=NULL; e=t->getNextKid()) {
      Distribute(e, tmpM);
    }   // for i<getNumKids

  } else if (p->isAKindOf(ssgTypeBranch())) {
    ssgBranch* b =(ssgBranch*)p;
    for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid()) {
      Distribute(e, m);
    }   // for i<getNumKids
  } else {
    printf("StaticSSG::Distribute: unkown type\n");
    p->print(stdout, 0, 0);
  }
}   // Distribute
// -----------------------------------------------------------------------------
void StaticSSG::StoreTriangle(ssgLeaf* l, int indx, sgVec3 vv1, 
			      sgVec3 vv2, sgVec3 vv3           ) {
  InfoTriangle* t;
  t = new InfoTriangle(l, indx, vv1, vv2, vv3);
  t->xMin = std::min(std::min(vv1[0],vv2[0]), vv3[0]);
  t->xMax = std::max(std::max(vv1[0],vv2[0]), vv3[0]);
  t->yMin = std::min(std::min(vv1[1],vv2[1]), vv3[1]);
  t->yMax = std::max(std::max(vv1[1],vv2[1]), vv3[1]);
  sgMakePlane(t->plane, vv1,vv2,vv3);
  int nMin, mMin, nMax, mMax;
  GetRowCol(t->xMin, t->yMin, &mMin, &nMin);
  GetRowCol(t->xMax, t->yMax, &mMax, &nMax);
  for(int j=nMin; j<=nMax; j++) {
    for(int i=mMin; i<=mMax; i++) {
      int nHash = GetHash(i, j);
      (*buckets)[nHash].push_back(t);
    }   // for i<=mMax
  }   // for j<=nMax

}   // StoreTriangle

// -----------------------------------------------------------------------------
float StaticSSG::hot(sgVec3 start, sgVec3 end, ssgLeaf** leaf, sgVec4** nrm) {

  float hot      = NOINTERSECT;
  int nHashStart = GetHash(start[0], start[1]);
  int nTriangles = (*buckets)[nHashStart].size();
  *leaf          = NULL;
  for(int i=0; i<nTriangles; i++) {
    InfoTriangle *t = (*buckets)[nHashStart][i];
    float hotnew = t->hot(start);
    if(hotnew>hot) {
      hot   = hotnew;
      *leaf = t->leaf;
      *nrm  = &t->plane;
    }
  }   // for i <nTriangles

  if(start[0]==end[0] && start[1]==end[1]) return hot;

  int nHashEnd = GetHash(end[0], end[1]);
  nTriangles   = (*buckets)[nHashEnd].size();
  for(int i=0; i<nTriangles; i++) {
    InfoTriangle *t = (*buckets)[nHashEnd][i];
    float hotnew = t->hot(end);
    if(hotnew>hot) {
      hot=hotnew;
      *leaf = t->leaf;
    }
  }   // for i <nTriangles
  return hot;
}   // StaticSSG::hot
// -----------------------------------------------------------------------------
int StaticSSG::collision(sgSphere *s, AllHits *allHits) {
  
  sgVec3 center; sgCopyVec3(center,s->getCenter());
  float  radius  = s->getRadius();

  /* testNumber is used to tag each triangle tested during this call to
     collision. This way we make sure that each triangle is tested at 
     most once, even if it belongs to different buckets. To remove the 
     need to reset the testNr value, we increase the testnumber at each test, 
     and only reset this flag if the testNumber wraps around again to 
     zero ... which is unlikely to happen :)                              */
  testNumber++;
  if(testNumber==0) {
    /* wrap around of the test number, reset all testNr values to zero,
       and set testNumber to 1 to have a clean start                      */
    for(unsigned int i=0; i<allLeaves.size(); i++) {
      allLeaves[i]->testNr=0;
    }
    testNumber=1;
  }   // if testNumber==0

  int nMin, nMax, mMin, mMax;
  GetRowCol(center[0]-radius, center[1]-radius, &mMin, &nMin);
  GetRowCol(center[0]+radius, center[1]+radius, &mMax, &nMax);
  int count=0;
  for(int j=nMin; j<=nMax; j++) {
    for(int i=mMin; i<=mMax; i++) {
      int nHash  = GetHash(i, j);
      if(nHash<0 || nHash>=n*m) {   // that should be a car off track
	continue;                   // rescue should take care of this
      }
      int nCount = (*buckets)[nHash].size();
      for(int k=0; k<nCount; k++) {
	InfoTriangle *t = (*buckets)[nHash][k];
	// Make sure that each triangle is tested only once
	if(t->testNr!=testNumber) {
	  t->testNr=testNumber;
	  if(t->collision(s)) {
	    allHits->push_back(t);
	    count++;
	  }   // if hit
	}   // if t->testNr!=testNumber
      }   // for k
    }   // for i<=mMax
  }   // for j<=nMax

  return count;
}   // StaticSSG::collision

// =============================================================================
float InfoTriangle::hot(sgVec3 s) {
  /*
    Does the X/Y coordinate lie outside the triangle's bbox, or
    does the Z coordinate lie beneath the bbox ?
  */
  if ( ( s[0] < vv1[0] && s[0] < vv2[0] && s[0] < vv3[0] ) ||
       ( s[1] < vv1[1] && s[1] < vv2[1] && s[1] < vv3[1] ) ||
       ( s[0] > vv1[0] && s[0] > vv2[0] && s[0] > vv3[0] ) ||
       ( s[1] > vv1[1] && s[1] > vv2[1] && s[1] > vv3[1] ) ||
       ( s[2] < vv1[2] && s[2] < vv2[2] && s[2] < vv3[2] ) ) return NOINTERSECT;


  /* No HOT from upside-down or vertical triangles */
  if ( leaf->getCullFace() && plane [ 2 ] <= 0 ) return NOINTERSECT;
  
  /* Find the point vertically below the text point
     as it crosses the plane of the polygon */
  float z = sgHeightOfPlaneVec2 ( plane, s );
    
  /* No HOT from below the triangle */
  if ( z > s[2] ) return NOINTERSECT;
  
  /* Outside the vertical extent of the triangle? */
  if ( ( z < vv1[2] && z < vv2[2] && z < vv3[2] ) ||
       ( z > vv1[2] && z > vv2[2] && z > vv3[2] ) ) return NOINTERSECT;
  
  /*
    Now it gets messy - the isect point is inside
    the bbox of the triangle - but that's not enough.
    Is it inside the triangle itself?
  */
  float  e1 =  s [0] * vv1[1] -  s [1] * vv1[0] ;
  float  e2 =  s [0] * vv2[1] -  s [1] * vv2[0] ;
  float  e3 =  s [0] * vv3[1] -  s [1] * vv3[0] ;
  float ep1 = vv1[0] * vv2[1] - vv1[1] * vv2[0] ;
  float ep2 = vv2[0] * vv3[1] - vv2[1] * vv3[0] ;
  float ep3 = vv3[0] * vv1[1] - vv3[1] * vv1[0] ;
  
  float ap = (float) fabs ( ep1 + ep2 + ep3 ) ;
  float ai = (float) ( fabs ( e1 + ep1 - e2 ) +
		       fabs ( e2 + ep2 - e3 ) +
		       fabs ( e3 + ep3 - e1 ) ) ;
  
  if ( ai > ap * 1.01 ) return NOINTERSECT;

  return z;
}   // InfoTriangle::hot
// =============================================================================

int InfoTriangle::collision(sgSphere *s) {
  
  const sgVec3 *center;
  center  = (sgVec3*)s->getCenter();
  float r = s->getRadius();
  
  /* First test: see if the sphere is outside the 2d bounding box
     of the triangle - a quite fast and easy test                 */
  if((*center)[0]+r<xMin || (*center)[0]-r>xMax ||
     (*center)[1]+r<yMin || (*center)[1]-r>yMax) {
    return 0;
  }
  
  dist = (float)fabs( sgDistToPlaneVec3(plane, s->getCenter()) );

  if ( dist > r ) return 0;

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

  sgAddVec3 ( vvX, plane, vv1 );
  sgMakePlane ( planeX, vv1, vv2, vvX );
  float dp1 = sgDistToPlaneVec3 ( planeX, s->getCenter() );
  
  if ( dp1 > s->getRadius() ) return 0;

  sgAddVec3 ( vvX, plane, vv2 );
  sgMakePlane ( planeX, vv2, vv3, vvX );
  float dp2 = sgDistToPlaneVec3 ( planeX, s->getCenter() );
  
  if ( dp2 > s->getRadius() ) return 0;

  sgAddVec3 ( vvX, plane, vv3 );
  sgMakePlane ( planeX, vv3, vv1, vvX );
  float dp3 = sgDistToPlaneVec3 ( planeX, s->getCenter() );
  
  if ( dp3 > s->getRadius() ) return 0;

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
 
  if ( dp1 <= 0 && dp2 <= 0 && dp3 <= 0 ) return 1;
  
  /*
    <sigh> ...now we really need that costly set of tests...
    
    If the sphere penetrates the plane of the triangle
      and the plane of the wall, then we can use pythagoras
      to determine if the sphere actually intersects that
      edge between the wall and the triangle.
      
      if ( dp_sqd + dp1_sqd > radius_sqd ) ...in! else ...out!
  */
  
  float r2 = s->getRadius() * s->getRadius() - dist * dist ;
  
  if ( dp1 * dp1 <= r2 || dp2 * dp2 <= r2 || dp3 * dp3 <= r2 ) {
    return 1;
  }
  return 0;
}   // InfoTriangle::collision
