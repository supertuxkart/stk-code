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

#include "tracks/track.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "irrlicht.h"
using namespace irr;

#include "animations/animation_manager.hpp"
#include "audio/sound_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/mesh_tools.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/scene.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "modes/world.hpp"
#include "physics/physical_object.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/race_manager.hpp"
#include "states_screens/race_gui.hpp"
#include "tracks/bezier_curve.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/quad_graph.hpp"
#include "tracks/quad_set.hpp"
#include "utils/string_utils.hpp"

const float Track::NOHIT           = -99999.9f;

// ----------------------------------------------------------------------------
Track::Track( std::string filename)
{
    m_filename             = filename;
    m_item_style           = "";
    m_description          = "";
    m_designer             = "";
    m_screenshot           = "";
    m_version              = 0;
    m_track_mesh           = new TriangleMesh();
    m_non_collision_mesh   = new TriangleMesh();
    m_all_nodes.clear();
    m_all_meshes.clear();
    m_has_final_camera     = false;
    m_is_arena             = false;
    m_quad_graph           = NULL;
	m_animation_manager    = NULL;
	m_check_manager        = NULL;
    loadTrack(m_filename);
}   // Track

//-----------------------------------------------------------------------------
/** Destructor, removes quad data structures etc. */
Track::~Track()
{
    if(m_quad_graph)            delete m_quad_graph;
	if(m_animation_manager)     delete m_animation_manager;
	if(m_check_manager)         delete m_check_manager;
    if(m_mini_map)              irr_driver->removeTexture(m_mini_map);
}   // ~Track

//-----------------------------------------------------------------------------
/** Prepates the track for a new race. This function must be called after all
 *  karts are created, since the check objects allocate data structures 
 *  depending on the number of karts.
 */
void Track::reset()
{
	if(m_animation_manager)
		m_animation_manager->reset();
    if(m_check_manager)
        m_check_manager->reset(*this);
}   // reset

//-----------------------------------------------------------------------------
/** Removes the physical body from the world.
 *  Called at the end of a race.
 */
void Track::cleanup()
{
    if(UserConfigParams::m_track_debug)
        m_quad_graph->cleanupDebugMesh();

    item_manager->cleanup();
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

    for(unsigned int i=0; i<m_all_meshes.size(); i++)
    {
        irr_driver->removeMesh(m_all_meshes[i]);
    }
    m_all_meshes.clear();


    delete m_non_collision_mesh;
    m_non_collision_mesh = new TriangleMesh();
    delete m_track_mesh;
    m_track_mesh = new TriangleMesh();

    // remove temporary materials loaded by the material manager
    material_manager->popTempMaterial();
}   // cleanup

//-----------------------------------------------------------------------------
const Vec3& Track::trackToSpatial(const int sector) const
{
    return m_quad_graph->getQuad(sector).getCenter();
}   // trackToSpatial

//-----------------------------------------------------------------------------
/** Returns the start coordinates for a kart on a given position pos
    (with pos ranging from 0 to kart_num-1).
*/
btTransform Track::getStartTransform(unsigned int pos) const
{

    Vec3 orig;
    
    if(isArena())
    {
        assert(pos < m_start_positions.size());
        orig.setX( m_start_positions[pos][0] );
        orig.setY( m_start_positions[pos][1] );
        orig.setZ( m_start_positions[pos][2] );
    }
    else
    {        
        orig.setX( pos<m_start_x.size() ? m_start_x[pos] : ((pos%2==0)?1.5f:-1.5f) );
        orig.setY( pos<m_start_y.size() ? m_start_y[pos] : -1.5f*pos-1.5f          );
        orig.setZ( pos<m_start_z.size() ? m_start_z[pos] : 1.0f                    );
    }
    btTransform start;
    start.setOrigin(orig);
    start.setRotation(btQuaternion(btVector3(0, 0, 1), 
                                   pos<m_start_heading.size() 
                                   ? DEGREE_TO_RAD*m_start_heading[pos]
                                   : 0.0f ));
    return start;
}   // getStartTransform

