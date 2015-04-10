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

#ifndef HEADER_ABSTRACT_CHARACTERISTICS_HPP
#define HEADER_ABSTRACT_CHARACTERISTICS_HPP

#include "utils/interpolation_array.hpp"
#include "utils/vec3.hpp"

#include <vector>

/**
 * Characteristics are the properties of a kart that influence
 * gameplay mechanics.
 * The biggest parts are:
 * - Physics
 * - Visuals
 * - Items
 * - and miscellaneous properties like nitro and startup boost.
 *
 * The documentation of these properties can be found in
 * the kart_characteristics.xml file.
 */
class AbstractCharacteristics
{
private:
	/* ---------- Physics ---------- */

	// Suspension
	float m_suspension_stiffness;
	float m_suspension_rest;
	float m_suspension_travel_cm;
	float m_suspension_exp_spring_response;
	float m_suspension_max_force;

	// Stability
	float m_stability_roll_influence;
	float m_stability_chassis_linear_damping;
	float m_stability_chassis_angular_damping;
	float m_stability_downward_impulse_factor;
	float m_stability_track_connection_accel;
	float m_stability_smooth_flying_impulse;

	// Turn
	std::vector<float> m_turn_radius;
	InterpolationArray m_turn_time_full_steer;
	float m_turn_time_reset_steer;

	// Engine
	std::vector<float> m_engine_power;
	std::vector<float> m_engine_max_speed;
	float m_brake_factor;
	float m_break_time_increase;
	float m_max_speed_reverse_ratio;

	// Gear
	std::vector<float> m_gear_switch_ratio;
	std::vector<float> m_gear_power_increase;

	// Mass
	float m_mass;

	// Wheels
	float m_damping_relaxation;
	float m_damping_compression;
	float m_wheel_radius;
	Vec3 m_wheel_position[4];


	/* ---------- Visuals ---------- */

	// Skid
	float m_skid_increase;
	float m_skid_decrease;
	float m_skid_max;
	float m_skid_time_till_max;
	float m_skid_visual;
	float m_skid_visual_time;
	float m_skid_revert_visual_time;
	float m_skid_min_speed;
	std::vector<float> m_skid_time_till_bonus;
	std::vector<float> m_skid_bonus_speed;
	std::vector<float> m_skid_bonus_time;
	std::vector<float> m_skid_bonus_force;
	float m_skid_physical_jump_time;
	float m_skid_graphical_jump_time;
	float m_skid_post_skid_rotate_factor;
	float m_skid_reduce_turn_min;
	float m_skid_reduce_turn_max;

	// Camera
	float m_camera_distance;
	float m_camera_forward_up_angle;
	float m_camera_backward_up_angle;

	// Jump
	float m_jump_animation_time;

	// Lean
	float m_lean_max;
	float m_lean_speed;


	/* ---------- Items ---------- */

	// Anvil
	float m_anvil_time;
	float m_anvil_weight;
	float m_anvil_speed_factor;

	// Parachute
	float m_parachute_friction;
	float m_parachute_time;
	float m_parachute_time_other;
	float m_parachute_lbound_fraction;
	float m_parachute_ubound_fraction;
	float m_parachute_max_speed;

	// Bubblegum
	float m_bubblegum_time;
	float m_bubblegum_speed_fraction;
	float m_bubblegum_torque;
	float m_bubblegum_fade_in_time;
	float m_bubblegum_shield_time;

	// Zipper
	float m_zipper_time;
	float m_zipper_force;
	float m_zipper_speed_gain;
	float m_zipper_max_speed_increase;
	float m_zipper_fade_out_time;

	// Swatter
	float m_swatter_duration;
	float m_swatter_distance;
	float m_swatter_squash_duration;
	float m_swatter_squash_slowdown;

	// Plunger
	float m_plunger_band_max_length;
	float m_plunger_band_force;
	float m_plunger_band_duration;
	float m_plunger_band_speed_increase;
	float m_plunger_band_fade_out_time;
	std::vector<float> m_plunger_in_face_time;


	/* ---------- Miscellaneous ---------- */

	// Startup
	std::vector<float> m_startup_time;
	std::vector<float> m_startup_boost;

	// Rescue
	float m_rescue_vert_offset;
	float m_rescue_time;
	float m_rescue_height;

	// Explosion
	float m_explosion_time;
	float m_explosion_radius;
	float m_explosion_invulnerability_time;

	// Nitro
	float m_nitro_engine_force;
	float m_nitro_consumption;
	float m_nitro_small_container;
	float m_nitro_big_container;
	float m_nitro_max_speed_increase;
	float m_nitro_duration;
	float m_nitro_fade_out_time;
	float m_nitro_max;

	// Slipstream
	float m_slipstream_length;
	float m_slipstream_width;
	float m_slipstream_collect_time;
	float m_slipstream_use_time;
	float m_slipstream_add_power;
	float m_slipstream_min_speed;
	float m_slipstream_max_speed_increase;
	float m_slipstream_duration;
	float m_slipstream_fade_out_time;

public:
};

#endif
