
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


int getGLUTKeystroke () ;

extern fntTexFont *font ;

