//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 the SuperTuxKart-Team
//  Copyright (C) 2026 sbilliet
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

#include "graphics/speed_streaks.hpp"
#include "karts/abstract_kart.hpp"
#include <irrMath.h>

using namespace irr;

SpeedStreaks::SpeedStreaks(const AbstractKart& kart)
    : m_kart(kart),
      m_filtered_boost_intensity(0.0f),
      m_radial_scroll_offset(0.0f),
      m_direction(Direction::FORWARD)
{
}

// Calculate the boost intensity value.
// See comments on s_boost_intensity_lower_ratio and s_boost_intensity_upper_ratio for parameters.
float SpeedStreaks::calcBoostIntensity(float lower_bound_ratio, float upper_bound_ratio) const
{
    assert(lower_bound_ratio >= 0.0f && upper_bound_ratio > lower_bound_ratio);
    float speed_ratio = m_kart.getCurrentAdditionalSpeedRatio();

    const float result = (speed_ratio - lower_bound_ratio) / (upper_bound_ratio - lower_bound_ratio);
    return core::clamp(result, 0.0f, 1.0f);
}

// Set the direction in which the streaks should move on screen (inwards/outwards).
void SpeedStreaks::setDirection(Direction dir)
{
    m_direction = dir;
}

void SpeedStreaks::reset()
{
    m_filtered_boost_intensity = 0.0f;
    m_radial_scroll_offset = 0.0f;
    m_direction = Direction::FORWARD;
}

void SpeedStreaks::update(float dt)
{
    // Filter the kart's boost intensity to avoid abrupt visual changes.
    // - Uses the last level and smoothly approaches the new value over time.
    // - The speed at which it does this is dictated by s_boost_smoothing_rate.
    const float interpolant = 1.0f - std::exp(-s_boost_intensity_smoothing_rate * dt); //exponential decay
    m_filtered_boost_intensity = core::lerp(
        m_filtered_boost_intensity,
        calcBoostIntensity(s_boost_intensity_lower_ratio, s_boost_intensity_upper_ratio),
        interpolant
    );

    // Accumulate UV scroll offset over time.
    // Accumulated on CPU to ensure a continuous offset over time;
    // Passing per-frame speed(dictated by intensity) and time values directly
    // to the shader would produce discontinuous UV scrolling.
    //
    // Encodes speed/time/ and direction together in a single uniform to be passed to the shader.
    const float streak_speed = s_streaks_speed_min + s_streaks_speed_max * m_filtered_boost_intensity;
    m_radial_scroll_offset += dt * streak_speed * m_direction;
}
