//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2013  Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2009-2013  Joerg Henrichs, Steve Baker
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

#include "tracks/track.hpp"

#include "addons/addon.hpp"
#include "audio/music_manager.hpp"
#include "challenges/challenge_status.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/CBatchingMesh.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/stk_text_billboard.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/linear_world.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/world.hpp"
#include "physics/physical_object.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "tracks/bezier_curve.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/model_definition_loader.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/quad_set.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IBillboardTextSceneNode.h>
#include <ILightSceneNode.h>
#include <IMeshCache.h>
#include <IMeshManipulator.h>
#include <IMeshSceneNode.h>
#include <ISceneManager.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <wchar.h>

using namespace irr;


const float Track::NOHIT           = -99999.9f;

// ----------------------------------------------------------------------------
Track::Track(const std::string &filename)
{
#ifdef DEBUG
    m_magic_number          = 0x17AC3802;
#endif

    m_materials_loaded      = false;
    m_filename              = filename;
    m_root                  =
        StringUtils::getPath(StringUtils::removeExtension(m_filename));
    m_ident                 = StringUtils::getBasename(m_root);
    // If this is an addon track, add "addon_" to the identifier - just in
    // case that an addon track has the same directory name (and therefore
    // identifier) as an included track.
    if(Addon::isAddon(filename))
        m_ident = Addon::createAddonId(m_ident);

    // The directory should always have a '/' at the end, but getBasename
    // above returns "" if a "/" is at the end, so we add the "/" here.
    m_root                 += "/";
    m_designer              = "";
    m_screenshot            = "";
    m_version               = 0;
    m_track_mesh            = NULL;
    m_gfx_effect_mesh       = NULL;
    m_internal              = false;
    m_enable_auto_rescue    = true;  // Below set to false in arenas
    m_enable_push_back      = true;
    m_reverse_available     = false;
    m_is_arena              = false;
    m_has_easter_eggs       = false;
    m_is_soccer             = false;
    m_is_cutscene           = false;
    m_camera_far            = 1000.0f;
    m_old_rtt_mini_map      = NULL;
    m_new_rtt_mini_map      = NULL;
    m_bloom                 = true;
    m_bloom_threshold       = 0.75f;
    m_color_inlevel         = core::vector3df(0.0,1.0, 255.0);
    m_color_outlevel        = core::vector2df(0.0, 255.0);
    m_clouds                = false;
    m_godrays               = false;
    m_displacement_speed    = 1.0f;
    m_caustics_speed        = 1.0f;
    m_shadows               = true;
    m_sky_particles         = NULL;
    m_sky_dx                = 0.05f;
    m_sky_dy                = 0.0f;
    m_godrays_opacity       = 1.0f;
    m_godrays_color         = video::SColor(255, 255, 255, 255);
    m_weather_lightning      = false;
    m_weather_sound         = "";
    m_cache_track           = UserConfigParams::m_cache_overworld &&
                              m_ident=="overworld";
    m_minimap_x_scale       = 1.0f;
    m_minimap_y_scale       = 1.0f;
    m_startup_run = false;
    m_default_number_of_laps= 3;
    m_all_nodes.clear();
    m_all_physics_only_nodes.clear();
    m_all_cached_meshes.clear();
    loadTrackInfo();
}   // Track

//-----------------------------------------------------------------------------
/** Destructor, removes quad data structures etc. */
Track::~Track()
{
    // Note that the music information in m_music is globally managed
    // by the music_manager, and is freed there. So no need to free it
    // here (esp. since various track might share the same music).
#ifdef DEBUG
    assert(m_magic_number == 0x17AC3802);
    m_magic_number = 0xDEADBEEF;
#endif
}   // ~Track

//-----------------------------------------------------------------------------
/** A < comparison of tracks. This is used to sort the tracks when displaying
 *  them in the gui.
 */
bool Track::operator<(const Track &other) const
{
    PlayerProfile *p = PlayerManager::getCurrentPlayer();
    bool this_is_locked = p->isLocked(getIdent());
    bool other_is_locked = p->isLocked(other.getIdent());
    if(this_is_locked == other_is_locked)
    {
        return getSortName() < other.getSortName();
    }
    else
        return other_is_locked;
}   // operator<

//-----------------------------------------------------------------------------
/** Returns the name of the track, which is e.g. displayed on the screen.
    \note this is the LTR name, invoke fribidi as needed. */
core::stringw Track::getName() const
{
    core::stringw translated = translations->w_gettext(m_name.c_str());
    int index = translated.find("|");
    if(index>-1)
    {
        translated = translated.subString(0, index);
    }
    return translated;
}   // getName

//-----------------------------------------------------------------------------
/** Returns the name of the track used to sort the tracks alphabetically.
 *  This can be used to e.g. sort 'The Island' as 'Island,The'; or
 *  to replace certain language-specific characters (e.g. German 'ae' with 'a')
 *  The sort name can be specified by setting the name of a track to:
 *  "normal name|sort name"
 */
core::stringw Track::getSortName() const
{
    core::stringw translated = translations->w_gettext(m_name.c_str());
    translated.make_lower();
    int index = translated.find("|");
    if(index>-1)
    {
        translated = translated.subString(index+1, translated.size());
    }
    return translated;
}   // getSortName

//-----------------------------------------------------------------------------
/** Returns true if this track belongs to the specified track group.
 *  \param group_name Group name to test for.
 */
bool Track::isInGroup(const std::string &group_name)
{
    return std::find(m_groups.begin(), m_groups.end(), group_name)
        != m_groups.end();
}   // isInGroup

//-----------------------------------------------------------------------------
/** Returns number of completed challenges */
unsigned int Track::getNumOfCompletedChallenges()
{
    unsigned int unlocked_challenges = 0;
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    for (unsigned int i=0; i<m_challenges.size(); i++)
    {
        if (m_challenges[i].m_challenge_id == "tutorial")
        {
            unlocked_challenges++;
            continue;
        }
        if (player->getChallengeStatus(m_challenges[i].m_challenge_id)
                ->isSolvedAtAnyDifficulty())
        {
            unlocked_challenges++;
        }
    }

    return unlocked_challenges;
}   // getNumOfCompletedChallenges

//-----------------------------------------------------------------------------
/** Removes all cached data structures. This is called before the resolution
 *  is changed.
 */
void Track::removeCachedData()
{
    m_materials_loaded = false;
}   // cleanCachedData

//-----------------------------------------------------------------------------
/** Prepates the track for a new race. This function must be called after all
 *  karts are created, since the check objects allocate data structures
 *  depending on the number of karts.
 */
void Track::reset()
{
    m_ambient_color = m_default_ambient_color;
    CheckManager::get()->reset(*this);
    ItemManager::get()->reset();
    m_track_object_manager->reset();
    m_startup_run = false;
}   // reset

//-----------------------------------------------------------------------------
/** Removes the physical body from the world.
 *  Called at the end of a race.
 */
void Track::cleanup()
{
    QuadGraph::destroy();
    ItemManager::destroy();
    VAOManager::kill();

    ParticleKindManager::get()->cleanUpTrackSpecificGfx();
    // Clear reminder of transformed textures
    resetTextureTable();
    // Clear reminder of the link between textures and file names.
    irr_driver->clearTexturesFileName();

    for (unsigned int i = 0; i < m_animated_textures.size(); i++)
    {
        delete m_animated_textures[i];
    }
    m_animated_textures.clear();

    for (unsigned int i = 0; i < m_all_nodes.size(); i++)
    {
        irr_driver->removeNode(m_all_nodes[i]);
    }
    m_all_nodes.clear();
    m_all_physics_only_nodes.clear();

    m_all_emitters.clearAndDeleteAll();

    CheckManager::destroy();

    delete m_track_object_manager;
    m_track_object_manager = NULL;

    irr_driver->removeNode(m_sun);

    delete m_track_mesh;
    m_track_mesh = NULL;

    delete m_gfx_effect_mesh;
    m_gfx_effect_mesh = NULL;


    // The m_all_cached_mesh contains each mesh loaded from a file, which
    // means that the mesh is stored in irrlichts mesh cache. To clean
    // everything loaded by this track, we drop the ref count for each mesh
    // here, till the ref count is 1, which means the mesh is only contained
    // in the mesh cache, and can therefore be removed. Meshes load more
    // than once are in m_all_cached_mesh more than once (which is easier
    // than storing the mesh only once, but then having to test for each
    // mesh if it is already contained in the list or not).
    for (unsigned int i = 0; i < m_all_cached_meshes.size(); i++)
    {
        irr_driver->dropAllTextures(m_all_cached_meshes[i]);
        // If a mesh is not in Irrlicht's texture cache, its refcount is
        // 1 (since its scene node was removed, so the only other reference
        // is in m_all_cached_meshes). In this case we only drop it once
        // and don't try to remove it from the cache.
        if (m_all_cached_meshes[i]->getReferenceCount() == 1)
        {
            m_all_cached_meshes[i]->drop();
            continue;
        }
        m_all_cached_meshes[i]->drop();
        if (m_all_cached_meshes[i]->getReferenceCount() == 1)
            irr_driver->removeMeshFromCache(m_all_cached_meshes[i]);
    }
    m_all_cached_meshes.clear();

    // Now free meshes that are not associated to any scene node.
    for (unsigned int i = 0; i < m_detached_cached_meshes.size(); i++)
    {
        irr_driver->dropAllTextures(m_detached_cached_meshes[i]);
        irr_driver->removeMeshFromCache(m_detached_cached_meshes[i]);
    }
    m_detached_cached_meshes.clear();

    if (m_old_rtt_mini_map)
    {
        assert(m_old_rtt_mini_map->getReferenceCount() == 1);
        irr_driver->removeTexture(m_old_rtt_mini_map);
        m_old_rtt_mini_map = NULL;
    }
    if (m_new_rtt_mini_map)
    {
        delete m_new_rtt_mini_map;
        m_new_rtt_mini_map = NULL;
    }

    for(unsigned int i=0; i<m_sky_textures.size(); i++)
    {
        m_sky_textures[i]->drop();
        if(m_sky_textures[i]->getReferenceCount()==1)
            irr_driver->removeTexture(m_sky_textures[i]);
    }
    m_sky_textures.clear();

    for (unsigned int i = 0; i<m_spherical_harmonics_textures.size(); i++)
    {
        m_spherical_harmonics_textures[i]->drop();
        if (m_spherical_harmonics_textures[i]->getReferenceCount() == 1)
            irr_driver->removeTexture(m_spherical_harmonics_textures[i]);
    }
    m_spherical_harmonics_textures.clear();

    if(m_cache_track)
        material_manager->makeMaterialsPermanent();
    else
    {
        // remove temporary materials loaded by the material manager
        material_manager->popTempMaterial();
    }

    irr_driver->clearGlowingNodes();
    irr_driver->clearLights();
    irr_driver->clearForcedBloom();
    irr_driver->clearBackgroundNodes();

    if(UserConfigParams::logMemory())
    {
        Log::debug("track",
              "[memory] After cleaning '%s': mesh cache %d texture cache %d\n",
                getIdent().c_str(),
                irr_driver->getSceneManager()->getMeshCache()->getMeshCount(),
                irr_driver->getVideoDriver()->getTextureCount());
#ifdef DEBUG
        scene::IMeshCache *cache = irr_driver->getSceneManager()->getMeshCache();
        for(unsigned int i=0; i<cache->getMeshCount(); i++)
        {
            const io::SNamedPath &name = cache->getMeshName(i);
            std::vector<std::string>::iterator p;
            p = std::find(m_old_mesh_buffers.begin(), m_old_mesh_buffers.end(),
                         name.getInternalName().c_str());
            if(p!=m_old_mesh_buffers.end())
                m_old_mesh_buffers.erase(p);
            else
            {
                Log::debug("track", "[memory] Leaked mesh buffer '%s'.\n",
                       name.getInternalName().c_str());
            }   // if name not found
        }   // for i < cache size

        video::IVideoDriver *vd = irr_driver->getVideoDriver();
        for(unsigned int i=0; i<vd->getTextureCount(); i++)
        {
            video::ITexture *t = vd->getTextureByIndex(i);
            std::vector<video::ITexture*>::iterator p;
            p = std::find(m_old_textures.begin(), m_old_textures.end(),
                          t);
            if(p!=m_old_textures.end())
            {
                m_old_textures.erase(p);
            }
            else
            {
                Log::debug("track", "[memory] Leaked texture '%s'.\n",
                    t->getName().getInternalName().c_str());
            }
        }
#endif
    }   // if verbose

#ifdef __DEBUG_DUMP_MESH_CACHE_AFTER_CLEANUP__
    scene::IMeshCache* meshCache = irr_driver->getSceneManager()->getMeshCache();
    int count = meshCache->getMeshCount();
    for (int i = 0; i < count; i++)
    {
        scene::IAnimatedMesh* mesh = meshCache->getMeshByIndex(i);
        io::SNamedPath path = meshCache->getMeshName(mesh);
        Log::info("CACHE", "[%i] %s", i, path.getPath().c_str());
    }
#endif
}   // cleanup

