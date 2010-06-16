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

#ifndef HEADER_TRACK_HPP
#define HEADER_TRACK_HPP

/** \defgroup tracks */

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif

#include <string>
#include <vector>

#include "irrlicht.h"
using namespace irr;

#include "LinearMath/btTransform.h"
#include "audio/music_information.hpp"
#include "graphics/material.hpp"
#include "items/item.hpp"
#include "tracks/quad_graph.hpp"
#include "utils/vec3.hpp"

class AnimationManager;
class BezierCurve;
class CheckManager;
class MovingTexture;
class PhysicalObject;
class TrackObjectManager;
class TriangleMesh;
class World;
class XMLNode;

/**
  * \ingroup tracks
  */
class Track
{
private:
    float                    m_gravity;
    std::string              m_ident;
    std::string              m_screenshot;
    std::vector<MusicInformation*> m_music;
    /** Start heading of karts (if specified in the scene file). */
    std::vector<float>       m_start_heading;

    /** Start positions of karts (if specified in the scene file). */
    std::vector<Vec3>        m_start_positions;

    /** A transform which is applied to the default start coordinates
     *  (i.e. only if no start coordinates are defined for the track).
     *  This is used to position the karts in case that the lap counting
     *  line is not centered around (0,0,0), or rotated. */
    btTransform              m_start_transform;
    /** The explicit angle of the lap counting line. This angle can
     *  not be easily deduced from m_start_transform (problem with the
     *  sign), so it is saved additionally so that karts can be rotated
     *  properly if no explicit start positions are given. */
    float                    m_start_angle;

    std::string              m_item_style;
    std::string              m_description;
    std::string              m_designer;
    /** The full filename of the config (xml) file. */
    std::string              m_filename;
    /** The base dir of all files of this track. */
    std::string              m_root;
    std::vector<std::string> m_groups;
    std::vector<scene::ISceneNode*> m_all_nodes;
    std::vector<scene::IMesh*>      m_all_meshes;
    scene::ILightSceneNode  *m_sun;
    TriangleMesh*            m_track_mesh;
    TriangleMesh*            m_non_collision_mesh;
    /** Minimum coordinates of this track. */
    Vec3                     m_aabb_min;
    /** Maximum coordinates of this track. */
    Vec3                     m_aabb_max;
    /** True if this track is an arena. */
    bool                     m_is_arena;
    /** The version of this track. A certain STK version will only support
     *  certain track versions. */
    int                      m_version;
    /** Far value for cameras for this track. */
    float                    m_camera_far;

    /** The graph used to connect the quads. */
    QuadGraph               *m_quad_graph;

    /** The type of sky to be used for the track. */
    enum {SKY_NONE, SKY_BOX, 
          SKY_DOME, SKY_COLOR}          m_sky_type;

    /** sky rotation speed */
    float m_sky_dx, m_sky_dy;
    
    /** A list of the textures for the sky to use. It contains one texture
     *  in case of a dome, and 6 textures for a box. */
    std::vector<std::string> m_sky_textures;

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

    /** A simple class to keep information about a track mode. */
    class TrackMode
    {
    public:
        std::string m_name;        /**< Name / description of this mode. */
        std::string m_quad_name;   /**< Name of the quad file to use.    */
        std::string m_graph_name;  /**< Name of the graph file to use.   */
        std::string m_scene;       /**< Name of the scene file to use.   */
        /** Default constructor, sets default names for all fields. */
        TrackMode() : m_name("default"),         m_quad_name("quads.xml"),
                      m_graph_name("graph.xml"), m_scene("scene.xml")   {};
    };   // TrackMode

    /** List of all modes for a track. */
    std::vector<TrackMode> m_all_modes;

    /** Name of the track to display. */
    irr::core::stringw  m_name;
    bool                m_use_fog;
    float               m_fog_density;
    float               m_fog_start;
    float               m_fog_end;
    core::vector3df     m_sun_position;
    /** The current ambient color for each kart. */
    video::SColor       m_ambient_color;
    video::SColor       m_default_ambient_color;
    video::SColor       m_sun_specular_color;
    video::SColor       m_sun_diffuse_color;
    video::SColor       m_fog_color;

    /** The texture for the mini map, which is displayed in the race gui. */
    video::ITexture         *m_mini_map;

    /** List of all bezier curves in the track - for e.g. camera, ... */
    std::vector<BezierCurve*> m_all_curves;

    /** Animation manager. */
    AnimationManager         *m_animation_manager;

    /** Checkline manager. */
    CheckManager             *m_check_manager;

