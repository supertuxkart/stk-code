//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2009-2015  Joerg Henrichs, Steve Baker
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

#ifndef HEADER_TRACK_HPP
#define HEADER_TRACK_HPP

/**
  * \defgroup tracks
  * Contains information about tracks, namely drivelines, checklines and track
  * objects.
  */

#include <string>
#include <vector>

#include <irrlicht.h>

using namespace irr;

#include "LinearMath/btTransform.h"

#include "utils/aligned_array.hpp"
#include "utils/translation.hpp"
#include "utils/vec3.hpp"
#include "utils/ptr_vector.hpp"

class AbstractKart;
class AnimationManager;
class BezierCurve;
class CheckManager;
class ModelDefinitionLoader;
class MovingTexture;
class MusicInformation;
class ParticleEmitter;
class ParticleKind;
class PhysicalObject;
class RenderTarget;
class TrackObject;
class TrackObjectManager;
class TriangleMesh;
class XMLNode;

const int HEIGHT_MAP_RESOLUTION = 256;

// TODO: eventually remove this and fully replace with scripting
struct OverworldChallenge
{
public:
    core::vector3df m_position;
    std::string m_challenge_id;

    OverworldChallenge(core::vector3df position, std::string challenge_id)
    {
        m_position = position;
        m_challenge_id = challenge_id;
    }
};


struct Subtitle
{
    int m_from, m_to;
    core::stringw m_text;

    Subtitle(int from, int to, core::stringw text)
    {
        m_from = from;
        m_to = to;
        m_text = text;
    }
    int getFrom() const { return m_from; }
    int getTo()   const { return m_to;   }
    const core::stringw& getText() const { return m_text; }
};

/**
  * \ingroup tracks
  */
class Track
{
private:

    /** If a race is in progress, this stores the active track object.
     *  NULL otherwise. */
    static Track *m_current_track;

#ifdef DEBUG
    unsigned int             m_magic_number;
#endif

    /* Gravity to be used for this track. */
    float                    m_gravity;

    /** Friction to be used for the track. */
    float                    m_friction;

    std::string              m_ident;
    std::string              m_screenshot;
    bool                     m_is_day;
    std::vector<MusicInformation*> m_music;

    /** Will only be used on overworld */
    std::vector<OverworldChallenge> m_challenges;

    std::vector<Subtitle> m_subtitles;

    /** Start transforms of karts (either the default, or the ones taken
     *  from the scene file). */
    AlignedArray<btTransform> m_start_transforms;

    std::string              m_item_style;
    std::string              m_description;
    core::stringw            m_designer;

    /* For running the startup script */
    bool m_startup_run;
    /** The full filename of the config (xml) file. */
    std::string              m_filename;

    /** The base dir of all files of this track. */
    std::string              m_root;
    std::vector<std::string> m_groups;

    /** The list of all nodes. */
    std::vector<scene::ISceneNode*> m_all_nodes;

    /** The list of all nodes that are to be converted into physics,
     *  but not to be drawn (e.g. invisible walls). */
    std::vector<scene::ISceneNode*> m_static_physics_only_nodes;

    /** Same concept but for track objects. stored separately due to different
      * memory management.
      */
    std::vector<scene::ISceneNode*> m_object_physics_only_nodes;

    /** The list of all meshes that are loaded from disk, which means
     *  that those meshes are being cached by irrlicht, and need to be freed. */
    std::vector<scene::IMesh*>      m_all_cached_meshes;

    /**
      * m_all_cached_meshes assumes meshes are attached to a scene node.
      * This one assumes the mesh is NOT connected to any node.
      */
    std::vector<scene::IMesh*>      m_detached_cached_meshes;

    /** A list of all textures loaded by the track, so that they can
     *  be removed from the cache at cleanup time. */
    std::vector<video::ITexture*>   m_all_cached_textures;

    /** True if the materials.xml file is already loaded. This is used
     * for the overworld to keep its textures loaded. */
    bool m_materials_loaded;

