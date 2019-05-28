//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2010-2015 Steve Baker, Joerg Henrichs
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

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "audio/sfx_base.hpp"
#include "audio/sfx_buffer.hpp"
#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "guiengine/engine.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/profile_world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"
#include "utils/vs.hpp"

#include <IMaterialRendererServices.h>
#include <ISceneNode.h>

using namespace irr::video;

const unsigned int UCLAMP = 1;
const unsigned int VCLAMP = 2;

//-----------------------------------------------------------------------------
/** Create a new material using the parameters specified in the xml file.
 *  \param node Node containing the parameters for this material.
 */
Material::Material(const XMLNode *node, bool deprecated)
{
    m_shader_name = "solid";
    m_deprecated = deprecated;
    m_installed = false;

    m_sampler_path[0] = "unicolor_white";
    node->get("name",      &m_texname);
    if (m_texname=="")
    {
        throw std::runtime_error("[Material] No texture name specified "
                                 "in file\n");
    }

    const std::string& relative_path = file_manager->searchTexture(m_texname);
    if (relative_path.empty())
    {
        Log::warn("Material", "Cannot determine texture full path: %s",
            m_texname.c_str());
    }
    else
    {
        m_full_path = m_sampler_path[0] =
            file_manager->getFileSystem()->getAbsolutePath
            (relative_path.c_str()).c_str();
    }

    if (m_full_path.size() > 0)
    {
        core::stringc texfname2(m_full_path.c_str());
        texfname2.make_lower();
        m_full_path = texfname2.c_str();
    }

    init();

    bool b = false;

    node->get("clampu", &b);  if (b) m_clamp_tex |= UCLAMP; //blender 2.4 style
    node->get("clampU", &b);  if (b) m_clamp_tex |= UCLAMP; //blender 2.5 style
    b = false;
    node->get("clampv", &b);  if (b) m_clamp_tex |= VCLAMP; //blender 2.4 style
    node->get("clampV", &b);  if (b) m_clamp_tex |= VCLAMP; //blender 2.5 style
    node->get("tex-compression", &m_tex_compression);

    std::string s;

    node->get("high-adhesion",    &m_high_tire_adhesion      );
    node->get("reset",            &m_drive_reset             );
    s = "";
    node->get("mirror-axis", &s);
    if (s == "u")
        s = "U";
    else if (s == "v")
        s = "V";
    if (s != "U" && s != "V")
        m_mirror_axis_when_reverse = ' ';
    else
        m_mirror_axis_when_reverse = s[0];
    // backwards compatibility
    bool crash_reset = false;
    node->get("crash-reset",      &crash_reset         );
    if (crash_reset)
    {
        m_collision_reaction = RESCUE;
        m_drive_reset = true; // if crash reset is enabled then drive reset should be too
    }

    std::string creaction;
    node->get("collision-reaction", &creaction);
    if (creaction == "reset")
    {
        m_collision_reaction = RESCUE;
    }
    else if (creaction == "push")
    {
        m_collision_reaction = PUSH_BACK;
    }
    else if (creaction == "push-soccer")
    {
        m_collision_reaction = PUSH_SOCCER_BALL;
    }
    else if (creaction.size() > 0)
    {
        Log::warn("Material","Unknown collision reaction '%s'",
                  creaction.c_str());
    }

    node->get("below-surface",    &m_below_surface     );
    node->get("falling-effect",   &m_falling_effect    );
    // A terrain with falling effect has to force a reset
    // when the kart is on it. So to make it easier for artists,
    // force the reset flag in this case.
    if(m_falling_effect)
        m_drive_reset=true;
    node->get("surface",             &m_surface            );
    node->get("ignore",              &m_ignore             );

    node->get("max-speed",           &m_max_speed_fraction );
    float f = stk_config->ticks2Time(m_slowdown_ticks);
    node->get("slowdown-time",       &f                    );
    m_slowdown_ticks = stk_config->time2Ticks(f);
    node->get("colorizable",         &m_colorizable        );
    node->get("colorization-factor", &m_colorization_factor);
    node->get("hue-settings",        &m_hue_settings       );

    node->get("mask",                &m_mask               );
    node->get("colorization-mask",   &m_colorization_mask  );
    std::string gloss_map;
    node->get("gloss-map",           &gloss_map            );
    node->get("jump",                &m_is_jump_texture    );
    node->get("has-gravity",         &m_has_gravity        );
    node->get("uv-two-tex",          &m_uv_two_tex         );

    if (!m_uv_two_tex.empty())
    {
        m_sampler_path[1] = m_uv_two_tex;
        core::stringc layer_two_tex(m_uv_two_tex.c_str());
        layer_two_tex.make_lower();
        m_uv_two_tex = layer_two_tex.c_str();
    }

    if (m_collision_reaction != NORMAL)
    {
        node->get("collision-particles", &m_collision_particles);

        if (m_collision_particles.size() == 0)
        {
            // backwards compatibility
            node->get("crash-reset-particles", &m_collision_particles);
        }
        if (!m_collision_particles.empty())
        {
            ParticleKindManager::get()->getParticles(m_collision_particles);
        }
    }

    s = "solid";
    std::string normal_map_tex;
    node->get("normal-map", &normal_map_tex);
    if (!node->get("shader", &s))
    {
        // BACKWARS COMPATIBILITY, EVENTUALLY REMOVE

        bool b = false;
        node->get("additive", &b);
        if (b)
        {
            m_shader_name = "alphablend";
        }

        b = false;
        node->get("transparency", &b);
        if (b)
        {
            m_shader_name = "alphatest";
        }

        //node->get("lightmap", &m_lightmap);

        b = false;
        node->get("alpha", &b);
        if (b)
        {
            m_shader_name = "alphablend";
        }

        b = true;
        node->get("light", &b);
        if (!b)
        {
            m_shader_name = "unlit";
        }
        if (node->get("compositing", &s))
        {
            if (s == "blend")
            {
                m_shader_name = "alphablend";
            }
            else if (s == "test")
            {
                m_shader_name = "alphatest";
            }
            else if (s == "additive")
            {
                m_shader_name = "additive";
            }
            else if (s == "coverage")
            {
                m_shader_name = "alphatest";
            }
            else if (s != "none")
                Log::warn("material", "Unknown compositing mode '%s'", s.c_str());
        }

        s = "";
        node->get("graphical-effect", &s);

        if (s == "normal_map")
        {
            node->get("normal-map", &normal_map_tex);
        }
        else if (s == "none")
        {
        }
        else if (s != "")
        {
            Log::warn("material",
                "Invalid graphical effect specification: '%s' - ignored.",
                s.c_str());
        }

        bool use_normal_map = false;
        node->get("use-normal-map", &use_normal_map);

        if (use_normal_map)
        {
            if (node->get("normal-map", &normal_map_tex))
            {
                //m_graphical_effect = GE_NORMAL_MAP;
            }
            else
            {
                Log::warn("material",
                    "Could not find normal map image in materials.xml");
            }
        }
        // ---- End backwards compatibility
    }

    if (m_shader_name == "solid" && !s.empty())
    {
        m_shader_name = s;
    }
    if (m_shader_name == "solid")
    {
        if (!normal_map_tex.empty())
        {
            m_shader_name = "normalmap";
        }
    }
    else if (m_shader_name == "normal_map")
    {
        m_shader_name = "normalmap";
    }

    // SP specific, now for backwards compatibility assign gloss map and
    // normal map to layer 2 and 3 (current all sp shaders use tex_layer_2 and
    // 3), and overwrite with tex-layer-(2-5) from xml
    if (!gloss_map.empty())
    {
        m_sampler_path[2] = gloss_map;
    }
    if (!normal_map_tex.empty())
    {
        m_sampler_path[3] = normal_map_tex;
    }
    for (int i = 2; i < 6; i++)
    {
        const std::string key =
            std::string("tex-layer-") + StringUtils::toString(i);
        node->get(key, &m_sampler_path[i]);
    }
    // Convert to full path
    for (int i = 1; i < 6; i++)
    {
        if (m_sampler_path[i].empty())
        {
            continue;
        }
        const std::string& relative_path =
            file_manager->searchTexture(m_sampler_path[i]);
        if (!relative_path.empty())
        {
            m_sampler_path[i] =
                file_manager->getFileSystem()->getAbsolutePath(
                relative_path.c_str()).c_str();
        }
        else
        {
            Log::warn("Material", "Cannot determine texture full path: %s",
                m_sampler_path[i].c_str());
            m_sampler_path[i] = "";
        }
    }
    loadContainerId();

    core::stringc texfname(m_texname.c_str());
    texfname.make_lower();
    m_texname = texfname.c_str();

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
            m_zipper                    = true;
            m_zipper_duration           = 3.5f;
            m_zipper_max_speed_increase = 15.0f;
            m_zipper_fade_out_time      = 3.0f;
            m_zipper_speed_gain         = 4.5f;
            m_zipper_engine_force       = 250;
            m_zipper_min_speed          = -1.0f;
            child_node->get("duration",          &m_zipper_duration          );
            child_node->get("fade-out-time",     &m_zipper_fade_out_time     );
            child_node->get("max-speed-increase",&m_zipper_max_speed_increase);
            child_node->get("speed-gain",        &m_zipper_speed_gain        );
            child_node->get("sengine-force",     &m_zipper_engine_force      );
            child_node->get("min-speed",         &m_zipper_min_speed         );
        }
        else
        {
            Log::warn("material", "Unknown node type '%s' for texture "
                      "'%s' - ignored.",
                      child_node->getName().c_str(), m_texname.c_str());
        }

    }   // for i <node->getNumNodes()

    if(m_has_gravity)
        m_high_tire_adhesion = true;
}   // Material

