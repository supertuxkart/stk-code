
class GUI
{
  class puMenuBar *main_menu_bar ;

  int hidden ;
  int paused ;

public:

  GUI () ;
  void update () ;
  void show   () ;
  void hide   () ;
  int  isHidden () { return hidden ; }

  int isPaused () { return paused ; }
  void keyboardInput () ;
  void joystickInput () ;
} ;


void motionfn ( int x, int y ) ;
void  mousefn ( int button, int updown, int x, int y ) ;

int getKeystroke () ;
int isKeyDown ( unsigned int k ) ;

extern fntTexFont *font ;

