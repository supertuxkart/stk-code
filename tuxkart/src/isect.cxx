
#include "tuxkart.h"


float getHeightAndNormal ( sgVec3 my_position, sgVec3 normal )
{
  /* Look for the nearest polygon *beneath* my_position */

  ssgHit *results ;
  int num_hits ;

  float hot ;        /* H.O.T == Height Of Terrain */
  sgVec3 HOTvec ;

  sgMat4 invmat ;
  sgMakeIdentMat4 ( invmat ) ;
  invmat[3][0] = - my_position [0] ;
  invmat[3][1] = - my_position [1] ;
  invmat[3][2] = 0.0 ;

  sgSetVec3 ( HOTvec, 0.0f, 0.0f, my_position [ 2 ] ) ;

  num_hits = ssgHOT ( scene, HOTvec, invmat, &results ) ;
  
  hot = DEEPEST_HELL ;

  for ( int i = 0 ; i < num_hits ; i++ )
  {
    ssgHit *h = &results [ i ] ;

    float hgt = - h->plane[3] / h->plane[2] ;

    if ( hgt >= hot )
    {
      hot = hgt ;

      if ( normal != NULL )
        sgCopyVec3 ( normal, h->plane ) ;
    }
  }

  return hot ;
}


