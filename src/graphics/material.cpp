//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//                2010 Steve Baker, Joerg Henrichs
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

#include "graphics/material.hpp"

#include <stdexcept>

#include "audio/sfx_base.hpp"
#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"


const unsigned int UCLAMP = 1;
const unsigned int VCLAMP = 2;

//-----------------------------------------------------------------------------
/** Create a new material using the parameters specified in the xml file.
 *  \param node Node containing the parameters for this material.
 *  \param index Index in material_manager.
 */
Material::Material(const XMLNode *node, int index)
{
    
    node->get("name", &m_texname);
    if (m_texname=="")
    {
        throw std::runtime_error("[Material] No texture name specified in file\n");
    }
    init(index);

    bool b = false;
    node->get("clampU", &b);  if (b) m_clamp_tex |= UCLAMP;
    b = false;
    node->get("clampV", &b);  if (b) m_clamp_tex |= VCLAMP;
    node->get("transparency",     &m_alpha_testing     );
    node->get("lightmap",         &m_lightmap          );
    node->get("alpha",            &m_alpha_blending    );
    node->get("light",            &m_lighting          );
    node->get("sphere",           &m_sphere_map        );
    node->get("friction",         &m_friction          );
    node->get("ignore",           &m_ignore            );
    node->get("reset",            &m_resetter          );
    node->get("additive",         &m_add               );
    node->get("max-speed",        &m_max_speed_fraction);
    node->get("slowdown-time",    &m_slowdown_time     );
    node->get("anisotropic",      &m_anisotropic       );
    node->get("backface-culling", &m_backface_culling  );
    node->get("disable-z-write",  &m_disable_z_write   );
    std::string s("");
    node->get("graphical-effect", &s                   );
    if(s=="water")
        m_graphical_effect = GE_WATER;
    else if (s!="")
        fprintf(stderr, 
            "Invalid graphical effect specification: '%s' - ignored.\n", 
            s.c_str());
    else
        m_graphical_effect = GE_NONE;

    if (node->get("compositing", &s))
    {
        if      (s == "blend")    m_alpha_blending = true;
        else if (s == "test")     m_alpha_testing = true;
        else if (s == "additive") m_add = true;
        else if (s != "none")     fprintf(stderr, "[Material] WARNING: Unknown compositing mode '%s'\n", s.c_str());
    }
    
    // Compatibility to track version 3, which stored zipper data
    // as attributes. Can be removed if track version 3 is not
    // supported anymore
    node->get("zipper",                    &m_zipper                   );
    node->get("zipper-duration",           &m_zipper_duration          );
    node->get("zipper-fade-out-time",      &m_zipper_fade_out_time     );
    node->get("zipper-max-speed-increase", &m_zipper_max_speed_increase);
    node->get("zipper-speed-gain",         &m_zipper_speed_gain        );

    // Terrain-specifc sound effect
    const unsigned int children_count = node->getNumNodes();
    for (unsigned int i=0; i<children_count; i++)
    {
        const XMLNode *child_node = node->getNode(i);
        
        if (child_node->getName() == "sfx")
        {
          
            initCustomSFX(child_node);
        }
        else if (child_node->getName() == "particles")
        {
            initParticlesEffect(child_node);
        }
        else if (child_node->getName() == "zipper")
        {
            // Track version 4 uses a separate node:
            m_zipper = true;
            child_node->get("duration",          &m_zipper_duration          );
            child_node->get("fade-out-time",     &m_zipper_fade_out_time     );
            child_node->get("max-speed-increase",&m_zipper_max_speed_increase);
            child_node->get("speed-gain",        &m_zipper_speed_gain        );
        }
        else
        {
            fprintf(stderr, "[Material] WARNING: unknown node type '%s' for texture '%s' - ignored.\n",
                    child_node->getName().c_str(), m_texname.c_str());
        }

    }   // for i <node->getNumNodes()
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
/** Inits all material data with the default settings.
 *  \param Index of this material in the material_manager index array.
 */
void Material::init(unsigned int index)
{
    m_index                     = index;
    m_clamp_tex                 = 0;
    m_alpha_testing             = false;
    m_lightmap                  = false;
    m_alpha_blending            = false;
    m_lighting                  = true;
    m_anisotropic               = false;
    m_backface_culling          = true;
    m_sphere_map                = false;
    m_friction                  = 1.0f;
    m_ignore                    = false;
    m_resetter                  = false;
    m_add                       = false;
    m_disable_z_write           = false;
    m_max_speed_fraction        = 1.0f;
    m_slowdown_time             = 1.0f;
    m_sfx_name                  = "";
    m_sfx_min_speed             = 0.0f;
    m_sfx_max_speed             = 30;
    m_sfx_min_pitch             = 1.0f;
    m_sfx_max_pitch             = 1.0f;
    m_graphical_effect          = GE_NONE;
    m_zipper                    = false;
    m_zipper_duration           = -1.0f;
    m_zipper_fade_out_time      = -1.0f;
    m_zipper_max_speed_increase = -1.0f;
    m_zipper_speed_gain         = -1.0f;
    
    for (int n=0; n<EMIT_KINDS_COUNT; n++)
    {
        m_particles_effects[n] = NULL;
    }
}   // init

//-----------------------------------------------------------------------------
void Material::install(bool is_full_path)
{
    const std::string &full_path = is_full_path 
                                 ? m_texname
                                 : file_manager->getTextureFile(m_texname);
    m_texture = irr_driver->getTexture(full_path);

    // now set the name to the basename, so that all tests work as expected
    m_texname  = StringUtils::getBasename(m_texname);
}   // install
//-----------------------------------------------------------------------------
Material::~Material()
{
    // If a special sfx is installed (that isn't part of stk itself), the
    // entry needs to be removed from the sfx_manager's mapping, since other
    // tracks might use the same name.
    if(m_sfx_name!="" && m_sfx_name==m_texname)
    {
        sfx_manager->deleteSFXMapping(m_sfx_name);
    }
}   // ~Material

//-----------------------------------------------------------------------------
/** Initialise the data structures for a custom sfx to be played when a
 *  kart is driving on that particular material.
 *  \param sfx The xml node containing the information for this sfx.
 */
void Material::initCustomSFX(const XMLNode *sfx)
{
    m_sfx_name="";
    // The name of the 'name' attribute must be the same as the one
    // used in sfx_manager, since the sfx_manager will be reading this
    // xml node, too.
    sfx->get("name", &m_sfx_name);
    if(m_sfx_name=="") m_sfx_name = m_texname;
    sfx->get("min-speed", &m_sfx_min_speed);
    sfx->get("max-speed", &m_sfx_max_speed);
    sfx->get("min-pitch", &m_sfx_min_pitch);
    sfx->get("max-pitch", &m_sfx_max_pitch);
    m_sfx_pitch_per_speed = (m_sfx_max_pitch - m_sfx_min_pitch)
                          / (m_sfx_max_speed - m_sfx_min_speed);

    if(!sfx_manager->soundExist(m_sfx_name))
    {
        std::string filename;
        sfx->get("filename", &filename);
        // The directory for the track was added to the model search path
        // so just misuse the getModelFile function
        const std::string full_path = file_manager->getModelFile(filename);
        sfx_manager->loadSingleSfx(sfx, full_path);
    }
}   // initCustomSFX

//-----------------------------------------------------------------------------

void Material::initParticlesEffect(const XMLNode *node)
{
    ParticleKindManager* pkm = ParticleKindManager::get();
    
    std::string base;
    node->get("base", &base);
    if (base.size() < 1)
    {
        fprintf(stderr, "[Material::initParticlesEffect] WARNING: Invalid particle settings for material '%s'\n",
                m_texname.c_str());
        return;
    }
    
    ParticleKind* particles = NULL;
    try
    {
        particles = pkm->getParticles(base.c_str());
    }
    catch (...)
    {
        fprintf(stderr, "[Material::initParticlesEffect] WARNING: Cannot find particles '%s' for material '%s'\n",
                base.c_str(), m_texname.c_str());
        return;
    }
    
    std::vector<std::string> conditions;
    node->get("condition", &conditions);
    
    const int count = conditions.size();
    
    if (count == 0)
    {
        fprintf(stderr, "[Material::initParticlesEffect] WARNING: Particles '%s' for material '%s' are declared but not used\n",
                base.c_str(), m_texname.c_str());
    }
    
    for (int c=0; c<count; c++)
    {
        if (conditions[c] == "skid")
        {
            m_particles_effects[EMIT_ON_SKID] = particles;
        }
        else if (conditions[c] == "drive")
        {
            m_particles_effects[EMIT_ON_DRIVE] = particles;
        }
        else
        {
            fprintf(stderr, "[Material::initParticlesEffect] WARNING: Unknown condition '%s' for material '%s'\n",
                    conditions[c].c_str(), m_texname.c_str());
        }
    }
} // initParticlesEffect

//-----------------------------------------------------------------------------
/** Adjusts the pitch of the given sfx depending on the given speed.
 *  \param sfx The sound effect to adjust.
 *  \param speed The speed of the kart.
 */
void Material::setSFXSpeed(SFXBase *sfx, float speed) const
{
    if(sfx->getStatus()==SFXManager::SFX_STOPPED)
    {
        if(speed<m_sfx_min_speed) return;
        sfx->play();
    }
    else if(sfx->getStatus()==SFXManager::SFX_PLAYING)
    {
        if(speed<m_sfx_min_speed) 
        {
            sfx->stop();
            return;
        }
    }
    if(speed > m_sfx_max_speed)
    {
        sfx->speed(m_sfx_max_pitch);
        return;
    }

    float f = m_sfx_pitch_per_speed * (speed-m_sfx_min_speed) + m_sfx_min_pitch;
    sfx->speed(f);
}   // setSFXSpeed

//-----------------------------------------------------------------------------
/** Sets the appropriate flags in an irrlicht SMaterial.
 *  \param material The irrlicht SMaterial which gets the flags set.
 */
void  Material::setMaterialProperties(video::SMaterial *m) const
{
    int modes = 0;
    
    if (m_alpha_testing)
    {
        // Note: if EMT_TRANSPARENT_ALPHA_CHANNEL is used, you have to use
        // scene_manager->getParameters()->setAttribute(
        //    scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);  and enable 
        // updates of the Z buffer of the material. Since the _REF 
        // approach is faster (and looks better imho), this is used for now.
        m->MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
        
        modes++;
    }
    if (m_alpha_blending)
    {
        //m->MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
        
        // EMT_TRANSPARENT_ALPHA_CHANNEL does include vertex color alpha into account, which
        // messes up fading in/out effects. So we use the more customizable EMT_ONETEXTURE_BLEND instead
        m->MaterialType = video::EMT_ONETEXTURE_BLEND ;
        m->MaterialTypeParam = pack_texureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE_MINUS_SRC_ALPHA,
                                                    video::EMFN_MODULATE_1X, video::EAS_TEXTURE | video::EAS_VERTEX_COLOR); 
        
        modes++;
    }
    if (m_sphere_map) 
    {
        m->MaterialType = video::EMT_SPHERE_MAP;
        modes++;
    }
    if (m_lightmap)
    {
        m->MaterialType = video::EMT_LIGHTMAP;
        modes++;
    }
    if (m_add)
    {
        //m->MaterialType = video::EMT_TRANSPARENT_ADD_COLOR;
        
        // EMT_TRANSPARENT_ADD_COLOR does include vertex color alpha into account, which
        // messes up fading in/out effects. So we use the more customizable EMT_ONETEXTURE_BLEND instead
        m->MaterialType = video::EMT_ONETEXTURE_BLEND ;
        m->MaterialTypeParam = pack_texureBlendFunc(video::EBF_SRC_ALPHA, video::EBF_ONE,
                                                    video::EMFN_MODULATE_1X, video::EAS_TEXTURE | video::EAS_VERTEX_COLOR); 
        modes++;
    }
    
    if (m_disable_z_write)
    {
        m->ZWriteEnable = false;
    }
    
    if (modes > 1)
    {
        std::cerr << "[Material::setMaterialProperties] More than one main mode set for " << m_texname.c_str() << "\n";
    }

    if (!m_lighting)
    {
        //m->setFlag( video::EMF_LIGHTING, false );
        m->AmbientColor  = video::SColor(255, 255, 255, 255);
        m->DiffuseColor  = video::SColor(255, 255, 255, 255);
        m->EmissiveColor = video::SColor(255, 255, 255, 255);
        m->SpecularColor = video::SColor(255, 255, 255, 255);
    }
    
    // anisotropic
    if (m_anisotropic && UserConfigParams::m_anisotropic)
    {
        m->setFlag(video::EMF_ANISOTROPIC_FILTER, true);
    }
    else if (UserConfigParams::m_trilinear)
    {
        m->setFlag(video::EMF_TRILINEAR_FILTER, true);
    }    
    
    if ( (m_clamp_tex & UCLAMP) != 0)
    {
        //  m->setFlag();
        //  n1->getMaterial(0).TextureLayer[0].TextureWrap = video::ETC_CLAMP;
        /**
        //! Texture is clamped to the last pixel
		ETC_CLAMP,
		//! Texture is clamped to the edge pixel
		ETC_CLAMP_TO_EDGE,
		//! Texture is clamped to the border pixel (if exists)
		ETC_CLAMP_TO_BORDER,
        */
        for (unsigned int n=0; n<video::MATERIAL_MAX_TEXTURES; n++)
        {
            m->TextureLayer[n].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
        }
    }
    if ( (m_clamp_tex & VCLAMP) != 0)
    {
        for (unsigned int n=0; n<video::MATERIAL_MAX_TEXTURES; n++)
        {
            m->TextureLayer[n].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
        }
    }


    if(!m_backface_culling)
        m->setFlag(video::EMF_BACK_FACE_CULLING, false);

    m->ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
    
}   // setMaterialProperties