    /** True if this track (textures and track data) should be cached. Used
     *  for the overworld. */
    bool m_cache_track;


#ifdef DEBUG
    /** A list of textures that were cached before the track is loaded.
     *  After cleanup of ta track it can be tested which new textures
     *  are still in the cache, and print a report of leaked textures
     *  (in debug mode only). */
    std::vector<video::ITexture*>   m_old_textures;

    /** Used to store all buffers in irrlicht's memory cache before a track
     *  is loaded. After cleanup of a track we can test which meshes are
     *  still in the cache, and print a report of leaked meshes (of course in
     *  debug mode only). */
    std::vector<std::string> m_old_mesh_buffers;
#endif

    PtrVector<ParticleEmitter>      m_all_emitters;
    scene::ISceneNode  *m_sun;
    /** Used to collect the triangles for the bullet mesh. */
    TriangleMesh*            m_track_mesh;
    /** Used to collect the triangles which do not have a physical
     *  representation, but are needed for some raycast effects. An
     *  example is a water surface: the karts ignore this (i.e.
     *  allowing the kart to drive in/partly under water), but the
     *  actual surface position is needed for the water splash effect. */
    TriangleMesh*            m_gfx_effect_mesh;
    /** Minimum coordinates of this track. */
    Vec3                     m_aabb_min;
    /** Maximum coordinates of this track. */
    Vec3                     m_aabb_max;
    /** True if this track is an arena. */
    bool                     m_is_arena;
    /** Max players supported by an arena. */
    unsigned int             m_max_arena_players;
    /** True if this track has easter eggs. */
    bool                     m_has_easter_eggs;
    /** True if this track has navmesh. */
    bool                     m_has_navmesh;
    /** True if this track is a soccer arena. */
    bool                     m_is_soccer;

    bool                     m_is_cutscene;

    /** The version of this track. A certain STK version will only support
     *  certain track versions. */
    int                      m_version;

    /** Far value for cameras for this track. */
    float                    m_camera_far;

    /** Whether this is an "internal" track. If so it won't be offered
      * in the track seelction screen
      */
    bool                     m_internal;

    /** Whether this track should be available in reverse version */
    bool                     m_reverse_available;

    /** If true a player kart will automatically be rescued if it is
     *  e.g. on a side, .... */
    bool                     m_enable_auto_rescue;

    /** If true any collision of a kart with the track will push the kart
     *  towards the nearest driveline. While this is (mostly) nice in tracks
     *  where a kart is pushed back towards the road, it doesn't work well
     *  on overworld, where karts have been pushed out of 'bubbles' . */
    bool                     m_enable_push_back;

    /** The type of sky to be used for the track. */
    enum {SKY_NONE, SKY_BOX,
          SKY_DOME, SKY_COLOR}          m_sky_type;

    /** sky rotation speed */
    float m_sky_dx, m_sky_dy;

    /** A list of the textures for the sky to use. It contains one texture
     *  in case of a dome, and 6 textures for a box. */
    std::vector<video::ITexture*> m_sky_textures;

    std::vector<video::ITexture*> m_spherical_harmonics_textures;

    /** Used if m_sky_type is SKY_COLOR only */
    irr::video::SColor m_sky_color;

    /** The list of all animated textures. */
    std::vector<MovingTexture*> m_animated_textures;

    /** Manager for all track objects. */
    TrackObjectManager *m_track_object_manager;

    /** If a sky dome is used, the number of horizontal segments
     *  the sphere should be divided in. */
    int                      m_sky_hori_segments;

    /** If a sky dome is used, the number of vertical segments
     *  the sphere should be divided in. */
    int                      m_sky_vert_segments;

    /** If a sky dome is used, percentage of the sphere to be used. */
    float                    m_sky_sphere_percent;

    /** If a sky dome is used, percentage of the texture to be used. */
    float                    m_sky_texture_percent;

    /** Particles emitted from the sky (wheather) */
    ParticleKind*            m_sky_particles;

    /** Use a special built-in wheather */
    bool                     m_weather_lightning;
    std::string              m_weather_sound;