//-----------------------------------------------------------------------------
video::ITexture* Material::getTexture(bool srgb, bool premul_alpha)
{
    if (!m_installed)
    {
        install(srgb, premul_alpha);
    }
    return m_texture;
}   // getTexture

//-----------------------------------------------------------------------------
void Material::loadContainerId()
{
    if (m_sampler_path[0] != "unicolor_white")
    {
        if (!file_manager->searchTextureContainerId(m_container_id, m_texname))
        {
            Log::warn("Material", "Missing container id for %s, no texture"
                " compression for it will be done.", m_texname.c_str());
        }
    }
}   // loadContainerId

//-----------------------------------------------------------------------------
/** Create a standard material using the default settings for materials.
 *  \param fname Name of the texture file.
 *  \param is_full_path If the fname contains the full path.
 */
Material::Material(const std::string& fname, bool is_full_path,
                   bool complain_if_not_found, bool load_texture,
                   const std::string& shader_name)
{
    m_shader_name = shader_name;
    m_deprecated = false;
    m_installed = false;
    init();

    m_sampler_path[0] = "unicolor_white";
    if (is_full_path)
    {
        m_texname = StringUtils::getBasename(fname);
        m_full_path = m_sampler_path[0] = fname;
    }
    else
    {
        m_texname = fname;
        if (m_texname != "unicolor_white")
        {
            const std::string& relative_path =
                file_manager->searchTexture(m_texname);
            if (!relative_path.empty())
            {
                m_full_path = m_sampler_path[0] =
                    file_manager->getFileSystem()->getAbsolutePath(
                    relative_path.c_str()).c_str();
            }
            else
            {
                Log::warn("Material", "Cannot determine texture full path: %s",
                    fname.c_str());
            }
        }
    }
    loadContainerId();

    core::stringc texfname(m_texname.c_str());
    texfname.make_lower();
    m_texname = texfname.c_str();

    core::stringc texfname2(m_full_path.c_str());
    texfname2.make_lower();
    m_full_path = texfname2.c_str();

    m_complain_if_not_found = complain_if_not_found;

    if (load_texture)
        install();
}   // Material

