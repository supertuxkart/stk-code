//  $Id: material_manager.hpp,v 1.2 2005/07/23 23:01:17 joh Exp $
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

#ifndef HEADER_MATERIALMANAGER_H
#define HEADER_MATERIALMANAGER_H

#include <plib/ssg.h>

struct Material;

class MaterialManager {
 private:

  char   *parseFileName(char **str);
  int     parseMaterial(FILE *fd);
  ulList *materials ;
 public:
          MaterialManager();
  void      loadMaterial();
  Material *getEntity(int i) {return (Material*)materials->getEntity(i);}
  int       addEntity(Material *m);
  int       searchForEntity(Material *m) {return materials->searchForEntity(m);}
  int       getNumEntities() {return materials->getNumEntities();}
  Material *getMaterial ( const char *texname ) ;
  Material *getMaterial ( ssgLeaf *lf ) ;
};

extern ssgState *fuzzy_gst, *herringbones_gst;

ssgState *getAppState ( char *fname ) ;
extern MaterialManager *material_manager;

#endif

/* EOF */
