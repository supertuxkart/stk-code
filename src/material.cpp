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

#include "file_manager.hpp"
#include "material.hpp"
#include "string_utils.hpp"

#define UCLAMP   1
#define VCLAMP   2

int clearSpheremap ( ssgEntity * )
{
    glDisable   ( GL_TEXTURE_GEN_S ) ;
    glDisable   ( GL_TEXTURE_GEN_T ) ;
    return true ;
}   // clearSpheremap

//=============================================================================
int setSpheremap ( ssgEntity * )
{
    glTexGeni   ( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
    glTexGeni   ( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
    glEnable    ( GL_TEXTURE_GEN_S ) ;
    glEnable    ( GL_TEXTURE_GEN_T ) ;
    return true ;
}   // setSpheremap

//=============================================================================
bool Material::parseBool ( char **p )
{
    /* Skip leading spaces */

    while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

    const bool RES = ( ( **p == 'Y' ) || ( **p == 'y' ) ) ;

    while ( **p > ' ' && **p != '\0' ) (*p)++ ;

    return RES ;
}   // parseBool

//-----------------------------------------------------------------------------
int Material::parseInt ( char **p )
{
    /* Skip leading spaces */

    while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

    return strtol ( *p, p, 0 ) ;
}   // parseInt 

//-----------------------------------------------------------------------------
float Material::parseFloat ( char **p )
{
    /* Skip leading spaces */

    while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

    return (float)strtod ( *p, p ) ;
}   // parseFloat 

//-----------------------------------------------------------------------------
Material::Material(int index)
{
    m_texname = "";
    m_predraw  = m_postdraw = NULL ;

    init   (index);
    install();
}   // Material

//-----------------------------------------------------------------------------
Material::Material(const std::string& fname, char *description, 
                   int index, bool is_full_path)
{
    m_texname = fname;
    m_predraw  = m_postdraw = NULL ;

    init (index);

    if(strlen(description)>0)
    {
        m_clamp_tex    = parseBool (&description) ? UCLAMP : 0 ;
        m_clamp_tex   += parseBool (&description) ? VCLAMP : 0 ;
        
        m_transparency = parseBool (&description);
        m_alpha_ref    = parseFloat(&description);
        m_lighting     = parseBool (&description);
        m_sphere_map   = parseBool (&description);
        m_friction     = parseFloat(&description);
        m_ignore       = parseBool (&description);
        m_zipper       = parseBool (&description);
        m_resetter     = parseBool (&description);
        m_collideable  = parseBool (&description);
    }
    install(is_full_path);
}   // Material

//-----------------------------------------------------------------------------
Material::~Material()
{
    ssgDeRefDelete(m_state);
}   // ~Material

//-----------------------------------------------------------------------------
void Material::init(int index)
{
    m_index        = index ;
    m_clamp_tex    = 0     ;
    m_transparency = false ;
    m_alpha_ref    = 0.1f  ;
    m_lighting     = true  ;
    m_sphere_map   = false ;
    m_friction     = 1.0f  ;
    m_ignore       = false ;
    m_zipper       = false ;
    m_resetter     = false ;
    m_collideable  = true  ;
}

//-----------------------------------------------------------------------------
void Material::applyToLeaf(ssgLeaf *l)
{
    if ( m_predraw  ) l -> setCallback ( SSG_CALLBACK_PREDRAW , m_predraw  ) ;
    if ( m_postdraw ) l -> setCallback ( SSG_CALLBACK_POSTDRAW, m_postdraw ) ;
}   // applyToLeaf

//-----------------------------------------------------------------------------
void Material::install(bool is_full_path)
{
    if ( isSphereMap () )
    {
        m_predraw  =   setSpheremap ;
        m_postdraw = clearSpheremap ;
    }

    m_state = new ssgSimpleState ;

    m_state -> ref () ;
    m_state -> setExternalPropertyIndex ( m_index ) ;
    if ( m_texname.size()>0 )
    {
        std::string fn=is_full_path ? m_texname 
                                    : file_manager->getTextureFile(m_texname);
        if(fn=="")
        {
            fprintf(stderr, "WARNING: texture '%s' not found.\n", 
                    m_texname.c_str());
        }
        m_state -> setTexture ( fn.c_str(), !(m_clamp_tex & UCLAMP),
                              !(m_clamp_tex & VCLAMP) );
        m_state -> enable  ( GL_TEXTURE_2D ) ;
    }
    else
        m_state -> disable ( GL_TEXTURE_2D ) ;

    if ( m_lighting )
        m_state -> enable  ( GL_LIGHTING ) ;
    else
        m_state -> disable ( GL_LIGHTING ) ;

    m_state -> setShadeModel ( GL_SMOOTH ) ;
    m_state -> enable        ( GL_COLOR_MATERIAL ) ;
    m_state -> enable        ( GL_CULL_FACE      ) ;
    m_state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
    m_state -> setMaterial   ( GL_EMISSION, 0, 0, 0, 1 ) ;
    m_state -> setMaterial   ( GL_SPECULAR, 0, 0, 0, 1 ) ;
    m_state -> setShininess  ( 0 ) ;

    if ( m_transparency )
    {
        m_state -> setTranslucent () ;
        m_state -> enable         ( GL_ALPHA_TEST ) ;
        m_state -> setAlphaClamp  ( m_alpha_ref ) ;
        m_state -> enable         ( GL_BLEND ) ;
    }
    else
    {
        m_state -> setOpaque () ;
        m_state -> disable   ( GL_BLEND ) ;
    }

    // now set the name to the basename, so that all tests work as expected
    m_texname  = StringUtils::basename(m_texname);
}

