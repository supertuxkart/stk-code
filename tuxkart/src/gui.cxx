//  $Id: gui.cxx,v 1.15 2004/08/01 00:13:28 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <plib/pw.h>
#include <plib/pu.h>
#include <plib/js.h>
#include "tuxkart.h"
#include "gui.h"
#include "sound.h"
#include "status.h"
#include "Driver.h"
#include "joystick.h"

static jsJoystick *joystick ;

fntTexFont *font ;


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



GUI::GUI ()
{
  paused = FALSE ;
  hidden = TRUE  ;

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
  main_menu_bar -> reveal () ;
}

void GUI::hide ()
{
  hidden = TRUE ;
  hide_status () ;
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
}

void GUI::keyboardInput ()
{
#ifndef HAVE_LIBSDL
  static int isWireframe = FALSE ;
  int c = getKeystroke () ;

  if ( c <= 0 )
    return ;

  int i;
  switch ( c )
    {
    case 0x1B /* Escape */      :
    case 'x'  /* X */      :
    case 'X'  /* X */      :
    case 0x03 /* Control-C */   : exit ( 0 ) ;

    case 'r' :
    case 'R' :
      finishing_position = -1 ;
      for ( i = 0 ; i < num_karts ; i++ )
        kart[i]->reset() ;
      return ;
 
    case 'w' :
    case 'W' : if ( isWireframe )
      glPolygonMode ( GL_FRONT_AND_BACK, GL_FILL ) ;
    else
      glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE ) ;
      isWireframe = ! isWireframe ;

      return ;
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
#endif
}

void GUI::joystickInput ()
{
  static JoyInfo ji ;

  if ( joystick -> notWorking () )
  {
    ji.data[0] = ji.data[1] = 0.0f ;
    ji.buttons = 0 ;
  }
  else
  {
    joystick -> read ( & ji.buttons, ji.data ) ;
    ji.data[0] *= 1.3 ;
  }

#ifdef HAVE_LIBSDL
  if ( keyState [ SDLK_LEFT ] ) ji.data [0] = -1.0f ;
  if ( keyState [ SDLK_RIGHT ] ) ji.data [0] =  1.0f ;
  if ( keyState [ SDLK_UP ] ) ji.buttons |= 0x01 ;
  if ( keyState [ SDLK_DOWN ] )  ji.buttons |= 0x02 ;

  if ( keyState [ SDLK_f ] ) ji.buttons |= 0x04 ;
  if ( keyState [ SDLK_a ] ) ji.buttons |= 0x20 ;
  if ( keyState [ SDLK_s ] ) ji.buttons |= 0x10 ;
  if ( keyState [ SDLK_d ] ) ji.buttons |= 0x08 ;
#else
  if ( isKeyDown ( PW_KEY_LEFT  ) ) ji.data [0] = -1.0f ;
  if ( isKeyDown ( PW_KEY_RIGHT ) ) ji.data [0] =  1.0f ;
  if ( isKeyDown ( PW_KEY_UP    ) ) ji.buttons |= 0x01 ;
  if ( isKeyDown ( PW_KEY_DOWN  ) ) ji.buttons |= 0x02 ;

  if ( isKeyDown ( 'f' ) || isKeyDown ( 'F' ) ||
       isKeyDown ( '\r' )|| isKeyDown ( '\n' )) ji.buttons |= 0x04 ;
  if ( isKeyDown ( 'a' ) || isKeyDown ( 'A' ) ) ji.buttons |= 0x20 ;
  if ( isKeyDown ( 's' ) || isKeyDown ( 'S' ) ) ji.buttons |= 0x10 ;
  if ( isKeyDown ( 'd' ) || isKeyDown ( 'D' ) ) ji.buttons |= 0x08 ;
#endif

  ji.hits        = (ji.buttons ^ ji.old_buttons) &  ji.buttons ;
  ji.releases    = (ji.buttons ^ ji.old_buttons) & ~ji.buttons ;
  ji.old_buttons =  ji.buttons ;

  ((PlayerKartDriver *)kart [ 0 ]) -> incomingJoystick ( &ji ) ;
}

/* EOF */
