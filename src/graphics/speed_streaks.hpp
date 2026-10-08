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

#ifndef HEADER_SPEED_STREAKS_HPP
#define HEADER_SPEED_STREAKS_HPP

class AbstractKart;
class SpeedStreaks
{
public:
    // Direction in which the streaks should move on screen
    // Tied to which camera is active, forward or reversing camera.
    enum Direction{
        REVERSE = -1,
        FORWARD = 1,
    };

    SpeedStreaks(const AbstractKart& kart);

    void  reset();
    void  update(float dt);

    // Set the direction in which the streaks should move on screen (inwards/outwards).
    void  setDirection(Direction direction);

    float getBoostIntensity() const { return m_filtered_boost_intensity; }
    float getRadialScrollOffset() const { return m_radial_scroll_offset; }

private:
    // Calculate the boost intensity value.
    // See comments on s_boost_intensity_lower_ratio and s_boost_intensity_upper_ratio for parameters.
    float calcBoostIntensity(float lower_bound_ratio = 0.0f, float upper_bound_ratio = 1.0f) const;

    // Reference to the kart to which these speed streak parameters belong.
    const AbstractKart& m_kart;

    // Smoothened kart boost intensity (lerped from current to the target each frame).
    float m_filtered_boost_intensity;

    // Integrated uv offset: encodes speed/time/direction.
    float m_radial_scroll_offset;

    // Speed streak direction (move inwards/outwards), based on camera facing.
    Direction m_direction;

    // How fast/slow should the boost intensity interpolation settle on the target.
    constexpr static float s_boost_intensity_smoothing_rate = 8.0f;

    // Speed excess (as a fraction of the generic engine max speed) at which the
    // streak intensity starts rising: 0.0 = the kart's own unboosted top speed.
    constexpr static float s_boost_intensity_lower_ratio = 0.0f;
    // Speed excess (as a fraction of the generic engine max speed) at which the
    // streak intensity reaches 1.0f.
    constexpr static float s_boost_intensity_upper_ratio = 0.75f;

    // How fast/slow should the streaks move across the screen at low and full intensity.
    constexpr static float s_streaks_speed_min    = 3.0f;
    constexpr static float s_streaks_speed_max    = 6.0f;
};

#endif
