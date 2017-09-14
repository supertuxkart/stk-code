//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2016 Joerg Henrichs
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

#include "graphics/camera_end.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "tracks/drive_graph.hpp"

#include "ICameraSceneNode.h"

AlignedArray<CameraEnd::EndCameraInformation> CameraEnd::m_end_cameras;
// ============================================================================
CameraEnd::CameraEnd(int camera_index, AbstractKart* kart) 
         : CameraNormal(Camera::CM_TYPE_END, camera_index, kart)
{
    reset();
    if(m_end_cameras.size()>0)
        m_camera->setPosition(m_end_cameras[0].m_position.toIrrVector());
    m_next_end_camera    = m_end_cameras.size()>1 ? 1 : 0;
    m_current_end_camera = 0;
    setFoV();
    update(0);
}   // Camera

//-----------------------------------------------------------------------------
/** This function clears all end camera data structure. This is necessary
 *  since all end cameras are shared between all camera instances (i.e. are
 *  static), otherwise (if no end camera is defined for a track) the old
 *  end camera structure would be used.
 */
void CameraEnd::clearEndCameras()
{
    m_end_cameras.clear();
}   // clearEndCameras

//-----------------------------------------------------------------------------
/** Reads the information about the end camera. This information is shared
 *  between all cameras, so this is a static function.
 *  \param node The XML node containing all end camera informations
 */
void CameraEnd::readEndCamera(const XMLNode &root)
{
    m_end_cameras.clear();
    for(unsigned int i=0; i<root.getNumNodes(); i++)
    {
        unsigned int index = i;
        // In reverse mode, reverse the order in which the
        // end cameras are read.
        if(DriveGraph::get() != NULL && DriveGraph::get()->isReverse())
            index = root.getNumNodes() - 1 - i;
        const XMLNode *node = root.getNode(index);
        EndCameraInformation eci;
        if(!eci.readXML(*node)) continue;
        m_end_cameras.push_back(eci);
    }   // for i<getNumNodes()
}   // readEndCamera


//-----------------------------------------------------------------------------
/** Called once per time frame to move the camera to the right position.
 * This function handles the end camera. It adjusts the camera position
 *  according to the current camera type, and checks if a switch to the
 *  next camera should be made.
 *  \param dt Time step size.
 */
void CameraEnd::update(float dt)
{
    Camera::update(dt);
    m_camera->setNearValue(1.0f);

    // First test if the kart is close enough to the next end camera, and
    // if so activate it.
    if( m_end_cameras.size()>0 &&
        m_end_cameras[m_next_end_camera].isReached(m_kart->getXYZ()))
    {
        m_current_end_camera = m_next_end_camera;
        if(m_end_cameras[m_current_end_camera].m_type
            ==EndCameraInformation::EC_STATIC_FOLLOW_KART)
        {
            m_camera->setPosition(
                m_end_cameras[m_current_end_camera].m_position.toIrrVector()
                );
        }
        setFoV();
        m_next_end_camera++;
        if(m_next_end_camera>=(unsigned)m_end_cameras.size())
            m_next_end_camera = 0;
    }

    EndCameraInformation::EndCameraType info
        = m_end_cameras.size()==0 ? EndCameraInformation::EC_AHEAD_OF_KART
                                  : m_end_cameras[m_current_end_camera].m_type;

    switch(info)
    {
    case EndCameraInformation::EC_STATIC_FOLLOW_KART:
        {
            // Since the camera has no parents, we can use the relative
            // position here (otherwise we need to call updateAbsolutePosition
            // after changing the relative position in order to get the right
            // position here).
            const core::vector3df &cp = m_camera->getPosition();
            const Vec3            &kp = m_kart->getXYZ();
            // Estimate the fov, assuming that the vector from the camera to
            // the kart and the kart length are orthogonal to each other
            // --> tan (fov) = kart_length / camera_kart_distance
            // In order to show a little bit of the surrounding of the kart
            // the kart length is multiplied by 6 (experimentally found)
            float fov = 6*atan2(m_kart->getKartLength(),
                                (cp-kp.toIrrVector()).getLength());
            m_camera->setFOV(fov);
            m_camera->setTarget(m_kart->getXYZ().toIrrVector());
            break;
        }
    case EndCameraInformation::EC_AHEAD_OF_KART:
        {
            float cam_angle = m_kart->getKartProperties()->getCameraBackwardUpAngle()
                            * DEGREE_TO_RAD;

            positionCamera(dt, /*above_kart*/0.75f,
                           cam_angle, /*side_way*/0,
                           2.0f*getDistanceToKart(), /*smoothing*/false);
            break;
        }
    default: break;
    }   // switch

}   // update
