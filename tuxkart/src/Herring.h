
 
class Shadow
{
  ssgBranch *sh ;
 
public:
  Shadow ( float x1, float x2, float y1, float y2 ) ;
  ssgEntity *getRoot () { return sh ; }
} ;                                                                             

 
class Herring
{
  float h ;
  ssgTransform *tr ;
 
  Shadow *sh ;
 
public:
  Herring ( sgVec4 colour ) ;
  ssgTransform *getRoot () { return tr ; }
  void update () ;
} ;


 
class ActiveThingInstance
{
public:
  sgVec3 xyz ;
  ssgTransform *scs ;
 
  ssgTransform *setup ( ssgEntity *thing, sgCoord *pos )
  {
    sgCopyVec3 ( xyz, pos->xyz );
 
    scs = new ssgTransform ;
    scs -> setTransform ( pos ) ;
    scs -> addKid ( thing ) ;
 
    return scs ;
  }
 
  ssgTransform *setup ( ssgEntity *thing, sgVec3 pos )
  {
    sgCoord c ;
    sgSetVec3  ( c.hpr, 0.0f, 0.0f, 0.0f ) ;
    sgCopyVec3 ( c.xyz, pos ) ;
    return setup ( thing, &c ) ;
  }
 
  int active () { return xyz [ 2 ] > -1000000.0f ; }
 
  void getPos ( sgVec3 pos ) { sgCopyVec3 ( pos, xyz ) ; } 
  void setPos ( sgVec3 pos )
  {
    sgCopyVec3 ( xyz, pos ) ;
    scs -> setTransform ( pos ) ;
  }
 
  virtual void update () = 0 ;
} ;                                                                             

 
class HerringInstance : public ActiveThingInstance
{
public:
  Herring *her    ;
  float    time_to_return ;
  int      eaten  ;
  int      type   ;
  int      effect ;
  void update () ;
} ;


extern Herring *silver_h ;
extern Herring *gold_h   ;
extern Herring *red_h    ;
extern Herring *green_h  ;                                                     

extern int num_herring   ;                                                      

#define EFFECT_DEFAULT   0
#define EFFECT_SPEEDUP   1
#define EFFECT_ROCKET    2

#define HE_RED           0
#define HE_GREEN         1
#define HE_GOLD          2
#define HE_SILVER        3
 
#define MAX_HERRING     50

extern HerringInstance herring [ MAX_HERRING ] ;                             