//-----------------------------------------------------------------------------
void Track::loadTrack(const std::string &filename)
{
    m_filename          = filename;
    m_ident             = StringUtils::basename(
                                StringUtils::without_extension(m_filename));
    std::string path    = StringUtils::without_extension(m_filename);

    // Default values
    m_use_fog           = false;
    m_fog_density       = 1.0f/100.0f;
    m_fog_start         = 0.0f;
    m_fog_end           = 1000.0f;
    m_gravity           = 9.80665f;
    m_sun_position      = core::vector3df(0.4f, 0.4f, 0.4f);
    m_sky_color         = video::SColorf(0.3f, 0.7f, 0.9f, 1.0f);
    m_fog_color         = video::SColorf(0.3f, 0.7f, 0.9f, 1.0f).toSColor();
    m_ambient_color     = video::SColorf(0.5f, 0.5f, 0.5f, 1.0f);
    m_specular_color    = video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_diffuse_color     = video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);    
    XMLNode *root       = file_manager->createXMLTree(m_filename);
        
    if(!root || root->getName()!="track")
    {
        std::ostringstream o;
        o<<"Can't load track '"<<filename<<"', no track element.";
        throw std::runtime_error(o.str());
    }
    root->get("name",                  &m_name);
    root->get("description",           &m_description);
    root->get("designer",              &m_designer);
    root->get("version",               &m_version);
    std::vector<std::string> filenames;
    root->get("music",                 &filenames);
    getMusicInformation(filenames, m_music);
    root->get("item",                  &m_item_style);
    root->get("screenshot",            &m_screenshot);
    root->get("sky-color",             &m_sky_color);
    root->get("start-x",               &m_start_x);
    root->get("start-y",               &m_start_y);
    root->get("start-z",               &m_start_z);
    root->get("start-heading",         &m_start_heading);
    root->get("use-fog",               &m_use_fog);
    root->get("fog-color",             &m_fog_color);
    root->get("fog-density",           &m_fog_density);
    root->get("fog-start",             &m_fog_start);
    root->get("fog-end",               &m_fog_end);
    root->get("sun-position",          &m_sun_position);
    root->get("sun-ambient",           &m_ambient_color);
    root->get("sun-specular",          &m_specular_color);
    root->get("sun-diffuse",           &m_diffuse_color);
    root->get("gravity",               &m_gravity);
    root->get("arena",                 &m_is_arena);
    root->get("groups",                &m_groups);
        
    if(m_groups.size()==0)
        m_groups.push_back("standard");
    // if both camera position and rotation are defined,
    // set the flag that the track has final camera position
    m_has_final_camera  = root->get("camera-final-position", 
                                    &m_camera_final_position)!=1;
    m_has_final_camera &= root->get("camera-final-hpr",
                                    &m_camera_final_hpr)     !=1;
    m_camera_final_hpr.degreeToRad();

    m_sky_type = SKY_NONE;
    const XMLNode *xml_node = root->getNode("sky-dome");
    if(xml_node)
    {
        m_sky_type            = SKY_DOME;
        m_sky_vert_segments   = 16;
        m_sky_hori_segments   = 16;
        m_sky_sphere_percent  = 1.0f;
        m_sky_texture_percent = 1.0f;
        std::string s;
        xml_node->get("texture",          &s                   );
        m_sky_textures.push_back(s);
        xml_node->get("vertical",        &m_sky_vert_segments  );
        xml_node->get("horizontal",      &m_sky_hori_segments  );
        xml_node->get("sphere-percent",  &m_sky_sphere_percent );
        xml_node->get("texture-percent", &m_sky_texture_percent);

    }   // if sky-dome
    xml_node = root->getNode("sky-box");
    if(xml_node)
    {
        std::string s;
        xml_node->get("texture", &s);
        m_sky_textures = StringUtils::split(s, ' ');
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
    }   // if sky-box

    xml_node = root->getNode("curves");
	if(xml_node)
		loadCurves(*xml_node);

	// Set the correct paths
    m_screenshot = file_manager->getTrackFile(m_screenshot, getIdent());
    delete root;

}   // loadTrack

//-----------------------------------------------------------------------------
void Track::loadCurves(const XMLNode &node)
{
	for(unsigned int i=0; i<node.getNumNodes(); i++)
	{
		const XMLNode *curve = node.getNode(i);
		m_all_curves.push_back(new BezierCurve(*curve));
	}   // for i<node.getNumNodes
}   // loadCurves

