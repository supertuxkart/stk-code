
class GFX
{
  int mirror ;

public:

   GFX ( int _mirror ) ;
   void update () ;
   void done   () ;
} ;


int stereoShift () ;
void reshape ( int x, int y ) ;
void keystroke ( int key, int updown, int x, int y ) ;

int getScreenWidth  () ;
int getScreenHeight () ;

void setScreenSize ( int w, int h ) ;

void updateGFX ( GFX *gfx ) ;