//-----------------------------------------------------------------------------
void Track::loadTrackInfo()
{
    // Default values
    m_use_fog               = false;
    m_fog_max               = 1.0f;
    m_fog_start             = 0.0f;
    m_fog_end               = 1000.0f;
    m_fog_height_start      = 0.0f;
    m_fog_height_end        = 100.0f;
    m_gravity               = 9.80665f;
    m_smooth_normals        = false;
    m_godrays               = false;
    m_godrays_opacity       = 1.0f;
    m_godrays_color         = video::SColor(255, 255, 255, 255);
                              /* ARGB */
    m_fog_color             = video::SColor(255, 77, 179, 230);
    m_default_ambient_color = video::SColor(255, 120, 120, 120);
    m_sun_specular_color    = video::SColor(255, 255, 255, 255);
    m_sun_diffuse_color     = video::SColor(255, 255, 255, 255);
    m_sun_position          = core::vector3df(0, 0, 0);
    irr_driver->setSSAORadius(1.);
    irr_driver->setSSAOK(1.5);
    irr_driver->setSSAOSigma(1.);
    XMLNode *root           = file_manager->createXMLTree(m_filename);

    if(!root || root->getName()!="track")
    {
        std::ostringstream o;
        o<<"Can't load track '"<<m_filename<<"', no track element.";
        throw std::runtime_error(o.str());
    }
    root->get("name",                  &m_name);

    std::string designer;
    root->get("designer",              &designer);
    m_designer = StringUtils::decodeFromHtmlEntities(designer);

    root->get("version",               &m_version);
    std::vector<std::string> filenames;
    root->get("music",                 &filenames);
    getMusicInformation(filenames, m_music);
    root->get("screenshot",            &m_screenshot);
    root->get("gravity",               &m_gravity);
    root->get("soccer",                &m_is_soccer);
    root->get("arena",                 &m_is_arena);
    root->get("cutscene",              &m_is_cutscene);
    root->get("groups",                &m_groups);
    root->get("internal",              &m_internal);
    root->get("reverse",               &m_reverse_available);
    root->get("default-number-of-laps",&m_default_number_of_laps);
    root->get("push-back",             &m_enable_push_back);
    root->get("clouds",                &m_clouds);
    root->get("bloom",                 &m_bloom);
    root->get("bloom-threshold",       &m_bloom_threshold);
    root->get("shadows",               &m_shadows);
    root->get("displacement-speed",    &m_displacement_speed);
    root->get("caustics-speed",        &m_caustics_speed);
    root->get("color-level-in",        &m_color_inlevel);
    root->get("color-level-out",       &m_color_outlevel);

    if (m_default_number_of_laps <= 0)
        m_default_number_of_laps = 3;
    m_actual_number_of_laps = m_default_number_of_laps;

    // Make the default for auto-rescue in battle mode and soccer mode to be false
    if(m_is_arena || m_is_soccer)
        m_enable_auto_rescue = false;
    root->get("auto-rescue",           &m_enable_auto_rescue);
    root->get("smooth-normals",        &m_smooth_normals);
    // Reverse is meaningless in arena
    if(m_is_arena || m_is_soccer)
        m_reverse_available = false;


    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *mode=root->getNode(i);
        if(mode->getName()!="mode") continue;
        TrackMode tm;
        mode->get("name",  &tm.m_name      );
        mode->get("quads", &tm.m_quad_name );
        mode->get("graph", &tm.m_graph_name);
        mode->get("scene", &tm.m_scene     );
        m_all_modes.push_back(tm);
    }
    // If no mode is specified, add a default mode.
    if(m_all_modes.size()==0)
    {
        TrackMode tm;
        m_all_modes.push_back(tm);
    }

    if(m_groups.size()==0) m_groups.push_back(DEFAULT_GROUP_NAME);
    const XMLNode *xml_node = root->getNode("curves");

    if(xml_node) loadCurves(*xml_node);

    // Set the correct paths
    m_screenshot = m_root+m_screenshot;
    delete root;

    std::string dir = StringUtils::getPath(m_filename);
    std::string easter_name = dir + "/easter_eggs.xml";

    XMLNode *easter = file_manager->createXMLTree(easter_name);

    if(easter)
    {
        for(unsigned int i=0; i<easter->getNumNodes(); i++)
        {
            const XMLNode *eggs = easter->getNode(i);
            if(eggs->getNumNodes() > 0)
            {
                m_has_easter_eggs = true;
                break;
            }
        }
        delete easter;
    }
}   // loadTrackInfo

//-----------------------------------------------------------------------------
/** Loads all curves from the XML node.
 */
void Track::loadCurves(const XMLNode &node)
{
    for(unsigned int i=0; i<node.getNumNodes(); i++)
    {
        const XMLNode *curve = node.getNode(i);
        m_all_curves.push_back(new BezierCurve(*curve));
    }   // for i<node.getNumNodes
}   // loadCurves

//-----------------------------------------------------------------------------
/** Loads all music information for the specified files (which is taken from
 *  the track.xml file).
 *  \param filenames List of filenames to load.
 *  \param music On return contains the music information object for the
 *         specified files.
 */
void Track::getMusicInformation(std::vector<std::string>&       filenames,
                                std::vector<MusicInformation*>& music    )
{
    for(int i=0; i<(int)filenames.size(); i++)
    {
        std::string full_path = m_root+filenames[i];
        MusicInformation* mi = music_manager->getMusicInformation(full_path);
        if(!mi)
        {
            try
            {
                std::string shared_name = file_manager->searchMusic(filenames[i]);
                if(shared_name!="")
                    mi = music_manager->getMusicInformation(shared_name);
            }
            catch (...)
            {
                mi = NULL;
            }
        }
        if(mi)
            m_music.push_back(mi);
        else
            Log::warn("track",
                      "Music information file '%s' not found for track '%s' - ignored.\n",
                      filenames[i].c_str(), m_name.c_str());

    }   // for i in filenames

}   // getMusicInformation

//-----------------------------------------------------------------------------
/** Select and set the music for this track (doesn't actually start it yet).
 */
void Track::startMusic() const
{
    // In case that the music wasn't found (a warning was already printed)
    if(m_music.size()>0)
        music_manager->startMusic(m_music[rand()% m_music.size()], false);
    else
        music_manager->clearCurrentMusic();
}   // startMusic

//-----------------------------------------------------------------------------
/** Loads the quad graph, i.e. the definition of all quads, and the way
 *  they are connected to each other.
 */