//-----------------------------------------------------------------------------
/** Inits all material data with the default settings.
 */
void Material::init()
{
    m_texture                   = NULL;
    m_clamp_tex                 = 0;
    m_high_tire_adhesion        = false;
    m_below_surface             = false;
    m_falling_effect            = false;
    m_surface                   = false;
    m_ignore                    = false;
    m_drive_reset               = false;
    m_mirror_axis_when_reverse  = ' ';
    m_collision_reaction        = NORMAL;
    m_colorizable               = false;
    m_tex_compression           = true;
    m_colorization_factor       = 0.0f;
    m_colorization_mask         = "";
    m_max_speed_fraction        = 1.0f;
    m_slowdown_ticks            = stk_config->time2Ticks(1.0f);
    m_sfx_name                  = "";
    m_sfx_min_speed             = 0.0f;
    m_sfx_max_speed             = 30;
    m_sfx_min_pitch             = 1.0f;
    m_sfx_max_pitch             = 1.0f;
    m_sfx_pitch_per_speed       = 0.0f;
    m_zipper                    = false;
    m_zipper_duration           = -1.0f;
    m_zipper_fade_out_time      = -1.0f;
    m_zipper_max_speed_increase = -1.0f;
    m_zipper_speed_gain         = -1.0f;
    m_zipper_engine_force       = -1.0f;
    m_zipper_min_speed          = -1.0f;
    m_is_jump_texture           = false;
    m_has_gravity               = false;
    m_complain_if_not_found     = true;
    for (int n=0; n<EMIT_KINDS_COUNT; n++)
    {
        m_particles_effects[n] = NULL;
    }
}   // init

