


class Explosion
{
  ssgTransform *dcs ;
  ssgSelector  *seq ;

  int step ;

public:

  Explosion ( ssgBranch *b ) ;

  void update () ;
  void start  ( sgVec3 where )
  {
    sound -> playSfx ( SOUND_EXPLOSION ) ;
    dcs -> setTransform ( where ) ;
    step = 0 ;
  }

  int  inUse  () { return (step >= 0) ; }
} ;

