
#include <plib/puSDL.h>

class OldGUI
{
private:
  class puMenuBar *main_menu_bar ;

  int hidden ;

public:

  OldGUI () ;
  void update () ;
  void show   () ;
  void hide   () ;  
  int  isHidden () { return hidden ; }
} ;

extern fntTexFont *oldfont ;

