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

#define NOINTERSECT -999999.9

class InfoTriangle{
 public:
  ssgLeaf*     leaf;
  int          indx;
  sgVec3       vv1,vv2,vv3;
  unsigned int testNr;
  sgVec4       plane;
  float        xMin, xMax, yMin, yMax;
  // distance to plane in case of a hit, undefined otherwise
  float        dist;
 public:
  InfoTriangle() {};
  InfoTriangle(ssgLeaf* l,int i,sgVec3 v1,sgVec3 v2,sgVec3 v3) {
                                              leaf = l; indx=i; 
					      sgCopyVec3(vv1,v1); 
					      sgCopyVec3(vv2,v2); 
					      sgCopyVec3(vv3,v3);
					      testNr=0;
					      l->ref();             } 
  ~InfoTriangle()                           { ssgDeRefDelete(leaf); }
  float hot(sgVec3 p);
  int   collision(sgSphere *s);
};   // InfoTriangle

// =============================================================================
typedef std::vector<InfoTriangle*> AllHits;

// =============================================================================
class StaticSSG {
 private:
  /* allLeavesType is a list storing all triangles which 
     touch a certain hash bucket.                        */
  typedef std::vector<InfoTriangle*>  allLeavesType;

  /* This is the list of buckets, each of which stores 
     a list of leaves at that hash value/bucket          */
  typedef std::vector<allLeavesType> allBucketsType;

  allLeavesType   allLeaves;
  allBucketsType* buckets;
  float           xMin, xMax, yMin, yMax;
  int             n, m;
  unsigned int    testNumber;
  float           invdx, invdy;
  ssgEntity*      start;

  void Setup        (int n);
  void MinMax       (ssgEntity* p, sgMat4 m);
  void Distribute   (ssgEntity* start, sgMat4 m);
  void StoreTriangle(ssgLeaf* l, int indx, sgVec3 vv1, sgVec3 vv2, sgVec3 vv3);
  int  GetHash  (float x, float y) { int m1; int n1;
                                     GetRowCol(x,y,&m1,&n1);
				     return GetHash(m1, n1);     }
  int  GetHash  (int m1,  int n1 ) { return m1+n1*m;             }
  void GetRowCol(float x, float y, int *m1, int *n1) { 
                                     *m1=(int)((x-xMin)*invdx);
                                     *n1=(int)((y-yMin)*invdy);      }
 public:
  StaticSSG(ssgEntity* start, int n);
  void Draw(ssgBranch* b);
  float hot(sgVec3 start, sgVec3 end, ssgLeaf** l, sgVec4** nrm);
  int collision(sgSphere *s, AllHits *allHits);
};
#endif
