
#include "tuxkart.h"
#include "oldgui.h"
#include "status.h"
#include "sound.h"
#include <plib/puSDL.h>

#include "Loader.h"

fntTexFont *oldfont ;


void credits_cb ( puObject * )
{
  hide_status () ;
  credits () ;
}

void versions_cb ( puObject * )
{
  hide_status () ;
  versions () ;
}

void about_cb ( puObject * )
{
  hide_status () ;
  about () ;
}

void help_cb ( puObject * )
{
  hide_status () ;
  help () ;
}

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

static char      *help_submenu    [] = { "Versions...", "Credits...", "About...",  "Help", NULL } ;
static puCallback help_submenu_cb [] = {   versions_cb,   credits_cb,   about_cb, help_cb, NULL } ;



OldGUI::OldGUI ():
hidden(TRUE)
{
  oldfont = new fntTexFont ;
  oldfont -> load ( loader->getPath("fonts/sorority.txf").c_str() ) ;
  puFont ff ( oldfont, 20 ) ;
  puSetDefaultFonts        ( ff, ff ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 0.1, 0.5, 0.1, 0.6 ) ;

  /* Make the menu bar */

  main_menu_bar = new puMenuBar () ;

  {
    main_menu_bar -> add_submenu ( "Exit", exit_submenu, exit_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Sound", sound_submenu, sound_submenu_cb ) ;
    main_menu_bar -> add_submenu ( "Help", help_submenu, help_submenu_cb ) ;
  }

  main_menu_bar -> close () ;
  main_menu_bar -> hide  () ;
}


void OldGUI::show ()
{
  hide_status () ;
  hidden = FALSE ;
  main_menu_bar -> reveal () ;
}

void OldGUI::hide ()
{
  hidden = TRUE ;
  hide_status () ;
  main_menu_bar -> hide () ;
}

void OldGUI::update ()
{
  drawStatusText () ;

  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
  glAlphaFunc ( GL_GREATER, 0.1f ) ;
  glEnable    ( GL_BLEND ) ;

  puDisplay () ;
}



/* EOF */