    /** A simple class to keep information about a track mode. */
    class TrackMode
    {
    public:
        std::string m_name;        /**< Name / description of this mode. */
        std::string m_quad_name;   /**< Name of the quad file to use.    */
        std::string m_graph_name;  /**< Name of the graph file to use.   */
        std::string m_scene;       /**< Name of the scene file to use.   */

#ifdef DEBUG
        unsigned int m_magic_number;
#endif

        /** Default constructor, sets default names for all fields. */
        TrackMode() : m_name("default"),         m_quad_name("quads.xml"),
                      m_graph_name("graph.xml"), m_scene("scene.xml")
        {
#ifdef DEBUG
            m_magic_number = 0x46825179;
#endif
        }

        ~TrackMode()
        {
#ifdef DEBUG
            assert(m_magic_number == 0x46825179);
            m_magic_number = 0xDEADBEEF;
#endif
        }

    };   // TrackMode

    /** List of all modes for a track. */
    std::vector<TrackMode> m_all_modes;

    /** Name of the track to display. */
    std::string         m_name;

    /** The name used in sorting the track. */
    core::stringw       m_sort_name;

    /** True if the track uses fog. */
    bool                m_use_fog;

    /** Can be set to force fog off (e.g. for rendering minimap). */
    bool                m_force_disable_fog;

    /** True if this track supports using smoothed normals. */
    bool                m_smooth_normals;

    float               m_fog_max;
    float               m_fog_start;
    float               m_fog_end;
    float               m_fog_height_start;
    float               m_fog_height_end;
    core::vector3df     m_sun_position;
    /** The current ambient color for each kart. */
    video::SColor       m_ambient_color;
    video::SColor       m_default_ambient_color;
    video::SColor       m_sun_specular_color;
    video::SColor       m_sun_diffuse_color;
    video::SColor       m_fog_color;

    /** The render target for the mini map, which is displayed in the race gui. */
    RenderTarget           *m_render_target;
    core::dimension2du      m_mini_map_size;
    float                   m_minimap_x_scale;
    float                   m_minimap_y_scale;

    bool m_clouds;

    bool m_bloom;
    float m_bloom_threshold;

    bool m_godrays;
    core::vector3df m_godrays_position;
    float m_godrays_opacity;
    video::SColor m_godrays_color;

    bool m_shadows;

    float m_displacement_speed;
    float m_caustics_speed;

    /** The levels for color correction
     * m_color_inlevel(black, gamma, white)
     * m_color_outlevel(black, white)*/
    core::vector3df m_color_inlevel;
    core::vector2df m_color_outlevel;

    /** List of all bezier curves in the track - for e.g. camera, ... */
    std::vector<BezierCurve*> m_all_curves;

    /** The number of laps the track will be raced in a random GP.
     * m_actual_number_of_laps is initialised with this value.*/
    int m_default_number_of_laps;

    /** The number of laps that is predefined in a track info dialog. */
    int m_actual_number_of_laps;

    void loadTrackInfo();
    void loadDriveGraph(unsigned int mode_id, const bool reverse);
    void loadArenaGraph(const XMLNode &node);
    btQuaternion getArenaStartRotation(const Vec3& xyz, float heading);
    void convertTrackToBullet(scene::ISceneNode *node);
    bool loadMainTrack(const XMLNode &node);
    void loadMinimap();
    void createWater(const XMLNode &node);
    void getMusicInformation(std::vector<std::string>&  filenames,
                             std::vector<MusicInformation*>& m_music   );
    void loadCurves(const XMLNode &node);
    void handleSky(const XMLNode &root, const std::string &filename);

public:

    /** Static function to get the current track. NULL if no current
     *  track is defined (i.e. no race is active atm) */
    static Track* getCurrentTrack() { return m_current_track;  }
    // ------------------------------------------------------------------------
    void handleAnimatedTextures(scene::ISceneNode *node, const XMLNode &xml);

    /** Flag to avoid loading navmeshes (useful to speedup debugging: e.g.
     *  the temple navmesh distance matric computation takes around 12
     *  minutes(!) in debug mode to be computed. */
    static bool        m_dont_load_navmesh;

    static const float NOHIT;

                       Track             (const std::string &filename);
                      ~Track             ();
    void               cleanup           ();
    void               removeCachedData  ();
    void               startMusic        () const;

