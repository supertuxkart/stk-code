//  $Id: track_sel.cpp 694 2006-08-29 07:42:36Z hiker $
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

#include <set>
#include "loader.hpp"
#include "string_utils.hpp"
#include "grand_prix_select.hpp"
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "config.hpp"

GrandPrixSelect::GrandPrixSelect() {
  menu_id = widgetSet -> varray(0);

  widgetSet -> label(menu_id, "Choose a Grand Prix", GUI_LRG, GUI_ALL, 0, 0);
  widgetSet -> space(menu_id);

  std::set<std::string> result;
  loader->listFiles(result, "data");

  // Findout which grand prixs are available and load them
  int nId = 0;
  for(std::set<std::string>::iterator i  = result.begin(); 
                                      i != result.end()  ; i++) {
    if (StringUtils::has_suffix(*i, ".cup")) {
      std::string fullPath= "data/" + (std::string)*i;
      CupData *cup = new CupData(fullPath.c_str());
      allCups.push_back(cup);
      int tmp=widgetSet -> state(menu_id, cup->getName().c_str(), GUI_SML, nId, 0);
      if(nId==0) widgetSet->set_active(tmp);
      nId++;
    }   // if
  }   // for i
  widgetSet -> space(menu_id);
  widgetSet -> start(menu_id,"Press <ESC> to go back", GUI_SML, -1);
  widgetSet -> layout(menu_id, 0, 0);
  rect = widgetSet->rect(10, 10, config->width-20, 34, GUI_ALL, 10);
}   // GrandPrixSelect

// -----------------------------------------------------------------------------
GrandPrixSelect::~GrandPrixSelect() {
  widgetSet -> delete_widget(menu_id) ;
  glDeleteLists(rect, 1);
}   // GrandPrixSelect

// -----------------------------------------------------------------------------
	
void GrandPrixSelect::update(float dt) {
  BaseGUI::update(dt); 
  int clicked_token= widgetSet->token(widgetSet->click());
  if(clicked_token==-1) return;

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, config->width, 0.0, config->height, -1.0, +1.0);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_BLEND);
  CupData *cup=allCups[clicked_token];
  glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    const GLfloat backgroundColour[4] = { 0.3f, 0.3f, 0.3f, 0.5f };
    glColor4fv(backgroundColour);
    glCallList(rect);
  glPopMatrix();
  widgetSet->drawText(cup->getDescription(), GUI_MED, SCREEN_CENTERED_TEXT, 10,
		      255,255,255);
  glDisable(GL_BLEND);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
		      
  return;
}

// -----------------------------------------------------------------------------
void GrandPrixSelect::select() {
  int clicked_token= widgetSet->token(widgetSet->click());
  if(clicked_token==-1) {
    menu_manager->popMenu();
    return;
  }
  race_manager->setGrandPrix(allCups[clicked_token]);
  menu_manager->pushMenu(MENUID_DIFFICULTY);
}   // select

/* EOF */
