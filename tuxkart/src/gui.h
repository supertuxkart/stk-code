
class GUI
{
private:
  class puMenuBar *main_menu_bar ;

  int paused ;
  int hidden ;

public:

  GUI () ;
  void update () ;
  void show   () ;
  void hide   () ;  
  int  isHidden () { return hidden ; }
  int  isPaused () { return paused ; }

  void keyboardInput () ; 
  void joystickInput () ;
} ;

extern fntTexFont *font ;