//-----------------------------------------------------------------------------
void Material::install(bool srgb, bool premul_alpha)
{
    // Don't load a texture that are not supposed to be loaded automatically
    if (m_installed) return;

    m_installed = true;

    if (m_texname.find(".") == std::string::npos || m_full_path.empty())
    {
        if (m_complain_if_not_found)
        {
            Log::error("material", "Cannot find texture '%s'.",
                m_texname.c_str());
        }
        m_texture = NULL;
    }
    else
    {
        TexConfig tc(srgb, premul_alpha, srgb/*mesh_tex*/);
        m_texture = STKTexManager::getInstance()
            ->getTexture(m_sampler_path[0], &tc);
    }

    if (m_texture == NULL) return;

    // now set the name to the basename, so that all tests work as expected
    m_texname  = StringUtils::getBasename(m_texname);

    core::stringc texfname(m_texname.c_str());
    texfname.make_lower();
    m_texname = texfname.c_str();

    m_texture->grab();
}   // install

//-----------------------------------------------------------------------------
Material::~Material()
{
    unloadTexture();

    // If a special sfx is installed (that isn't part of stk itself), the
    // entry needs to be removed from the sfx_manager's mapping, since other
    // tracks might use the same name.
    if(m_sfx_name!="" && m_sfx_name==m_texname)
    {
        SFXManager::get()->deleteSFXMapping(m_sfx_name);
    }
}   // ~Material

//-----------------------------------------------------------------------------

void Material::unloadTexture()
{
    if (m_texture != NULL)
    {
        m_texture->drop();
        if (m_texture->getReferenceCount() == 1)
        {
            irr_driver->removeTexture(m_texture);
        }
        m_texture = NULL;
        m_installed = false;
    }
}

//-----------------------------------------------------------------------------
/** Initialise the data structures for a custom sfx to be played when a
 *  kart is driving on that particular material.
 *  \param sfx The xml node containing the information for this sfx.
 */
