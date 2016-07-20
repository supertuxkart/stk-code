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

#ifndef HEADER_CAMERA_END_HPP
#define HEADER_CAMERA_END_HPP

#include "graphics/camera_normal.hpp"

#include "utils/cpp2011.hpp"

/**
  * Handles the end race camera. It inherits from CameraNormal to make
  * use of the normal camera implementation of a reverse camera. 
  * \ingroup graphics
  */
class CameraEnd : public CameraNormal
{
private:
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
                Log::warn("Camera", "Invalid camera type '%s' - camera is ignored.",
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

    void handleEndCamera(float dt);

    friend class Camera;   // Give Camera access to constructor
             CameraEnd(int camera_index, AbstractKart* kart);
    virtual ~CameraEnd() {}
public:

    static void readEndCamera(const XMLNode &root);
    static void clearEndCameras();
    virtual void update(float dt) OVERRIDE;
};   // class CameraEnd

#endif

/* EOF */