    void               createPhysicsModel(unsigned int main_track_count);
    void               update(float dt);
    void               reset();
    void               adjustForFog(scene::ISceneNode *node);
    void               adjustForFog(scene::IMesh* mesh,
                                    scene::ISceneNode* parent_scene_node);
    void               itemCommand(const XMLNode *node);
    core::stringw      getName() const;
    core::stringw      getSortName() const;
    bool               isInGroup(const std::string &group_name);
    const core::vector3df& getSunRotation();
    /** Sets the current ambient color for a kart with index k. */
    void               setAmbientColor(const video::SColor &color,
                                       unsigned int k);
    void               handleExplosion(const Vec3 &pos,
                                       const PhysicalObject *mp,
                                       bool secondary_hits=true) const;
    void               loadTrackModel  (bool reverse_track = false,
                                        unsigned int mode_id=0);
    bool findGround(AbstractKart *kart);

    std::vector< std::vector<float> > buildHeightMap();
    void               drawMiniMap(const core::rect<s32>& dest_rect) const;
    // ------------------------------------------------------------------------
    /** Returns true if this track has an arena mode. */
    bool isArena() const { return m_is_arena; }
    // ------------------------------------------------------------------------
    /** Returns true if this track is a racing track. This means it is not an
     *  internal track (like cut scenes), arena, or soccer field. */
    bool isRaceTrack() const
    {
        return !m_internal && !m_is_arena && !m_is_soccer;
    }   // isRaceTrack
    // ------------------------------------------------------------------------
    /** Returns true if this track has easter eggs. */
    bool hasEasterEggs() const { return m_has_easter_eggs; }
    // ------------------------------------------------------------------------
    /** Returns true if this race can be driven in reverse. */
    bool reverseAvailable() const { return m_reverse_available; }
    // ------------------------------------------------------------------------
    /** Returns true if this track navmesh. */
    bool hasNavMesh() const { return m_has_navmesh; }
    // ------------------------------------------------------------------------
    void loadObjects(const XMLNode* root, const std::string& path,
        ModelDefinitionLoader& lod_loader, bool create_lod_definitions,
        scene::ISceneNode* parent, TrackObject* parent_library);
    // ------------------------------------------------------------------------
    bool               isSoccer             () const { return m_is_soccer; }
    // ------------------------------------------------------------------------
    void               addMusic          (MusicInformation* mi)
                                                  {m_music.push_back(mi);     }
    // ------------------------------------------------------------------------
    float              getGravity        () const {return m_gravity;          }
    // ------------------------------------------------------------------------
    /** Returns the version of the .track file. */
    int                getVersion        () const {return m_version;          }
    // ------------------------------------------------------------------------
    /** Returns the length of the main driveline. */
    float              getTrackLength    () const;
    // ------------------------------------------------------------------------
    /** Returns a unique identifier for this track (the directory name). */
    const std::string& getIdent          () const {return m_ident;            }
    // ------------------------------------------------------------------------
    /** Returns all groups this track belongs to. */
    const std::vector<std::string>&
                       getGroups         () const {return m_groups;           }
    // ------------------------------------------------------------------------
    /** Returns the filename of this track. */
    const std::string& getFilename       () const {return m_filename;         }
    // ------------------------------------------------------------------------
    /** Returns the name of the designer. */
    const core::stringw& getDesigner     () const {return m_designer;         }
    // ------------------------------------------------------------------------
    /** Returns an absolute path to the screenshot file of this track */
    const std::string& getScreenshotFile () const {return m_screenshot;       }
    // ------------------------------------------------------------------------
    /** Returns if the track is during day time */
    const bool getIsDuringDay () const {return m_is_day;               }
    // ------------------------------------------------------------------------
    /** Returns the start coordinates for a kart with a given index.
     *  \param index Index of kart ranging from 0 to kart_num-1. */
    const btTransform& getStartTransform (unsigned int index) const
    {
        if (index >= m_start_transforms.size())
            Log::fatal("Track", "No start position for kart %i.", index);
        return m_start_transforms[index];
    }
    // ------------------------------------------------------------------------
    /** Sets pointer to the aabb of this track. */
    void               getAABB(const Vec3 **min, const Vec3 **max) const
                       { *min = &m_aabb_min; *max = &m_aabb_max; }
    // ------------------------------------------------------------------------
    /** Returns 'a' angle for quad n. This angle is used to position a kart
     *  after a rescue, and to detect wrong directions. This function will
     *  always return the angle towards the first successor, i.e. the angle
     *  in the direction of the default way on the track.
     *  \param n Number of the quad for which the angle is asked.
     */
    float              getAngle(int n) const;
    // ------------------------------------------------------------------------
    /** Returns the 2d coordinates of a point when drawn on the mini map
     *  texture.
     *  \param xyz Coordinates of the point to map.
     *  \param draw_at The coordinates in pixel on the mini map of the point,
     *         only the first two coordinates will be used.
     */
    void               mapPoint2MiniMap(const Vec3 &xyz, Vec3 *draw_at) const;
    // ------------------------------------------------------------------------
    /** Returns the full path of a given file inside this track directory. */
    std::string        getTrackFile(const std::string &s) const
                                { return m_root+"/"+s; }
    // ------------------------------------------------------------------------
    /** Returns the number of modes available for this track. */
    unsigned int       getNumberOfModes() const { return (unsigned int) m_all_modes.size();  }
    // ------------------------------------------------------------------------
    /** Returns number of completed challenges. */
    unsigned int getNumOfCompletedChallenges();
    // ------------------------------------------------------------------------
    /** Returns the name of the i-th. mode. */
    const std::string &getModeName(unsigned int i) const
                                              { return m_all_modes[i].m_name; }
    // ------------------------------------------------------------------------
    /** Returns the default ambient color. */
    const video::SColor &getDefaultAmbientColor() const
                                            { return m_default_ambient_color; }
    // ------------------------------------------------------------------------
    /** Returns the far value for cameras. */
    float  getCameraFar() const { return m_camera_far; }
    // ------------------------------------------------------------------------
    /** Returns the triangle mesh for this track. */
    const TriangleMesh *getPtrTriangleMesh() const { return m_track_mesh; }
    const TriangleMesh& getTriangleMesh() const {return *m_track_mesh; }
    // ------------------------------------------------------------------------
    /** Returns the graphical effect mesh for this track. */
    const TriangleMesh& getGFXEffectMesh() const {return *m_gfx_effect_mesh;}
    // ------------------------------------------------------------------------
    /** Get the max players supported for this track, for arena only. */
    unsigned int getMaxArenaPlayers() const
                                                { return m_max_arena_players; }
    // ------------------------------------------------------------------------
    /** Get the number of start positions defined in the scene file. */
    unsigned int getNumberOfStartPositions() const
                            { return (unsigned int)m_start_transforms.size(); }
    // ------------------------------------------------------------------------
    bool getWeatherLightning() {return m_weather_lightning;}
    // ------------------------------------------------------------------------
    const std::string& getWeatherSound() {return m_weather_sound;}
    // ------------------------------------------------------------------------
    ParticleKind* getSkyParticles         () { return m_sky_particles; }
    // ------------------------------------------------------------------------
    /** Override track fog value to force disabled */
    void forceFogDisabled(bool v) { m_force_disable_fog = v; }
    //-------------------------------------------------------------------------
    /** Returns if fog is currently enabled. It can be disabled per track, or
     *  temporary be disabled (e.g. for rendering mini map). */
    bool isFogEnabled() const
    {
        return !m_force_disable_fog && m_use_fog;
    }   // isFogEnabled