void Track::loadQuadGraph(unsigned int mode_id, const bool reverse)
{
    QuadGraph::create(m_root+m_all_modes[mode_id].m_quad_name,
                      m_root+m_all_modes[mode_id].m_graph_name,
                      reverse);

    QuadGraph::get()->setupPaths();
#ifdef DEBUG
    for(unsigned int i=0; i<QuadGraph::get()->getNumNodes(); i++)
    {
        assert(QuadGraph::get()->getNode(i).getPredecessor(0)!=-1);
    }
#endif

    if(QuadGraph::get()->getNumNodes()==0)
    {
        Log::warn("track", "No graph nodes defined for track '%s'\n",
                m_filename.c_str());
        if (race_manager->getNumberOfKarts() > 1)
        {
            Log::fatal("track", "I can handle the lack of driveline in single"
                "kart mode, but not with AIs\n");
        }
    }
    else
    {
        //Check whether the hardware can do nonsquare or
        // non power-of-two textures
        video::IVideoDriver* const video_driver = irr_driver->getVideoDriver();
        bool nonpower = false; //video_driver->queryFeature(video::EVDF_TEXTURE_NPOT);
        bool nonsquare =
            video_driver->queryFeature(video::EVDF_TEXTURE_NSQUARE);

        //Create the minimap resizing it as necessary.
        m_mini_map_size = World::getWorld()->getRaceGUI()->getMiniMapSize();
        core::dimension2du size = m_mini_map_size
                                 .getOptimalSize(!nonpower,!nonsquare);

        QuadGraph::get()->makeMiniMap(size, "minimap::" + m_ident, video::SColor(127, 255, 255, 255),
            &m_old_rtt_mini_map, &m_new_rtt_mini_map);
        if (m_old_rtt_mini_map)
        {
            m_minimap_x_scale = float(m_mini_map_size.Width) / float(m_old_rtt_mini_map->getSize().Width);
            m_minimap_y_scale = float(m_mini_map_size.Height) / float(m_old_rtt_mini_map->getSize().Height);
        }
        else if (m_new_rtt_mini_map)
        {
            m_minimap_x_scale = float(m_mini_map_size.Width) / float(m_new_rtt_mini_map->getWidth());
            m_minimap_y_scale = float(m_mini_map_size.Height) / float(m_new_rtt_mini_map->getHeight());
        }
        else
        {
            m_minimap_x_scale = 0;
            m_minimap_y_scale = 0;
        }
    }
}   // loadQuadGraph

// -----------------------------------------------------------------------------
void Track::mapPoint2MiniMap(const Vec3 &xyz, Vec3 *draw_at) const
{
    QuadGraph::get()->mapPoint2MiniMap(xyz, draw_at);
    draw_at->setX(draw_at->getX() * m_minimap_x_scale);
    draw_at->setY(draw_at->getY() * m_minimap_y_scale);
}
// -----------------------------------------------------------------------------
/** Convert the track tree into its physics equivalents.
 *  \param main_track_count The number of meshes that are already converted
 *         when the main track was converted. Only the additional meshes
 *         added later still need to be converted.
 */
void Track::createPhysicsModel(unsigned int main_track_count)
{
    // Remove the temporary track rigid body, and then convert all objects
    // (i.e. the track and all additional objects) into a new rigid body
    // and convert this again. So this way we have an optimised track
    // rigid body which includes all track objects.
    // Note that removing the rigid body does not remove the already collected
    // triangle information, so there is no need to convert the actual track
    // (first element in m_track_mesh) again!

    if (m_track_mesh == NULL)
    {
        Log::error("track",
                   "m_track_mesh == NULL, cannot createPhysicsModel\n");
        return;
    }

    m_track_mesh->removeAll();
    m_gfx_effect_mesh->removeAll();
    for(unsigned int i=main_track_count; i<m_all_nodes.size(); i++)
    {
        convertTrackToBullet(m_all_nodes[i]);
    }
    m_track_mesh->createPhysicalBody();
    m_gfx_effect_mesh->createCollisionShape();
}   // createPhysicsModel

// -----------------------------------------------------------------------------


/** Convert the graohics track into its physics equivalents.
 *  \param mesh The mesh to convert.
 *  \param node The scene node.
 */
void Track::convertTrackToBullet(scene::ISceneNode *node)
{
    if (node->getType() == scene::ESNT_LOD_NODE)
    {
        node = ((LODNode*)node)->getFirstNode();
        if (node == NULL)
        {
            Log::warn("track",
                      "This track contains an empty LOD group.");
            return;
        }
    }
    node->updateAbsolutePosition();

    std::vector<core::matrix4> matrices;
    matrices.push_back(node->getAbsoluteTransformation());

    scene::IMesh *mesh;
    // In case of readonly materials we have to get the material from
    // the mesh, otherwise from the node. This is esp. important for
    // water nodes, which only have the material defined in the node,
    // but not in the mesh at all!
    bool is_readonly_material=false;


    switch(node->getType())
    {
        case scene::ESNT_MESH          :
        case scene::ESNT_WATER_SURFACE :
        case scene::ESNT_OCTREE        :
             mesh = ((scene::IMeshSceneNode*)node)->getMesh();
             is_readonly_material =
                 ((scene::IMeshSceneNode*)node)->isReadOnlyMaterials();
             break;
        case scene::ESNT_ANIMATED_MESH :
             mesh = ((scene::IAnimatedMeshSceneNode*)node)->getMesh();
             is_readonly_material =
                 ((scene::IAnimatedMeshSceneNode*)node)->isReadOnlyMaterials();
             break;
        case scene::ESNT_SKY_BOX :
        case scene::ESNT_SKY_DOME:
        case scene::ESNT_PARTICLE_SYSTEM :
        case scene::ESNT_TEXT:
            // These are non-physical
            return;
            break;
        default:
            int type_as_int = node->getType();
            char* type = (char*)&type_as_int;
            Log::debug("track",
                "[convertTrackToBullet] Unknown scene node type : %c%c%c%c.\n",
                   type[0], type[1], type[2], type[3]);
            return;
    }   // switch node->getType()

    //core::matrix4 mat;
    //mat.setRotationDegrees(hpr);
    //mat.setTranslation(pos);
    //core::matrix4 mat_scale;
    //// Note that we can't simply call mat.setScale, since this would
    //// overwrite the elements on the diagonal, making any rotation incorrect.
    //mat_scale.setScale(scale);
    //mat *= mat_scale;

    for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        // FIXME: take translation/rotation into account
        if (mb->getVertexType() != video::EVT_STANDARD &&
            mb->getVertexType() != video::EVT_2TCOORDS &&
            mb->getVertexType() != video::EVT_TANGENTS)
        {
            Log::warn("track", "convertTrackToBullet: Ignoring type '%d'!\n",
                mb->getVertexType());
            continue;
        }

        // Handle readonly materials correctly: mb->getMaterial can return
        // NULL if the node is not using readonly materials. E.g. in case
        // of a water scene node, the mesh (which is the animated copy of
        // the original mesh) does not contain any material information,
        // the material is only available in the node.
        const video::SMaterial &irrMaterial =
            is_readonly_material ? mb->getMaterial()
                                 : node->getMaterial(i);
        video::ITexture* t=irrMaterial.getTexture(0);

        const Material* material=0;
        TriangleMesh *tmesh = m_track_mesh;
        if(t)
        {
            std::string image = std::string(core::stringc(t->getName()).c_str());

            // the third boolean argument is false because at this point we're
            // dealing physics, so it's useless to warn about missing textures,
            // we'd just get duplicate/useless warnings
            material=material_manager->getMaterial(StringUtils::getBasename(image),
                                                   false, false, false);
            // Special gfx meshes will not be stored as a normal physics body,
            // but converted to a collision body only, so that ray tests
            // against them can be done.
            if(material->isSurface())
                tmesh = m_gfx_effect_mesh;
            // A material which is a surface must be converted,
            // even if it's marked as ignore. So only ignore
            // non-surface materials.
            else if(material->isIgnore())
                continue;
        }

        u16 *mbIndices = mb->getIndices();
        Vec3 vertices[3];
        Vec3 normals[3];

        if (mb->getVertexType() == video::EVT_STANDARD)
        {
            irr::video::S3DVertex* mbVertices=(video::S3DVertex*)mb->getVertices();
            for (unsigned int matrix_index = 0; matrix_index < matrices.size(); matrix_index++)
            {
                for (unsigned int j = 0; j < mb->getIndexCount(); j += 3)
                {
                    for (unsigned int k = 0; k < 3; k++)
                    {
                        int indx = mbIndices[j + k];
                        core::vector3df v = mbVertices[indx].Pos;
                        matrices[matrix_index].transformVect(v);
                        vertices[k] = v;
                        normals[k] = mbVertices[indx].Normal;
                    }   // for k

                    if (tmesh)
                    {
                        tmesh->addTriangle(vertices[0], vertices[1],
                            vertices[2], normals[0],
                            normals[1], normals[2],
                            material);
                    }
                }   // for j
            } // for matrix_index
        }
        else if (mb->getVertexType() == video::EVT_2TCOORDS)
        {
            irr::video::S3DVertex2TCoords* mbVertices = (video::S3DVertex2TCoords*)mb->getVertices();
            for (unsigned int matrix_index = 0; matrix_index < matrices.size(); matrix_index++)
            {
                for (unsigned int j = 0; j < mb->getIndexCount(); j += 3)
                {
                    for (unsigned int k = 0; k < 3; k++)
                    {
                        int indx = mbIndices[j + k];
                        core::vector3df v = mbVertices[indx].Pos;
                        matrices[matrix_index].transformVect(v);
                        vertices[k] = v;
                        normals[k] = mbVertices[indx].Normal;
                    }   // for k

                    if (tmesh)
                    {
                        tmesh->addTriangle(vertices[0], vertices[1],
                            vertices[2], normals[0],
                            normals[1], normals[2],
                            material);
                    }
                }   // for j
            } // for matrix_index
        }
        else if (mb->getVertexType() == video::EVT_TANGENTS)
        {
            irr::video::S3DVertexTangents* mbVertices = (video::S3DVertexTangents*)mb->getVertices();
            for (unsigned int matrix_index = 0; matrix_index < matrices.size(); matrix_index++)
            {
                for (unsigned int j = 0; j < mb->getIndexCount(); j += 3)
                {
                    for (unsigned int k = 0; k < 3; k++)
                    {
                        int indx = mbIndices[j + k];
                        core::vector3df v = mbVertices[indx].Pos;
                        matrices[matrix_index].transformVect(v);
                        vertices[k] = v;
                        normals[k] = mbVertices[indx].Normal;
                    }   // for k

                    if (tmesh)
                    {
                        tmesh->addTriangle(vertices[0], vertices[1],
                            vertices[2], normals[0],
                            normals[1], normals[2],
                            material);
                    }
                }   // for j
            } // for matrix_index
        }

    }   // for i<getMeshBufferCount

}   // convertTrackToBullet