//-----------------------------------------------------------------------------
void Track::getMusicInformation(std::vector<std::string>&       filenames, 
                                std::vector<MusicInformation*>& music    )
{
    for(int i=0; i<(int)filenames.size(); i++)
    {
        std::string full_path = file_manager->getTrackFile(filenames[i], getIdent());
        MusicInformation* mi;
        try
        {
            mi = sound_manager->getMusicInformation(full_path);
        }
        catch(std::runtime_error)
        {
            mi = sound_manager->getMusicInformation(file_manager->getMusicFile(filenames[i]));
        }
        if(!mi)
        {
            fprintf(stderr, "Music information file '%s' not found - ignored.\n",
                    filenames[i].c_str());
            continue;
        }
        m_music.push_back(mi);
    }   // for i in filenames
}   // getMusicInformation

//-----------------------------------------------------------------------------
void Track::startMusic() const 
{
    // In case that the music wasn't found (a warning was already printed)
    if(m_music.size()>0)
        sound_manager->startMusic(m_music[rand()% m_music.size()]);
}   // startMusic

//-----------------------------------------------------------------------------
/** Loads the quad graph, i.e. the definition of all quads, and the way
 *  they are connected to each other.
 */
void Track::loadQuadGraph()
{
    m_quad_graph = new QuadGraph(file_manager->getTrackFile(m_ident+".quads"), 
                                 file_manager->getTrackFile(m_ident+".graph"));
    m_mini_map   = m_quad_graph->makeMiniMap(RaceManager::getWorld()->getRaceGUI()->getMiniMapSize(), 
                                             "minimap::"+m_ident);
    if(m_quad_graph->getNumNodes()==0)
    {
        fprintf(stderr, "No graph nodes defined for track '%s'\n",
                m_filename.c_str());
        exit(-1);
    }
}   // loadQuadGraph

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Track::createPhysicsModel()
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
    if (m_non_collision_mesh == NULL)
    {
        fprintf(stderr, "ERROR: m_track_mesh == NULL, cannot createPhysicsModel\n");
        return;
    }

    m_track_mesh->removeBody();
    for(unsigned int i=1; i<m_all_meshes.size(); i++)
    {
        convertTrackToBullet(m_all_meshes[i]);
    }
    m_track_mesh->createBody();
    m_non_collision_mesh->createBody(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    
}   // createPhysicsModel

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Track::convertTrackToBullet(const scene::IMesh *mesh)
{
    for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++) {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        // FIXME: take translation/rotation into account
        if(mb->getVertexType()!=video::EVT_STANDARD) {
            fprintf(stderr, "WARNING: Physics::convertTrack: Ignoring type '%d'!", 
                mb->getVertexType());
            continue;
        }
        video::SMaterial &irrMaterial=mb->getMaterial();
        video::ITexture* t=irrMaterial.getTexture(0);

        const Material* material=0;
        TriangleMesh *tmesh = m_track_mesh;
        if(t) {
            std::string image = std::string(t->getName().c_str());
            material=material_manager->getMaterial(StringUtils::basename(image));
            if(material->isZipper()) tmesh = m_non_collision_mesh;
        } 

        u16 *mbIndices = mb->getIndices();
        Vec3 vertices[3];
        irr::video::S3DVertex* mbVertices=(video::S3DVertex*)mb->getVertices();
        for(unsigned int j=0; j<mb->getIndexCount(); j+=3) {
            for(unsigned int k=0; k<3; k++) {
                int indx=mbIndices[j+k];
                vertices[k] = Vec3(mbVertices[indx].Pos);
            }   // for k
            if(tmesh) tmesh->addTriangle(vertices[0], vertices[1], 
                                         vertices[2], material     );
        }   // for j
    }   // for i<getMeshBufferCount

}   // convertTrackToBullet
// ----------------------------------------------------------------------------
/** Loads the main track model (i.e. all other objects contained in the
 *  scene might use raycast on this track model to determine the actual
 *  height of the terrain.
 */
