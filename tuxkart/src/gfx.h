
class GFX
{
  int mirror ;

public:

   GFX ( int _mirror ) ;
   void update () ;
   void done   () ;
} ;


int stereoShift () ;


int getScreenWidth  () ;
int getScreenHeight () ;

void setScreenSize ( int w, int h ) ;

void updateGFX ( GFX *gfx ) ;

