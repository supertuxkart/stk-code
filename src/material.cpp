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

#include "material.hpp"

#include <stdexcept>

#include "stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

#define UCLAMP   1
#define VCLAMP   2

#ifndef HAVE_IRRLICHT
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
#endif

//=============================================================================
#ifndef HAVE_IRRLICHT
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
#endif
//-----------------------------------------------------------------------------
Material::Material(unsigned int index)
{
    m_texname = "";
#ifndef HAVE_IRRLICHT
    m_predraw  = m_postdraw = NULL ;
#endif

    init   (index);
    install();
}   // Material

//-----------------------------------------------------------------------------
#ifdef HAVE_IRRLICHT
/** Create a new material using the parameters specified in the xml file.
 *  \param node Node containing the parameters for this material.
 *  \param index Index in material_manager.
 */
Material::Material(const XMLNode *node, int index)
{
    node->get("name", &m_texname);
    if(m_texname=="")
    {
        throw std::runtime_error("No texture name specified in %s file\n");
    }
    init(index);
    bool b=false;
    node->get("clampU", &b);  if(b) m_clamp_tex +=UCLAMP;
    b=false;
    node->get("clampV", &b);  if(b) m_clamp_tex +=VCLAMP;
    node->get("transparency", &m_transparency      );
    node->get("alpha",        &m_alpha_ref         );
    node->get("light",        &m_lighting          );
    node->get("sphere",       &m_sphere_map        );
    node->get("friction",     &m_friction          );
    node->get("ignore",       &m_ignore            );
    node->get("zipper",       &m_zipper            );
    node->get("reset",        &m_resetter          );
    node->get("collide",      &m_collideable       );
    node->get("maxSpeed",     &m_max_speed_fraction);
    node->get("slowdownTime", &m_slowdown          );

    install(/*is_full_path*/false);
}   // Material

//-----------------------------------------------------------------------------
/** Create a standard material using the default settings for materials.
 *  \param fname Name of the texture file.
 *  \param index Unique index in material_manager.
 *  \param is_full_path If the fname contains the full path.
 */
Material::Material(const std::string& fname, int index, bool is_full_path)
{
    m_texname = fname;
    init(index);
    install(is_full_path);
}   // Material

#else
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
        
        m_transparency       = parseBool (&description);
        m_alpha_ref          = parseFloat(&description);
        m_lighting           = parseBool (&description);
        m_sphere_map         = parseBool (&description);
        m_friction           = parseFloat(&description);
        m_ignore             = parseBool (&description);
        m_zipper             = parseBool (&description);
        m_resetter           = parseBool (&description);
        m_collideable        = parseBool (&description);
        m_max_speed_fraction = parseFloat(&description);
        m_slowdown           = parseFloat(&description);
        // Set the optional parameters.
        if(m_max_speed_fraction <= 0.0f)
            m_max_speed_fraction = 1.0f;
        if(m_slowdown           <= 0.0f)
            m_slowdown           = stk_config->m_slowdown_factor;
    }
    install(is_full_path);
}   // Material
#endif

//-----------------------------------------------------------------------------
Material::~Material()
{
#ifndef HAVE_IRRLICHT
    ssgDeRefDelete(m_state);
#endif
}   // ~Material

//-----------------------------------------------------------------------------
void Material::init(unsigned int index)
{
    m_index              = index ;
    m_clamp_tex          = 0     ;
    m_transparency       = false ;
    m_alpha_ref          = 0.1f  ;
    m_lighting           = true  ;
    m_sphere_map         = false ;
    m_friction           = 1.0f  ;
    m_ignore             = false ;
    m_zipper             = false ;
    m_resetter           = false ;
    m_collideable        = true  ;
    m_max_speed_fraction = 1.0f;
    m_slowdown           = stk_config->m_slowdown_factor;
}

//-----------------------------------------------------------------------------
#ifndef HAVE_IRRLICHT
void Material::applyToLeaf(ssgLeaf *l)
{
    if ( m_predraw  ) l -> setCallback ( SSG_CALLBACK_PREDRAW , m_predraw  ) ;
    if ( m_postdraw ) l -> setCallback ( SSG_CALLBACK_POSTDRAW, m_postdraw ) ;
}   // applyToLeaf
#endif
//-----------------------------------------------------------------------------
void Material::install(bool is_full_path)
{
#ifdef HAVE_IRRLICHT
    m_texture = irr_driver->getTexture(file_manager->getTextureFile(m_texname));
#else
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
#endif

    // now set the name to the basename, so that all tests work as expected
    m_texname  = StringUtils::basename(m_texname);
}

