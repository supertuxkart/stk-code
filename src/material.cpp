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

#include "loader.hpp"
#include "material.hpp"
#include "material_manager.hpp"

#define UCLAMP   1
#define VCLAMP   2

int clearSpheremap ( ssgEntity * )
{
    glDisable   ( GL_TEXTURE_GEN_S ) ;
    glDisable   ( GL_TEXTURE_GEN_T ) ;
    return true ;
}

//=============================================================================
int setSpheremap ( ssgEntity * )
{
    glTexGeni   ( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
    glTexGeni   ( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
    glEnable    ( GL_TEXTURE_GEN_S ) ;
    glEnable    ( GL_TEXTURE_GEN_T ) ;
    return true ;
}

//=============================================================================
bool Material::parseBool ( char **p )
{
    /* Skip leading spaces */

    while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

    const bool RES = ( ( **p == 'Y' ) || ( **p == 'y' ) ) ;

    while ( **p > ' ' && **p != '\0' ) (*p)++ ;

    return RES ;
}

//-----------------------------------------------------------------------------
int Material::parseInt ( char **p )
{
    /* Skip leading spaces */

    while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

    return strtol ( *p, p, 0 ) ;
}

//-----------------------------------------------------------------------------
float Material::parseFloat ( char **p )
{
    /* Skip leading spaces */

    while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

    return (float)strtod ( *p, p ) ;
}

//-----------------------------------------------------------------------------
Material::Material ()
{
    m_texname = new char [ 1 ] ;
    m_texname [ 0 ] = '\0' ;
    m_predraw  = m_postdraw = NULL ;

    init    () ;
    install () ;
}

//-----------------------------------------------------------------------------
Material::Material (const char *fname, char *description )
{
    m_texname = new char [ strlen ( fname ) + 1 ] ;
    strcpy ( m_texname, fname ) ;
    m_predraw  = m_postdraw = NULL ;

    init () ;

    if(strlen(description)>0)
    {
        m_clamp_tex    = parseBool  ( & description ) ? UCLAMP : 0 ;
        m_clamp_tex   += parseBool  ( & description ) ? VCLAMP : 0 ;
        
        m_transparency = parseBool  ( & description ) ;
        m_alpha_ref    = parseFloat ( & description ) ;
        m_lighting     = parseBool  ( & description ) ;
        m_sphere_map   = parseBool  ( & description ) ;
        m_friction     = parseFloat ( & description ) ;
        m_ignore       = parseBool  ( & description ) ;
        m_zipper       = parseBool  ( & description ) ;
        m_resetter     = parseBool  ( & description ) ;
        m_collideable  = parseBool  ( & description ) ;
    }
    install () ;
}

//-----------------------------------------------------------------------------
Material::~Material()
{
    ssgDeRefDelete(m_state);
    delete[] m_texname;
}

//-----------------------------------------------------------------------------
void Material::init ()
{
    m_index = material_manager -> addEntity ( this ) ;
    m_clamp_tex    = 0     ;
    m_transparency = false ;
    m_alpha_ref    = 0.1f  ;
    m_lighting     = true  ;
    m_sphere_map   = false ;
    m_friction     = 1.0   ;
    m_ignore       = false ;
    m_zipper       = false ;
    m_resetter     = false ;
    m_collideable  = true  ;
}

//-----------------------------------------------------------------------------
void Material::applyToLeaf ( ssgLeaf *l )
{
    if ( m_predraw  ) l -> setCallback ( SSG_CALLBACK_PREDRAW , m_predraw  ) ;
    if ( m_postdraw ) l -> setCallback ( SSG_CALLBACK_POSTDRAW, m_postdraw ) ;
}

//-----------------------------------------------------------------------------
void Material::install ()
{
    if ( isSphereMap () )
    {
        m_predraw  =   setSpheremap ;
        m_postdraw = clearSpheremap ;
    }

    m_state = new ssgSimpleState ;

    m_state -> ref () ;
    m_state -> setExternalPropertyIndex ( m_index ) ;
    if ( m_texname != NULL && m_texname [ 0 ] != '\0' )
    {
        std::string fn=std::string("images")+Loader::DIR_SEPARATOR+m_texname;
        m_state -> setTexture ( loader->getPath(fn).c_str(), !(m_clamp_tex & UCLAMP),
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
}

