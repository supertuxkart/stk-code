
#include "tuxkart.h"

static sgVec3 hell = { -1000000, -1000000, -1000000 } ;

static ssgSelector *find_selector ( ssgBranch *b )
{
  if ( b == NULL )
    return NULL ;

  if ( ! b -> isAKindOf ( ssgTypeBranch () ) )
    return NULL ;

  if ( b -> isAKindOf ( ssgTypeSelector () ) )
    return (ssgSelector *) b ;

  for ( int i = 0 ; i < b -> getNumKids() ; i++ )
  {
    ssgSelector *res = find_selector ( (ssgBranch *)(b ->getKid(i)) ) ;

    if ( res != NULL )
      return res ;
  }

  return NULL ;
}
 

Explosion::Explosion ( ssgBranch *b )
{
  ssgSelector *e = find_selector ( b ) ;
  ssgCutout *cut ;

  if ( e == NULL )
  {
    fprintf ( stderr, "Explode.ac doesn't have an 'explosion' object.\n" ) ;
    exit ( 1 ) ;
  }

  step = -1 ;

  dcs = new ssgTransform ;
  cut = new ssgCutout ;
  seq = (ssgSelector *) e ;

  scene -> addKid ( dcs ) ;
  dcs   -> addKid ( cut ) ;
  cut   -> addKid ( seq ) ;

  dcs -> setTransform ( hell ) ;
  seq -> select ( 0 ) ;
}



void Explosion::update ()
{
  if ( step < 0 )
  {
    dcs -> setTransform ( hell ) ;
    return ;
  }

  seq -> selectStep ( step ) ;

  if ( ++step >= 16 )
    step = -1 ;
}


