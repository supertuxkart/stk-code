//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_CAMERA_HPP
#define HEADER_CAMERA_HPP

#include <vector>

#include <SColor.h>
#include <vector2d.h>
#include <rect.h>
namespace irr
{
    namespace scene { class ICameraSceneNode; }
}
using namespace irr;

#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"
#include "utils/aligned_array.hpp"
#include "utils/vec3.hpp"

class Kart;

/**
  * \brief Handles the game camera
  * \ingroup graphics
  */
class Camera : public NoCopy
{
public:
    enum Mode {
        CM_NORMAL,        //!< Normal camera mode
        CM_CLOSEUP,       //!< Closer to kart
        CM_REVERSE,       //!< Looking backwards
        CM_LEADER_MODE,   //!< for deleted player karts in follow the leader
        CM_FINAL,         //!< Final camera
        CM_SIMPLE_REPLAY,
        CM_FALLING
    };

    enum Style {
        CS_MODERN,  //!< Flexible link between kart and camera
        CS_CLASSIC, //!< Fixed position style, like STK v0.6
    };

private:
    /** The camera scene node. */
    scene::ICameraSceneNode *m_camera;

    /** Camera's mode. */
    Mode            m_mode;

    /** The index of this camera which is the index of the kart it is 
     *  attached to. */
    unsigned int    m_index;

    /** Current ambient light for this camera. */
    video::SColor   m_ambient_light;

    /** Distance between the camera and the kart. */
    float           m_distance;

    /** The speed at which the camera changes position. */
    float           m_position_speed;

    /** The speed at which the camera target changes position. */
    float           m_target_speed;

    /** Factor of the effects of steering in camera aim. */
    float           m_rotation_range;

    /** The kart that the camera follows. It can't be const,
     *  since in profile mode the camera might change its owner. */
    Kart           *m_kart;

    /** The list of viewports for this cameras. */
    core::recti     m_viewport;

    /** The scaling necessary for each axis. */
    core::vector2df m_scaling;

    /** Field of view for the camera. */
    float           m_fov;

    /** Aspect ratio for camera. */
    float           m_aspect;

    /** Linear velocity of the camera, only used for end camera. */
    core::vector3df m_lin_velocity;

    /** Velocity of the target of the camera, only used for end camera. */
    core::vector3df m_target_velocity;

    /* Whether we should use the pre-0.7 camera style or the
     * modern style. Should default to modern. */
    Style           m_camera_style;
    
    /** A class that stores information about the different end cameras
     *  which can be specified in the scene.xml file. */
    class EndCameraInformation
    {
    public:
        /** The camera type:
            EC_STATIC_FOLLOW_KART A static camera that always points at the 
                                  kart.
            EC_AHEAD_OF_KART      A camera that flies ahead of the kart 
                                  always pointing at the kart.
        */
        typedef enum {EC_STATIC_FOLLOW_KART,
                      EC_AHEAD_OF_KART} EndCameraType;
        EndCameraType m_type;

        /** Position of the end camera. */
        Vec3    m_position;  

        /** Distance to kart by which this camera is activated. */
        float   m_distance2;

        /** Reads end camera information from XML. Returns false if an
         *  error occurred.
         *  \param node XML Node with the end camera information. */
        bool    readXML(const XMLNode &node)
        {
            std::string s;
            node.get("type", &s);
            if(s=="static_follow_kart")
                m_type = EC_STATIC_FOLLOW_KART;
            else if(s=="ahead_of_kart")
                m_type = EC_AHEAD_OF_KART;
            else
            {
                fprintf(stderr, 
                        "Invalid camera type '%s' - camera is ignored.\n",
                        s.c_str());
                 return false;
            }
            node.get("xyz", &m_position);
            node.get("distance", &m_distance2);
            // Store the squared value
            m_distance2 *= m_distance2;
            return true;
        }   // readXML
        // --------------------------------------------------------------------
        /** Returns true if the specified position is close enough to this
         *  camera, so that this camera should become the next end camera.
         *  \param xyz Position to test for distance.
         *  \returns True if xyz is close enough to this camera.
         */
        bool    isReached(const Vec3 &xyz) 
                { return (xyz-m_position).length2() < m_distance2; }
    };   // EndCameraInformation
    // ------------------------------------------------------------------------

    /** List of all end camera information. This information is shared 
     *  between all cameras, so it's static. */
    static AlignedArray<EndCameraInformation> m_end_cameras;

    /** Index of the current end camera. */
    unsigned int m_current_end_camera;

    /** The next end camera to be activated. */
    unsigned int  m_next_end_camera;

    void setupCamera();
    void smoothMoveCamera(float dt, const Vec3 &wanted_position,
                          const Vec3 &wanted_target);
    void computeNormalCameraPosition(Vec3 *wanted_position,
                                     Vec3 *wanted_target);
    void handleEndCamera(float dt);
    void getCameraSettings(float *above_kart, float *cam_angle, 
                           float *side_way, float *distance,
                           bool *smoothing);
    void positionCamera(float dt, float above_kart, float cam_angle,
                        float side_way, float distance, float smoothing);
public:
         Camera            (int camera_index, Kart* kart);
        ~Camera            ();
    static void readEndCamera(const XMLNode &root);
    static void clearEndCameras();
    void setMode           (Mode mode_);    /** Set the camera to the given mode */
    Mode getMode();
    /** Returns the camera index (or player kart index, which is the same). */
    int  getIndex() const  {return m_index;}
    void reset             ();
    void setInitialTransform();
    void activate();
    void update            (float dt);
    void changeOwner       (Kart *new_kart);

    // ------------------------------------------------------------------------
    /** Sets the ambient light for this camera. */
    void setAmbientLight(const video::SColor &color) { m_ambient_light=color; }

    // ------------------------------------------------------------------------
    /** Returns the current ambient light. */
    const video::SColor &getAmbientLight() const {return m_ambient_light; }

    // ------------------------------------------------------------------------
    /** Returns the viewport of this camera. */
    const core::recti& getViewport() const {return m_viewport; }

    // ------------------------------------------------------------------------
    /** Returns the scaling in x/y direction for this camera. */
    const core::vector2df& getScaling() const {return m_scaling; }
    
    // ------------------------------------------------------------------------
    /** Returns the camera scene node. */
    scene::ICameraSceneNode *getCameraSceneNode() { return m_camera; }
} ;

#endif

/* EOF */