void Material::initCustomSFX(const XMLNode *sfx)
{

    std::string filename;
    sfx->get("filename", &filename);

    if (filename.empty())
    {
        Log::warn("material", "Sfx node has no 'filename' "
                  "attribute, sound effect will be ignored.");
        return;
    }

    m_sfx_name = StringUtils::removeExtension(filename);
    sfx->get("min-speed", &m_sfx_min_speed); // 2.4 style
    sfx->get("min_speed", &m_sfx_min_speed); // 2.5 style

    sfx->get("max-speed", &m_sfx_max_speed); // 2.4 style
    sfx->get("max_speed", &m_sfx_max_speed); // 2.5 style

    sfx->get("min-pitch", &m_sfx_min_pitch); // 2.4 style
    sfx->get("min_pitch", &m_sfx_min_pitch); // 2.5 style

    sfx->get("max-pitch", &m_sfx_max_pitch); // 2.4 style
    sfx->get("max_pitch", &m_sfx_max_pitch); // 2.5 style

    if (m_sfx_max_speed == m_sfx_min_speed)
    {
        m_sfx_pitch_per_speed = 0.0f;
    }
    else
    {
        m_sfx_pitch_per_speed = (m_sfx_max_pitch - m_sfx_min_pitch)
            / (m_sfx_max_speed - m_sfx_min_speed);
    }

    if(!SFXManager::get()->soundExist(m_sfx_name))
    {

        // The directory for the track was added to the model search path
        // so just misuse the searchModel function
        std::string path = file_manager->searchModel(filename);
        path = StringUtils::getPath(path);
        SFXBuffer* buffer = SFXManager::get()->loadSingleSfx(sfx, path);

        if (buffer != NULL)
        {
            buffer->setPositional(true);
        }
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
        Log::warn("Material::initParticlesEffect"
                  "Invalid particle settings for material '%s'\n",
                  m_texname.c_str());
        return;
    }

    ParticleKind* particles = NULL;
    try
    {
        particles = pkm->getParticles(base);

        if (particles == NULL)
        {
            Log::warn("Material::initParticlesEffect",
                      "Error loading particles '%s' for material '%s'\n",
                      base.c_str(), m_texname.c_str());
        }
    }
    catch (...)
    {
        Log::warn("Material::initParticlesEffect",
                  "Cannot find particles '%s' for material '%s'\n",
                  base.c_str(), m_texname.c_str());
        return;
    }

    std::vector<std::string> conditions;
    node->get("condition", &conditions);

    const int count = (int)conditions.size();

    if (count == 0)
    {
        Log::warn("material", "initParticlesEffect: Particles "
                  "'%s' for material '%s' are declared but not used "
                  "(no emission condition set).",
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
            Log::warn("material", "initParticlesEffect: Unknown "
                            "condition '%s' for material '%s'",
                    conditions[c].c_str(), m_texname.c_str());
        }
    }
} // initParticlesEffect

//-----------------------------------------------------------------------------
/** Adjusts the pitch of the given sfx depending on the given speed.
 *  \param sfx The sound effect to adjust.
 *  \param speed The speed of the kart.
 *  \param should_be_paused Pause for other reasons, i.e. kart is rescued.
 */
void Material::setSFXSpeed(SFXBase *sfx, float speed, bool should_be_paused) const
{
    // Still make a sound when driving backwards on the material.
    if (speed < 0) speed = -speed;

    // If we paused it due to too low speed earlier, we can continue now.
    if (sfx->getStatus() == SFXBase::SFX_PAUSED)
    {
        if (speed<m_sfx_min_speed || should_be_paused == 1) return;
        // TODO: Do we first need to stop the sound completely so it
        // starts over?
        sfx->play();
    }
    else if (sfx->getStatus() == SFXBase::SFX_PLAYING)
    {
        if (speed<m_sfx_min_speed || should_be_paused == 1)
        {
            // Pausing it to differentiate with sounds that ended etc
            sfx->pause();
            return;
        }
    }
    if (speed > m_sfx_max_speed)
    {
        assert(!std::isnan(m_sfx_max_speed));
        sfx->setSpeed(m_sfx_max_pitch);
        return;
    }

    assert(!std::isnan(speed));

    float f = m_sfx_pitch_per_speed*(speed-m_sfx_min_speed) + m_sfx_min_pitch;
    assert(!std::isnan(f));
    sfx->setSpeed(f);
}   // setSFXSpeed

//-----------------------------------------------------------------------------
/** Sets the appropriate flags in an irrlicht SMaterial.
 *  \param material The irrlicht SMaterial which gets the flags set.
 */