bool Track::loadMainTrack(const XMLNode &xml_node)
{
    std::string model_name;
    xml_node.get("model", &model_name);
    std::string full_path = file_manager->getTrackFile(model_name, 
                                                       getIdent());
    scene::IMesh *mesh = irr_driver->getAnimatedMesh(full_path);
    if(!mesh)
    {
        fprintf(stderr, "Warning: Main track model '%s' in '%s' not found, aborting.\n",
                xml_node.getName().c_str(), model_name.c_str());
        exit(-1);
    }

    m_all_meshes.push_back(mesh);

    Vec3 min, max;
    MeshTools::minMax3D(mesh, &min, &max);
    RaceManager::getWorld()->getPhysics()->init(min, max);
    // This will (at this stage) only convert the main track model.
    convertTrackToBullet(mesh);
    if (m_track_mesh == NULL)
    {
        fprintf(stderr, "ERROR: m_track_mesh == NULL, cannot loadMainTrack\n");
        return false;
    }
    
    m_track_mesh->createBody();


    scene::ISceneNode *scene_node = irr_driver->addOctTree(mesh);
    core::vector3df xyz(0,0,0);
    xml_node.getXYZ(&xyz);
    core::vector3df hpr(0,0,0);
    xml_node.getHPR(&hpr);
    scene_node->setPosition(xyz);
    scene_node->setRotation(hpr);
    handleAnimatedTextures(scene_node, xml_node);
    m_all_nodes.push_back(scene_node);
    scene_node->setMaterialFlag(video::EMF_LIGHTING, true);
    scene_node->setMaterialFlag(video::EMF_GOURAUD_SHADING, true);

    return true;
}   // loadMainTrack

// ----------------------------------------------------------------------------
/** Handles animated textures.
 *  \param node The node containing the data for the animated notion.
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
                    StringUtils::basename(t->getName().c_str());
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
    for(unsigned int i=0; i<m_animated_textures.size(); i++)
    {
        m_animated_textures[i]->update(dt);
    }
    for(unsigned int i=0; i<m_physical_objects.size(); i++)
    {
        m_physical_objects[i]->update(dt);
    }
	if(m_animation_manager)
		m_animation_manager->update(dt);
	if(m_check_manager)
		m_check_manager->update(dt);
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
    for(std::vector<PhysicalObject*>::const_iterator i=m_physical_objects.begin();
        i!=m_physical_objects.end(); i++)
    {
        (*i)->handleExplosion(pos, obj==(*i));
    }
}   // handleExplosion

// ----------------------------------------------------------------------------
/** Creates a water node.
 *  \param node The XML node containing the specifications for the water node.
 */
void Track::createWater(const XMLNode &node)
{
    std::string model_name;
    node.get("model", &model_name);
    std::string full_path = file_manager->getTrackFile(model_name, 
                                                       getIdent());

    //scene::IMesh *mesh = irr_driver->getMesh(full_path);
    scene::IAnimatedMesh *mesh = irr_driver->getSceneManager()->getMesh(full_path.c_str());
    //irr_driver->getSceneManager()->addWaterSurfaceSceneNode(mesh->getMesh(0));
    //scene::IAnimatedMesh *mesh = irr_driver->getSceneManager()->addHillPlaneMesh("myHill",
    //            core::dimension2d<f32>(20,20),
    //            core::dimension2d<u32>(40,40), 0, 0,
    //            core::dimension2d<f32>(0,0),
    //            core::dimension2d<f32>(10,10));

    //scene::SMeshBuffer b(*(scene::SMeshBuffer*)(mesh->getMesh(0)->getMeshBuffer(0)));
    //scene::SMeshBuffer* buffer = new scene::SMeshBuffer(*(scene::SMeshBuffer*)(mesh->getMeshBuffer(0)));
    
    float wave_height  = 2.0f;
    float wave_speed   = 300.0f;
    float wave_length  = 10.0f;
    node.get("height", &wave_height);
    node.get("speed",  &wave_speed);
    node.get("length", &wave_length);
    scene::ISceneNode* scene_node = irr_driver->addWaterNode(mesh,
                                                             wave_height, 
                                                             wave_speed,
                                                             wave_length);

    if(!mesh || !scene_node)
    {
        fprintf(stderr, "Warning: Water model '%s' in '%s' not found, ignored.\n",
                node.getName().c_str(), model_name.c_str());
        return;
    }
    m_all_meshes.push_back(mesh);
    core::vector3df xyz(0,0,0);
    node.getXYZ(&xyz);
    core::vector3df hpr(0,0,0);
    node.getHPR(&hpr);
    scene_node->setPosition(xyz);
    scene_node->setRotation(hpr);
    m_all_nodes.push_back(scene_node);
}   // createWater

// ----------------------------------------------------------------------------
/** This function load the actual scene, i.e. all parts of the track, 
 *  animations, items, ... It  is called from world during initialisation. 
 *  Track is the first model to be loaded, so at this stage the root scene node 
 * is empty.
 */
