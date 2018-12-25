//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

/*! \file smooth_network_body.hpp
 *  \brief This class help to smooth the graphicial transformation of network
 *  controlled object. In case there is any difference between server and
 *  client predicted values, instead of showing the server one immediately,
 *  it will interpolate between them with an extrapolated value from the old
 *  predicted values stored in m_adjust_control_point estimated by current
 *  speed of object.
 */

#ifndef HEADER_SMOOTH_NETWORK_BODY_HPP
#define HEADER_SMOOTH_NETWORK_BODY_HPP

#include "utils/log.hpp"
#include "utils/vec3.hpp"

#include "LinearMath/btTransform.h"

#include <utility>

class SmoothNetworkBody
{
private:
    enum SmoothingState
    {
        SS_NONE = 0,
        SS_TO_ADJUST,
        SS_TO_REAL
    };

    /** Client prediction in networked games might cause the visual
     *  and physical position to be different. For visual smoothing
     *  these variable accumulate the error and reduces it over time. */
    std::pair<Vec3, btQuaternion> m_start_smoothing_postion,
        m_adjust_position;

    Vec3 m_adjust_control_point;

    std::pair<btTransform, Vec3> m_prev_position_data;

    btTransform m_smoothed_transform;

    float m_adjust_time, m_adjust_time_dt;

    SmoothingState m_smoothing;

    bool m_enabled;

    bool m_smooth_rotation;

    bool m_adjust_vertical_offset;

    float m_min_adjust_length, m_max_adjust_length, m_min_adjust_speed,
        m_max_adjust_time, m_adjust_length_threshold;

public:
    SmoothNetworkBody(bool enable = false);
    // ------------------------------------------------------------------------
    virtual ~SmoothNetworkBody() {}
    // ------------------------------------------------------------------------
    void reset()
    {
        m_smoothed_transform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
        m_start_smoothing_postion = m_adjust_position =
            std::make_pair(Vec3(0.0f, 0.0f, 0.0f),
            btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
        m_prev_position_data = std::make_pair(m_smoothed_transform, Vec3());
        m_smoothing = SS_NONE;
        m_adjust_time = m_adjust_time_dt = 0.0f;
    }
    // ------------------------------------------------------------------------
    void setEnable(bool val)                               { m_enabled = val; }
    // ------------------------------------------------------------------------
    bool isEnabled() const                                { return m_enabled; }
    // ------------------------------------------------------------------------
    void setSmoothRotation(bool val)               { m_smooth_rotation = val; }
    // ------------------------------------------------------------------------
    void setAdjustVerticalOffset(bool val)  { m_adjust_vertical_offset = val; }
    // ------------------------------------------------------------------------
    void prepareSmoothing(const btTransform& current_transform,
                          const Vec3& current_velocity);
    // ------------------------------------------------------------------------
    void checkSmoothing(const btTransform& current_transform,
                        const Vec3& current_velocity);
    // ------------------------------------------------------------------------
    void updateSmoothedGraphics(const btTransform& current_transform,
                                const Vec3& current_velocity,
                                float dt);
    // ------------------------------------------------------------------------
    void setSmoothedTransform(const btTransform& t)
                                                  { m_smoothed_transform = t; }
    // ------------------------------------------------------------------------
    const btTransform& getSmoothedTrans() const
                                               { return m_smoothed_transform; }
    // ------------------------------------------------------------------------
    const Vec3& getSmoothedXYZ() const
                            { return (Vec3&)m_smoothed_transform.getOrigin(); }
    // ------------------------------------------------------------------------
    void setMinAdjustLength(float val)           { m_min_adjust_length = val; }
    // ------------------------------------------------------------------------
    void setMaxAdjustLength(float val)           { m_max_adjust_length = val; }
    // ------------------------------------------------------------------------
    void setMinAdjustSpeed(float val)             { m_min_adjust_speed = val; }
    // ------------------------------------------------------------------------
    void setMaxAdjustTime(float val)               { m_max_adjust_time = val; }
    // ------------------------------------------------------------------------
    void setAdjustLengthThreshold(float val)
                                           { m_adjust_length_threshold = val; }

};

#endif // HEADER_SMOOTH_NETWORK_BODY_HPP
