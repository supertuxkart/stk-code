//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 SuperTuxKart-Team, Steve Baker
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

#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"
#include "utils/aligned_array.hpp"
#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/vec3.hpp"

#include "matrix4.h"
#include "rect.h"
#include "SColor.h"
#include "vector2d.h"

#include <vector>

#include "ICameraSceneNode.h"

class Kart;

/**
  * \brief This is the base class for all cameras. It also includes some
  *  static functions to keep track of all cameras (e.g. a static function to
  *  create a camera, get a camera with a specified index).
  * \ingroup graphics
  */
class Camera : public NoCopy
{
public:
    /** The different camera types that can be used. */
    enum CameraType
    {
        CM_TYPE_NORMAL,
        CM_TYPE_DEBUG,         //!< A debug camera.
        CM_TYPE_FPS,           //!< FPS Camera
        CM_TYPE_END            //!< End camera
    };   // CameraType

    // Quick method to tell the difference between cameras meant to
    // be used by a player and those exclusively used for spectating.
#define PLAYER_CAMERA(ID) (1000+ID)
#define SPECTATING_CAMERA(ID) (2000+ID)

    /* Only used for the normal camera. */
    enum Mode
    {
        CM_NORMAL             = PLAYER_CAMERA(0),     // Normal camera mode
        CM_CLOSEUP            = PLAYER_CAMERA(1),     // Closer to the kart
        CM_REVERSE            = PLAYER_CAMERA(2),     // Looking backwards
        CM_LEADER_MODE        = PLAYER_CAMERA(3),     // for deleted player karts in follow the leader
        CM_FALLING            = PLAYER_CAMERA(4),     // Used when a kart is detected as falling off-track
        CM_SPECTATOR_SOCCER   = SPECTATING_CAMERA(0), // Camera oriented towards the soccer ball
        CM_SPECTATOR_TOP_VIEW = SPECTATING_CAMERA(1), // Top view of the ball if soccer or top view on kart
        CM_SPECTATOR_TV       = SPECTATING_CAMERA(2), // Cameras placed in the scene (soccer), always follow ball
        CM_SIMPLE_REPLAY, // Currently unused
    };   // Mode


private:
    static Camera* s_active_camera;

    /** The project-view matrix of the previous frame, used for the blur shader. */
    core::matrix4 m_previous_pv_matrix;

    /** Camera's mode. */
    Mode            m_mode;
    Mode            m_last_non_spectating_mode;

    /** The type of the camera. */
    CameraType      m_type;

    /** The default type for any newly created camera. Used to store command
     *  line parameters. */
    static CameraType m_default_type;

    /** The index of this camera which is the index of the kart it is
     *  attached to. */
    unsigned int    m_index;

    /** Current ambient light for this camera. */
    video::SColor   m_ambient_light;

    /** A pointer to the original kart the camera was pointing at when it
     *  was created. Used when restarting a race (since the camera might
     *  get attached to another kart if a kart is elimiated). */
    Kart   *m_original_kart;

    /** The viewport for this camera (portion of the game window covered by this camera) */
    core::recti     m_viewport;

    /** The scaling necessary for each axis. */
    core::vector2df m_scaling;

    /** Field of view for the camera. */
    float           m_fov;

    /** Aspect ratio for camera. */
    float           m_aspect;


    /** List of all cameras. */
    static std::vector<Camera*> m_all_cameras;

protected:
    /** The camera scene node. */
    scene::ICameraSceneNode *m_camera;

    /** The kart that the camera follows. It can't be const,
    *  since in profile mode the camera might change its owner.
    *  May be NULL (example: cutscene camera)
    */
    Kart   *m_kart;

    // ========================================================================
    // Camera shake effect state
    // ========================================================================
    /** Current shake intensity magnitude [0.0, 1.0]. */
    float m_shake_intensity;

    /** Remaining shake duration in seconds. */
    float m_shake_duration;

    /** Initial shake duration for normalized decay calculation. */
    float m_shake_initial_duration;

    /** Shake oscillation frequency (higher = faster shake). */
    float m_shake_frequency;

    /** Current frame's shake offset applied to camera position. */
    Vec3 m_shake_offset;

    /** Accumulated shake time for sine wave generation (per-camera instance). */
    float m_shake_time;

    // ========================================================================
    // Dynamic FOV effect state
    // ========================================================================
    /** Base FOV from configuration (in radians). */
    float m_base_fov;

    /** Current smoothly-interpolated FOV (in radians). */
    float m_current_fov;

    /** Target FOV based on speed (in radians). */
    float m_target_fov;

    static Camera* createCamera(unsigned int index, CameraType type,
                                Kart* kart);

             Camera(CameraType type, int camera_index, Kart* kart);
    virtual ~Camera();
    virtual void reset();
public:
    LEAK_CHECK()