    void loadTrackInfo();
    void itemCommand(const Vec3 &xyz, Item::ItemType item_type, 
                     bool drop);
    void loadQuadGraph(unsigned int mode_id);
    void convertTrackToBullet(const scene::ISceneNode *node);
    bool loadMainTrack(const XMLNode &node);
    void createWater(const XMLNode &node);
    void getMusicInformation(std::vector<std::string>&  filenames, 
                             std::vector<MusicInformation*>& m_music   );
    void loadCurves(const XMLNode &node);
    void handleAnimatedTextures(scene::ISceneNode *node, const XMLNode &xml);
    void handleSky(const XMLNode &root, const std::string &filename);

public:

    static const float NOHIT;

                       Track             (std::string filename);
                      ~Track             ();
    bool               isArena           () const { return m_is_arena; }
    void               cleanup           ();
    /** Returns the texture with the mini map for this track. */
    const video::ITexture*getMiniMap     () const { return m_mini_map; }
    const Vec3&        trackToSpatial    (const int SECTOR) const;
    void               loadTrackModel    (World* parent, unsigned int mode_id=0);
    void               addMusic          (MusicInformation* mi)
                                                  {m_music.push_back(mi);       }
    float              getGravity        () const {return m_gravity;            }

    /** Returns the version of the .track file. */
    int                getVersion        () const {return m_version;            }

    /** Returns the length of the main driveline. */
    float              getTrackLength    () const {return m_quad_graph->getLapLength(); }

    /** Returns a unique identifier for this track (the directory name). */
    const std::string& getIdent          () const {return m_ident;              }

    /** Returns the name of the track, which is e.g. displayed on the screen. */
    const irr::core::stringw& getName           () const {return m_name;               }

    /** Returns all groups this track belongs to. */
    const std::vector<std::string>
                       getGroups         () const {return m_groups;             }

    /** Starts the music for this track. */
    void               startMusic        () const;

    /** Returns the filename of this track. */
    const std::string& getFilename       () const {return m_filename;           }

    //const std::string& getDescription    () const {return m_description;        }
    const std::string& getDesigner       () const {return m_designer;           }
    
    /** Returns an absolute path to the screenshot file of this track */
    const std::string& getScreenshotFile () const {return m_screenshot;         }
    
    btTransform        getStartTransform (unsigned int pos) const;
    void               getTerrainInfo(const Vec3 &pos, float *hot, Vec3* normal,
                                      const Material **material) const;
    float              getTerrainHeight(const Vec3 &pos) const;
    void               createPhysicsModel(unsigned int main_track_count);
    void               update(float dt);
    void               reset();
    void               adjustForFog(scene::ISceneNode *node);
    void               setStartCoordinates(const core::line2df& line);
    void               handleExplosion(const Vec3 &pos, const PhysicalObject *mp) const;
    /** Sets pointer to the aabb of this track. */
    void               getAABB(const Vec3 **min, const Vec3 **max) const
                       { *min = &m_aabb_min; *max = &m_aabb_max; }
    /** Returns the graph of quads, mainly for the AI. */
    QuadGraph&         getQuadGraph() const { return *m_quad_graph; }

    /** Returns 'a' angle for quad n. This angle is used to position a kart
     *  after a rescue, and to detect wrong directions. This function will
     *  always return the angle towards the first successor, i.e. the angle
     *  in the direction of the default way on the track.
     *  \param n Number of the quad for which the angle is asked. 
     */
    float              getAngle(int n) const 
                                { return m_quad_graph->getAngleToNext(n, 0);    }
    /** Returns the 2d coordinates of a point when drawn on the mini map 
     *  texture.
     *  \param xyz Coordinates of the point to map.
     *  \param draw_at The coordinates in pixel on the mini map of the point,
     *         only the first two coordinates will be used.
     */
    void               mapPoint2MiniMap(const Vec3 &xyz, Vec3 *draw_at) const
                                { m_quad_graph->mapPoint2MiniMap(xyz, draw_at); }
    /** Returns the full path of a given file inside this track directory. */
    std::string        getTrackFile(const std::string &s) const 
                                { return m_root+"/"+s; }
    /** Returns the number of modes available for this track. */
    unsigned int       getNumberOfModes() const { return m_all_modes.size();  }
    /** Returns the name of the i-th. mode. */
    const std::string &getModeName(unsigned int i) const 
                                                { return m_all_modes[i].m_name;}
    /** Returns the default ambient color. */
    const video::SColor &getDefaultAmbientColor() const
                                                { return m_default_ambient_color;}
    /** Sets the current ambient color for a kart with index k. */
    void   setAmbientColor(const video::SColor &color,
                           unsigned int k);
    /** Returns the far value for cameras. */
    float  getCameraFar() const { return m_camera_far; }

    /** Get the number of start positions defined in the scene file. */
    unsigned int getNumberOfStartPositions() const 
                                           { return m_start_positions.size(); }
    /** Returns the i-th. start position. */
    const Vec3 &getStartPosition(unsigned int i) {return m_start_positions[i];}
    /** Returns the heading of the i-th. start position. */
    const float getStartHeading(unsigned int i) {return m_start_heading[i]; }
};   // class Track

#endif
