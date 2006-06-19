//  $Id: SingleWindowMenu.cxx 305 2006-01-20 18:02:01Z joh $
//
//  SuperTuxKart - a fun racing game with go-kart
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

#include <signal.h>
#include <plib/pw.h>
#include <plib/pu.h>
#include "SingleWindowMenu.h"
#include "TrackManager.h"
#include "Track.h"
#include "RaceManager.h"
#include "KartManager.h"
#include "KartProperties.h"
#include "Loader.h"
#include "plibdrv.h"

enum GuiState {GUI_ACTIVE, GUI_QUIT, GUI_START_GAME};

static GuiState         gui_state         ;
static ssgSimpleState  *intro_gst         ;
static puSlider        *numLapsSlider     ;
static puButton        *pleaseWaitButton  ;
static puButton        *numLapsText       ;
static fntTexFont      *fnt               ;
static puButton        *playButton        ;
static puButton        *exitButton        ;
static puButtonBox     *trackButtons      ;
static puFont          *avantGarde        ;
static int              numLaps           ;
static char             numLapsLegend[100];
static char           **track_names       ;

/***********************************\
*                                   *
* Here are the PUI widget callback  *
* functions.                        *
*                                   *
\***********************************/

void play_cb ( puObject * )
{
  puSetDefaultColourScheme ( 123.0f/255.0f, 0.0f/255.0f, 34.0f/255.0f, 1.0) ;
  pleaseWaitButton = new puButton ( 100, 240,
                               "LOADING: PLEASE WAIT FOR A MINUTE OR TWO"  ) ;

  gui_state=GUI_START_GAME;
}


static void exit_cb ( puObject * )
{
  gui_state=GUI_QUIT;
}



static void numLapsSlider_cb ( puObject *)
{
  float d ;

  numLapsSlider->getValue ( & d ) ;

  numLaps = 1 + (int)( d / 0.05f ) ;

  if ( numLaps <  1 ) numLaps =  1 ;
  if ( numLaps > 20 ) numLaps = 20 ;

  sprintf ( numLapsLegend, "%2d", numLaps ) ;
  numLapsText->setLegend ( numLapsLegend ) ;
}

void displayfn (void)
{
  glMatrixMode   ( GL_PROJECTION ) ;
  glLoadIdentity () ;
  glMatrixMode   ( GL_MODELVIEW ) ;
  glLoadIdentity () ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glDisable      ( GL_ALPHA_TEST ) ;
  glOrtho        ( 0, 640, 0, 480, 0, 100 ) ;

  intro_gst -> force () ;

  glBegin ( GL_QUADS ) ;
  glColor3f    ( 1, 1, 1 ) ;
  glTexCoord2f ( 0, 0 ) ; glVertex2i (   0,   0 ) ;
  glTexCoord2f ( 1, 0 ) ; glVertex2i ( 640,   0 ) ;
  glTexCoord2f ( 1, 1 ) ; glVertex2i ( 640, 480 ) ;
  glTexCoord2f ( 0, 1 ) ; glVertex2i (   0, 480 ) ;
  glEnd () ;

  /* Make PUI redraw */

  glEnable ( GL_BLEND ) ;
  puDisplay () ;
  
  /* Off we go again... */

  pwSwapBuffers   () ;
}

static void install_material ()
{
  intro_gst = new ssgSimpleState ;
 
  if ( getenv ( "MESA_GLX_FX" ) != NULL )
    intro_gst -> setTexture ( loader->getPath("images/title_screen_small.rgb").c_str(),
			      TRUE, TRUE ) ;
  else
    intro_gst -> setTexture ( loader->getPath("images/title_screen.rgb").c_str(), 
			      TRUE, TRUE ) ;

  intro_gst -> enable      ( GL_TEXTURE_2D ) ;
  intro_gst -> disable     ( GL_LIGHTING  ) ;
  intro_gst -> disable     ( GL_CULL_FACE ) ;
  intro_gst -> setOpaque   () ;
  intro_gst -> disable     ( GL_BLEND ) ;
  intro_gst -> setShadeModel ( GL_SMOOTH ) ;
  intro_gst -> disable     ( GL_COLOR_MATERIAL ) ;
  intro_gst -> enable      ( GL_CULL_FACE      ) ;
  intro_gst -> setMaterial ( GL_EMISSION, 0, 0, 0, 1 ) ;
  intro_gst -> setMaterial ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  intro_gst -> setMaterial ( GL_DIFFUSE, 0, 0, 0, 1 ) ;
  intro_gst -> setMaterial ( GL_AMBIENT, 0, 0, 0, 1 ) ;
  intro_gst -> setShininess ( 0 ) ;
}


