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

//-----------------------------------------------------------------------------
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
void Material::install(bool is_full_path)
{
    // FIXME: do we actually still need the texture here? Irrlicht should
    // cache them anyway.
    // Avoid irrlicht warning about not being able to load texture.
    m_texture = irr_driver->getTexture(file_manager->getTextureFile(m_texname));

    // now set the name to the basename, so that all tests work as expected
    m_texname  = StringUtils::basename(m_texname);
}   // isntall

//-----------------------------------------------------------------------------
/** Sets the appropriate flags in an irrlicht SMaterial.
 *  \param material The irrlicht SMaterial which gets the flags set.
 */
void  Material::setMaterialProperties(scene::IMeshBuffer *mb) const
{
    if(m_transparency)
        // Note: if EMT_TRANSPARENT_ALPHA_CHANNEL is used, you have to use
        // scene_manager->getParameters()->setAttribute(
        //    scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);  and enable 
        // updates of the Z buffer of the material. Since the _REF 
        // approach is faster (and looks better imho), this is used for now.
        mb->getMaterial().MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
    if(m_sphere_map)
        mb->getMaterial().MaterialType = video::EMT_SPHERE_MAP;
    // FIXME: more parameters need to be set!
}   // setMaterialProperties