// ----------------------------------------------------------------------------
/** Loads the main track model (i.e. all other objects contained in the
 *  scene might use raycast on this track model to determine the actual
 *  height of the terrain.
 */
bool Track::loadMainTrack(const XMLNode &root)
{
    assert(m_track_mesh==NULL);
    assert(m_gfx_effect_mesh==NULL);

    m_challenges.clear();
    m_force_fields.clear();

    m_track_mesh      = new TriangleMesh();
    m_gfx_effect_mesh = new TriangleMesh();

    const XMLNode *track_node = root.getNode("track");
    std::string model_name;
    track_node->get("model", &model_name);
    std::string full_path = m_root+model_name;

    scene::IMesh *mesh;
    // If the hd texture option is disabled, we generate smaller textures
    // and configure the path to them before loading the mesh.
    if (!UserConfigParams::m_high_definition_textures)
    {
        std::string cached_textures_dir =
            irr_driver->generateSmallerTextures(m_root);

        irr::io::IAttributes* scene_params =
            irr_driver->getSceneManager()->getParameters();
        // Before changing the texture path, we retrieve the older one to restore it later
        std::string texture_default_path =
            scene_params->getAttributeAsString(scene::B3D_TEXTURE_PATH).c_str();
        scene_params->setAttribute(scene::B3D_TEXTURE_PATH, cached_textures_dir.c_str());

        mesh = irr_driver->getMesh(full_path);

        scene_params->setAttribute(scene::B3D_TEXTURE_PATH, texture_default_path.c_str());
    }
    else // Load mesh with default (hd) textures
    {
        mesh = irr_driver->getMesh(full_path);
    }

    if(!mesh)
    {
        Log::fatal("track",
                   "Main track model '%s' in '%s' not found, aborting.\n",
                   track_node->getName().c_str(), model_name.c_str());
    }

    // The mesh as returned does not have all mesh buffers with the same
    // texture combined. This can result in a _HUGE_ overhead. E.g. instead
    // of 46 different mesh buffers over 500 (for some tracks even >1000)
    // were created. This means less effect from hardware support, less
    // vertices per opengl operation, more overhead on CPU, ...
    // So till we have a better b3d exporter which can combine the different
    // meshes which use the same texture when exporting, the meshes are
    // combined using CBatchingMesh.
    scene::CBatchingMesh *merged_mesh = new scene::CBatchingMesh();
    merged_mesh->addMesh(mesh);
    merged_mesh->finalize();

    scene::IMesh* tangent_mesh = MeshTools::createMeshWithTangents(merged_mesh, &MeshTools::isNormalMap);

    adjustForFog(tangent_mesh, NULL);

    // The merged mesh is grabbed by the octtree, so we don't need
    // to keep a reference to it.
    scene::ISceneNode *scene_node = irr_driver->addMesh(tangent_mesh, "track_main");
    //scene::IMeshSceneNode *scene_node = irr_driver->addOctTree(merged_mesh);
    // We should drop the merged mesh (since it's now referred to in the
    // scene node), but then we need to grab it since it's in the
    // m_all_cached_meshes.
    m_all_cached_meshes.push_back(tangent_mesh);
    irr_driver->grabAllTextures(tangent_mesh);

    // The reference count of the mesh is 1, since it is in irrlicht's
    // cache. So we only have to remove it from the cache.
    irr_driver->removeMeshFromCache(mesh);

#ifdef DEBUG
    std::string debug_name=model_name+" (main track, octtree)";
    scene_node->setName(debug_name.c_str());
#endif
    //merged_mesh->setHardwareMappingHint(scene::EHM_STATIC);

    core::vector3df xyz(0,0,0);
    track_node->getXYZ(&xyz);
    core::vector3df hpr(0,0,0);
    track_node->getHPR(&hpr);
    scene_node->setPosition(xyz);
    scene_node->setRotation(hpr);
    handleAnimatedTextures(scene_node, *track_node);
    m_all_nodes.push_back(scene_node);

    MeshTools::minMax3D(merged_mesh, &m_aabb_min, &m_aabb_max);
    // Increase the maximum height of the track: since items that fly
    // too high explode, e.g. cakes can not be show when being at the
    // top of the track (since they will explode when leaving the AABB
    // of the track). While the test for this in Flyable::updateAndDelete
    // could be relaxed to fix this, it is not certain how the physics
    // will handle items that are out of the AABB
    m_aabb_max.setY(m_aabb_max.getY()+30.0f);
    World::getWorld()->getPhysics()->init(m_aabb_min, m_aabb_max);

    ModelDefinitionLoader lodLoader(this);

    // Load LOD groups
    const XMLNode *lod_xml_node = root.getNode("lod");
    if (lod_xml_node != NULL)
    {
        for (unsigned int i = 0; i < lod_xml_node->getNumNodes(); i++)
        {
            const XMLNode* lod_group_xml = lod_xml_node->getNode(i);
            for (unsigned int j = 0; j < lod_group_xml->getNumNodes(); j++)
            {
                lodLoader.addModelDefinition(lod_group_xml->getNode(j));
            }
        }
    }

    for (unsigned int i=0; i<track_node->getNumNodes(); i++)
    {
        const XMLNode *n=track_node->getNode(i);
        // Animated textures have already been handled
        if(n->getName()=="animated-texture") continue;
        // Only "object" entries are allowed now inside of the model tag
        if(n->getName()!="static-object")
        {
            Log::error("track",
                "Incorrect tag '%s' inside <model> of scene file - ignored\n",
                    n->getName().c_str());
            continue;
        }


        core::vector3df xyz(0,0,0);
        n->get("xyz", &xyz);
        core::vector3df hpr(0,0,0);
        n->get("hpr", &hpr);
        core::vector3df scale(1.0f, 1.0f, 1.0f);
        n->get("scale", &scale);

        // some static meshes are conditional
        std::string condition;
        n->get("if", &condition);
        if (condition == "splatting")
        {
            if (!irr_driver->supportsSplatting()) continue;
        }
        else if (condition == "trophies")
        {
            // Associate force fields and challenges
            // FIXME: this assumes that challenges will appear before force fields in scene.xml
            //        (which however seems to be the case atm)
            int closest_challenge_id = -1;
            float closest_distance = 99999.0f;
            for (unsigned int c=0; c<m_challenges.size(); c++)
            {

                float dist = xyz.getDistanceFromSQ(m_challenges[c].m_position);
                if (closest_challenge_id == -1 || dist < closest_distance)
                {
                    closest_challenge_id = c;
                    closest_distance = dist;
                }
            }

            assert(closest_challenge_id >= 0);
            assert(closest_challenge_id < (int)m_challenges.size());

            const std::string &s = m_challenges[closest_challenge_id].m_challenge_id;
            const ChallengeData* challenge = unlock_manager->getChallengeData(s);
            if (challenge == NULL)
            {
                if (s != "tutorial")
                    Log::error("track", "Cannot find challenge named '%s'\n",
                        m_challenges[closest_challenge_id].m_challenge_id.c_str());
                continue;
            }

            const unsigned int val = challenge->getNumTrophies();
            bool shown = (PlayerManager::getCurrentPlayer()->getPoints() < val);
            m_force_fields.push_back(OverworldForceField(xyz, shown, val));

            m_challenges[closest_challenge_id].setForceField(
                                 m_force_fields[m_force_fields.size() - 1]);

            core::stringw msg = StringUtils::toWString(val);
            core::dimension2d<u32> textsize = GUIEngine::getHighresDigitFont()
                                                   ->getDimension(msg.c_str());

            assert(GUIEngine::getHighresDigitFont() != NULL);

            if (irr_driver->isGLSL())
            {
                gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
                STKTextBillboard* tb = new STKTextBillboard(msg.c_str(), font,
                    video::SColor(255, 255, 225, 0),
                    video::SColor(255, 255, 89, 0),
                    irr_driver->getSceneManager()->getRootSceneNode(),
                    irr_driver->getSceneManager(), -1, xyz,
                    core::vector3df(1.5f, 1.5f, 1.5f));
                m_all_nodes.push_back(tb);
            }
            else
            {
                scene::ISceneManager* sm = irr_driver->getSceneManager();
                scene::ISceneNode* sn =
                    sm->addBillboardTextSceneNode(GUIEngine::getHighresDigitFont(),
                    msg.c_str(),
                    NULL,
                    core::dimension2df(textsize.Width / 35.0f,
                    textsize.Height / 35.0f),
                    xyz,
                    -1, // id
                    video::SColor(255, 255, 225, 0),
                    video::SColor(255, 255, 89, 0));
                m_all_nodes.push_back(sn);
            }

            if (!shown) continue;
        }
        else if (condition == "allchallenges")
        {
            // allow ONE unsolved challenge : the last one
            if (getNumOfCompletedChallenges() < m_challenges.size() - 1)
                continue;
        }
        else if (condition.size() > 0)
        {
            Log::error("track", "Unknown condition <%s>\n", condition.c_str());
        }

        std::string neg_condition;
        n->get("ifnot", &neg_condition);
        if (neg_condition == "splatting")
        {
            if (irr_driver->supportsSplatting()) continue;
        }
        else if (neg_condition == "allchallenges")
        {
            // allow ONE unsolved challenge : the last one
            if (getNumOfCompletedChallenges() >= m_challenges.size() - 1)
                continue;
        }
        else if (neg_condition.size() > 0)
        {
            Log::error("track", "Unknown condition <%s>\n",
                       neg_condition.c_str());
        }

        bool tangent = false;
        n->get("tangents", &tangent);

        scene::ISceneNode* scene_node;
        model_name="";
        n->get("model", &model_name);
        full_path = m_root+model_name;
        std::string interaction;
        n->get("interaction", &interaction);

        // a special challenge orb object for overworld
        std::string challenge;
        n->get("challenge", &challenge);

        bool lod_instance = false;
        n->get("lod_instance", &lod_instance);

        if (tangent)
        {
            scene::IMesh* original_mesh = irr_driver->getMesh(full_path);

            if (std::find(m_detached_cached_meshes.begin(),
                          m_detached_cached_meshes.end(),
                          original_mesh) == m_detached_cached_meshes.end())
            {
                m_detached_cached_meshes.push_back(original_mesh);
            }

            // create a node out of this mesh just for bullet; delete it after, normal maps are special
            // and require tangent meshes
            scene_node = irr_driver->addMesh(original_mesh, "original_mesh");

            scene_node->setPosition(xyz);
            scene_node->setRotation(hpr);
            scene_node->setScale(scale);

            convertTrackToBullet(scene_node);
            scene_node->remove();
            irr_driver->grabAllTextures(original_mesh);

            scene::IMesh* mesh = MeshTools::createMeshWithTangents(original_mesh, &MeshTools::isNormalMap);
            mesh->grab();
            irr_driver->grabAllTextures(mesh);

            m_all_cached_meshes.push_back(mesh);
            scene_node = irr_driver->addMesh(mesh, "original_mesh_with_tangents");
            scene_node->setPosition(xyz);
            scene_node->setRotation(hpr);
            scene_node->setScale(scale);

#ifdef DEBUG
            std::string debug_name = model_name+" (tangent static track-object)";
            scene_node->setName(debug_name.c_str());
#endif

            handleAnimatedTextures(scene_node, *n);
            m_all_nodes.push_back( scene_node );
        }
        else if (lod_instance)
        {
            LODNode* node = lodLoader.instanciateAsLOD(n, NULL);
            if (node != NULL)
            {
                node->setPosition(xyz);
                node->setRotation(hpr);
                node->setScale(scale);
                node->updateAbsolutePosition();

                m_all_nodes.push_back( node );
            }
        }
        else
        {
            // TODO: check if mesh is animated or not
            scene::IMesh *a_mesh = irr_driver->getMesh(full_path);
            if(!a_mesh)
            {
                Log::error("track", "Object model '%s' not found, ignored.\n",
                           full_path.c_str());
                continue;
            }

            // The meshes loaded here are in irrlicht's mesh cache. So we
            // have to keep track of them in order to properly remove them
            // from memory. We could add each track only once in a list, but
            // it's actually faster to add meshes multipl times (if they are
            // used more than once), and increase the ref count each time.
            // When removing the meshes, we drop them till the ref count is
            // 1 - which means that the only reference is now in the cache,
            // and can therefore be removed.
            m_all_cached_meshes.push_back(a_mesh);
            irr_driver->grabAllTextures(a_mesh);
            a_mesh->grab();
            scene_node = irr_driver->addMesh(a_mesh, model_name);
            scene_node->setPosition(xyz);
            scene_node->setRotation(hpr);
            scene_node->setScale(scale);
#ifdef DEBUG
            std::string debug_name = model_name+" (static track-object)";
            scene_node->setName(debug_name.c_str());
#endif

            handleAnimatedTextures(scene_node, *n);

            // for challenge orbs, a bit more work to do
            if (challenge.size() > 0)
            {
                const ChallengeData* c = NULL;

                if (challenge != "tutorial")
                {
                    c = unlock_manager->getChallengeData(challenge);
                    if (c == NULL)
                    {
                        Log::error("track", "Cannot find challenge named <%s>\n",
                                   challenge.c_str());
                        scene_node->remove();
                        continue;
                    }
                }

                m_challenges.push_back( OverworldChallenge(xyz, challenge) );

                if (c && c->isGrandPrix())
                {

                }
                else
                {
                    if (challenge != "tutorial")
                    {
                        Track* t = track_manager->getTrack(c->getTrackId());
                        if (t == NULL)
                        {
                            Log::error("track", "Cannot find track named <%s>\n",
                                       c->getTrackId().c_str());
                            continue;
                        }

                        std::string sshot = t->getScreenshotFile();
                        video::ITexture* screenshot = irr_driver->getTexture(sshot);

                        if (screenshot == NULL)
                        {
                            Log::error("track",
                                       "Cannot find track screenshot <%s>",
                                       sshot.c_str());
                            continue;
                        }
                        scene_node->getMaterial(0).setTexture(0, screenshot);
                    }
                }

                // make transparent
                for (unsigned int m=0; m<a_mesh->getMeshBufferCount(); m++)
                {
                    scene::IMeshBuffer* mb = a_mesh->getMeshBuffer(m);
                    if (mb->getVertexType() == video::EVT_STANDARD)
                    {
                        video::S3DVertex* v = (video::S3DVertex*)mb->getVertices();
                        for (unsigned int n=0; n<mb->getVertexCount(); n++)
                        {
                            v[n].Color.setAlpha(125);
                        }
                    }
                }


                LODNode* lod_node = new LODNode("challenge_orb",
                                                irr_driver->getSceneManager()->getRootSceneNode(),
                                                irr_driver->getSceneManager());
                lod_node->setPosition(xyz);
                lod_node->add(50, scene_node, true /* reparent */);

                m_all_nodes.push_back( lod_node );
            }
            else
            {
                if(interaction=="physics-only")
                    m_all_physics_only_nodes.push_back( scene_node );
                else
                    m_all_nodes.push_back( scene_node );
            }
        }

    }   // for i

    // This will (at this stage) only convert the main track model.
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        convertTrackToBullet(m_all_nodes[i]);
    }

    // Now convert all objects that are only used for the physics
    // (like invisible walls).
    for(unsigned int i=0; i<m_all_physics_only_nodes.size(); i++)
    {
        convertTrackToBullet(m_all_physics_only_nodes[i]);
        irr_driver->removeNode(m_all_physics_only_nodes[i]);
    }
    m_all_physics_only_nodes.clear();

    if (m_track_mesh == NULL)
    {
        Log::fatal("track", "m_track_mesh == NULL, cannot loadMainTrack\n");
    }

    m_gfx_effect_mesh->createCollisionShape();
    scene_node->setMaterialFlag(video::EMF_LIGHTING, true);
    scene_node->setMaterialFlag(video::EMF_GOURAUD_SHADING, true);

    return true;
}   // loadMainTrack

