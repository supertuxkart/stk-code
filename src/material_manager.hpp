//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#ifndef HEADER_MATERIAL_MANAGER_HP
#define HEADER_MATERIAL_MANAGER_HPP

#define _WINSOCKAPI_
#include <plib/ssg.h>
#include <string>
#include <vector>

class Material;
class XMLReader;

class MaterialManager
{
private:

#ifndef HAVE_IRRLICHT
    char   *parseFileName(char **str);
    int     parseMaterial(FILE *fd);
#endif
    void    parseMaterialFile(const std::string& filename);
    int     m_shared_material_index;

    std::vector<Material*> m_materials;
public:
    MaterialManager();
    void      loadMaterial     ();
    void      reInit           ();
    int       addEntity        (Material *m);
    Material *getMaterial      (ssgLeaf *lf);
    Material *getMaterial      (const std::string& t, bool is_full_path=false,
                                bool make_permanent=false);
    void      addSharedMaterial(const std::string& filename);
    bool      pushTempMaterial (const std::string& filename);
    void      popTempMaterial  ();
};

extern ssgState *fuzzy_gst;//, *herringbones_gst;
#ifndef HAVE_IRRLICHT
ssgState *getAppState ( char *fname ) ;
#endif
extern MaterialManager *material_manager;

#endif

/* EOF */
