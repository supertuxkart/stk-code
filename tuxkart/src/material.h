
#include <plib/ssg.h>

void initMaterials () ;

class Material
{
  ssgState *state ;

  int   index ;

  char *texname      ;

  bool  collideable  ;
  bool  zipper       ;
  bool  resetter     ;
  bool  ignore       ;

  int   clamp_tex    ;
  bool  lighting     ;
  bool  transparency ;
  float alpha_ref    ;
  float friction     ;

  bool  parseBool  ( char **p ) ;
  int   parseInt   ( char **p ) ;
  float parseFloat ( char **p ) ;

  void init    () ;
  void install () ;

public:

  Material () ;
  Material ( char *fname, char *description ) ;

  ~Material ()
  {
    ssgDeRefDelete ( state ) ;
    delete texname ;
  }

  int matches ( char *tx ) ;

  bool isIgnore    () { return ignore      ; }
  bool isZipper    () { return zipper      ; }
  bool isCrashable () { return collideable ; }
  bool isReset     () { return resetter    ; }
  float getFriction() { return friction    ; }

  ssgState *getState () { return state ; }
  void      apply    () { state -> apply () ; }

  char *getTexFname    () { return texname     ; }

} ;


Material *getMaterial ( char *texname ) ;
Material *getMaterial ( ssgLeaf *lf ) ;

ssgState *getAppState ( char *fname ) ;