int SingleWindowMenu() {
  pwSetCallbacks ( keyfn, gui_mousefn, gui_motionfn, NULL, NULL ) ;

  gui_state = GUI_ACTIVE;
  fnt = new fntTexFont ;
  fnt -> load ( loader->getPath("fonts/AvantGarde-Demi.txf").c_str(), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
  avantGarde = new puFont ( fnt, 12 ) ;

  puSetDefaultFonts        ( *avantGarde, *avantGarde ) ;
  puSetDefaultStyle        ( PUSTYLE_SMALL_SHADED ) ;
  puSetDefaultColourScheme ( 243.0f/255.0f, 140.0f/255.0f, 34.0f/255.0f, 1.0) ;

  playButton = new puButton     ( 10, 10, 150, 50 ) ;
  playButton->setLegend         ( "Start Game"  ) ;
  playButton->setCallback       ( play_cb ) ;
  playButton->makeReturnDefault ( TRUE ) ;

  exitButton = new puButton     ( 180, 10, 250, 50 ) ;
  exitButton->setLegend         ( "Quit"  ) ;
  exitButton->setCallback       ( exit_cb ) ;
   
  numLapsSlider = new puSlider  ( 10, 80, 150 ) ;
  numLapsSlider->setLabelPlace ( PUPLACE_ABOVE ) ;
  numLapsSlider->setLabel  ( "How Many Laps?" ) ;
  numLapsSlider->setDelta  ( 0.05 ) ;
  numLapsSlider->setCBMode ( PUSLIDER_ALWAYS ) ;
  numLapsSlider->setValue  ( 1.0f*0.05f*(5.0f-1.0f) ) ;
  numLapsSlider->setCallback ( numLapsSlider_cb ) ;

  numLapsText = new puButton ( 160, 80, " 5" ) ;
  numLapsText->setStyle ( PUSTYLE_BOXED ) ;

  unsigned int nTracks = track_manager->getTrackCount();
  track_names = new char*[nTracks+1];
  for(size_t i=0; i<nTracks; i++) {
    track_names[i] = (char *)track_manager->getTrack(i)->getIdent();
  }
  track_names[nTracks]=NULL;

  trackButtons = new puButtonBox ( 400, 10, 630, 150, track_names, TRUE ) ;
  trackButtons -> setLabel ( "Which Track?" ) ;
  trackButtons -> setLabelPlace ( PUPLACE_ABOVE ) ;
  trackButtons -> setValue ( 0 ) ; 

  install_material () ;

  signal ( 11, SIG_DFL ) ;

  while(gui_state==GUI_ACTIVE) {
    displayfn();
  }
  // The callbacks have top be restored, otherwise the
  // game will have no keyboard input.
  pwSetCallbacks(keystroke, gui_mousefn, gui_motionfn, NULL, NULL);

  if(gui_state==GUI_QUIT) {
    CleanupSingleWindowMenu();
    printf("Returning 1\n");
    return 1;
  }

  /* Set some defaults, since those values currenlty
     can't be set with the single window menu        */
  race_manager->setNumPlayers(1);
  race_manager->setPlayerKart(0, kart_manager->getKartById(0)->getIdent());
  race_manager->setNumKarts  (4);
  race_manager->setRaceMode  (RaceSetup::RM_QUICK_RACE);
  race_manager->setDifficulty(RD_MEDIUM);

  /* Now set the values from the GUI */
  int t;
  trackButtons -> getValue ( & t ) ;
  race_manager->setTrack     (track_names[t]);

  int nl = atoi ( numLapsText->getLegend () ) ;
  race_manager->setNumLaps   (nl);

  CleanupSingleWindowMenu    ();
  return 0;
}

int CleanupSingleWindowMenu() {
  puDeleteObject ( pleaseWaitButton ) ;
  puDeleteObject ( numLapsSlider ) ;
  puDeleteObject ( numLapsText   ) ;
  puDeleteObject ( playButton    ) ;
  puDeleteObject ( exitButton    ) ;
  puDeleteObject ( trackButtons  ) ;
  delete intro_gst ;
  delete avantGarde  ;
  delete fnt       ;
  delete track_names;
  return 0;
}