void Track::loadTrackModel()
{
    // Load the graph only now: this function is called from world, after
    // the race gui was created. The race gui is needed since it stores
    // the information about the size of the texture to render the mini
    // map to.
    loadQuadGraph();
    // Add the track directory to the texture search path
    file_manager->pushTextureSearchPath(file_manager->getTrackFile("",getIdent()));
    file_manager->pushModelSearchPath  (file_manager->getTrackFile("",getIdent()));
    // First read the temporary materials.dat file if it exists
    try
    {
        std::string materials_file = file_manager->getTrackFile("materials.xml",getIdent());
        material_manager->pushTempMaterial(materials_file);
    }
    catch (std::exception& e)
    {
        // no temporary materials.dat file, ignore
        (void)e;
    }

    // Start building the scene graph
    std::string path = file_manager->getTrackFile(getIdent()+".scene");
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
    const XMLNode *node = root->getNode("track");
    loadMainTrack(*node);
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        const std::string name = node->getName();
        // The track object was already converted before the loop
        if(name=="track") continue;
        if(name=="physical-object")
        {
            m_physical_objects.push_back(new PhysicalObject(node));
        }
        else if(name=="water")
        {
            createWater(*node);
        }
        else if(name=="model")
        {
            std::string model_name;
            node->get("model", &model_name);
            std::string full_path = file_manager->getTrackFile(model_name, 
                getIdent());
            scene::IMesh *mesh = irr_driver->getAnimatedMesh(full_path);
            if(!mesh)
            {
                fprintf(stderr, "Warning: Main track model '%s' in '%s' not found, aborting.\n",
                    node->getName().c_str(), model_name.c_str());
                exit(-1);
            }
            m_all_meshes.push_back(mesh);
            scene::ISceneNode *scene_node = irr_driver->addOctTree(mesh);
            core::vector3df xyz(0,0,0);
            node->getXYZ(&xyz);
            core::vector3df hpr(0,0,0);
            node->getHPR(&hpr);
            scene_node->setPosition(xyz);
            scene_node->setRotation(hpr);
            handleAnimatedTextures(scene_node, *node);
            m_all_nodes.push_back(scene_node);
            scene_node->setMaterialFlag(video::EMF_LIGHTING, true);
        }
        else if(name=="banana"      || name=="item" || 
                name=="small-nitro" || name=="big-nitro")
        {
            Item::ItemType type;
            if     (name=="banana"     ) type = Item::ITEM_BANANA;
            else if(name=="item"       ) type = Item::ITEM_BONUS_BOX;
            else if(name=="small-nitro") type = Item::ITEM_SILVER_COIN;
            else                         type = Item::ITEM_GOLD_COIN;
            Vec3 xyz;
            int bits = node->getXYZ(&xyz);
            // Height is needed if bit 2 (for z) is not set
            itemCommand(xyz, type, /* need_height */ !XMLNode::hasZ(bits) );
        }
		else if(name=="animations")
		{
			m_animation_manager = new AnimationManager(getIdent(), *node);
		}
		else if(name=="checks")
		{
			m_check_manager = new CheckManager(*node);
		}
        else
        {
            fprintf(stderr, "Warning: element '%s' not found.\n",
                    node->getName().c_str());
        }

    }
    delete root;

    // Init all physical objects
    for(std::vector<PhysicalObject*>::const_iterator i=m_physical_objects.begin();
        i!=m_physical_objects.end(); i++)
    {
        (*i)->init();
    }

    // Sky dome and boxes support
    // --------------------------
    if(m_sky_type==SKY_DOME)
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
                m_animated_textures.push_back(new MovingTexture(m, 0.5f, 0.5f));
            }   // for j<MATERIAL_MAX_TEXTURES
        }   // for i<getMaterialCount

        m_all_nodes.push_back(irr_driver->addSkyDome(m_sky_textures[0],
                                                     m_sky_hori_segments, 
                                                     m_sky_vert_segments, 
                                                     m_sky_texture_percent, 
                                                     m_sky_sphere_percent) );
    }
    else if(m_sky_type==SKY_BOX)
    {
        m_all_nodes.push_back(irr_driver->addSkyBox(m_sky_textures));
    }

    file_manager->popTextureSearchPath();
    file_manager->popModelSearchPath  ();

    irr_driver->getSceneManager()->setAmbientLight(video::SColor(255, 120, 120, 120));

    m_light = irr_driver->getSceneManager()->addLightSceneNode(NULL, m_sun_position, 
                                                               video::SColorf(1.0f,1.0f,1.0f));
    m_light->setLightType(video::ELT_DIRECTIONAL);
    m_light->setRotation( core::vector3df(180, 45, 45) );

    m_light->getLightData().DiffuseColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    m_light->getLightData().SpecularColor = irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f);
    
    /*
    m_light = irr_driver->getSceneManager()->addLightSceneNode(0, m_sun_position);
    video::SLight light;
    // HACK & TEST: checking how ambient looks for some things, must be properly done once we reach an agreement
    light.AmbientColor = irr::video::SColorf(0.666666f, 0.666666f, 0.666666f, 0.0f);
    m_light->setLightData(light);
     */

    if(m_use_fog)
    {
        irr_driver->getVideoDriver()->setFog(m_fog_color, true, m_fog_start, m_fog_end, m_fog_density);
    }
    
    // Note: the physics world for irrlicht is created in loadMainTrack
    createPhysicsModel();
    if(UserConfigParams::m_track_debug)
        m_quad_graph->createDebugMesh();
}   // loadTrack

