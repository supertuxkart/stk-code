

class Camera
{
protected:

  ssgContext *context  ;
  sgCoord     location ;
  sgCoord final_camera ;

  int    whichKart ;
  float x, y, w, h ;
  float cam_delay ;

  static int numSplits ;

  void init () ;

public:

  Camera ( int id ) ;

  static void setNumSplits ( int ns ) { numSplits = ns ; }
  static int  getNumSplits () { return numSplits ; }

  void setScreenPosition ( int pos ) ;

  void update () ;
  void apply  () ;
} ;


extern Camera *camera [ 4 ] ;

void initCameras   () ;
void updateCameras () ;

