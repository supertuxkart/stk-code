//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//                2009 Joerg Henrichs, Steve Baker
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

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <IBillboardTextSceneNode.h>

using namespace irr;

#include "audio/music_manager.hpp"
#include "challenges/challenge.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/CBatchingMesh.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/particle_emitter.hpp"
#include "graphics/particle_kind.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "physics/physical_object.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "tracks/bezier_curve.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/lod_node_loader.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/quad_set.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ISceneManager.h>
#include <IMeshSceneNode.h>
#include <IMeshManipulator.h>
#include <ILightSceneNode.h>
#include <IMeshCache.h>

const float Track::NOHIT           = -99999.9f;

// ----------------------------------------------------------------------------
Track::Track(const std::string &filename)
{
#ifdef DEBUG
    m_magic_number          = 0x17AC3802;
#endif
    
    m_materials_loaded      = false;
    m_filename              = filename;
    m_root                  = StringUtils::getPath(StringUtils::removeExtension(m_filename));
    m_ident                 = StringUtils::getBasename(m_root);
    m_designer              = "";
    m_screenshot            = "";
    m_version               = 0;
    m_track_mesh            = NULL;
    m_gfx_effect_mesh       = NULL;
    m_internal              = false;
    m_reverse_available     = true;
    m_all_nodes.clear();
    m_all_cached_meshes.clear();
    m_is_arena              = false;
    m_is_cutscene           = false;
    m_camera_far            = 1000.0f;
    m_mini_map              = NULL;
    m_sky_particles         = NULL;
    m_sky_dx                = 0.05f;
    m_sky_dy                = 0.0f;
    m_weather_type          = WEATHER_NONE;
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
}   // reset

//-----------------------------------------------------------------------------
/** Removes the physical body from the world.
 *  Called at the end of a race.
 */
void Track::cleanup()
{
    QuadGraph::destroy();
    ItemManager::destroy();

    ParticleKindManager::get()->cleanUpTrackSpecificGfx();
    
    for(unsigned int i=0; i<m_animated_textures.size(); i++)
    {
        delete m_animated_textures[i];
    }
    m_animated_textures.clear();

    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        irr_driver->removeNode(m_all_nodes[i]);
    }
    m_all_nodes.clear();
    
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
    for(unsigned int i=0; i<m_all_cached_meshes.size(); i++)
    {
        irr_driver->dropAllTextures(m_all_cached_meshes[i]);
        // If a mesh is not in Irrlicht's texture cache, its refcount is 
        // 1 (since its scene node was removed, so the only other reference 
        // is in m_all_cached_meshes). In this case we only drop it once
        // and don't try to remove it from the cache.
        if(m_all_cached_meshes[i]->getReferenceCount()==1)
        {
            m_all_cached_meshes[i]->drop();
            continue;
        }
        m_all_cached_meshes[i]->drop();
        if(m_all_cached_meshes[i]->getReferenceCount()==1)
            irr_driver->removeMeshFromCache(m_all_cached_meshes[i]);
    }
    m_all_cached_meshes.clear();

    // Now free meshes that are not associated to any scene node.
    for (unsigned int i=0; i<m_detached_cached_meshes.size(); i++)
    {
        irr_driver->dropAllTextures(m_detached_cached_meshes[i]);
        irr_driver->removeMeshFromCache(m_detached_cached_meshes[i]);
    }
    m_detached_cached_meshes.clear();
    
    if(m_mini_map)      
    {
        assert(m_mini_map->getReferenceCount()==1);
        irr_driver->removeTexture(m_mini_map);
        m_mini_map = NULL;
    }

    for(unsigned int i=0; i<m_sky_textures.size(); i++)
    {
        m_sky_textures[i]->drop();
        if(m_sky_textures[i]->getReferenceCount()==1)
            irr_driver->removeTexture(m_sky_textures[i]);
    }
    m_sky_textures.clear();

    if(m_ident!="overworld")
    {
        // remove temporary materials loaded by the material manager
        material_manager->popTempMaterial();
    }
    else
        material_manager->makeMaterialsPermanent();

    if(UserConfigParams::logMemory())
    {
        printf("[memory] After cleaning '%s': mesh cache %d texture cache %d\n",
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
                printf("[memory] Leaked mesh buffer '%s'.\n", 
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
                printf("[memory] Leaked texture '%s'.\n", 
                    t->getName().getInternalName().c_str());
            }
        }
#endif
    }   // if verbose

}   // cleanup

