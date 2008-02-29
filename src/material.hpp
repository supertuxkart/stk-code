//  $Id$
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

#ifndef HEADER_MATERIAL_H
#define HEADER_MATERIAL_H

#include <string>
#include <plib/ssg.h>

class Material
{
private:
    ssgSimpleState *m_state;
    ssgCallback     m_predraw;
    ssgCallback     m_postdraw;
    int             m_index;
    std::string     m_texname;
    bool            m_collideable;
    bool            m_zipper;
    bool            m_resetter;
    bool            m_ignore;
    int             m_clamp_tex;
    bool            m_lighting;
    bool            m_sphere_map;
    bool            m_transparency;
    float           m_alpha_ref;
    float           m_friction;

    bool  parseBool  ( char **p );
    int   parseInt   ( char **p );
    float parseFloat ( char **p );

    void init    (int index);
    void install (bool is_full_path=false);

public:

    Material(int index);
    Material(const std::string& fname, char *description, int index,
             bool is_full_path=false);

    ~Material ();

    int matches ( char *tx ) ;

    ssgSimpleState 
         *getState    () const { return m_state ;      }
    bool  isIgnore    () const { return m_ignore;      }
    bool  isZipper    () const { return m_zipper;      }
    bool  isSphereMap () const { return m_sphere_map;  }
    bool  isCrashable () const { return m_collideable; }
    bool  isReset     () const { return m_resetter;    }
    float getFriction () const { return m_friction;    }
    const std::string& 
          getTexFname () const { return m_texname;     }
    int   getIndex    () const { return m_index;       }
    void  apply       ()       { m_state ->apply();    }
    void  force       ()       { m_state ->force();    }

    void applyToLeaf ( ssgLeaf *l ) ;

} ;


#endif

/* EOF */