// ----------------------------------------------------------------------------
/** Handles animated textures.
 *  \param node The scene node for which animated textures are handled.
 *  \param xml The node containing the data for the animated notion.
 */
void Track::handleAnimatedTextures(scene::ISceneNode *node, const XMLNode &xml)
{
    for(unsigned int node_number = 0; node_number<xml.getNumNodes();
        node_number++)
    {
        const XMLNode *texture_node = xml.getNode(node_number);
        if(texture_node->getName()!="animated-texture") continue;
        std::string name;
        texture_node->get("name", &name);
        if(name=="")
        {
            Log::error("track",
                "Animated texture: no texture name specified for track '%s'\n",
                 m_ident.c_str());
            continue;
        }

        // to lower case, for case-insensitive comparison
        name = StringUtils::toLowerCase(name);

        for(unsigned int i=0; i<node->getMaterialCount(); i++)
        {
            video::SMaterial &irrMaterial=node->getMaterial(i);
            for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
            {
                video::ITexture* t=irrMaterial.getTexture(j);
                if(!t) continue;
                std::string texture_name =
                    StringUtils::getBasename(core::stringc(t->getName()).c_str());

                // to lower case, for case-insensitive comparison
                texture_name = StringUtils::toLowerCase(texture_name);

                if (texture_name != name) continue;
                core::matrix4 *m = &irrMaterial.getTextureMatrix(j);
                m_animated_textures.push_back(new MovingTexture(m, *texture_node));
            }   // for j<MATERIAL_MAX_TEXTURES
        }   // for i<getMaterialCount
    }   // for node_number < xml->getNumNodes
}   // handleAnimatedTextures

// ----------------------------------------------------------------------------
/** Update, called once per frame.
 *  \param dt Timestep.
 */
void Track::update(float dt)
{
    if (!m_startup_run) // first time running update = good point to run startup script
    {
        Scripting::ScriptEngine* script_engine = World::getWorld()->getScriptEngine();
        script_engine->runScript("start");
        m_startup_run = true;
    }
    m_track_object_manager->update(dt);

    for(unsigned int i=0; i<m_animated_textures.size(); i++)
    {
        m_animated_textures[i]->update(dt);
    }
    CheckManager::get()->update(dt);
    ItemManager::get()->update(dt);
    Scripting::ScriptEngine* script_engine = World::getWorld()->getScriptEngine();
    script_engine->runScript("update");
}   // update

// ----------------------------------------------------------------------------
/** Handles an explosion, i.e. it makes sure that all physical objects are
 *  affected accordingly.
 *  \param pos  Position of the explosion.
 *  \param obj  If the hit was a physical object, this object will be affected
 *              more. Otherwise this is NULL.
 *  \param secondary_hits True if items that are not directly hit should
 *         also be affected. */
void Track::handleExplosion(const Vec3 &pos, const PhysicalObject *obj,
                            bool secondary_hits) const
{
    m_track_object_manager->handleExplosion(pos, obj, secondary_hits);
}   // handleExplosion

// ----------------------------------------------------------------------------
/** Creates a water node. OBSOLETE, kept for backwards compat only
 *  \param node The XML node containing the specifications for the water node.
 */
