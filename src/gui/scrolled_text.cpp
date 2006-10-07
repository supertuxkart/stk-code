//  $Id: credits_menu.cpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include <SDL/SDL.h>
#include "scrolled_text.hpp"
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "config.hpp"

ScrolledText::ScrolledText(){
  float r       = config->width/800.0f;
  xLeft         = (int)(30.0*r);  xRight = config->width -xLeft;
  r             = config->height/600.0f;
  yBottom       = (int)(50.0*r);  yTop   = config->height-(int)(50.0f*r);
  ySpeed        = 50.0f;
  fontSize      = 24;
  yPos          = yBottom-fontSize;
  rect          = 0;
  menu_id = widgetSet -> varray(0);
  widgetSet->layout(menu_id, 0, 0);
}   // ScrolledText

// -----------------------------------------------------------------------------
ScrolledText::~ScrolledText() {
  glDeleteLists(rect, 1);
}   // ~ScrolledText
	
// -----------------------------------------------------------------------------
void ScrolledText::setText(StringList sl_) {
  sl=sl_;
  if(rect) glDeleteLists(rect, 1);
  rect = widgetSet->rect(xLeft, yBottom, xRight-xLeft, yTop-yBottom,
			 GUI_ALL, 10);
}   // setText
// -----------------------------------------------------------------------------
void ScrolledText::update(float dt){
  BaseGUI::update(dt);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, config->width, 0.0, config->height, -1.0, +1.0);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_BLEND);

  glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    const GLfloat backgroundColour[4] = { 0.3f, 0.3f, 0.3f, 0.5f };
    glColor4fv(backgroundColour);
    glCallList(rect);
  glPopMatrix();
  widgetSet->drawText("Press <ESC> to go back", 24, 
		      SCREEN_CENTERED_TEXT, 20, 255, 255, 255);
  glViewport(xLeft, yBottom, xRight-xLeft, yTop-yBottom);

  glScalef(1.0f, config->width/(yTop-yBottom), 1.0f);

  for(unsigned int i=0; i<sl.size(); i++) {

    if((yPos-i*fontSize < yTop + yBottom ) && yPos-i*fontSize > -fontSize)
        widgetSet->drawText(sl[i],24,
			xLeft,(int)yPos-i*fontSize,255,255,255);
  }
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glViewport(0,0,config->width, config->height);
  yPos=yPos+dt*ySpeed;
  if(ySpeed>0 && yPos>sl.size()*fontSize+yTop-yBottom) yPos=-fontSize;
  if(ySpeed<0 && yPos<0) yPos=sl.size()*fontSize+yTop-yBottom;
}   // update

// -----------------------------------------------------------------------------
void ScrolledText::keybd(int key) {
  switch(key) {
    case SDLK_PLUS      :
    case SDLK_UP        : ySpeed += 10.0f; break;
    case SDLK_PAGEUP    : ySpeed += 50.0f; break;
    case SDLK_PAGEDOWN  : ySpeed -= 50.0f; break;
    case SDLK_MINUS     :
    case SDLK_DOWN      : ySpeed -= 10.0f; break;
    default             : menu_manager->popMenu();
  }   // switch
}   // keybd
// -----------------------------------------------------------------------------
void ScrolledText::select() {
  // must be esc, nothing else is available. So just pop this menu
  menu_manager->popMenu();
}   // select

// -----------------------------------------------------------------------------
/* EOF */
