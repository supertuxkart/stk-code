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

class AbstractKart;

/**
  * \brief This is the base class for all cameras. It also includes some
  *  static functios to keep track of all cameras (e.g. a static function to
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

    /* Only used for the normal camera. */
    enum Mode
    {
        CM_NORMAL,            //!< Normal camera mode
        CM_CLOSEUP,           //!< Closer to kart
        CM_REVERSE,           //!< Looking backwards
        CM_LEADER_MODE,       //!< for deleted player karts in follow the leader
        CM_SIMPLE_REPLAY,
        CM_FALLING
    };   // Mode


private:
    static Camera* s_active_camera;

    /** The project-view matrix of the previous frame, used for the blur shader. */
    core::matrix4 m_previous_pv_matrix;

    /** Camera's mode. */
    Mode            m_mode;

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
    AbstractKart   *m_original_kart;

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

    void setupCamera();

protected:
    /** The camera scene node. */
    scene::ICameraSceneNode *m_camera;

    /** The kart that the camera follows. It can't be const,
    *  since in profile mode the camera might change its owner.
    *  May be NULL (example: cutscene camera)
    */
    AbstractKart   *m_kart;

    static Camera* createCamera(unsigned int index, CameraType type,
                                AbstractKart* kart);

             Camera(CameraType type, int camera_index, AbstractKart* kart);
    virtual ~Camera();
    virtual void reset();
public:
    LEAK_CHECK()

    // ========================================================================
    // Static functions
    static Camera* createCamera(AbstractKart* kart);
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
    /** Remove all cameras. */
    static void removeAllCameras()
    {
        for(unsigned int i=0; i<m_all_cameras.size(); i++)
            delete m_all_cameras[i];
        m_all_cameras.clear();
    }   // removeAllCameras

    // ========================================================================

    void setMode(Mode mode);    /** Set the camera to the given mode */
    Mode getMode();
    void setKart(AbstractKart *new_kart);
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
    const AbstractKart* getKart() const { return m_kart; }

    // ------------------------------------------------------------------------
    /** Returns the kart to which this camera is attached. */
    AbstractKart* getKart() { return m_kart; }

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
};   // class Camera

#endif

/* EOF */
