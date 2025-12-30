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

#ifndef HEADER_CAMERA_NORMAL_HPP
#define HEADER_CAMERA_NORMAL_HPP

#include "graphics/camera/camera.hpp"
#include "karts/boost_observer.hpp"
#include "karts/crash_observer.hpp"

#include "utils/cpp2011.hpp"

class Kart;
class Material;

/**
  * \brief Handles the normal racing camera
  * \ingroup graphics
  * Also implements IBoostObserver to receive boost activation events
  * for triggering speed lines shader + camera pull-back effect.
  * Also implements ICrashObserver for camera shake on collisions.
  */
class CameraNormal : public Camera, public IBoostObserver, public ICrashObserver
{

private:

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

    Vec3            m_camera_offset;

    Mode            m_last_smooth_mode;

    void moveCamera(float dt, bool smooth, float cam_angle, float distance);

    void getCameraSettings(Mode mode,
                           float *above_kart, float *cam_angle,
                           float *side_way, float *distance,
                           bool *smoothing, float *cam_roll_angle);
    
    void positionCamera(float dt, float above_kart, float cam_angle,
                        float side_way, float distance, float smoothing,
                        float cam_roll_angle);

    btVector3 m_kart_position;
    btQuaternion m_kart_rotation;

    // TV spectator cameras defined by track designer (soccer). Always follow ball.
    static std::vector<Vec3> m_tv_cameras;

    // TV camera selection smoothing
    int   m_tv_current_index = -1;
    float m_tv_switch_cooldown = 0.0f; // seconds remaining before next switch
    static float m_tv_min_delta2;       // required improvement (squared distance) to switch
    static float m_tv_cooldown_default; // default cooldown after a switch (seconds)

    // Speed lines effect state (triggered by boost activation events)
    float m_speed_lines_intensity = 0.0f;   // Current effect intensity (decays over time)
    float m_speed_lines_boost_intensity = 0.0f; // Boost-specific intensity for color shifting
    float m_speed_lines_timer = 0.0f;       // Remaining effect duration

    // NOTE: Edge detection (rising edge) is now handled by MaxSpeed observer pattern
    // The onBoostActivated() callback is called on activation, eliminating polling

    // Camera pull-back effect state
    float m_distance_boost = 0.0f;          // Additional distance during boost (0.0 to ~0.4)
    float m_distance_boost_timer = 0.0f;    // Decay timer for pull-back effect
    static constexpr float PULLBACK_DURATION = 1.5f;  // Effect lasts 1.5 seconds

    // Give a few classes access to the constructor (mostly for inheritance)
    friend class Camera;
    friend class CameraDebug;
    friend class CameraEnd;
             CameraNormal(Camera::CameraType type, int camera_index,
                          Kart* kart);
    virtual ~CameraNormal();
public:

    void restart();
    // ------------------------------------------------------------------------
    bool isDebug() { return false; }
    // ------------------------------------------------------------------------
    bool isFPS() { return false; }
    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Sets the ambient light for this camera. */
    void setAmbientLight(const video::SColor &color) { m_ambient_light=color; }
    // ------------------------------------------------------------------------
    void setDistanceToKart(float distance) { m_distance = distance; }
    // ------------------------------------------------------------------------
    float getDistanceToKart() const { return m_distance; }
    // ------------------------------------------------------------------------
    /** Returns the current pull-back distance boost (decays over time). */
    float getCurrentDistanceBoost() const
    {
        if (m_distance_boost_timer <= 0.0f) return 0.0f;
        float decay = m_distance_boost_timer / PULLBACK_DURATION;
        return m_distance_boost * decay * decay;  // Ease-out curve
    }
    // ------------------------------------------------------------------------
    /** Returns the current ambient light. */
    const video::SColor &getAmbientLight() const {return m_ambient_light; }
    // ------------------------------------------------------------------------
    // Load TV cameras from XML section <tv-cameras> in scene.xml
    static void readTVCameras(const XMLNode &root);
    // ------------------------------------------------------------------------
    // clean all TV camera
    static void clearTVCameras();
    // ------------------------------------------------------------------------
    // Returns true if at least one TV camera was loaded
    static bool hasTVCameras() { return !m_tv_cameras.empty(); }

    // ========================================================================
    // IBoostObserver implementation
    // ========================================================================
    /** Called by MaxSpeed when a boost activates.
     *  Triggers speed lines shader + camera pull-back for THIS camera's kart only.
     *  @param kart The kart that activated the boost
     *  @param category The boost type (MS_INCREASE_*)
     *  @param add_speed The speed increase value
     *  @param duration_ticks How long the boost lasts
     */
    void onBoostActivated(Kart* kart, unsigned int category,
                          float add_speed, int duration_ticks) override;

    // ========================================================================
    // ICrashObserver implementation
    // ========================================================================
    /** Called when a kart collides with another kart.
     *  Triggers camera shake for THIS camera's kart only.
     *  @param kart The kart that crashed
     *  @param other_kart The kart that was hit
     *  @param intensity Collision intensity [0.0, 1.0]
     */
    void onKartCrash(Kart* kart, Kart* other_kart, float intensity) override;

    /** Called when a kart collides with track/wall.
     *  Triggers camera shake for THIS camera's kart only.
     *  @param kart The kart that crashed
     *  @param material The material hit (may be NULL)
     *  @param intensity Collision intensity [0.0, 1.0]
     */
    void onTrackCrash(Kart* kart, const Material* material, float intensity) override;

};   // class CameraNormal

#endif

/* EOF */