
#include "tuxkart.h"
#include <plib/pu.h>

static jsJoystick *joystick ;

static int mouse_x ;
static int mouse_y ;
static int mouse_dx = 0 ;
static int mouse_dy = 0 ;
static int mouse_buttons = 0 ;
static int mousetrap = TRUE ;
static int mousemode = FALSE ;

fntTexFont *font ;

static void motionfn ( int x, int y )
{
  mouse_x = x ;
  mouse_y = y ;
  mouse_dx += mouse_x - 320 ;
  mouse_dy += mouse_y - 240 ;
  puMouse ( x, y ) ;
}

static void mousefn ( int button, int updown, int x, int y )
{
  mouse_x = x ;
  mouse_y = y ;

  if ( updown == GLUT_DOWN )
    mouse_buttons |= (1<<button) ;
  else
    mouse_buttons &= ~(1<<button) ;

  mouse_dx += mouse_x - 320 ;
  mouse_dy += mouse_y - 240 ;

  puMouse ( button, updown, x, y ) ;

  if ( updown == GLUT_DOWN )
    hide_status () ;
}

static void credits_cb ( puObject * )
{
  hide_status () ;
  credits () ;
}

static void versions_cb ( puObject * )
{
  hide_status () ;
  versions () ;
}

static void about_cb ( puObject * )
{
  hide_status () ;
  about () ;
}

static void help_cb ( puObject * )
{
  hide_status () ;
  help () ;
}

static void mouse_off_cb     ( puObject * ) { mousemode = FALSE ; } 
static void mouse_on_cb      ( puObject * ) { mousemode = TRUE  ; } 
static void mousetrap_off_cb ( puObject * ) { mousetrap = FALSE ; } 
static void mousetrap_on_cb  ( puObject * ) { mousetrap = TRUE  ; } 
static void music_off_cb     ( puObject * ) { sound->disable_music () ; } 
static void music_on_cb      ( puObject * ) { sound->enable_music  () ; } 
static void sfx_off_cb       ( puObject * ) { sound->disable_sfx   () ; } 
static void sfx_on_cb        ( puObject * ) { sound->enable_sfx    () ; } 

static void exit_cb ( puObject * )
{
  fprintf ( stderr, "Exiting TuxKart.\n" ) ;
  exit ( 1 ) ;
}

/* Menu bar entries: */

static char      *exit_submenu    [] = {  "Exit", NULL } ;
static puCallback exit_submenu_cb [] = { exit_cb, NULL } ;

static char      *sound_submenu    [] = { "Turn off Music", "Turn off Sounds", "Turn on Music", "Turn on Sounds", NULL } ;
static puCallback sound_submenu_cb [] = {  music_off_cb,        sfx_off_cb,     music_on_cb,        sfx_on_cb, NULL } ;

static char      *view_submenu    [] = { "Mousetrap Mode",   "No Mousetrap", NULL } ;
static puCallback view_submenu_cb [] = { mousetrap_on_cb , mousetrap_off_cb, NULL } ;

static char      *ctrl_submenu    [] = { "Use Mouse",   "Use Joystick", NULL } ;
static puCallback ctrl_submenu_cb [] = { mouse_on_cb , mouse_off_cb, NULL } ;

static char      *help_submenu    [] = { "Versions...", "Credits...", "About...",  "Help", NULL } ;
static puCallback help_submenu_cb [] = {   versions_cb,   credits_cb,   about_cb, help_cb, NULL } ;



