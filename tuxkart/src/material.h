
#define MAT_IGN    1
#define MAT_CRASH  2
#define MAT_ZIP    4
#define MAT_RESET  8

struct Material
{
public:
  ssgSimpleState **gst ;

  char *texture_map  ;
  int   clamp_tex    ;
  int   transparency ;
  float alpha_ref    ;
  int   lighting     ;
  float friction     ;
  unsigned int flags ;

  int  isNull ()    { return gst == NULL ; } ;
  int  isIgnore()   { return flags & MAT_IGN   ; }
  int  isCrashable(){ return flags & MAT_CRASH ; }
  int  isZipper()   { return flags & MAT_ZIP   ; }
  int  isReset()    { return flags & MAT_RESET ; }
  void install ( int index ) ;
  
  ssgState *getState    () { return *gst ; }
  char     *getTexFname () { return texture_map ; }
} ;


void initMaterials () ;
Material *getMaterial ( ssgState *s ) ;
Material *getMaterial ( ssgLeaf  *l ) ;

ssgState *getAppState ( char *fname ) ;

extern ssgSimpleState *default_gst ;
extern ssgSimpleState *fuzzy_gst   ;
extern ssgSimpleState *herring_gst ;
extern ssgSimpleState *herringbones_gst ;
extern ssgSimpleState *spark_gst   ;
extern ssgSimpleState *flamemissile_gst ;
extern ssgSimpleState *missile_gst ;
extern ssgSimpleState *magnet_gst  ;
extern ssgSimpleState *players_gst ;
extern ssgSimpleState *zipper_gst  ;