void  Material::setMaterialProperties(video::SMaterial *m, scene::IMeshBuffer* mb)
{
    if (!m_installed)
    {
        install();
    }

    if (m_deprecated ||
        (m->getTexture(0) != NULL &&
         ((core::stringc)m->getTexture(0)->getName()).find("deprecated") != -1))
    {
        Log::warn("material", "Track uses deprecated texture '%s'",
                  m_texname.c_str());
    }

    // Default solid
    m->MaterialType = video::EMT_SOLID;
    if (race_manager->getReverseTrack() &&
        m_mirror_axis_when_reverse != ' ')
    {
        if (m_mirrorred_mesh_buffers.find((void*)mb) == m_mirrorred_mesh_buffers.end())
        {
            m_mirrorred_mesh_buffers[(void*)mb] = true;
            //irr::video::S3DVertex* mbVertices = (video::S3DVertex*)mb->getVertices();
            for (unsigned int i = 0; i < mb->getVertexCount(); i++)
            {
                core::vector2df &tc = mb->getTCoords(i);
                if (m_mirror_axis_when_reverse == 'V')
                    tc.Y = 1 - tc.Y;
                else
                    tc.X = 1 - tc.X;
            }
        }
    }   // reverse track and texture needs mirroring

    if (m_shader_name == "unlit")
    {
        m->AmbientColor = video::SColor(255, 255, 255, 255);
        m->DiffuseColor = video::SColor(255, 255, 255, 255);
        m->EmissiveColor = video::SColor(255, 255, 255, 255);
        m->SpecularColor = video::SColor(255, 255, 255, 255);
        m->MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
    }
    else if (m_shader_name == "alphatest")
    {
        m->MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
    }
    else if (m_shader_name == "alphablend" || m_shader_name == "displace")
    {
        // EMT_TRANSPARENT_ALPHA_CHANNEL doesn't include vertex color alpha into
        // account, which messes up fading in/out effects. So we use the more
        // customizable EMT_ONETEXTURE_BLEND instead.
        m->MaterialType = video::EMT_ONETEXTURE_BLEND;
        m->MaterialTypeParam =
            pack_textureBlendFunc(video::EBF_SRC_ALPHA,
                                  video::EBF_ONE_MINUS_SRC_ALPHA,
                                  video::EMFN_MODULATE_1X,
                                  video::EAS_TEXTURE | video::EAS_VERTEX_COLOR);
    }
    else if (m_shader_name == "additive")
    {
        // EMT_TRANSPARENT_ADD_COLOR doesn't include vertex color alpha into
        // account, which messes up fading in/out effects. So we use the
        // more customizable EMT_ONETEXTURE_BLEND instead
        m->MaterialType = video::EMT_ONETEXTURE_BLEND;
        m->MaterialTypeParam = pack_textureBlendFunc(video::EBF_SRC_ALPHA,
            video::EBF_ONE,
            video::EMFN_MODULATE_1X,
            video::EAS_TEXTURE |
            video::EAS_VERTEX_COLOR);
    }
    else if (m_shader_name == "grass")
    {
#ifdef USE_GLES2
        m->MaterialType = video::EMT_STK_GRASS;
#else
        m->MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

#ifndef SERVER_ONLY
        // A hack that makes the grass more bright in legacy pipeline, so that
        // it looks more similar to our shader-based pipeline
        if (!CVS->isGLSL())
        {
            m->AmbientColor  = video::SColor(255, 150, 150, 150);
            m->DiffuseColor  = video::SColor(255, 150, 150, 150);
            m->EmissiveColor = video::SColor(255, 150, 150, 150);
            m->SpecularColor = video::SColor(255, 150, 150, 150);
        }
#endif

#endif
    }

    if (isTransparent())
    {
        m->ZWriteEnable = false;
        m->BackfaceCulling = false;
    }

#ifdef DEBUG
    if(UserConfigParams::m_rendering_debug)
    {
        m->Shininess = 100.0f;
        m->DiffuseColor  = video::SColor(200, 255, 0, 0);
        m->AmbientColor  = video::SColor(200, 0, 0, 255);
        m->SpecularColor = video::SColor(200, 0, 255, 0);
    }
#endif

    if (UserConfigParams::m_anisotropic > 0)
    {
        for (u32 i=0; i<video::MATERIAL_MAX_TEXTURES; ++i)
        {
            m->TextureLayer[i].AnisotropicFilter =
                    UserConfigParams::m_anisotropic;
        }
    }
    m->setFlag(video::EMF_TRILINEAR_FILTER, true);

    // UV clamping
    if ( (m_clamp_tex & UCLAMP) != 0)
    {
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

    // Material color
    m->ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;

#ifdef DEBUG
    if (UserConfigParams::m_rendering_debug)
    {
        m->ColorMaterial = video::ECM_NONE; // Override one above
    }
#endif

} // setMaterialProperties
