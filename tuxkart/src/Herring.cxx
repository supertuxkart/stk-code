
#include "tuxkart.h"

void Herring::update ()
{
  sgCoord c = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } ;
 
  c . hpr [ 0 ] = h ;
 
  h += 5.0f ;
 
  tr -> setTransform ( &c ) ;
}


Herring::Herring ( sgVec3 colour )
{
  ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
  ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
  ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
  ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

  sgSetVec3(v, -0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v, -0.5, 0.0, 0.5 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.5 ) ; va->add(v) ;
  sgSetVec3(v, -0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.0 ) ; va->add(v) ;

  sgSetVec3(n,  0.0f,  1.0f,  0.0f ) ; na->add(n) ;

  sgCopyVec3 ( c, colour ) ; c[ 3 ] = 1.0f ; ca->add(c) ;
 
  sgSetVec2(t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 0.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2(t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 0.0 ) ; ta->add(t) ;
 

  ssgLeaf *gset = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;
 
  gset -> setState ( herring_gst ) ;
 
  h = 0.0f ;
 
  sh = new Shadow ( -0.5, 0.5, -0.25, 0.25 ) ;
 
  tr = new ssgTransform () ;
 
  tr -> addKid ( sh -> getRoot () ) ;
  tr -> addKid ( gset ) ;
  tr -> ref () ; /* Make sure it doesn't get deleted by mistake */
}


 
Shadow::Shadow ( float x1, float x2, float y1, float y2 )
{
  ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
  ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
  ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
  ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

  sgSetVec4 ( c, 0.0f, 0.0f, 0.0f, 1.0f ) ; ca->add(c) ;
  sgSetVec3 ( n, 0.0f, 0.0f, 1.0f ) ; na->add(n) ;
 
  sgSetVec3 ( v, x1, y1, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x2, y1, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x1, y2, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x2, y2, 0.10 ) ; va->add(v) ;
 
  sgSetVec2 ( t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 1.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 0.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 1.0, 1.0 ) ; ta->add(t) ;
 
  sh = new ssgBranch ;
  sh -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
 
  sh -> setName ( "Shadow" ) ;
 
  ssgVtxTable *gs = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;
 
  gs -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
  gs -> setState ( fuzzy_gst ) ;
  sh -> addKid ( gs ) ;
  sh -> ref () ; /* Make sure it doesn't get deleted by mistake */
}


void HerringInstance::update ()
{
  if ( eaten )
  {
    float t = time_to_return - fclock->getAbsTime () ;

    if ( t > 0 )
    {
      sgVec3 hell ;
      sgCopyVec3 ( hell, xyz ) ;

      if ( t > 1.0f )
        hell [ 2 ] = -1000000.0f ;
      else
        hell [ 2 ] = -t / 2.0f ;

      scs -> setTransform ( hell ) ;
    }
    else
    {
      eaten = FALSE ;
      scs -> setTransform ( xyz ) ;
    }
  }
}


