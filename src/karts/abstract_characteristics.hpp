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

class SkiddingProperties;

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
    InterpolationArray m_turn_radius;
    InterpolationArray m_turn_time_full_steer;
    float m_turn_time_reset_steer;

    // Engine
    float m_engine_power;
    float m_engine_max_speed;
    float m_brake_factor;
    float m_brake_time_increase;
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
    std::vector<Vec3> m_wheel_position;


    /* ---------- Visuals ---------- */

    // Skid
    SkiddingProperties *m_skidding;

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
    float m_anvil_duration;
    float m_anvil_weight;
    float m_anvil_speed_factor;

    // Parachute
    float m_parachute_friction;
    float m_parachute_duration;
    float m_parachute_duration_other;
    float m_parachute_lbound_fraction;
    float m_parachute_ubound_fraction;
    float m_parachute_max_speed;

    // Bubblegum
    float m_bubblegum_duration;
    float m_bubblegum_speed_fraction;
    float m_bubblegum_torque;
    float m_bubblegum_fade_in_time;
    float m_bubblegum_shield_duration;

    // Zipper
    float m_zipper_duration;
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
    float m_plunger_in_face_time;


    /* ---------- Miscellaneous ---------- */

    // Startup
    std::vector<float> m_startup_time;
    std::vector<float> m_startup_boost;

    // Rescue
    float m_rescue_vert_offset;
    float m_rescue_duration;
    float m_rescue_height;

    // Explosion
    float m_explosion_duration;
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
    AbstractCharacteristics();

    float getSuspensionStiffness() const              { return m_suspension_stiffness; }
    float getSuspensionRest() const                   { return m_suspension_rest; }
    float getSuspensionTravelCM() const               { return m_suspension_travel_cm; }
    float getSuspensionExpSpringResponse() const      { return m_suspension_exp_spring_response; }
    float getSuspensionMaxForce() const               { return m_suspension_max_force; }

    float getStabilityRollInfluence() const           { return m_stability_roll_influence; }
    float getStabilityChassisLinearDamping() const    { return m_stability_chassis_linear_damping; }
    float getStabilityChassisAngularDamping() const   { return m_stability_chassis_angular_damping; }
    float getStabilityDownwardImpulseFactor() const   { return m_stability_downward_impulse_factor; }
    float getStabilityTrackConnectionAccel() const    { return m_stability_track_connection_accel; }
    float getStabilitySmoothFlyingImpulse() const     { return m_stability_smooth_flying_impulse; }

    float getTurnRadius(float steer) const            { return m_turn_radius.get(steer); }
    float getTurnTimeFullSteer(float steer) const     { return m_turn_time_full_steer.get(steer); }
    float getTurnTimeResetSteer() const               { return m_turn_time_reset_steer; }

    float getEnginePower() const                      { return m_engine_power; }
    float getEngineMaxSpeed() const                   { return m_engine_max_speed; }
    float getBreakFactor() const                      { return m_brake_factor; }
    float getBrakeTimeIncrease() const                { return m_brake_time_increase; }
    float getMaxSpeedReverseRatio() const;

    float getGearSwitchRatio(int gear) const;
    float getGearPowerIncrease(int gear) const;
    
    float getMass() const;

    float getDampingRelaxation() const;
    float getDampingCompression() const;
    float getWheelRadius() const;
    Vec3 getWheelPosition(int wheel) const;

    SkiddingProperties* getSkiddingProperties() const;

    float getCameraDistance() const;
    float getCameraForwardUpAngle() const;
    float getCameraBackwardUpAngle() const;

    float getJumpAnimationTime() const;

    float getLeanMax() const;
    float getLeanSpeed() const;

    float getAnvilDuration() const;
    float getAnvilWeight() const;
    float getAnvilSpeedFactor() const;

    float getParachuteFriction() const;
    float getParachuteDuration() const;
    float getParachuteDurationOther() const;
    float getParachuteLBoundFraction() const;
    float getParachuteUBoundFranction() const;
    float getParachuteMaxSpeed() const;

    float getBubblegumDuration() const;
    float getBubblegumSpeedFraction() const;
    float getBubblegumTorque() const;
    float getBubblegumFadeInTime() const;
    float getBubblegumShieldDuration() const;

    float getZipperDuration() const;
    float getZipperForce() const;
    float getZipperSpeedGain() const;
    float getZipperMaxSpeedIncrease() const;
    float getZipperFadeOutTime() const;
    float getSwatterDuration() const;
    float getSwatterDistance() const;
    float getSwatterSquashDuration() const;
    float getSwatterSquashSlowdown() const;

    float getPlungerBandMaxLength() const;
    float getPlungerBandForce() const;
    float getPlungerBandDuration() const;
    float getPlungerBandSpeedIncrease() const;
    float getPlungerBandFadeOutTime() const;
    float getPlungerInFaceTime() const;

    float getStartupBoost(float time) const;

    float getRescueVertOffset() const;
    float getRescueDuration() const;
    float getRescueHeight() const;

    float getExplosionDuration() const;
    float getExplosionRadius() const;
    float getExplosionInvulnerabilityTime() const;

    float getNitroEngineForce() const;
    float getNitroConsumption() const;
    float getNitroSmallContainer() const;
    float getNitroBigContainer() const;
    float getNitroMaxSpeedIncrease() const;
    float getNitroDuration() const;
    float getNitroFadeOutTime() const;
    float getNitroMax() const;

    float getSlipstreamLength() const;
    float getSlipstreamWidth() const;
    float getSlipstreamCollectTime() const;
    float getSlipstreamUseTime() const;
    float getSlipstreamAddPower() const;
    float getSlipstreamMinSpeed() const;
    float getSlipstreamMaxSpeedIncrease() const;
    float getSlipstreamDuration() const;
    float getSlipstreamFadeOutTime() const;

protected:
    void setSuspensionStiffness(float value);
};

#endif