void Track::createWater(const XMLNode &node)
{
    std::string model_name;
    node.get("model", &model_name);
    std::string full_path = m_root+model_name;

    scene::IMesh *mesh = irr_driver->getMesh(full_path);
    if (mesh == NULL)
    {
        Log::warn("Track", "Water not found : '%s'", full_path.c_str());
        return;
    }

    /*
    float wave_height  = 2.0f;
    float wave_speed   = 300.0f;
    float wave_length  = 10.0f;
    node.get("height", &wave_height);
    float time;
    if(node.get("time", &time))
    {
        wave_speed = time * 1000.0f/(2.0f*M_PI);
    }
    else
        node.get("speed",  &wave_speed);
    if(wave_speed==0)
    {
        // A speed of 0 results in a division by zero, so avoid this.
        // The actual time for a wave from one maximum to the next is
        // given by 2*M_PI*speed/1000.
        Log::warn("Track",
                  "Wave-speed or time is 0, resetting it to the default.");
        wave_speed =300.0f;
    }
    node.get("length", &wave_length);
    */
    scene::ISceneNode* scene_node = NULL;
    /*
    if (UserConfigParams::m_graphical_effects)
    {
        scene::IMesh *welded;
        scene_node = irr_driver->addWaterNode(mesh, &welded,
                                              wave_height,
                                              wave_speed,
                                              wave_length);

        mesh->grab();
        irr_driver->grabAllTextures(mesh);
        m_all_cached_meshes.push_back(mesh);

        mesh = welded;
    }
    else
    {*/
        scene_node = irr_driver->addMesh(mesh, "water");
    //}

    if(!mesh || !scene_node)
    {
        Log::error("track", "Water model '%s' in '%s' not found, ignored.\n",
                    node.getName().c_str(), model_name.c_str());
        return;
    }

#ifdef DEBUG
    std::string debug_name = model_name+"(water node)";
    scene_node->setName(debug_name.c_str());
#endif
    mesh->grab();
    m_all_cached_meshes.push_back(mesh);
    irr_driver->grabAllTextures(mesh);

    core::vector3df xyz(0,0,0);
    node.get("xyz", &xyz);
    core::vector3df hpr(0,0,0);
    node.get("hpr", &hpr);
    scene_node->setPosition(xyz);
    scene_node->setRotation(hpr);
    m_all_nodes.push_back(scene_node);
    handleAnimatedTextures(scene_node, node);

    scene_node->getMaterial(0).setFlag(video::EMF_GOURAUD_SHADING, true);
}   // createWater

// ----------------------------------------------------------------------------
/** This function load the actual scene, i.e. all parts of the track,
 *  animations, items, ... It  is called from world during initialisation.
 *  Track is the first model to be loaded, so at this stage the root scene node
 *  is empty.
 *  \param parent The actual world.
 *  \param reverse_track True if the track should be run in reverse.
 *  \param mode_id Which of the modes of a track to use. This determines which
 *         scene, quad, and graph file to load.
 */
void Track::loadTrackModel(bool reverse_track, unsigned int mode_id)
{
    // Use m_filename to also get the path, not only the identifier
    irr_driver->setTextureErrorMessage("While loading track '%s'",
                                       m_filename                  );
    if(!m_reverse_available)
    {
        reverse_track = false;
    }
    CheckManager::create();
    assert(m_all_cached_meshes.size()==0);
    if(UserConfigParams::logMemory())
    {
        Log::debug("[memory] Before loading '%s': mesh cache %d "
                   "texture cache %d\n",
            getIdent().c_str(),
            irr_driver->getSceneManager()->getMeshCache()->getMeshCount(),
            irr_driver->getVideoDriver()->getTextureCount());
#ifdef DEBUG
        scene::IMeshCache *cache =
            irr_driver->getSceneManager()->getMeshCache();
        m_old_mesh_buffers.clear();
        for(unsigned int i=0; i<cache->getMeshCount(); i++)
        {
            const io::SNamedPath &name=cache->getMeshName(i);
            m_old_mesh_buffers.push_back(name.getInternalName().c_str());
        }

        m_old_textures.clear();
        video::IVideoDriver *vd = irr_driver->getVideoDriver();
        for(unsigned int i=0; i<vd->getTextureCount(); i++)
        {
            video::ITexture *t=vd->getTextureByIndex(i);
            m_old_textures.push_back(t);
        }
#endif
    }

    Camera::clearEndCameras();
    m_sky_type             = SKY_NONE;
    m_track_object_manager = new TrackObjectManager();

    // Add the track directory to the texture search path
    file_manager->pushTextureSearchPath(m_root);
    file_manager->pushModelSearchPath  (m_root);

    // If the hd texture option is disabled, we generate smaller textures
    // and we also add the cache directory to the texture search path
    if (!UserConfigParams::m_high_definition_textures)
    {
        std::string cached_textures_dir =
            irr_driver->generateSmallerTextures(m_root);
        file_manager->pushTextureSearchPath(cached_textures_dir);
    }

    // First read the temporary materials.xml file if it exists
    try
    {
        std::string materials_file = m_root+"materials.xml";
        if(m_cache_track)
        {
            if(!m_materials_loaded)
                material_manager->addSharedMaterial(materials_file);
            m_materials_loaded = true;
        }
        else
            material_manager->pushTempMaterial(materials_file);
    }
    catch (std::exception& e)
    {
        // no temporary materials.xml file, ignore
        (void)e;
    }

    // Load the graph only now: this function is called from world, after
    // the race gui was created. The race gui is needed since it stores
    // the information about the size of the texture to render the mini
    // map to.
    if (!m_is_arena && !m_is_soccer && !m_is_cutscene) loadQuadGraph(mode_id, reverse_track);

    ItemManager::create();

    // Set the default start positions. Node that later the default
    // positions can still be overwritten.
    float forwards_distance  = 1.5f;
    float sidewards_distance = 3.0f;
    float upwards_distance   = 0.1f;
    int   karts_per_row      = 2;


    // Start building the scene graph
    std::string path = m_root + m_all_modes[mode_id].m_scene;
    XMLNode *root    = file_manager->createXMLTree(path);

    // Make sure that we have a track (which is used for raycasts to
    // place other objects).
    if (!root || root->getName()!="scene")
    {
        std::ostringstream msg;
        msg<< "No track model defined in '"<<path
           <<"', aborting.";
        throw std::runtime_error(msg.str());
    }

    const XMLNode *default_start = root->getNode("default-start");
    if (default_start)
    {
        default_start->get("forwards-distance",  &forwards_distance );
        default_start->get("sidewards-distance", &sidewards_distance);
        default_start->get("upwards-distance",   &upwards_distance  );
        default_start->get("karts-per-row",      &karts_per_row     );
    }

    if (!m_is_arena && !m_is_soccer && !m_is_cutscene)
    {
        m_start_transforms.resize(race_manager->getNumberOfKarts());
        QuadGraph::get()->setDefaultStartPositions(&m_start_transforms,
                                                   karts_per_row,
                                                   forwards_distance,
                                                   sidewards_distance,
                                                   upwards_distance);
    }

    // we need to check for fog before loading the main track model
    if (const XMLNode *node = root->getNode("sun"))
    {
        node->get("xyz",           &m_sun_position );
        node->get("ambient",       &m_default_ambient_color);
        node->get("sun-specular",  &m_sun_specular_color);
        node->get("sun-diffuse",   &m_sun_diffuse_color);
        node->get("fog",           &m_use_fog);
        node->get("fog-color",     &m_fog_color);
        node->get("fog-max",       &m_fog_max);
        node->get("fog-start",     &m_fog_start);
        node->get("fog-end",       &m_fog_end);
        node->get("fog-start-height", &m_fog_height_start);
        node->get("fog-end-height",   &m_fog_height_end);
    }

    if (const XMLNode *node = root->getNode("lightshaft"))
    {
        m_godrays = true;
        node->get("opacity", &m_godrays_opacity);
        node->get("color", &m_godrays_color);
        node->get("xyz", &m_godrays_position);
    }

    loadMainTrack(*root);
    unsigned int main_track_count = (unsigned int)m_all_nodes.size();

    ModelDefinitionLoader model_def_loader(this);

    // Load LOD groups
    const XMLNode *lod_xml_node = root->getNode("lod");
    if (lod_xml_node != NULL)
    {
        for (unsigned int i = 0; i < lod_xml_node->getNumNodes(); i++)
        {
            const XMLNode* lod_group_xml = lod_xml_node->getNode(i);
            for (unsigned int j = 0; j < lod_group_xml->getNumNodes(); j++)
            {
                model_def_loader.addModelDefinition(lod_group_xml->getNode(j));
            }
        }
    }

    loadObjects(root, path, model_def_loader, true, NULL);

    model_def_loader.cleanLibraryNodesAfterLoad();

    // Init all track objects
    m_track_object_manager->init();


    // ---- Fog
    // It's important to execute this BEFORE the code that creates the skycube,
    // otherwise the skycube node could be modified to have fog enabled, which
    // we don't want
    if (m_use_fog && !UserConfigParams::m_camera_debug && !irr_driver->isGLSL())
    {
        /* NOTE: if LINEAR type, density does not matter, if EXP or EXP2, start
           and end do not matter */
        irr_driver->getVideoDriver()->setFog(m_fog_color,
                                             video::EFT_FOG_LINEAR,
                                             m_fog_start, m_fog_end,
                                             1.0f);
    }

    // Enable for for all track nodes if fog is used
    const unsigned int count = (int)m_all_nodes.size();
    for(unsigned int i=0; i<count; i++)
    {
        adjustForFog(m_all_nodes[i]);
    }
    m_track_object_manager->enableFog(m_use_fog);

    // Sky dome and boxes support
    // --------------------------
    irr_driver->suppressSkyBox();
    if(m_sky_type==SKY_DOME && m_sky_textures.size() > 0)
    {
        scene::ISceneNode *node = irr_driver->addSkyDome(m_sky_textures[0],
                                                         m_sky_hori_segments,
                                                         m_sky_vert_segments,
                                                         m_sky_texture_percent,
                                                         m_sky_sphere_percent);
        for(unsigned int i=0; i<node->getMaterialCount(); i++)
        {
            video::SMaterial &irrMaterial=node->getMaterial(i);
            for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
            {
                video::ITexture* t=irrMaterial.getTexture(j);
                if(!t) continue;
                core::matrix4 *m = &irrMaterial.getTextureMatrix(j);
                m_animated_textures.push_back(new MovingTexture(m, m_sky_dx, m_sky_dy));
            }   // for j<MATERIAL_MAX_TEXTURES
        }   // for i<getMaterialCount

        m_all_nodes.push_back(node);
    }
    else if(m_sky_type==SKY_BOX && m_sky_textures.size() == 6)
    {
        //if (m_spherical_harmonics_textures.size() > 0)
            m_all_nodes.push_back(irr_driver->addSkyBox(m_sky_textures, m_spherical_harmonics_textures));
        //else
        //    m_all_nodes.push_back(irr_driver->addSkyBox(m_sky_textures, m_sky_textures));
    }
    else if(m_sky_type==SKY_COLOR)
    {
        World::getWorld()->setClearbackBufferColor(m_sky_color);
    }

    if (!UserConfigParams::m_high_definition_textures)
    {
        file_manager->popTextureSearchPath();
    }
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath  ();

    // ---- Set ambient color
    m_ambient_color = m_default_ambient_color;
    irr_driver->getSceneManager()->setAmbientLight(m_ambient_color);

    // ---- Create sun (non-ambient directional light)
    if (m_sun_position.getLengthSQ() < 0.03f)
    {
        m_sun_position = core::vector3df(500, 250, 250);
    }

    const video::SColorf tmpf(m_sun_diffuse_color);
    m_sun = irr_driver->addLight(m_sun_position, 0., 0., tmpf.r, tmpf.g, tmpf.b, true);

    if (!irr_driver->isGLSL())
    {
        scene::ILightSceneNode *sun = (scene::ILightSceneNode *) m_sun;

        sun->setLightType(video::ELT_DIRECTIONAL);

        // The angle of the light is rather important - let the sun
        // point towards (0,0,0).
        if (m_sun_position.getLengthSQ() < 0.03f)
            // Backward compatibility: if no sun is specified, use the
            // old hardcoded default angle
            m_sun->setRotation( core::vector3df(180, 45, 45) );
        else
            m_sun->setRotation((-m_sun_position).getHorizontalAngle());

        sun->getLightData().SpecularColor = m_sun_specular_color;
    }


    createPhysicsModel(main_track_count);


    for (unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        const std::string &name = node->getName();
        if (name=="banana"      || name=="item"      ||
            name=="small-nitro" || name=="big-nitro" ||
            name=="easter-egg"                           )
        {
            itemCommand(node);
        }
    }   // for i<root->getNumNodes()

    delete root;

    if (UserConfigParams::m_track_debug &&
        race_manager->getMinorMode()!=RaceManager::MINOR_MODE_3_STRIKES &&
        !m_is_cutscene)
    {
        QuadGraph::get()->createDebugMesh();
    }

    // Only print warning if not in battle mode, since battle tracks don't have
    // any quads or check lines.
    if (CheckManager::get()->getCheckStructureCount()==0  &&
        race_manager->getMinorMode()!=RaceManager::MINOR_MODE_3_STRIKES && !m_is_cutscene)
    {
        Log::warn("track", "No check lines found in track '%s'.",
                  m_ident.c_str());
        Log::warn("track", "Lap counting will not work, and start "
                  "positions might be incorrect.");
    }

    if (UserConfigParams::logMemory())
    {
        Log::debug("track", "[memory] After loading  '%s': mesh cache %d "
                   "texture cache %d\n", getIdent().c_str(),
                irr_driver->getSceneManager()->getMeshCache()->getMeshCount(),
                irr_driver->getVideoDriver()->getTextureCount());
    }

    World *world = World::getWorld();
    if (world->useChecklineRequirements())
    {
        QuadGraph::get()->computeChecklineRequirements();
    }

    EasterEggHunt *easter_world = dynamic_cast<EasterEggHunt*>(world);
    if(easter_world)
    {
        std::string dir = StringUtils::getPath(m_filename);
        easter_world->readData(dir+"/easter_eggs.xml");
    }

    irr_driver->unsetTextureErrorMessage();
}   // loadTrackModel