//-----------------------------------------------------------------------------
void Track::loadTrackInfo()
{
    // Default values
    m_use_fog               = false;
    m_fog_density           = 1.0f/100.0f;
    m_fog_start             = 0.0f;
    m_fog_end               = 1000.0f;
    m_gravity               = 9.80665f;
    m_smooth_normals        = false;
                              /* ARGB */
    m_fog_color             = video::SColor(255, 77, 179, 230);
    m_default_ambient_color = video::SColor(255, 120, 120, 120);
    m_sun_specular_color    = video::SColor(255, 255, 255, 255);
    m_sun_diffuse_color     = video::SColor(255, 255, 255, 255); 
    m_sun_position          = core::vector3df(0, 0, 0);
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
    root->get("arena",                 &m_is_arena);
    root->get("cutscene",              &m_is_cutscene);
    root->get("groups",                &m_groups);
    root->get("internal",              &m_internal);
    root->get("reverse",               &m_reverse_available);
    root->get("smooth-normals",        &m_smooth_normals);
    // Reverse is meaningless in arena
    m_reverse_available = !m_is_arena && m_reverse_available;

    
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
    m_screenshot = m_root+"/"+m_screenshot;
    delete root;

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
        std::string full_path = m_root+"/"+filenames[i];
        MusicInformation* mi = music_manager->getMusicInformation(full_path);
        if(!mi)
        {
            try
            {
                std::string shared_name = file_manager->getMusicFile(filenames[i]);
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
            fprintf(stderr,
                    "Music information file '%s' not found - ignored.\n",
                    filenames[i].c_str());

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
    QuadGraph::create(m_root+"/"+m_all_modes[mode_id].m_quad_name,
                      m_root+"/"+m_all_modes[mode_id].m_graph_name,
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
        fprintf(stderr, 
                "[Track] WARNING: No graph nodes defined for track '%s'\n",
                m_filename.c_str());
        if (race_manager->getNumberOfKarts() > 1)
        {
            fprintf(stderr, 
                "[Track] FATAL: I can handle the lack of driveline in single"
                "kart mode, but not with AIs\n");
            exit(-1);
        }
    }
    else
    {
        //Check whether the hardware can do nonsquare or 
        // non power-of-two textures
        video::IVideoDriver* const video_driver = irr_driver->getVideoDriver();
        bool nonpower = video_driver->queryFeature(video::EVDF_TEXTURE_NPOT);
        bool nonsquare = 
            video_driver->queryFeature(video::EVDF_TEXTURE_NSQUARE);

        //Create the minimap resizing it as necessary.
        core::dimension2du size = World::getWorld()->getRaceGUI()
                                 ->getMiniMapSize()
                                 .getOptimalSize(!nonpower,!nonsquare);
        m_mini_map = QuadGraph::get()->makeMiniMap(size, "minimap::"+m_ident);

    }
}   // loadQuadGraph

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
        fprintf(stderr, "ERROR: m_track_mesh == NULL, cannot createPhysicsModel\n");
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
            fprintf(stderr, "[Track] WARNING: this track contains an empty LOD group : '%s'\n",
                    ((LODNode*)node)->getGroupName().c_str());
            return;
        }
    }
    
    const core::vector3df &pos   = node->getPosition();
    const core::vector3df &hpr   = node->getRotation();
    const core::vector3df &scale = node->getScale();

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
            printf("[Track::convertTrackToBullet] Unknown scene node type : %c%c%c%c.\n",
                   type[0], type[1], type[2], type[3]);
            return;
    }   // switch node->getType()

    core::matrix4 mat;
    mat.setRotationDegrees(hpr);
    mat.setTranslation(pos);
    core::matrix4 mat_scale;
    // Note that we can't simply call mat.setScale, since this would
    // overwrite the elements on the diagonal, making any rotation incorrect.
    mat_scale.setScale(scale);
    mat *= mat_scale;
    for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        // FIXME: take translation/rotation into account
        if (mb->getVertexType() != video::EVT_STANDARD &&
            mb->getVertexType() != video::EVT_2TCOORDS &&
            mb->getVertexType() != video::EVT_TANGENTS)
        {
            fprintf(stderr, "WARNING: Tracl::convertTrackToBullet: Ignoring type '%d'!\n", 
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
            for(unsigned int j=0; j<mb->getIndexCount(); j+=3)
            {
                for(unsigned int k=0; k<3; k++)
                {
                    int indx=mbIndices[j+k];
                    core::vector3df v = mbVertices[indx].Pos;
                    mat.transformVect(v);
                    vertices[k]=v;
                    normals[k]=mbVertices[indx].Normal;
                }   // for k
                if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                             vertices[2], normals[0],
                                             normals[1],  normals[2],
                                             material                 );
            }   // for j
        }
        else if (mb->getVertexType() == video::EVT_2TCOORDS)
        {
            irr::video::S3DVertex2TCoords* mbVertices = (video::S3DVertex2TCoords*)mb->getVertices();
            for(unsigned int j=0; j<mb->getIndexCount(); j+=3)
            {
                for(unsigned int k=0; k<3; k++)
                {
                    int indx=mbIndices[j+k];
                    core::vector3df v = mbVertices[indx].Pos;
                    mat.transformVect(v);
                    vertices[k]=v;
                    normals[k]=mbVertices[indx].Normal;
                }   // for k
                if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                             vertices[2], normals[0],
                                             normals[1],  normals[2],
                                             material                 );
            }   // for j
        }
        else if (mb->getVertexType() == video::EVT_TANGENTS)
        {
            irr::video::S3DVertexTangents* mbVertices = (video::S3DVertexTangents*)mb->getVertices();
            for(unsigned int j=0; j<mb->getIndexCount(); j+=3)
            {
                for(unsigned int k=0; k<3; k++)
                {
                    int indx=mbIndices[j+k];
                    core::vector3df v = mbVertices[indx].Pos;
                    mat.transformVect(v);
                    vertices[k]=v;
                    normals[k]=mbVertices[indx].Normal;
                }   // for k
                if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                             vertices[2], normals[0],
                                             normals[1],  normals[2],
                                             material                 );
            }   // for j
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
    
    const XMLNode *track_node= root.getNode("track");
    std::string model_name;
    track_node->get("model", &model_name);
    std::string full_path = m_root+"/"+model_name;
    scene::IMesh *mesh = irr_driver->getMesh(full_path);
    if(!mesh)
    {
        fprintf(stderr, "Warning: Main track model '%s' in '%s' not found, aborting.\n",
                track_node->getName().c_str(), model_name.c_str());
        exit(-1);
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

    adjustForFog(merged_mesh, NULL);
    
    // The merged mesh is grabbed by the octtree, so we don't need
    // to keep a reference to it.
    //scene::ISceneNode *scene_node = irr_driver->addMesh(merged_mesh);
    scene::IMeshSceneNode *scene_node = irr_driver->addOctTree(merged_mesh);
    // We should drop the merged mesh (since it's now referred to in the
    // scene node), but then we need to grab it since it's in the 
    // m_all_cached_meshes.
    m_all_cached_meshes.push_back(merged_mesh);
    irr_driver->grabAllTextures(merged_mesh);

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

    LodNodeLoader lodLoader;

    for(unsigned int i=0; i<track_node->getNumNodes(); i++)
    {
        const XMLNode *n=track_node->getNode(i);
        // Animated textures have already been handled
        if(n->getName()=="animated-texture") continue;
        // Only "object" entries are allowed now inside of the model tag
        if(n->getName()!="static-object")
        {
            fprintf(stderr, "Incorrect tag '%s' inside <model> of scene file - ignored\n",
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
            
            const ChallengeData* challenge = unlock_manager->getChallenge(m_challenges[closest_challenge_id].m_challenge_id);
            if (challenge == NULL)
            {
                fprintf(stderr, "[Track] WARNING: Cannot find challenge named '%s'\n",
                        m_challenges[closest_challenge_id].m_challenge_id.c_str());
                continue;
            }
            
            const int val = challenge->getNumTrophies();
            bool shown = (unlock_manager->getCurrentSlot()->getPoints() < val);
            m_force_fields.push_back(OverworldForceField(xyz, shown, val));
            
            m_challenges[closest_challenge_id].setForceField(m_force_fields[m_force_fields.size() - 1]);
            
            core::stringw msg = StringUtils::toWString(val);
            core::dimension2d<u32> textsize = GUIEngine::getHighresDigitFont()->getDimension(msg.c_str());
            scene::ISceneManager* sm = irr_driver->getSceneManager();
                            
            assert(GUIEngine::getHighresDigitFont() != NULL);
            
            scene::ISceneNode* sn =
                sm->addBillboardTextSceneNode(GUIEngine::getHighresDigitFont(),
                                              msg.c_str(),
                                              NULL,
                                              core::dimension2df(textsize.Width/45.0f,
                                                                 textsize.Height/45.0f),
                                              xyz,
                                              -1 /* id */,
                                              video::SColor(255, 255, 225, 0),
                                              video::SColor(255, 255, 89, 0));
            m_all_nodes.push_back(sn);
            if (!shown) continue;
        }
        else if (condition.size() > 0)
        {
            fprintf(stderr, "[Track] WARNING: unknown condition <%s>\n", condition.c_str());
        }
        
        std::string neg_condition;
        n->get("ifnot", &neg_condition);
        if (neg_condition == "splatting")
        {
            if (irr_driver->supportsSplatting()) continue;
        }
        else if (neg_condition.size() > 0)
        {
            fprintf(stderr, "[Track] WARNING: unknown condition <%s>\n", neg_condition.c_str());
        }
        
        bool tangent = false;
        n->get("tangents", &tangent);
        
        scene::ISceneNode* scene_node;
        model_name="";
        n->get("model", &model_name);
        full_path = m_root+"/"+model_name;
        
        // a special challenge orb object for overworld
        std::string challenge;
        n->get("challenge", &challenge);
        
        bool is_lod = lodLoader.check(n);
        
        if (tangent)
        {
            scene::IMeshManipulator* manip = irr_driver->getVideoDriver()->getMeshManipulator();
            
            scene::IMesh* original_mesh = irr_driver->getMesh(full_path);
            
            if (std::find(m_detached_cached_meshes.begin(),
                          m_detached_cached_meshes.end(),
                          original_mesh) == m_detached_cached_meshes.end())
            {
                m_detached_cached_meshes.push_back(original_mesh);
            }
            
            // create a node out of this mesh just for bullet; delete it after, normal maps are special
            // and require tangent meshes
            scene_node = irr_driver->addMesh(original_mesh);
            
            scene_node->setPosition(xyz);
            scene_node->setRotation(hpr);
            scene_node->setScale(scale);

            convertTrackToBullet(scene_node);
            scene_node->remove();
            irr_driver->grabAllTextures(original_mesh);

            scene::IMesh* mesh = manip->createMeshWithTangents(original_mesh);
            mesh->grab();
            irr_driver->grabAllTextures(mesh);

            m_all_cached_meshes.push_back(mesh);
            scene_node = irr_driver->addMesh(mesh);
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
        else if (is_lod)
        {
            // nothing to do
        }
        else
        {
            // TODO: check if mesh is animated or not
            scene::IMesh *a_mesh = irr_driver->getMesh(full_path);
            if(!a_mesh)
            {
                fprintf(stderr, "Warning: object model '%s' not found, ignored.\n",
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
            scene_node = irr_driver->addMesh(a_mesh);
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
                const ChallengeData* c = unlock_manager->getChallenge(challenge);
                if (c == NULL)
                {
                    fprintf(stderr, "[WARNING] Cannot find challenge named <%s>\n", challenge.c_str());
                    scene_node->remove();
                    continue;
                }
                
                m_challenges.push_back( OverworldChallenge(xyz, challenge) );
                
                if (c->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
                {
                    
                }
                else
                {
                    Track* t = track_manager->getTrack(c->getTrackId());
                    if (t == NULL)
                    {
                        fprintf(stderr, "[WARNING] Cannot find track named <%s>\n", c->getTrackId().c_str());
                        continue;
                    }
                    
                    std::string sshot = t->getScreenshotFile();
                    video::ITexture* screenshot = irr_driver->getTexture(sshot);
                    
                    if (screenshot == NULL)
                    {
                        fprintf(stderr, "[WARNING] Cannot find track screenshot <%s>\n", sshot.c_str());
                        continue;
                    }
                    scene_node->getMaterial(0).setTexture(0, screenshot);
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
                lod_node->add(50, scene_node, true /* reparent */);
                                
                m_all_nodes.push_back( lod_node );
            }
            else
            {
                m_all_nodes.push_back( scene_node );
            }
        }

    }   // for i
    
    // Create LOD nodes
    std::vector<LODNode*> lod_nodes;
    lodLoader.done(this, m_root, m_all_cached_meshes, lod_nodes);
    for (unsigned int n=0; n<lod_nodes.size(); n++)
    {
        // FIXME: support for animated textures on LOD objects
        // handleAnimatedTextures( lod_nodes[n], *node );
        m_all_nodes.push_back( lod_nodes[n] );
    }
    
    // This will (at this stage) only convert the main track model.
    for(unsigned int i=0; i<m_all_nodes.size(); i++)
    {
        convertTrackToBullet(m_all_nodes[i]);
    }
    if (m_track_mesh == NULL)
    {
        fprintf(stderr, "ERROR: m_track_mesh == NULL, cannot loadMainTrack\n");
        exit(-1);
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
            fprintf(stderr, 
                    "Animated texture: no texture name specified for track '%s'\n",
                    m_ident.c_str());
            continue;
        }

        for(unsigned int i=0; i<node->getMaterialCount(); i++)
        {
            video::SMaterial &irrMaterial=node->getMaterial(i);
            for(unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
            {
                video::ITexture* t=irrMaterial.getTexture(j);
                if(!t) continue;
                const std::string texture_name = 
                    StringUtils::getBasename(core::stringc(t->getName()).c_str());
                if(texture_name!=name) continue;
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
    m_track_object_manager->update(dt);

    for(unsigned int i=0; i<m_animated_textures.size(); i++)
    {
        m_animated_textures[i]->update(dt);
    }
    CheckManager::get()->update(dt);
    ItemManager::get()->update(dt);

}   // update

// ----------------------------------------------------------------------------
/** Handles an explosion, i.e. it makes sure that all physical objects are
 *  affected accordingly.
 *  \param pos  Position of the explosion.
 *  \param obj  If the hit was a physical object, this object will be affected
 *              more. Otherwise this is NULL.
 */
void Track::handleExplosion(const Vec3 &pos, const PhysicalObject *obj) const
{
    m_track_object_manager->handleExplosion(pos, obj);
}   // handleExplosion

// ----------------------------------------------------------------------------
/** Creates a water node.
 *  \param node The XML node containing the specifications for the water node.
 */
void Track::createWater(const XMLNode &node)
{
    std::string model_name;
    node.get("model", &model_name);
    std::string full_path = m_root+"/"+model_name;

    scene::IMesh *mesh = irr_driver->getMesh(full_path);
    if (mesh == NULL) return;
    
    float wave_height  = 2.0f;
    float wave_speed   = 300.0f;
    float wave_length  = 10.0f;
    node.get("height", &wave_height);
    node.get("speed",  &wave_speed);
    node.get("length", &wave_length);
    scene::ISceneNode* scene_node = NULL;
    
    if (UserConfigParams::m_graphical_effects)
    {
        scene_node = irr_driver->addWaterNode(mesh,
                                              wave_height, 
                                              wave_speed,
                                              wave_length);
        
        // 'addWaterNode' welds the mesh so keep both the original and the welded copy
        mesh->grab();
        irr_driver->grabAllTextures(mesh);
        m_all_cached_meshes.push_back(mesh);
        
        mesh = ((scene::IMeshSceneNode*)scene_node)->getMesh();
    }
    else
    {
        scene_node = irr_driver->addMesh(mesh);
    }
    
    if(!mesh || !scene_node)
    {
        fprintf(stderr, "Warning: Water model '%s' in '%s' not found, ignored.\n",
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
 * is empty.
 *  \param mode_id Which of the modes of a track to use. This determines which
 *         scene, quad, and graph file to load.
 */

void Track::loadTrackModel(World* parent, bool reverse_track, 
			   unsigned int mode_id               )
{
    if(!m_reverse_available) 
    {
        reverse_track = false;
    }
    CheckManager::create();
    assert(m_all_cached_meshes.size()==0);
    if(UserConfigParams::logMemory())
    {
        printf("[memory] Before loading '%s': mesh cache %d texture cache %d\n",
            getIdent().c_str(),
            irr_driver->getSceneManager()->getMeshCache()->getMeshCount(),
            irr_driver->getVideoDriver()->getTextureCount());
#ifdef DEBUG
        scene::IMeshCache *cache = irr_driver->getSceneManager()->getMeshCache();
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
    // First read the temporary materials.dat file if it exists
    try
    {
        std::string materials_file = m_root+"/materials.xml";
        if(m_ident=="overworld")
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
        // no temporary materials.dat file, ignore
        (void)e;
    }

    // Start building the scene graph
    std::string path = m_root+"/"+m_all_modes[mode_id].m_scene;
    XMLNode *root    = file_manager->createXMLTree(path);

    // Make sure that we have a track (which is used for raycasts to 
    // place other objects).
    if(!root || root->getName()!="scene")
    {
        std::ostringstream msg;
        msg<< "No track model defined in '"<<path
           <<"', aborting.";
        throw std::runtime_error(msg.str());
    }

    // Load the graph only now: this function is called from world, after
    // the race gui was created. The race gui is needed since it stores
    // the information about the size of the texture to render the mini
    // map to.
    if (!m_is_arena && !m_is_cutscene) loadQuadGraph(mode_id, reverse_track);

    ItemManager::create();

    // Set the default start positions. Node that later the default
    // positions can still be overwritten.
    float forwards_distance  = 1.5f;
    float sidewards_distance = 3.0f;
    float upwards_distance   = 0.1f;
    int   karts_per_row      = 2;

    const XMLNode *default_start=root->getNode("default-start");
    if(default_start)
    {
        default_start->get("forwards-distance",  &forwards_distance );
        default_start->get("sidewards-distance", &sidewards_distance);
        default_start->get("upwards-distance",   &upwards_distance  );
        default_start->get("karts-per-row",      &karts_per_row     );
    }
    if(!m_is_arena && !m_is_cutscene)
    {
        m_start_transforms.resize(race_manager->getNumberOfKarts());
        QuadGraph::get()->setDefaultStartPositions(&m_start_transforms,
                                                   karts_per_row,
                                                   forwards_distance,
                                                   sidewards_distance,
                                                   upwards_distance);
    }
    
    unsigned int main_track_count = m_all_nodes.size();
    unsigned int start_position_counter = 0;

    // we need to check for fog before loading the main track model
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        const std::string name = node->getName();
        
        if(name=="sun")
        {
            node->get("xyz",           &m_sun_position );
            node->get("ambient",       &m_default_ambient_color);
            node->get("sun-specular",  &m_sun_specular_color);
            node->get("sun-diffuse",   &m_sun_diffuse_color);
            node->get("fog",           &m_use_fog);
            node->get("fog-color",     &m_fog_color);
            node->get("fog-density",   &m_fog_density);
            node->get("fog-start",     &m_fog_start);
            node->get("fog-end",       &m_fog_end);
        }
    }
    
    loadMainTrack(*root);
    
    LodNodeLoader lod_loader;
    
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        const std::string name = node->getName();
        // The track object was already converted before the loop, and the
        // default start was already used, too - so ignore those.
        if(name=="track" || name=="default-start") continue;
        if(name=="object")
        {
            lod_loader.check(node);
            m_track_object_manager->add(*node);
        }
        else if(name=="water")
        {
            createWater(*node);
        }
        else if(name=="banana"      || name=="item" || 
                name=="small-nitro" || name=="big-nitro")
        {
            // will be handled later
        }
        else if (name=="start")
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
        else if(name=="camera")
        {
            node->get("far", &m_camera_far);
        }
        else if(name=="checks")
        {
            CheckManager::get()->load(*node);
        }
        else if (name=="particle-emitter")
        {
            if (UserConfigParams::m_graphical_effects)
            {
                m_track_object_manager->add(*node);
            }
        }
        else if(name=="sky-dome" || name=="sky-box" || name=="sky-color")
        {
            handleSky(*node, path);
        }
        else if(name=="end-cameras")
        {
            Camera::readEndCamera(*node);
        }
        else if(name=="weather")
        {
            std::string weather_particles;
            std::string weather_type;
            node->get("particles", &weather_particles);
            node->get("type", &weather_type);

            if (weather_particles.size() > 0)
            {
                m_sky_particles = 
                    ParticleKindManager::get()->getParticles(weather_particles);
            }
            else if (weather_type.size() > 0)
            {
                if (weather_type == "rain")
                {
                    m_weather_type = WEATHER_RAIN;
                }
                else
                {
                    fprintf(stderr, "[Track] ERROR: Unknown weather type : '%s'\n", weather_type.c_str());
                }
            }
            else
            {
                fprintf(stderr, 
                        "[Track] ERROR: Warning: bad weather node found - ignored.\n");
                continue;
            }
        }
        else if (name == "sun")
        {
            // handled above
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
            fprintf(stderr, "Warning: while loading track '%s', element '%s' was met but is unknown.\n",
                    m_ident.c_str(), node->getName().c_str());
        }

    }   // for i<root->getNumNodes()
    
    // -------- Create and assign LOD nodes --------
    // recheck the static area, we will need LOD info
    const XMLNode* track_node = root->getNode("track");
    for(unsigned int i=0; i<track_node->getNumNodes(); i++)
    {
        const XMLNode* n = track_node->getNode(i);
        bool is_instance = false;
        n->get("lod_instance", &is_instance);
        
        if (!is_instance) lod_loader.check(n);
    }
    
    std::vector<LODNode*> lod_nodes;
    std::vector<scene::IMesh*> devnull;
    lod_loader.done(this, m_root, devnull, lod_nodes);
    
    m_track_object_manager->assingLodNodes(lod_nodes);
    // ---------------------------------------------
    
    // Init all track objects
    m_track_object_manager->init();

    
    // ---- Fog
    // It's important to execute this BEFORE the code that creates the skycube,
    // otherwise the skycube node could be modified to have fog enabled, which
    // we don't want
    if (m_use_fog && !UserConfigParams::m_camera_debug)
    {
        /* NOTE: if LINEAR type, density does not matter, if EXP or EXP2, start
           and end do not matter */
        irr_driver->getVideoDriver()->setFog(m_fog_color, 
                                             video::EFT_FOG_LINEAR, 
                                             m_fog_start, m_fog_end,
                                             m_fog_density);
    }
    
    // Enable for for all track nodes if fog is used
    //if(m_use_fog)
    //{
        const unsigned int count = m_all_nodes.size();
        for(unsigned int i=0; i<count; i++)
        {
            adjustForFog(m_all_nodes[i]);
        }
    //}
    m_track_object_manager->enableFog(m_use_fog);
    
    // Sky dome and boxes support
    // --------------------------
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
        m_all_nodes.push_back(irr_driver->addSkyBox(m_sky_textures));
    }
    else if(m_sky_type==SKY_COLOR)
    {
        parent->setClearBackBuffer(true);
        parent->setClearbackBufferColor(m_sky_color);
    }

    
    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath  ();

    // ---- Set ambient color
    m_ambient_color = m_default_ambient_color;
    irr_driver->getSceneManager()->setAmbientLight(m_ambient_color);
    
    // ---- Create sun (non-ambient directional light)
    m_sun = irr_driver->getSceneManager()->addLightSceneNode(NULL, 
                                                             m_sun_position,
                                                             m_sun_diffuse_color);
    m_sun->setLightType(video::ELT_DIRECTIONAL);

    // The angle of the light is rather important - let the sun
    // point towards (0,0,0).
    if(m_sun_position.getLengthSQ()==0)
        // Backward compatibility: if no sun is specified, use the 
        // old hardcoded default angle
        m_sun->setRotation( core::vector3df(180, 45, 45) );
    else
        m_sun->setRotation((-m_sun_position).getHorizontalAngle());

    m_sun->getLightData().SpecularColor = m_sun_specular_color;


    createPhysicsModel(main_track_count);
    
    
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        const std::string name = node->getName();
        if (name=="banana"      || name=="item" || 
            name=="small-nitro" || name=="big-nitro")
        {
            Item::ItemType type;
            if     (name=="banana"     ) type = Item::ITEM_BANANA;
            else if(name=="item"       ) type = Item::ITEM_BONUS_BOX;
            else if(name=="small-nitro") type = Item::ITEM_NITRO_SMALL;
            else                         type = Item::ITEM_NITRO_BIG;
            Vec3 xyz;
            // Set some kind of default in case Z is not defined in the file
            // (with the new track exporter it always is defined anyway).
            // Z is the height from which the item is dropped on the track.
            xyz.setY(1000);
            node->getXYZ(&xyz);
            bool drop=true;
            node->get("drop", &drop);
            // Height is needed if bit 2 (for z) is not set
            itemCommand(xyz, type, drop);
        }
    }   // for i<root->getNumNodes()

    delete root;

    if (UserConfigParams::m_track_debug &&
        race_manager->getMinorMode()!=RaceManager::MINOR_MODE_3_STRIKES && 
        !m_is_cutscene) 
        QuadGraph::get()->createDebugMesh();
    
    // Only print warning if not in battle mode, since battle tracks don't have
    // any quads or check lines.
    if(CheckManager::get()->getCheckStructureCount()==0  &&
        race_manager->getMinorMode()!=RaceManager::MINOR_MODE_3_STRIKES && !m_is_cutscene)
    {
        printf("WARNING: no check lines found in track '%s'.\n", 
               m_ident.c_str());
        printf("Lap counting will not work, and start positions might be incorrect.\n");
    }

    if(UserConfigParams::logMemory())
        printf("[memory] After loading  '%s': mesh cache %d texture cache %d\n",
                getIdent().c_str(),
                irr_driver->getSceneManager()->getMeshCache()->getMeshCount(),
                irr_driver->getVideoDriver()->getTextureCount());

    if (World::getWorld()->useChecklineRequirements())
    {
        QuadGraph::get()->computeChecklineRequirements();
    }
}   // loadTrackModel

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
            printf("Sky-dome texture '%s' not found - ignored.\n", s.c_str());
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
                printf("Sky-box texture '%s' not found - ignored.\n",
                       v[i].c_str());
            }
        }   // for i<v.size()
        if(m_sky_textures.size()!=6)
        {
            fprintf(stderr, "A skybox needs 6 textures, but %d are specified\n",
                    (int)m_sky_textures.size());
            fprintf(stderr, "in '%s'.\n", filename.c_str());

        }
        else
        {
            m_sky_type = SKY_BOX;
        }
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
void Track::itemCommand(const Vec3 &xyz, Item::ItemType type, 
                        bool drop)
{
    // Some modes (e.g. time trial) don't have any bonus boxes
    if(type==Item::ITEM_BONUS_BOX && 
       !World::getWorld()->haveBonusBoxes()) 
        return;

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
            printf("Item at position (%f,%f,%f) can not be dropped\n",
                loc.getX(), loc.getY(), loc.getZ());
            printf("onto terrain - position unchanged.\n");
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
            
            /*
            if (out[i][j] < -50)
            {
                printf(" ");
            }
            else if (out[i][j] < -25)
            {
                printf("`");
            }
            else if (out[i][j] < 0)
            {
                printf("-");
            }
            else if (out[i][j] < 25)
            {
                printf(":");
            }
            else if (out[i][j] < 50)
            {
                printf("!");
            }
            else if (out[i][j] < 100)
            {
                printf("#");
            }
             */
        }
        
        //printf("\n");
        x += x_step;
    }
    
    return out;
}

// ----------------------------------------------------------------------------
/** Returns the rotation of the sun. */
const core::vector3df& Track::getSunRotation()
{
    return m_sun->getRotation();
}
