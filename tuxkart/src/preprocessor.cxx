#include "tuxkart.h"

void preProcessObj ( ssgEntity *n, int mirror )
{
  if ( n == NULL ) return ;

  n -> dirtyBSphere () ;

  if ( n -> isAKindOf ( ssgTypeLeaf() ) )
  {
    if ( mirror )
      for ( int i = 0 ; i < ((ssgLeaf *)n) -> getNumVertices () ; i++ )
        ((ssgLeaf *)n) -> getVertex ( i ) [ 0 ] *= -1.0f ;

    getMaterial ( (ssgLeaf *) n ) -> applyToLeaf ( (ssgLeaf *) n ) ;
    return ;
  }

  if ( mirror && n -> isAKindOf ( ssgTypeTransform () ) )
  {
    sgMat4 xform ;

    ((ssgTransform *)n) -> getTransform ( xform ) ;
    xform [ 0 ][ 0 ] *= -1.0f ;
    xform [ 1 ][ 0 ] *= -1.0f ;
    xform [ 2 ][ 0 ] *= -1.0f ;
    xform [ 3 ][ 0 ] *= -1.0f ;
    ((ssgTransform *)n) -> setTransform ( xform ) ;
  }

  ssgBranch *b = (ssgBranch *) n ;

  for ( int i = 0 ; i < b -> getNumKids () ; i++ )
    preProcessObj ( b -> getKid ( i ), mirror ) ;
}