//-----------------------------------------------------------------------------

void Track::loadObjects(const XMLNode* root, const std::string& path, ModelDefinitionLoader& model_def_loader,
                        bool create_lod_definitions, scene::ISceneNode* parent)
{
    unsigned int start_position_counter = 0;

    unsigned int node_count = root->getNumNodes();
    for (unsigned int i = 0; i < node_count; i++)
    {
        const XMLNode *node = root->getNode(i);
        const std::string name = node->getName();
        // The track object was already converted before the loop, and the
        // default start was already used, too - so ignore those.
        if (name == "track" || name == "default-start") continue;
        if (name == "object" || name == "library")
        {
            m_track_object_manager->add(*node, parent, model_def_loader);
        }
        else if (name == "water")
        {
            createWater(*node);
        }
        else if (name == "banana"      || name == "item" ||
                 name == "small-nitro" || name == "big-nitro" ||
                 name == "easter-egg"                           )
        {
            // will be handled later
        }
        else if (name == "start")
        {
            unsigned int position = start_position_counter;
            start_position_counter++;
            node->get("position", &position);
            Vec3 xyz(0,0,0);
            node->getXYZ(&xyz);
            float h=0;
            node->get("h", &h);

            if (position >= m_start_transforms.size())
            {
                m_start_transforms.resize(position + 1);
            }

            m_start_transforms[position].setOrigin(xyz);
            m_start_transforms[position].setRotation(
                                           btQuaternion(btVector3(0,1,0),
                                                        h*DEGREE_TO_RAD ) );
        }
        else if (name == "camera")
        {
            node->get("far", &m_camera_far);
        }
        else if (name == "checks")
        {
            CheckManager::get()->load(*node);
        }
        else if (name == "particle-emitter")
        {
            if (UserConfigParams::m_graphical_effects)
            {
                m_track_object_manager->add(*node, parent, model_def_loader);
            }
        }
        else if (name == "sky-dome" || name == "sky-box" || name == "sky-color")
        {
            handleSky(*node, path);
        }
        else if (name == "end-cameras")
        {
            Camera::readEndCamera(*node);
        }
        else if (name == "light")
        {
            m_track_object_manager->add(*node, parent, model_def_loader);
        }
        else if (name == "weather")
        {
            std::string weather_particles;

            node->get("particles", &weather_particles);
            node->get("lightning", &m_weather_lightning);
            node->get("sound", &m_weather_sound);

            if (weather_particles.size() > 0)
            {
                m_sky_particles =
                    ParticleKindManager::get()->getParticles(weather_particles);
            }
            else
            {
                Log::error("track", "Bad weather node found - ignored.\n");
                continue;
            }
        }
        else if (name == "sun")
        {
            // handled above
        }
        else if (name == "lod")
        {
            // handled above
        }
        else if (name == "lightshaft")
        {
            // handled above
        }
        else if (name == "instancing")
        {
            // TODO: eventually remove, this is now automatic
        }
        else if (name == "subtitles")
        {
            std::vector<XMLNode*> subtitles;
            node->getNodes("subtitle", subtitles);
            for (unsigned int i = 0; i < subtitles.size(); i++)
            {
                int from = -1, to = -1;
                std::string subtitle_text;
                subtitles[i]->get("from", &from);
                subtitles[i]->get("to", &to);
                subtitles[i]->get("text", &subtitle_text);
                if (from != -1 && to != -1 && subtitle_text.size() > 0)
                {
                    m_subtitles.push_back( Subtitle(from, to, _(subtitle_text.c_str())) );
                }
            }
        }
        else
        {
            Log::warn("track", "While loading track '%s', element '%s' was "
                      "met but is unknown.",
                      m_ident.c_str(), node->getName().c_str());
        }

    }   // for i<root->getNumNodes()
}

//-----------------------------------------------------------------------------
/** Changes all materials of the given mesh to use the current fog
 *  setting (true/false).
 *  \param node Scene node for which fog should be en/dis-abled.
 */
