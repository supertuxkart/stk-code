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
    /* The following lines are the input for a script that generates code for this class
    Suspension: stiffness, rest, travelCm, expSpringResponse, maxForce
    Stability: rollInfluence, chassisLinearDamping, chassisAngularDamping, downwardImpulseFactor, trackConnectionAccel, smoothFlyingImpulse
    Turn: radius, timeFullSteer, timeResetSteer
    Engine: power, maxSpeed, brakeFactor, brakeTimeIncrease, maxSpeedReverseRatio
    Gear: switchRatio, powerIncrease
    Mass
    Wheels: dampingRelaxation, dampingCompression, wheelRadius
    Skidding
    Camera: distance, forwardUpAngle, backwardUpAngle
    Jump: animationTime
    Lean: max, speed
    Anvil: duration, weight, speedFactor
    Parachute: friction, duration, durationOther, lboundFranction, uboundFranction, maxSpeed
    Bubblegum: duration, speedFraction, torque, fadeInTime, shieldDuration
    Zipper: duration, force, speedGain, speedIncrease, fadeOutTime
    Swatter: duration, distance, squashDuration, squashSlowdown
    Plunger: maxLength, force, duration, speedIncrease, fadeOutTime, inFaceTime
    Startup: time, boost
    Rescue: duration, vertOffset, height
    Explosion: duration, radius, invulnerabilityTime
    Nitro: duration, engineForce, consumption, smallContainer, bigContainer, maxSpeedIncrease, fadeOutTime, max
    Slipstream: duration, length, width, collectTime, useTime, addPower, minSpeed, maxSpeedIncrease, fadeOutTime
    */

public:
    enum CharacteristicType
    {
        // Suspension
        SUSPENSION_STIFFNESS,
        SUSPENSION_REST,
        SUSPENSION_TRAVEL_CM,
        SUSPENSION_EXP_SPRING_RESPONSE,
        SUSPENSION_MAX_FORCE,
        // Stability
        STABILITY_ROLL_INFLUENCE,
        STABILITY_CHASSIS_LINEAR_DAMPING,
        STABILITY_CHASSIS_ANGULAR_DAMPING,
        STABILITY_DOWNWARD_IMPULSE_FACTOR,
        STABILITY_TRACK_CONNECTION_ACCEL,
        STABILITY_SMOOTH_FLYING_IMPULSE,
        // Turn
        TURN_TIME_RESET_STEER,
        // Engine
        ENGINE_POWER,
        ENGINE_MAX_SPEED,
        ENGINE_BRAKE_FACTOR,
        ENGINE_BRAKE_TIME_INCREASE,
        ENGINE_MAX_SPEED_REVERSE_RATIO,
        // Mass
        MASS,
        // Wheels
        WHEELS_DAMPING_RELAXATION,
        WHEELS_DAMPING_COMPRESSION,
        WHEELS_WHEEL_RADIUS,
        // Camera
        CAMERA_DISTANCE,
        CAMERA_FORWARD_UP_ANGLE,
        CAMERA_BACKWARD_UP_ANGLE,
        // Jump
        JUMP_ANIMATION_TIME,
        // Lean
        LEAN_MAX,
        LEAN_SPEED,
        // Anvil
        ANVIL_DURATION,
        ANVIL_WEIGHT,
        ANVIL_SPEED_FACTOR,
        // Parachute
        PARACHUTE_FRICTION,
        PARACHUTE_DURATION,
        PARACHUTE_DURATION_OTHER,
        PARACHUTE_LBOUND_FRANCTION,
        PARACHUTE_UBOUND_FRANCTION,
        PARACHUTE_MAX_SPEED,
        // Bubblegum
        BUBBLEGUM_DURATION,
        BUBBLEGUM_SPEED_FRACTION,
        BUBBLEGUM_TORQUE,
        BUBBLEGUM_FADE_IN_TIME,
        BUBBLEGUM_SHIELD_DURATION,
        // Zipper
        ZIPPER_DURATION,
        ZIPPER_FORCE,
        ZIPPER_SPEED_GAIN,
        ZIPPER_SPEED_INCREASE,
        ZIPPER_FADE_OUT_TIME,
        // Swatter
        SWATTER_DURATION,
        SWATTER_DISTANCE,
        SWATTER_SQUASH_DURATION,
        SWATTER_SQUASH_SLOWDOWN,
        // Plunger
        PLUNGER_MAX_LENGTH,
        PLUNGER_FORCE,
        PLUNGER_DURATION,
        PLUNGER_SPEED_INCREASE,
        PLUNGER_FADE_OUT_TIME,
        PLUNGER_IN_FACE_TIME,
        // Rescue
        RESCUE_DURATION,
        RESCUE_VERT_OFFSET,
        RESCUE_HEIGHT,
        // Explosion
        EXPLOSION_DURATION,
        EXPLOSION_RADIUS,
        EXPLOSION_INVULNERABILITY_TIME,
        // Nitro
        NITRO_DURATION,
        NITRO_ENGINE_FORCE,
        NITRO_CONSUMPTION,
        NITRO_SMALL_CONTAINER,
        NITRO_BIG_CONTAINER,
        NITRO_MAX_SPEED_INCREASE,
        NITRO_FADE_OUT_TIME,
        NITRO_MAX,
        // Slipstream
        SLIPSTREAM_DURATION,
        SLIPSTREAM_LENGTH,
        SLIPSTREAM_WIDTH,
        SLIPSTREAM_COLLECT_TIME,
        SLIPSTREAM_USE_TIME,
        SLIPSTREAM_ADD_POWER,
        SLIPSTREAM_MIN_SPEED,
        SLIPSTREAM_MAX_SPEED_INCREASE,
        SLIPSTREAM_FADE_OUT_TIME,

        // Count
        CHARACTERISTIC_COUNT
    };
private:
    // Turn
    /*InterpolationArray m_turn_radius;
    InterpolationArray m_turn_time_full_steer;

    // Gear
    std::vector<float> m_gear_switch_ratio;
    std::vector<float> m_gear_power_increase;

    // Wheels
    std::vector<Vec3> m_wheel_position;*/

    // Skid
    /** The skididing properties for this kart, as a separate object in order
     *  to reduce dependencies (and therefore compile time) when changing
     *  any skidding property. */
    /*SkiddingProperties *m_skidding;

    // Startup
    std::vector<float> m_startup_time;
    std::vector<float> m_startup_boost;*/

public:
    AbstractCharacteristics();

    virtual const SkiddingProperties* getSkiddingProperties() const;

    virtual float processFloat(CharacteristicType type, float value) const;
    virtual std::vector<float> processFloatVector(CharacteristicType type,
        const std::vector<float> &value) const;
    virtual InterpolationArray processInterpolationArray(CharacteristicType type,
        const InterpolationArray &value) const;

	virtual float getFloat(CharacteristicType type) const;
	virtual std::vector<float> getFloatVector(CharacteristicType type) const;
	virtual InterpolationArray getInterpolationArray(CharacteristicType type) const;
};

#endif