    // ========================================================================
    // Static functions
    static Camera* createCamera(Kart* kart, const int index);
    static void resetAllCameras();
    static void changeCamera(unsigned int camera_index, CameraType type);

    // ------------------------------------------------------------------------
    /** Sets the default type for each camera that will be created. Used for
     *  command line parameters to select a debug etc camera. */
    static void setDefaultCameraType(CameraType type) { m_default_type = type;}
    // ------------------------------------------------------------------------
    /** Returns the default type for each camera that will be created. Used 
     *  for command line parameters to select a debug etc camera. */
    static CameraType getDefaultCameraType() { return m_default_type;}
    // ------------------------------------------------------------------------
    /** Returns the number of cameras used. */
    static unsigned int getNumCameras()
    {
        return (unsigned int)m_all_cameras.size(); 
    }   // getNumCameras
    // ------------------------------------------------------------------------
    /** Returns a camera. */
    static Camera *getCamera(unsigned int n) { return m_all_cameras[n]; }
    // ------------------------------------------------------------------------
    /** Returns the currently active camera. */
    static Camera* getActiveCamera() { return s_active_camera; }
    // ------------------------------------------------------------------------
    /** Remove all cameras and clean up observer registrations. */
    static void removeAllCameras();   // Implemented in camera.cpp

    // ========================================================================

    // ------------------------------------------------------------------------
    Mode getMode() const { return m_mode; }
    // ------------------------------------------------------------------------
    /** Returns true if the camera is a spectator-only camera.
     *  The enum ID of the mode tells us if it's a spectator camera or not. */
    bool isSpectatorMode() const
    {
        const int id = (int)m_mode;
        if(id >= 2000 && id < 3000)
            return true;
        else
            return false;
    }   // isSpectatorMode
    // ------------------------------------------------------------------------
    void setMode(Mode mode);    /** Set the camera to the given mode */
    void setNextSpectatorMode();
    void setKart(Kart *new_kart);
    virtual void setInitialTransform();
    virtual void activate(bool alsoActivateInIrrlicht=true);
    virtual void update(float dt);
    // ------------------------------------------------------------------------
    /** Returns the type of this camera. */
    CameraType getType() { return m_type; }
    // ------------------------------------------------------------------------
    /** Sets the field of view for the irrlicht camera. */
    void setFoV() { m_camera->setFOV(m_fov); }
    // ------------------------------------------------------------------------
    /** Returns the camera index (or player kart index, which is the same). */
    int  getIndex() const  {return m_index;}
    // ------------------------------------------------------------------------
    /** Returns the project-view matrix of the previous frame. */
    core::matrix4 getPreviousPVMatrix() const { return m_previous_pv_matrix; }

    // ------------------------------------------------------------------------
    /** Returns the project-view matrix of the previous frame. */
    void setPreviousPVMatrix(core::matrix4 mat) { m_previous_pv_matrix = mat; }

    // ------------------------------------------------------------------------
    /** Returns the kart to which this camera is attached. */
    const Kart* getKart() const { return m_kart; }

    // ------------------------------------------------------------------------
    /** Returns the kart to which this camera is attached. */
    Kart* getKart() { return m_kart; }

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
    // ------------------------------------------------------------------------
    /** Returs the absolute position of the camera. */
    Vec3 getXYZ() { return Vec3(m_camera->getPosition()); }
    // ------------------------------------------------------------------------
    void setupCamera();

    // ========================================================================
    // Camera shake methods
    // ========================================================================
    /** Trigger a camera shake effect.
     *  \param intensity Shake magnitude [0.0, 1.0], will be clamped.
     *  \param duration How long the shake lasts in seconds.
     *  \param frequency Oscillation speed (default 25.0f).
     */
    void triggerShake(float intensity, float duration, float frequency = 25.0f);

    /** Update shake state and compute current frame's offset.
     *  \param dt Delta time since last frame.
     */
    void updateShake(float dt);

    /** Returns the current shake offset to be applied to camera position. */
    const Vec3& getShakeOffset() const { return m_shake_offset; }

    /** Returns true if camera is currently shaking. */
    bool isShaking() const { return m_shake_duration > 0.0f; }

    // ========================================================================
    // Dynamic FOV methods
    // ========================================================================

    /** Update dynamic FOV based on speed and boost state.
     *  \param dt Delta time since last frame.
     *  \param speed_ratio Current speed as ratio of max speed [0.0, 1.0+].
     *  \param boost_active True if boost (nitro/zipper) is active.
     */
    void updateDynamicFOV(float dt, float speed_ratio, bool boost_active);

    /** Returns the current dynamic FOV in radians. */
    float getCurrentFOV() const { return m_current_fov; }
};   // class Camera

#endif

/* EOF */