void Track::adjustForFog(scene::IMesh* mesh, scene::ISceneNode* parent_scene_node)
{
    unsigned int n = mesh->getMeshBufferCount();
    for (unsigned int i=0; i<n; i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        video::SMaterial &irr_material=mb->getMaterial();
        for (unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
        {
            video::ITexture* t = irr_material.getTexture(j);
            if (t) material_manager->adjustForFog(t, mb, parent_scene_node, m_use_fog);

        }   // for j<MATERIAL_MAX_TEXTURES
    }  // for i<getMeshBufferCount()
}

//-----------------------------------------------------------------------------
/** Changes all materials of the given scene node to use the current fog
 *  setting (true/false).
 *  \param node Scene node for which fog should be en/dis-abled.
 */
void Track::adjustForFog(scene::ISceneNode *node)
{
    //irr_driver->setAllMaterialFlags(scene::IMesh *mesh)


    if (node->getType() == scene::ESNT_OCTREE)
    {
        // do nothing
    }
    else if (node->getType() == scene::ESNT_MESH)
    {
        scene::IMeshSceneNode* mnode = (scene::IMeshSceneNode*)node;
        scene::IMesh* mesh = mnode->getMesh();
        adjustForFog(mesh, mnode);
    }
    else if (node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IAnimatedMeshSceneNode* mnode = (scene::IAnimatedMeshSceneNode*)node;
        scene::IMesh* mesh = mnode->getMesh();
        adjustForFog(mesh, mnode);
    }
    else
    {
        node->setMaterialFlag(video::EMF_FOG_ENABLE, m_use_fog);
    }

    if (node->getType() == scene::ESNT_LOD_NODE)
    {
        std::vector<scene::ISceneNode*>& subnodes = ((LODNode*)node)->getAllNodes();
        for (unsigned int n=0; n<subnodes.size(); n++)
        {
            adjustForFog(subnodes[n]);
            //subnodes[n]->setMaterialFlag(video::EMF_FOG_ENABLE, m_use_fog);
        }
    }
}   // adjustForFog

//-----------------------------------------------------------------------------
/** Handles a sky-dome or sky-box. It takes the xml node with the
 *  corresponding data for the sky and stores the corresponding data in
 *  the corresponding data structures.
 *  \param xml_node XML node with the sky data.
 *  \param filename Name of the file which is read, only used to print
 *         meaningful error messages.
 */
void Track::handleSky(const XMLNode &xml_node, const std::string &filename)
{
    if(xml_node.getName()=="sky-dome")
    {
        m_sky_type            = SKY_DOME;
        m_sky_vert_segments   = 16;
        m_sky_hori_segments   = 16;
        m_sky_sphere_percent  = 1.0f;
        m_sky_texture_percent = 1.0f;
        std::string s;
        xml_node.get("texture",          &s                   );
        video::ITexture *t = irr_driver->getTexture(s);
        if (t != NULL)
        {
            t->grab();
            m_sky_textures.push_back(t);
            xml_node.get("vertical",        &m_sky_vert_segments  );
            xml_node.get("horizontal",      &m_sky_hori_segments  );
            xml_node.get("sphere-percent",  &m_sky_sphere_percent );
            xml_node.get("texture-percent", &m_sky_texture_percent);
            xml_node.get("speed-x", &m_sky_dx );
            xml_node.get("speed-y", &m_sky_dy);
        }
        else
        {
            Log::error("track", "Sky-dome texture '%s' not found - ignored.",
                        s.c_str());
        }
    }   // if sky-dome
    else if(xml_node.getName()=="sky-box")
    {
        std::string s;
        xml_node.get("texture", &s);
        std::vector<std::string> v = StringUtils::split(s, ' ');
        for(unsigned int i=0; i<v.size(); i++)
        {
            video::ITexture *t = irr_driver->getTexture(v[i]);
            if(t)
            {
                t->grab();
                m_sky_textures.push_back(t);
            }
            else
            {
                Log::error("track","Sky-box texture '%s' not found - ignored.",
                           v[i].c_str());
            }
        }   // for i<v.size()
        if(m_sky_textures.size()!=6)
        {
            Log::error("track",
                       "A skybox needs 6 textures, but %d are specified",
                       (int)m_sky_textures.size());
            Log::error("track", "in '%s'.", filename.c_str());

        }
        else
        {
            m_sky_type = SKY_BOX;
        }

        std::string sh_textures;
        xml_node.get("sh-texture", &sh_textures);
        v = StringUtils::split(sh_textures, ' ');
        for (unsigned int i = 0; i<v.size(); i++)
        {
            video::ITexture *t = irr_driver->getTexture(v[i]);
            if (t)
            {
                t->grab();
                m_spherical_harmonics_textures.push_back(t);
            }
            else
            {
                Log::error("track", "Sky-box spherical harmonics texture '%s' not found - ignored.",
                    v[i].c_str());
            }
        }   // for i<v.size()
    }
    else if (xml_node.getName() == "sky-color")
    {
        m_sky_type = SKY_COLOR;
        xml_node.get("rgb", &m_sky_color);
    }   // if sky-box
}   // handleSky

//-----------------------------------------------------------------------------
/** Handle creation and placement of an item.
 *  \param xyz The position of the item.
 *  \param type The item type.
 *  \param drop True if the item Z position should be determined based on
 *         the track topology.
 */
void Track::itemCommand(const XMLNode *node)
{
    const std::string &name = node->getName();

    Item::ItemType type;
    if     (name=="banana"     ) type = Item::ITEM_BANANA;
    else if(name=="item"       ) type = Item::ITEM_BONUS_BOX;
    else if(name=="small-nitro") type = Item::ITEM_NITRO_SMALL;
    else if(name=="easter-egg" ) type = Item::ITEM_EASTER_EGG;
    else                         type = Item::ITEM_NITRO_BIG;
    Vec3 xyz;
    // Set some kind of default in case Y is not defined in the file
    // (with the new track exporter it always is defined anyway).
    // Y is the height from which the item is dropped on the track.
    xyz.setY(1000);
    node->getXYZ(&xyz);
    bool drop=true;
    node->get("drop", &drop);

    // Some modes (e.g. time trial) don't have any bonus boxes
    if(type==Item::ITEM_BONUS_BOX &&
       !World::getWorld()->haveBonusBoxes())
        return;

    // Only do easter eggs in easter egg mode.
    if(type==Item::ITEM_EASTER_EGG &&
        !(race_manager->getMinorMode()==RaceManager::MINOR_MODE_EASTER_EGG))
    {
        Log::warn("track",
                  "Found easter egg in non-easter-egg mode - ignored.\n");
        return;
    }

    Vec3 loc(xyz);
    // if only 2d coordinates are given, let the item fall from very high
    if(drop)
    {
        // If raycast is used, increase the start position slightly
        // in case that the point is too close to the actual surface
        // (e.g. floating point errors can cause a problem here).
        loc += Vec3(0,0.1f,0);
#ifndef DEBUG
        // Avoid unused variable warning in case of non-debug compilation.
        setTerrainHeight(&loc);
#else
        bool drop_success = setTerrainHeight(&loc);
        if(!drop_success)
        {
            Log::warn("track",
                      "Item at position (%f,%f,%f) can not be dropped",
                      loc.getX(), loc.getY(), loc.getZ());
            Log::warn("track", "onto terrain - position unchanged.");
        }
#endif
    }

    // Don't tilt the items, since otherwise the rotation will look odd,
    // i.e. the items will not rotate around the normal, but 'wobble'
    // around.
    //Vec3 normal(0.7071f, 0, 0.7071f);
    Vec3 normal(0, 1, 0);
    ItemManager::get()->newItem(type, loc, normal);
}   // itemCommand

// ----------------------------------------------------------------------------
/** Does a raycast from the given position, and if terrain was found
 *  adjust the Y position of the given vector to the actual terrain
 *  height. If no terrain is found, false is returned and the
 *  y position is not modified.
 *  \param pos Pointer to the position at which to determine the
 *         height. If terrain is found, its Y position will be
 *         set to the actual height.
 *  \return True if terrain was found and the height was adjusted.
 */
bool Track::setTerrainHeight(Vec3 *pos) const
{
    Vec3  hit_point;
    Vec3  normal;
    const Material *m;
    Vec3 to=*pos+Vec3(0,-10000,0);
    if(m_track_mesh->castRay(*pos, to, &hit_point, &m, &normal))
    {
        pos->setY(hit_point.getY());
        return true;
    }
    return false;
}   // setTerrainHeight

// ----------------------------------------------------------------------------

std::vector< std::vector<float> > Track::buildHeightMap()
{
    std::vector< std::vector<float> > out(HEIGHT_MAP_RESOLUTION);

    float x = m_aabb_min.getX();
    const float x_len = m_aabb_max.getX() - m_aabb_min.getX();
    const float z_len = m_aabb_max.getZ() - m_aabb_min.getZ();

    const float x_step = x_len/HEIGHT_MAP_RESOLUTION;
    const float z_step = z_len/HEIGHT_MAP_RESOLUTION;

    btVector3 hitpoint;
    const Material* material;
    btVector3 normal;

    for (int i=0; i<HEIGHT_MAP_RESOLUTION; i++)
    {
        out[i].resize(HEIGHT_MAP_RESOLUTION);
        float z = m_aabb_min.getZ();

        for (int j=0; j<HEIGHT_MAP_RESOLUTION; j++)
        {
            btVector3 pos(x, 100.0f, z);
            btVector3 to = pos;
            to.setY(-100000.f);

            m_track_mesh->castRay(pos, to, &hitpoint, &material, &normal);
            z += z_step;

            out[i][j] = hitpoint.getY();
        }   // j<HEIGHT_MAP_RESOLUTION
        x += x_step;
    }

    return out;
}   // buildHeightMap

// ----------------------------------------------------------------------------
/** Returns the rotation of the sun. */
const core::vector3df& Track::getSunRotation()
{
    return m_sun->getRotation();
}
//-----------------------------------------------------------------------------
/** Determines if the kart is over ground.
 *  Used in setting the starting positions of all the karts.
 *  \param k The kart to project downward.
 *  \return True of the kart is on terrain.
 */

bool Track::findGround(AbstractKart *kart)
{
    btVector3 to(kart->getXYZ());
    to.setY(-100000.f);

    // Material and hit point are not needed;
    const Material *m;
    Vec3 hit_point, normal;
    bool over_ground = m_track_mesh->castRay(kart->getXYZ(), to, &hit_point,
                                             &m, &normal);
    const Vec3 &xyz = kart->getXYZ();
    if(!over_ground)
    {
        Log::warn("physics", "Kart at (%f %f %f) can not be dropped.",
                  xyz.getX(),xyz.getY(),xyz.getZ());
        return false;
    }

    // Check if the material the kart is about to be placed on would trigger
    // a reset. If so, this is not a valid position.
    if(m && m->isDriveReset())
    {
        Log::warn("physics","Kart at (%f %f %f) over reset terrain '%s'",
                   xyz.getX(),xyz.getY(),xyz.getZ(),
                   m->getTexFname().c_str());
        return false;
    }

    // See if the kart is too high above the ground - it would drop
    // too long.
    if(xyz.getY() - hit_point.getY() > 5)
    {
        Log::warn("physics",
                  "Kart at (%f %f %f) is too high above ground at (%f %f %f)",
                  xyz.getX(),xyz.getY(),xyz.getZ(),
                  hit_point.getX(),hit_point.getY(),hit_point.getZ());
        return false;
    }


    btTransform t = kart->getBody()->getCenterOfMassTransform();
    // The computer offset is slightly too large, it should take
    // the default suspension rest insteat of suspension rest (i.e. the
    // length of the suspension with the weight of the kart resting on
    // it). On the other hand this initial bouncing looks nice imho
    // - so I'll leave it in for now.
    float offset = kart->getKartProperties()->getSuspensionRest() +
                   kart->getKartProperties()->getWheelRadius();
    t.setOrigin(hit_point+Vec3(0, offset, 0) );
    kart->getBody()->setCenterOfMassTransform(t);
    kart->setTrans(t);

    return true;
}   // findGround