    // ------------------------------------------------------------------------
    float getFogStart()  const { return m_fog_start; }
    // ------------------------------------------------------------------------
    void setFogStart(float start) { m_fog_start = start; }
    // ------------------------------------------------------------------------
    float getFogEnd()    const { return m_fog_end; }
    // ------------------------------------------------------------------------
    void setFogEnd(float end) { m_fog_end = end; }
    // ------------------------------------------------------------------------
    float getFogStartHeight()  const { return m_fog_height_start; }
    // ------------------------------------------------------------------------
    float getFogEndHeight()    const { return m_fog_height_end; }
    // ------------------------------------------------------------------------
    float getFogMax()    const { return m_fog_max; }
    // ------------------------------------------------------------------------
    void setFogMax(float max) { m_fog_max = max; }
    // ------------------------------------------------------------------------
    video::SColor getFogColor() const { return m_fog_color; }
    // ------------------------------------------------------------------------
    void setFogColor(video::SColor& color) { m_fog_color = color; }
    // ------------------------------------------------------------------------
    video::SColor getSunColor() const { return m_sun_diffuse_color; }
    // ------------------------------------------------------------------------
    /** Whether this is an "internal" track. If so it won't be offered
     * in the track selection screen. */
    bool isInternal() const { return m_internal; }
    // ------------------------------------------------------------------------
    /** Returns true if auto rescue is enabled. */
    bool isAutoRescueEnabled() const { return m_enable_auto_rescue; }
    // ------------------------------------------------------------------------
    /** True if push back of karts towards the track should be enabled. */
    bool isPushBackEnabled() const { return m_enable_push_back; }
    // ------------------------------------------------------------------------
    /** Returns true if the normals of this track can be smoothed. */
    bool smoothNormals() const { return m_smooth_normals; }
    // ------------------------------------------------------------------------
    /** Returns the track object manager. */
    TrackObjectManager* getTrackObjectManager() const
    {
        return m_track_object_manager;
    }   // getTrackObjectManager

