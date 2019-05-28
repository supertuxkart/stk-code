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

#include "network/smooth_network_body.hpp"
#include "config/stk_config.hpp"

#include <algorithm>

// ----------------------------------------------------------------------------
SmoothNetworkBody::SmoothNetworkBody(bool enable)
{
    reset();
    m_enabled = enable;
    m_smooth_rotation = true;
    m_adjust_vertical_offset = true;
    m_min_adjust_length = stk_config->m_snb_min_adjust_length;
    m_max_adjust_length = stk_config->m_snb_max_adjust_length;
    m_min_adjust_speed = stk_config->m_snb_min_adjust_speed;
    m_max_adjust_time = stk_config->m_snb_max_adjust_time;
    m_adjust_length_threshold = stk_config->m_snb_adjust_length_threshold;
}   // SmoothNetworkBody

// ----------------------------------------------------------------------------
void SmoothNetworkBody::prepareSmoothing(const btTransform& current_transform,
                                         const Vec3& current_velocity)
{
#ifndef SERVER_ONLY
    // Continuous smooth enabled
    //if (m_smoothing != SS_NONE)
    //    return;

    m_prev_position_data = std::make_pair(current_transform,
        current_velocity);
#endif
}   // prepareSmoothing

// ----------------------------------------------------------------------------
/** Adds a new error between graphical and physical position/rotation. Called
 * in case of a rewind to allow to for smoothing the visuals in case of
 * incorrect client prediction.
 */
void SmoothNetworkBody::checkSmoothing(const btTransform& current_transform,
                                       const Vec3& current_velocity)
{
#ifndef SERVER_ONLY
    // Continuous smooth enabled
    //if (m_smoothing != SS_NONE)
    //    return;

    float adjust_length = (current_transform.getOrigin() -
        m_prev_position_data.first.getOrigin()).length();
    if (adjust_length < m_min_adjust_length ||
        adjust_length > m_max_adjust_length)
        return;

    float speed = m_prev_position_data.second.length();
    speed = std::max(speed, current_velocity.length());
    if (speed < m_min_adjust_speed)
        return;

    float adjust_time = (adjust_length * m_adjust_length_threshold) / speed;
    if (adjust_time > m_max_adjust_time)
        return;

    m_start_smoothing_postion.first = m_smoothing == SS_NONE ?
        m_prev_position_data.first.getOrigin() :
        m_smoothed_transform.getOrigin();
    m_start_smoothing_postion.second = m_smoothing == SS_NONE ?
        m_prev_position_data.first.getRotation() :
        m_smoothed_transform.getRotation();
    m_start_smoothing_postion.second.normalize();

    m_smoothing = SS_TO_ADJUST;
    m_adjust_time_dt = 0.0f;
    m_adjust_time = adjust_time;

    m_adjust_control_point = m_start_smoothing_postion.first +
        m_prev_position_data.second * m_adjust_time;
    Vec3 p2 = current_transform.getOrigin() + current_velocity * m_adjust_time;

    m_adjust_position.first.setInterpolate3(m_adjust_control_point, p2, 0.5f);
    m_adjust_position.second = current_transform.getRotation();
    m_adjust_position.second.normalize();
#endif
}   // checkSmoothing

// ------------------------------------------------------------------------
void SmoothNetworkBody::updateSmoothedGraphics(
    const btTransform& current_transform, const Vec3& current_velocity,
    float dt)
{
#ifndef SERVER_ONLY
    Vec3 cur_xyz = current_transform.getOrigin();
    btQuaternion cur_rot = current_transform.getRotation();

    if (!m_enabled)
    {
        m_smoothed_transform.setOrigin(cur_xyz);
        m_smoothed_transform.setRotation(cur_rot);
        return;
    }

    float ratio = 0.0f;
    if (m_smoothing != SS_NONE)
    {
        float adjust_time_dt = m_adjust_time_dt + dt;
        ratio = adjust_time_dt / m_adjust_time;
        if (ratio > 1.0f)
        {
            ratio -= 1.0f;
            m_adjust_time_dt = adjust_time_dt - m_adjust_time;
            if (m_smoothing == SS_TO_ADJUST)
            {
                m_smoothing = SS_TO_REAL;
                m_adjust_control_point = m_adjust_position.first +
                    current_velocity * m_adjust_time;
            }
            else
                m_smoothing = SS_NONE;
        }
        else
            m_adjust_time_dt = adjust_time_dt;
    }

    assert(m_adjust_time_dt >= 0.0f);
    assert(ratio >= 0.0f);
    if (m_smoothing == SS_TO_ADJUST)
    {
        cur_xyz.setInterpolate3(m_start_smoothing_postion.first,
            m_adjust_position.first, ratio);
        Vec3 to_control;
        to_control.setInterpolate3(m_start_smoothing_postion.first,
            m_adjust_control_point, ratio);
        cur_xyz.setInterpolate3(cur_xyz, to_control, 1.0f - ratio);
        if (m_smooth_rotation)
        {
            cur_rot = m_start_smoothing_postion.second;
            if (dot(cur_rot, m_adjust_position.second) < 0.0f)
                cur_rot = -cur_rot;
            cur_rot = cur_rot.slerp(m_adjust_position.second, ratio);
        }
    }
    else if (m_smoothing == SS_TO_REAL)
    {
        Vec3 to_control;
        to_control.setInterpolate3(m_adjust_position.first,
            m_adjust_control_point, ratio);
        float ratio_sqrt = sqrtf(ratio);
        cur_xyz.setInterpolate3(m_adjust_position.first, cur_xyz, ratio_sqrt);
        cur_xyz.setInterpolate3(to_control, cur_xyz, ratio);
        if (m_smooth_rotation)
        {
            cur_rot.normalize();
            if (dot(cur_rot, m_adjust_position.second) < 0.0f)
                cur_rot = -cur_rot;
            cur_rot = cur_rot.slerp(m_adjust_position.second, 1.0f - ratio);
        }
    }

    m_smoothed_transform.setOrigin(cur_xyz);
    m_smoothed_transform.setRotation(cur_rot);

    if (m_adjust_vertical_offset && m_smoothing != SS_NONE)
    {
        Vec3 lc = current_transform.inverse()(cur_xyz);
        // Adjust vertical position for up/down-sloping
        cur_xyz = m_smoothed_transform(Vec3(0.0f, -lc.y(), 0.0f));
        m_smoothed_transform.setOrigin(cur_xyz);
    }
#endif
}   // updateSmoothedGraphics
