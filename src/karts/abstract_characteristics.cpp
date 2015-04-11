//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "karts/abstract_characteristics.hpp"

AbstractCharacteristics::AbstractCharacteristics()
{
    // Set all variables to their default value (1)
    m_suspension_stiffness = m_suspension_rest = m_suspension_travel_cm =
        m_suspension_exp_spring_response = m_suspension_max_force =
        m_stability_roll_influence = m_stability_chassis_linear_damping =
        m_stability_chassis_angular_damping = m_stability_downward_impulse_factor =
        m_stability_track_connection_accel = m_stability_smooth_flying_impulse =
        m_turn_time_reset_steer = m_engine_power = m_engine_max_speed =
        m_engine_brake_factor = m_engine_brake_time_increase =
        m_engine_max_speed_reverse_ratio = m_mass = m_wheels_damping_relaxation =
        m_wheels_damping_compression = m_wheels_wheel_radius = m_camera_distance =
        m_camera_forward_up_angle = m_camera_backward_up_angle =
        m_jump_animation_time = m_lean_max = m_lean_speed = m_anvil_duration =
        m_anvil_weight = m_anvil_speed_factor = m_parachute_friction =
        m_parachute_duration = m_parachute_duration_other =
        m_parachute_lbound_franction = m_parachute_ubound_franction =
        m_parachute_max_speed = m_bubblegum_duration = m_bubblegum_speed_fraction =
        m_bubblegum_torque = m_bubblegum_fade_in_time =
        m_bubblegum_shield_duration = m_zipper_duration = m_zipper_force =
        m_zipper_speed_gain = m_zipper_speed_increase = m_zipper_fade_out_time =
        m_swatter_duration = m_swatter_distance = m_swatter_squash_duration =
        m_swatter_squash_slowdown = m_plunger_max_length = m_plunger_force =
        m_plunger_duration = m_plunger_speed_increase = m_plunger_fade_out_time =
        m_plunger_in_face_time = m_rescue_duration = m_rescue_vert_offset =
        m_rescue_height = m_explosion_duration = m_explosion_radius =
        m_explosion_invulnerability_time = m_nitro_duration =
        m_nitro_engine_force = m_nitro_consumption = m_nitro_small_container =
        m_nitro_big_container = m_nitro_max_speed_increase =
        m_nitro_fade_out_time = m_nitro_max = m_slipstream_duration =
        m_slipstream_length = m_slipstream_width = m_slipstream_collect_time =
        m_slipstream_use_time = m_slipstream_add_power = m_slipstream_min_speed =
        m_slipstream_max_speed_increase = m_slipstream_fade_out_time = 1;
}