    // ------------------------------------------------------------------------
    /** Get list of challenges placed on that world. Works only for overworld. */
    const std::vector<OverworldChallenge>& getChallengeList() const
        { return m_challenges; }

    // ------------------------------------------------------------------------
    const std::vector<Subtitle>& getSubtitles() const { return m_subtitles; }

    // ------------------------------------------------------------------------
    bool hasClouds() const { return m_clouds; }

    // ------------------------------------------------------------------------
    bool hasBloom() const { return m_bloom; }

    // ------------------------------------------------------------------------
    float getBloomThreshold() const { return m_bloom_threshold; }

    // ------------------------------------------------------------------------
    /** Return the color levels for color correction shader */
    core::vector3df getColorLevelIn() const { return m_color_inlevel; }
    // ------------------------------------------------------------------------
    core::vector2df getColorLevelOut() const { return m_color_outlevel; }
    // ------------------------------------------------------------------------
    bool hasGodRays() const { return m_godrays; }
    // ------------------------------------------------------------------------
    core::vector3df getGodRaysPosition() const { return m_godrays_position; }
    // ------------------------------------------------------------------------
    float getGodRaysOpacity() const { return m_godrays_opacity; }
    // ------------------------------------------------------------------------
    video::SColor getGodRaysColor() const { return m_godrays_color; }
    // ------------------------------------------------------------------------
    bool hasShadows() const { return m_shadows; }
    // ------------------------------------------------------------------------
    void addNode(scene::ISceneNode* node) { m_all_nodes.push_back(node); }
    // ------------------------------------------------------------------------
    void addPhysicsOnlyNode(scene::ISceneNode* node)
    {
        m_object_physics_only_nodes.push_back(node);
    }
    // ------------------------------------------------------------------------
    float getDisplacementSpeed() const { return m_displacement_speed;    }
    // ------------------------------------------------------------------------
    float getCausticsSpeed() const { return m_caustics_speed;        }
    // ------------------------------------------------------------------------
    const int getDefaultNumberOfLaps() const { return m_default_number_of_laps;}
    // ------------------------------------------------------------------------
    const int getActualNumberOfLap() const { return m_actual_number_of_laps; }
    // ------------------------------------------------------------------------
    void setActualNumberOfLaps(unsigned int laps)
                                         { m_actual_number_of_laps = laps; }
    // ------------------------------------------------------------------------
    bool operator<(const Track &other) const;
    // ------------------------------------------------------------------------
    /** Adds mesh to cleanup list */
    void addCachedMesh(scene::IMesh* mesh) { m_all_cached_meshes.push_back(mesh); }
};   // class Track

#endif