GUI::GUI ()
{
  paused = FALSE ;
  hidden = TRUE  ;
  mouse_x = 320 ;
  mouse_y = 240 ;

  glutMouseFunc         ( mousefn   ) ;
  glutMotionFunc        ( motionfn  ) ;
  glutPassiveMotionFunc ( motionfn  ) ;
 
/*
  Already done in start_tuxkart!

  ssgInit () ;
  puInit () ;
*/
  font = new fntTexFont ;
  font -> load ( "fonts/sorority.txf" ) ;
  puFont ff ( font, 20 ) ;
  puSetDefaultFonts        ( ff, ff ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.1, 0.5, 0.1, 0.6 ) ;

  /* Make the menu bar */

  main_menu_bar = new puMenuBar () ;

  {
    main_menu_bar -> add_submenu ( "Exit", exit_submenu, exit_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Sound", sound_submenu, sound_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "View", view_submenu, view_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Control", ctrl_submenu, ctrl_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Help", help_submenu, help_submenu_cb ) ;
  }

  main_menu_bar -> close () ;
  main_menu_bar -> hide  () ;

  joystick = new jsJoystick ( 0 ) ;
  joystick -> setDeadBand ( 0, 0.1 ) ;
  joystick -> setDeadBand ( 1, 0.1 ) ;
}


void GUI::show ()
{
  hide_status () ;
  hidden = FALSE ;

  if ( mousetrap )
    glutWarpPointer ( 320, 240 ) ;

  glutSetCursor(GLUT_CURSOR_INHERIT);
  puShowCursor () ;
  main_menu_bar -> reveal () ;
}

void GUI::hide ()
{
  hidden = TRUE ;
  hide_status () ;
  glutSetCursor(GLUT_CURSOR_NONE);
  puHideCursor () ;
  main_menu_bar -> hide () ;
}

void GUI::update ()
{
  keyboardInput  () ;
  joystickInput  () ;
  drawStatusText () ;

  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  glAlphaFunc ( GL_GREATER, 0.1f ) ;
  glEnable    ( GL_BLEND ) ;

  puDisplay () ;

  /*
    Don't let the cursor escape from the screen!
  */

  if ( isHidden () )
    glutWarpPointer ( 320, 240 ) ;
  else
  {
    int warp = FALSE ;

    if ( mouse_x < 1 ) { mouse_x = 2 ; warp = TRUE ; }
    else if ( mouse_x > 639 ) { mouse_x = 638 ; warp = TRUE ; }
    if ( mouse_y < 1 ) { mouse_y = 2 ; warp = TRUE ; }
    else if ( mouse_y > 479 ) { mouse_y = 478 ; warp = TRUE ; }

    if ( warp && mousetrap )
      glutWarpPointer ( mouse_x, mouse_y ) ;

    mouse_dx = mouse_dy = 0 ;
    mouse_buttons = 0 ;
  }
}


void GUI::keyboardInput ()
{
  int c = getGLUTKeystroke () ;

  if ( c <= 0 )
    return ;

  switch ( c )
  {
    case 0x1B /* Escape */      :
    case 'x'  /* X */      :
    case 'X'  /* X */      :
    case 0x03 /* Control-C */   : exit ( 0 ) ;

    case (256+GLUT_KEY_PAGE_UP)   : cam_follow-- ; break ;
    case (256+GLUT_KEY_PAGE_DOWN) : cam_follow++ ; break ;

    case 'r' :
    case 'R' : for ( int i = 0 ; i < num_karts ; i++ )
                 kart[i]->reset() ;
               return ;
 
    case 'w' :
    case 'W' : glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ; return ;
    case 'f' :
    case 'F' : glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ; return ;
    case 'z' :
    case 'Z' : stToggle () ; return ;
    case 'h' :
    case 'H' : hide_status () ; help  () ; return ;
    case 'P' :
    case 'p' : paused = ! paused ; return ;

    case ' ' : if ( isHidden () )
		 show () ;
	       else
		 hide () ;
	       return ;

    default : ((PlayerKartDriver*)kart[0])->incomingKeystroke ( c ) ; break ;
  }
}


void GUI::joystickInput ()
{
  static JoyInfo ji ;

  joystick -> read ( & ji.buttons, ji.data ) ;

  if ( isGLUTKeyDown ( GLUT_KEY_LEFT +256 ) ) ji.data [0] = -1.0f ;
  if ( isGLUTKeyDown ( GLUT_KEY_RIGHT+256 ) ) ji.data [0] =  1.0f ;
  if ( isGLUTKeyDown ( GLUT_KEY_UP   +256 ) ) ji.buttons |= 0x01 ;
  if ( isGLUTKeyDown ( GLUT_KEY_DOWN +256 ) ) ji.buttons |= 0x02 ;

  if ( isGLUTKeyDown ( '\r' )|| isGLUTKeyDown ( '\n' )) ji.buttons |= 0x04 ;
  if ( isGLUTKeyDown ( 'a' ) || isGLUTKeyDown ( 'A' ) ) ji.buttons |= 0x20 ;
  if ( isGLUTKeyDown ( 's' ) || isGLUTKeyDown ( 'S' ) ) ji.buttons |= 0x10 ;
  if ( isGLUTKeyDown ( 'd' ) || isGLUTKeyDown ( 'D' ) ) ji.buttons |= 0x08 ;

  ji.hits        = (ji.buttons ^ ji.old_buttons) &  ji.buttons ;
  ji.releases    = (ji.buttons ^ ji.old_buttons) & ~ji.buttons ;
  ji.old_buttons =  ji.buttons ;

  ((PlayerKartDriver *)kart [ 0 ]) -> incomingJoystick ( &ji ) ;
}