//-----------------------------------------------------------------------------
void Track::itemCommand(const Vec3 &xyz, Item::ItemType type, 
                        int bNeedHeight)
{
    // Some modes (e.g. time trial) don't have any bonus boxes
    if(type==Item::ITEM_BONUS_BOX && 
       !RaceManager::getWorld()->enableBonusBoxes()) 
        return;

    Vec3 loc(xyz);
    // if only 2d coordinates are given, let the item fall from very high
    if(bNeedHeight)
    {
        loc.setZ(1000);
        loc.setZ(getTerrainHeight(loc));
    }

    // Don't tilt the items, since otherwise the rotation will look odd,
    // i.e. the items will not rotate around the normal, but 'wobble'
    // around.
    Vec3 normal(0, 0, 0.0f);
    item_manager->newItem(type, loc, normal);
}   // itemCommand

// ----------------------------------------------------------------------------
void  Track::getTerrainInfo(const Vec3 &pos, float *hot, Vec3 *normal, 
                            const Material **material) const
{
    btVector3 to_pos(pos);
    to_pos.setZ(-100000.f);

    class MaterialCollision : public btCollisionWorld::ClosestRayResultCallback
    {
    public:
        const Material* m_material;
        MaterialCollision(btVector3 p1, btVector3 p2) : 
            btCollisionWorld::ClosestRayResultCallback(p1,p2) {m_material=NULL;}
        virtual btScalar AddSingleResult(btCollisionWorld::LocalRayResult& rayResult,
                                         bool normalInWorldSpace) {
             if(rayResult.m_localShapeInfo && rayResult.m_localShapeInfo->m_shapePart>=0 )
             {
                 m_material = ((TriangleMesh*)rayResult.m_collisionObject->getUserPointer())->getMaterial(rayResult.m_localShapeInfo->m_triangleIndex);
             }
             else
             {
                 // This can happen if the raycast hits a kart. This should 
                 // actually be impossible (since the kart is removed from
                 // the collision group), but now and again the karts don't
                 // have the broadphase handle set (kart::update() for 
                 // details), and it might still happen. So in this case
                 // just ignore this callback and don't add it.
                 return 1.0f;
             }
             return btCollisionWorld::ClosestRayResultCallback::AddSingleResult(rayResult, 
                                                                                normalInWorldSpace);
        }   // AddSingleResult
    };   // myCollision
    MaterialCollision rayCallback(pos, to_pos);
    RaceManager::getWorld()->getPhysics()->getPhysicsWorld()->rayTest(pos, to_pos, rayCallback);

    if(!rayCallback.HasHit()) 
    {
        *hot      = NOHIT;
        *material = NULL;
        return;
    }

    *hot      = rayCallback.m_hitPointWorld.getZ();
    *normal   = rayCallback.m_hitNormalWorld;
    *material = rayCallback.m_material;
    // Note: material might be NULL. This happens if the ray cast does not
    // hit the track, but another rigid body (kart, moving_physics) - e.g.
    // assume two karts falling down, one over the other. Bullet does not
    // have any triangle/material information in this case!
}   // getTerrainInfo

// ----------------------------------------------------------------------------
/** Simplified version to determine only the height of the terrain.
 *  \param pos Position at which to determine the height (x,y coordinates
 *             are only used).
 *  \return The height at the x,y coordinates.
 */
float Track::getTerrainHeight(const Vec3 &pos) const
{
    float hot;
    Vec3  normal;
    const Material *m;
    getTerrainInfo(pos, &hot, &normal, &m);
    return hot;
}   // getTerrainHeight
