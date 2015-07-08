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

#include "utils/log.hpp"
#include "utils/interpolation_array.hpp"

#include <cmath>

AbstractCharacteristics::AbstractCharacteristics()
{
}

const SkiddingProperties* AbstractCharacteristics::getSkiddingProperties() const
{
    return nullptr;
}

void AbstractCharacteristics::process(CharacteristicType type, Value value, bool *is_set) const
{
    Log::warn("AbstractCharacteristics", "This type does not do anything");
}


AbstractCharacteristics::ValueType AbstractCharacteristics::getType(CharacteristicType type)
{
    switch (type)
    {
    case CHARACTERISTIC_COUNT:
        Log::fatal("AbstractCharacteristics::getType", "Can't get type of COUNT");
        break;
    // Script-generated content
    case SUSPENSION_STIFFNESS:
        return TYPE_FLOAT;
    case SUSPENSION_REST:
        return TYPE_FLOAT;
    case SUSPENSION_TRAVEL_CM:
        return TYPE_FLOAT;
    case SUSPENSION_EXP_SPRING_RESPONSE:
        return TYPE_FLOAT;
    case SUSPENSION_MAX_FORCE:
        return TYPE_FLOAT;
    case STABILITY_ROLL_INFLUENCE:
        return TYPE_FLOAT;
    case STABILITY_CHASSIS_LINEAR_DAMPING:
        return TYPE_FLOAT;
    case STABILITY_CHASSIS_ANGULAR_DAMPING:
        return TYPE_FLOAT;
    case STABILITY_DOWNWARD_IMPULSE_FACTOR:
        return TYPE_FLOAT;
    case STABILITY_TRACK_CONNECTION_ACCEL:
        return TYPE_FLOAT;
    case STABILITY_SMOOTH_FLYING_IMPULSE:
        return TYPE_FLOAT;
    case TURN_RADIUS:
        return TYPE_INTERPOLATION_ARRAY;
    case TURN_TIME_RESET_STEER:
        return TYPE_FLOAT;
    case TURN_TIME_FULL_STEER:
        return TYPE_INTERPOLATION_ARRAY;
    case ENGINE_POWER:
        return TYPE_FLOAT;
    case ENGINE_MAX_SPEED:
        return TYPE_FLOAT;
    case ENGINE_BRAKE_FACTOR:
        return TYPE_FLOAT;
    case ENGINE_BRAKE_TIME_INCREASE:
        return TYPE_FLOAT;
    case ENGINE_MAX_SPEED_REVERSE_RATIO:
        return TYPE_FLOAT;
    case GEAR_SWITCH_RATIO:
        return TYPE_FLOAT_VECTOR;
    case GEAR_POWER_INCREASE:
        return TYPE_FLOAT_VECTOR;
    case MASS:
        return TYPE_FLOAT;
    case WHEELS_DAMPING_RELAXATION:
        return TYPE_FLOAT;
    case WHEELS_DAMPING_COMPRESSION:
        return TYPE_FLOAT;
    case WHEELS_RADIUS:
        return TYPE_FLOAT;
    case WHEELS_POSITION:
        return TYPE_FLOAT_VECTOR;
    case CAMERA_DISTANCE:
        return TYPE_FLOAT;
    case CAMERA_FORWARD_UP_ANGLE:
        return TYPE_FLOAT;
    case CAMERA_BACKWARD_UP_ANGLE:
        return TYPE_FLOAT;
    case JUMP_ANIMATION_TIME:
        return TYPE_FLOAT;
    case LEAN_MAX:
        return TYPE_FLOAT;
    case LEAN_SPEED:
        return TYPE_FLOAT;
    case ANVIL_DURATION:
        return TYPE_FLOAT;
    case ANVIL_WEIGHT:
        return TYPE_FLOAT;
    case ANVIL_SPEED_FACTOR:
        return TYPE_FLOAT;
    case PARACHUTE_FRICTION:
        return TYPE_FLOAT;
    case PARACHUTE_DURATION:
        return TYPE_FLOAT;
    case PARACHUTE_DURATION_OTHER:
        return TYPE_FLOAT;
    case PARACHUTE_LBOUND_FRANCTION:
        return TYPE_FLOAT;
    case PARACHUTE_UBOUND_FRANCTION:
        return TYPE_FLOAT;
    case PARACHUTE_MAX_SPEED:
        return TYPE_FLOAT;
    case BUBBLEGUM_DURATION:
        return TYPE_FLOAT;
    case BUBBLEGUM_SPEED_FRACTION:
        return TYPE_FLOAT;
    case BUBBLEGUM_TORQUE:
        return TYPE_FLOAT;
    case BUBBLEGUM_FADE_IN_TIME:
        return TYPE_FLOAT;
    case BUBBLEGUM_SHIELD_DURATION:
        return TYPE_FLOAT;
    case ZIPPER_DURATION:
        return TYPE_FLOAT;
    case ZIPPER_FORCE:
        return TYPE_FLOAT;
    case ZIPPER_SPEED_GAIN:
        return TYPE_FLOAT;
    case ZIPPER_SPEED_INCREASE:
        return TYPE_FLOAT;
    case ZIPPER_FADE_OUT_TIME:
        return TYPE_FLOAT;
    case SWATTER_DURATION:
        return TYPE_FLOAT;
    case SWATTER_DISTANCE:
        return TYPE_FLOAT;
    case SWATTER_SQUASH_DURATION:
        return TYPE_FLOAT;
    case SWATTER_SQUASH_SLOWDOWN:
        return TYPE_FLOAT;
    case PLUNGER_MAX_LENGTH:
        return TYPE_FLOAT;
    case PLUNGER_FORCE:
        return TYPE_FLOAT;
    case PLUNGER_DURATION:
        return TYPE_FLOAT;
    case PLUNGER_SPEED_INCREASE:
        return TYPE_FLOAT;
    case PLUNGER_FADE_OUT_TIME:
        return TYPE_FLOAT;
    case PLUNGER_IN_FACE_TIME:
        return TYPE_FLOAT;
    case STARTUP_TIME:
        return TYPE_FLOAT_VECTOR;
    case STARTUP_BOOST:
        return TYPE_FLOAT_VECTOR;
    case RESCUE_DURATION:
        return TYPE_FLOAT;
    case RESCUE_VERT_OFFSET:
        return TYPE_FLOAT;
    case RESCUE_HEIGHT:
        return TYPE_FLOAT;
    case EXPLOSION_DURATION:
        return TYPE_FLOAT;
    case EXPLOSION_RADIUS:
        return TYPE_FLOAT;
    case EXPLOSION_INVULNERABILITY_TIME:
        return TYPE_FLOAT;
    case NITRO_DURATION:
        return TYPE_FLOAT;
    case NITRO_ENGINE_FORCE:
        return TYPE_FLOAT;
    case NITRO_CONSUMPTION:
        return TYPE_FLOAT;
    case NITRO_SMALL_CONTAINER:
        return TYPE_FLOAT;
    case NITRO_BIG_CONTAINER:
        return TYPE_FLOAT;
    case NITRO_MAX_SPEED_INCREASE:
        return TYPE_FLOAT;
    case NITRO_FADE_OUT_TIME:
        return TYPE_FLOAT;
    case NITRO_MAX:
        return TYPE_FLOAT;
    case SLIPSTREAM_DURATION:
        return TYPE_FLOAT;
    case SLIPSTREAM_LENGTH:
        return TYPE_FLOAT;
    case SLIPSTREAM_WIDTH:
        return TYPE_FLOAT;
    case SLIPSTREAM_COLLECT_TIME:
        return TYPE_FLOAT;
    case SLIPSTREAM_USE_TIME:
        return TYPE_FLOAT;
    case SLIPSTREAM_ADD_POWER:
        return TYPE_FLOAT;
    case SLIPSTREAM_MIN_SPEED:
        return TYPE_FLOAT;
    case SLIPSTREAM_MAX_SPEED_INCREASE:
        return TYPE_FLOAT;
    case SLIPSTREAM_FADE_OUT_TIME:
        return TYPE_FLOAT;
    }
    Log::fatal("AbstractCharacteristics::getType", "Unknown type");
    return TYPE_FLOAT;
}

std::string AbstractCharacteristics::getName(CharacteristicType type)
{
    switch (type)
    {
    case CHARACTERISTIC_COUNT:
        return "CHARACTERISTIC_COUNT";
    // Script-generated content
    case SUSPENSION_STIFFNESS:
        return "SUSPENSION_STIFFNESS";
    case SUSPENSION_REST:
        return "SUSPENSION_REST";
    case SUSPENSION_TRAVEL_CM:
        return "SUSPENSION_TRAVEL_CM";
    case SUSPENSION_EXP_SPRING_RESPONSE:
        return "SUSPENSION_EXP_SPRING_RESPONSE";
    case SUSPENSION_MAX_FORCE:
        return "SUSPENSION_MAX_FORCE";
    case STABILITY_ROLL_INFLUENCE:
        return "STABILITY_ROLL_INFLUENCE";
    case STABILITY_CHASSIS_LINEAR_DAMPING:
        return "STABILITY_CHASSIS_LINEAR_DAMPING";
    case STABILITY_CHASSIS_ANGULAR_DAMPING:
        return "STABILITY_CHASSIS_ANGULAR_DAMPING";
    case STABILITY_DOWNWARD_IMPULSE_FACTOR:
        return "STABILITY_DOWNWARD_IMPULSE_FACTOR";
    case STABILITY_TRACK_CONNECTION_ACCEL:
        return "STABILITY_TRACK_CONNECTION_ACCEL";
    case STABILITY_SMOOTH_FLYING_IMPULSE:
        return "STABILITY_SMOOTH_FLYING_IMPULSE";
    case TURN_RADIUS:
        return "TURN_RADIUS";
    case TURN_TIME_RESET_STEER:
        return "TURN_TIME_RESET_STEER";
    case TURN_TIME_FULL_STEER:
        return "TURN_TIME_FULL_STEER";
    case ENGINE_POWER:
        return "ENGINE_POWER";
    case ENGINE_MAX_SPEED:
        return "ENGINE_MAX_SPEED";
    case ENGINE_BRAKE_FACTOR:
        return "ENGINE_BRAKE_FACTOR";
    case ENGINE_BRAKE_TIME_INCREASE:
        return "ENGINE_BRAKE_TIME_INCREASE";
    case ENGINE_MAX_SPEED_REVERSE_RATIO:
        return "ENGINE_MAX_SPEED_REVERSE_RATIO";
    case GEAR_SWITCH_RATIO:
        return "GEAR_SWITCH_RATIO";
    case GEAR_POWER_INCREASE:
        return "GEAR_POWER_INCREASE";
    case MASS:
        return "MASS";
    case WHEELS_DAMPING_RELAXATION:
        return "WHEELS_DAMPING_RELAXATION";
    case WHEELS_DAMPING_COMPRESSION:
        return "WHEELS_DAMPING_COMPRESSION";
    case WHEELS_RADIUS:
        return "WHEELS_RADIUS";
    case WHEELS_POSITION:
        return "WHEELS_POSITION";
    case CAMERA_DISTANCE:
        return "CAMERA_DISTANCE";
    case CAMERA_FORWARD_UP_ANGLE:
        return "CAMERA_FORWARD_UP_ANGLE";
    case CAMERA_BACKWARD_UP_ANGLE:
        return "CAMERA_BACKWARD_UP_ANGLE";
    case JUMP_ANIMATION_TIME:
        return "JUMP_ANIMATION_TIME";
    case LEAN_MAX:
        return "LEAN_MAX";
    case LEAN_SPEED:
        return "LEAN_SPEED";
    case ANVIL_DURATION:
        return "ANVIL_DURATION";
    case ANVIL_WEIGHT:
        return "ANVIL_WEIGHT";
    case ANVIL_SPEED_FACTOR:
        return "ANVIL_SPEED_FACTOR";
    case PARACHUTE_FRICTION:
        return "PARACHUTE_FRICTION";
    case PARACHUTE_DURATION:
        return "PARACHUTE_DURATION";
    case PARACHUTE_DURATION_OTHER:
        return "PARACHUTE_DURATION_OTHER";
    case PARACHUTE_LBOUND_FRANCTION:
        return "PARACHUTE_LBOUND_FRANCTION";
    case PARACHUTE_UBOUND_FRANCTION:
        return "PARACHUTE_UBOUND_FRANCTION";
    case PARACHUTE_MAX_SPEED:
        return "PARACHUTE_MAX_SPEED";
    case BUBBLEGUM_DURATION:
        return "BUBBLEGUM_DURATION";
    case BUBBLEGUM_SPEED_FRACTION:
        return "BUBBLEGUM_SPEED_FRACTION";
    case BUBBLEGUM_TORQUE:
        return "BUBBLEGUM_TORQUE";
    case BUBBLEGUM_FADE_IN_TIME:
        return "BUBBLEGUM_FADE_IN_TIME";
    case BUBBLEGUM_SHIELD_DURATION:
        return "BUBBLEGUM_SHIELD_DURATION";
    case ZIPPER_DURATION:
        return "ZIPPER_DURATION";
    case ZIPPER_FORCE:
        return "ZIPPER_FORCE";
    case ZIPPER_SPEED_GAIN:
        return "ZIPPER_SPEED_GAIN";
    case ZIPPER_SPEED_INCREASE:
        return "ZIPPER_SPEED_INCREASE";
    case ZIPPER_FADE_OUT_TIME:
        return "ZIPPER_FADE_OUT_TIME";
    case SWATTER_DURATION:
        return "SWATTER_DURATION";
    case SWATTER_DISTANCE:
        return "SWATTER_DISTANCE";
    case SWATTER_SQUASH_DURATION:
        return "SWATTER_SQUASH_DURATION";
    case SWATTER_SQUASH_SLOWDOWN:
        return "SWATTER_SQUASH_SLOWDOWN";
    case PLUNGER_MAX_LENGTH:
        return "PLUNGER_MAX_LENGTH";
    case PLUNGER_FORCE:
        return "PLUNGER_FORCE";
    case PLUNGER_DURATION:
        return "PLUNGER_DURATION";
    case PLUNGER_SPEED_INCREASE:
        return "PLUNGER_SPEED_INCREASE";
    case PLUNGER_FADE_OUT_TIME:
        return "PLUNGER_FADE_OUT_TIME";
    case PLUNGER_IN_FACE_TIME:
        return "PLUNGER_IN_FACE_TIME";
    case STARTUP_TIME:
        return "STARTUP_TIME";
    case STARTUP_BOOST:
        return "STARTUP_BOOST";
    case RESCUE_DURATION:
        return "RESCUE_DURATION";
    case RESCUE_VERT_OFFSET:
        return "RESCUE_VERT_OFFSET";
    case RESCUE_HEIGHT:
        return "RESCUE_HEIGHT";
    case EXPLOSION_DURATION:
        return "EXPLOSION_DURATION";
    case EXPLOSION_RADIUS:
        return "EXPLOSION_RADIUS";
    case EXPLOSION_INVULNERABILITY_TIME:
        return "EXPLOSION_INVULNERABILITY_TIME";
    case NITRO_DURATION:
        return "NITRO_DURATION";
    case NITRO_ENGINE_FORCE:
        return "NITRO_ENGINE_FORCE";
    case NITRO_CONSUMPTION:
        return "NITRO_CONSUMPTION";
    case NITRO_SMALL_CONTAINER:
        return "NITRO_SMALL_CONTAINER";
    case NITRO_BIG_CONTAINER:
        return "NITRO_BIG_CONTAINER";
    case NITRO_MAX_SPEED_INCREASE:
        return "NITRO_MAX_SPEED_INCREASE";
    case NITRO_FADE_OUT_TIME:
        return "NITRO_FADE_OUT_TIME";
    case NITRO_MAX:
        return "NITRO_MAX";
    case SLIPSTREAM_DURATION:
        return "SLIPSTREAM_DURATION";
    case SLIPSTREAM_LENGTH:
        return "SLIPSTREAM_LENGTH";
    case SLIPSTREAM_WIDTH:
        return "SLIPSTREAM_WIDTH";
    case SLIPSTREAM_COLLECT_TIME:
        return "SLIPSTREAM_COLLECT_TIME";
    case SLIPSTREAM_USE_TIME:
        return "SLIPSTREAM_USE_TIME";
    case SLIPSTREAM_ADD_POWER:
        return "SLIPSTREAM_ADD_POWER";
    case SLIPSTREAM_MIN_SPEED:
        return "SLIPSTREAM_MIN_SPEED";
    case SLIPSTREAM_MAX_SPEED_INCREASE:
        return "SLIPSTREAM_MAX_SPEED_INCREASE";
    case SLIPSTREAM_FADE_OUT_TIME:
        return "SLIPSTREAM_FADE_OUT_TIME";
    }
    Log::error("AbstractCharacteristics::getName", "Unknown type");
    return "Unknown type";
}

// Script-generated getter
float AbstractCharacteristics::getSuspensionStiffness() const
{
    float result;
    bool is_set = false;
    process(SUSPENSION_STIFFNESS, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SUSPENSION_STIFFNESS).c_str());
    return result;
}

float AbstractCharacteristics::getSuspensionRest() const
{
    float result;
    bool is_set = false;
    process(SUSPENSION_REST, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SUSPENSION_REST).c_str());
    return result;
}

float AbstractCharacteristics::getSuspensionTravelCm() const
{
    float result;
    bool is_set = false;
    process(SUSPENSION_TRAVEL_CM, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SUSPENSION_TRAVEL_CM).c_str());
    return result;
}

float AbstractCharacteristics::getSuspensionExpSpringResponse() const
{
    float result;
    bool is_set = false;
    process(SUSPENSION_EXP_SPRING_RESPONSE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SUSPENSION_EXP_SPRING_RESPONSE).c_str());
    return result;
}

float AbstractCharacteristics::getSuspensionMaxForce() const
{
    float result;
    bool is_set = false;
    process(SUSPENSION_MAX_FORCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SUSPENSION_MAX_FORCE).c_str());
    return result;
}

float AbstractCharacteristics::getStabilityRollInfluence() const
{
    float result;
    bool is_set = false;
    process(STABILITY_ROLL_INFLUENCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STABILITY_ROLL_INFLUENCE).c_str());
    return result;
}

float AbstractCharacteristics::getStabilityChassisLinearDamping() const
{
    float result;
    bool is_set = false;
    process(STABILITY_CHASSIS_LINEAR_DAMPING, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STABILITY_CHASSIS_LINEAR_DAMPING).c_str());
    return result;
}

float AbstractCharacteristics::getStabilityChassisAngularDamping() const
{
    float result;
    bool is_set = false;
    process(STABILITY_CHASSIS_ANGULAR_DAMPING, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STABILITY_CHASSIS_ANGULAR_DAMPING).c_str());
    return result;
}

float AbstractCharacteristics::getStabilityDownwardImpulseFactor() const
{
    float result;
    bool is_set = false;
    process(STABILITY_DOWNWARD_IMPULSE_FACTOR, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STABILITY_DOWNWARD_IMPULSE_FACTOR).c_str());
    return result;
}

float AbstractCharacteristics::getStabilityTrackConnectionAccel() const
{
    float result;
    bool is_set = false;
    process(STABILITY_TRACK_CONNECTION_ACCEL, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STABILITY_TRACK_CONNECTION_ACCEL).c_str());
    return result;
}

float AbstractCharacteristics::getStabilitySmoothFlyingImpulse() const
{
    float result;
    bool is_set = false;
    process(STABILITY_SMOOTH_FLYING_IMPULSE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STABILITY_SMOOTH_FLYING_IMPULSE).c_str());
    return result;
}

InterpolationArray AbstractCharacteristics::getTurnRadius() const
{
    InterpolationArray result;
    bool is_set = false;
    process(TURN_RADIUS, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(TURN_RADIUS).c_str());
    return result;
}

float AbstractCharacteristics::getTurnTimeResetSteer() const
{
    float result;
    bool is_set = false;
    process(TURN_TIME_RESET_STEER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(TURN_TIME_RESET_STEER).c_str());
    return result;
}

InterpolationArray AbstractCharacteristics::getTurnTimeFullSteer() const
{
    InterpolationArray result;
    bool is_set = false;
    process(TURN_TIME_FULL_STEER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(TURN_TIME_FULL_STEER).c_str());
    return result;
}

float AbstractCharacteristics::getEnginePower() const
{
    float result;
    bool is_set = false;
    process(ENGINE_POWER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ENGINE_POWER).c_str());
    return result;
}

float AbstractCharacteristics::getEngineMaxSpeed() const
{
    float result;
    bool is_set = false;
    process(ENGINE_MAX_SPEED, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ENGINE_MAX_SPEED).c_str());
    return result;
}

float AbstractCharacteristics::getEngineBrakeFactor() const
{
    float result;
    bool is_set = false;
    process(ENGINE_BRAKE_FACTOR, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ENGINE_BRAKE_FACTOR).c_str());
    return result;
}

float AbstractCharacteristics::getEngineBrakeTimeIncrease() const
{
    float result;
    bool is_set = false;
    process(ENGINE_BRAKE_TIME_INCREASE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ENGINE_BRAKE_TIME_INCREASE).c_str());
    return result;
}

float AbstractCharacteristics::getEngineMaxSpeedReverseRatio() const
{
    float result;
    bool is_set = false;
    process(ENGINE_MAX_SPEED_REVERSE_RATIO, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ENGINE_MAX_SPEED_REVERSE_RATIO).c_str());
    return result;
}

std::vector<float> AbstractCharacteristics::getGearSwitchRatio() const
{
    std::vector<float> result;
    bool is_set = false;
    process(GEAR_SWITCH_RATIO, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(GEAR_SWITCH_RATIO).c_str());
    return result;
}

std::vector<float> AbstractCharacteristics::getGearPowerIncrease() const
{
    std::vector<float> result;
    bool is_set = false;
    process(GEAR_POWER_INCREASE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(GEAR_POWER_INCREASE).c_str());
    return result;
}

float AbstractCharacteristics::getMass() const
{
    float result;
    bool is_set = false;
    process(MASS, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(MASS).c_str());
    return result;
}

float AbstractCharacteristics::getWheelsDampingRelaxation() const
{
    float result;
    bool is_set = false;
    process(WHEELS_DAMPING_RELAXATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(WHEELS_DAMPING_RELAXATION).c_str());
    return result;
}

float AbstractCharacteristics::getWheelsDampingCompression() const
{
    float result;
    bool is_set = false;
    process(WHEELS_DAMPING_COMPRESSION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(WHEELS_DAMPING_COMPRESSION).c_str());
    return result;
}

float AbstractCharacteristics::getWheelsRadius() const
{
    float result;
    bool is_set = false;
    process(WHEELS_RADIUS, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(WHEELS_RADIUS).c_str());
    return result;
}

std::vector<float> AbstractCharacteristics::getWheelsPosition() const
{
    std::vector<float> result;
    bool is_set = false;
    process(WHEELS_POSITION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(WHEELS_POSITION).c_str());
    return result;
}

float AbstractCharacteristics::getCameraDistance() const
{
    float result;
    bool is_set = false;
    process(CAMERA_DISTANCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(CAMERA_DISTANCE).c_str());
    return result;
}

float AbstractCharacteristics::getCameraForwardUpAngle() const
{
    float result;
    bool is_set = false;
    process(CAMERA_FORWARD_UP_ANGLE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(CAMERA_FORWARD_UP_ANGLE).c_str());
    return result;
}

float AbstractCharacteristics::getCameraBackwardUpAngle() const
{
    float result;
    bool is_set = false;
    process(CAMERA_BACKWARD_UP_ANGLE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(CAMERA_BACKWARD_UP_ANGLE).c_str());
    return result;
}

float AbstractCharacteristics::getJumpAnimationTime() const
{
    float result;
    bool is_set = false;
    process(JUMP_ANIMATION_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(JUMP_ANIMATION_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getLeanMax() const
{
    float result;
    bool is_set = false;
    process(LEAN_MAX, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(LEAN_MAX).c_str());
    return result;
}

float AbstractCharacteristics::getLeanSpeed() const
{
    float result;
    bool is_set = false;
    process(LEAN_SPEED, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(LEAN_SPEED).c_str());
    return result;
}

float AbstractCharacteristics::getAnvilDuration() const
{
    float result;
    bool is_set = false;
    process(ANVIL_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ANVIL_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getAnvilWeight() const
{
    float result;
    bool is_set = false;
    process(ANVIL_WEIGHT, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ANVIL_WEIGHT).c_str());
    return result;
}

float AbstractCharacteristics::getAnvilSpeedFactor() const
{
    float result;
    bool is_set = false;
    process(ANVIL_SPEED_FACTOR, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ANVIL_SPEED_FACTOR).c_str());
    return result;
}

float AbstractCharacteristics::getParachuteFriction() const
{
    float result;
    bool is_set = false;
    process(PARACHUTE_FRICTION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PARACHUTE_FRICTION).c_str());
    return result;
}

float AbstractCharacteristics::getParachuteDuration() const
{
    float result;
    bool is_set = false;
    process(PARACHUTE_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PARACHUTE_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getParachuteDurationOther() const
{
    float result;
    bool is_set = false;
    process(PARACHUTE_DURATION_OTHER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PARACHUTE_DURATION_OTHER).c_str());
    return result;
}

float AbstractCharacteristics::getParachuteLboundFranction() const
{
    float result;
    bool is_set = false;
    process(PARACHUTE_LBOUND_FRANCTION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PARACHUTE_LBOUND_FRANCTION).c_str());
    return result;
}

float AbstractCharacteristics::getParachuteUboundFranction() const
{
    float result;
    bool is_set = false;
    process(PARACHUTE_UBOUND_FRANCTION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PARACHUTE_UBOUND_FRANCTION).c_str());
    return result;
}

float AbstractCharacteristics::getParachuteMaxSpeed() const
{
    float result;
    bool is_set = false;
    process(PARACHUTE_MAX_SPEED, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PARACHUTE_MAX_SPEED).c_str());
    return result;
}

float AbstractCharacteristics::getBubblegumDuration() const
{
    float result;
    bool is_set = false;
    process(BUBBLEGUM_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(BUBBLEGUM_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getBubblegumSpeedFraction() const
{
    float result;
    bool is_set = false;
    process(BUBBLEGUM_SPEED_FRACTION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(BUBBLEGUM_SPEED_FRACTION).c_str());
    return result;
}

float AbstractCharacteristics::getBubblegumTorque() const
{
    float result;
    bool is_set = false;
    process(BUBBLEGUM_TORQUE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(BUBBLEGUM_TORQUE).c_str());
    return result;
}

float AbstractCharacteristics::getBubblegumFadeInTime() const
{
    float result;
    bool is_set = false;
    process(BUBBLEGUM_FADE_IN_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(BUBBLEGUM_FADE_IN_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getBubblegumShieldDuration() const
{
    float result;
    bool is_set = false;
    process(BUBBLEGUM_SHIELD_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(BUBBLEGUM_SHIELD_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getZipperDuration() const
{
    float result;
    bool is_set = false;
    process(ZIPPER_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ZIPPER_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getZipperForce() const
{
    float result;
    bool is_set = false;
    process(ZIPPER_FORCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ZIPPER_FORCE).c_str());
    return result;
}

float AbstractCharacteristics::getZipperSpeedGain() const
{
    float result;
    bool is_set = false;
    process(ZIPPER_SPEED_GAIN, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ZIPPER_SPEED_GAIN).c_str());
    return result;
}

float AbstractCharacteristics::getZipperSpeedIncrease() const
{
    float result;
    bool is_set = false;
    process(ZIPPER_SPEED_INCREASE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ZIPPER_SPEED_INCREASE).c_str());
    return result;
}

float AbstractCharacteristics::getZipperFadeOutTime() const
{
    float result;
    bool is_set = false;
    process(ZIPPER_FADE_OUT_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(ZIPPER_FADE_OUT_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getSwatterDuration() const
{
    float result;
    bool is_set = false;
    process(SWATTER_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SWATTER_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getSwatterDistance() const
{
    float result;
    bool is_set = false;
    process(SWATTER_DISTANCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SWATTER_DISTANCE).c_str());
    return result;
}

float AbstractCharacteristics::getSwatterSquashDuration() const
{
    float result;
    bool is_set = false;
    process(SWATTER_SQUASH_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SWATTER_SQUASH_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getSwatterSquashSlowdown() const
{
    float result;
    bool is_set = false;
    process(SWATTER_SQUASH_SLOWDOWN, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SWATTER_SQUASH_SLOWDOWN).c_str());
    return result;
}

float AbstractCharacteristics::getPlungerMaxLength() const
{
    float result;
    bool is_set = false;
    process(PLUNGER_MAX_LENGTH, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PLUNGER_MAX_LENGTH).c_str());
    return result;
}

float AbstractCharacteristics::getPlungerForce() const
{
    float result;
    bool is_set = false;
    process(PLUNGER_FORCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PLUNGER_FORCE).c_str());
    return result;
}

float AbstractCharacteristics::getPlungerDuration() const
{
    float result;
    bool is_set = false;
    process(PLUNGER_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PLUNGER_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getPlungerSpeedIncrease() const
{
    float result;
    bool is_set = false;
    process(PLUNGER_SPEED_INCREASE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PLUNGER_SPEED_INCREASE).c_str());
    return result;
}

float AbstractCharacteristics::getPlungerFadeOutTime() const
{
    float result;
    bool is_set = false;
    process(PLUNGER_FADE_OUT_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PLUNGER_FADE_OUT_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getPlungerInFaceTime() const
{
    float result;
    bool is_set = false;
    process(PLUNGER_IN_FACE_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(PLUNGER_IN_FACE_TIME).c_str());
    return result;
}

std::vector<float> AbstractCharacteristics::getStartupTime() const
{
    std::vector<float> result;
    bool is_set = false;
    process(STARTUP_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STARTUP_TIME).c_str());
    return result;
}

std::vector<float> AbstractCharacteristics::getStartupBoost() const
{
    std::vector<float> result;
    bool is_set = false;
    process(STARTUP_BOOST, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(STARTUP_BOOST).c_str());
    return result;
}

float AbstractCharacteristics::getRescueDuration() const
{
    float result;
    bool is_set = false;
    process(RESCUE_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(RESCUE_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getRescueVertOffset() const
{
    float result;
    bool is_set = false;
    process(RESCUE_VERT_OFFSET, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(RESCUE_VERT_OFFSET).c_str());
    return result;
}

float AbstractCharacteristics::getRescueHeight() const
{
    float result;
    bool is_set = false;
    process(RESCUE_HEIGHT, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(RESCUE_HEIGHT).c_str());
    return result;
}

float AbstractCharacteristics::getExplosionDuration() const
{
    float result;
    bool is_set = false;
    process(EXPLOSION_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(EXPLOSION_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getExplosionRadius() const
{
    float result;
    bool is_set = false;
    process(EXPLOSION_RADIUS, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(EXPLOSION_RADIUS).c_str());
    return result;
}

float AbstractCharacteristics::getExplosionInvulnerabilityTime() const
{
    float result;
    bool is_set = false;
    process(EXPLOSION_INVULNERABILITY_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(EXPLOSION_INVULNERABILITY_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getNitroDuration() const
{
    float result;
    bool is_set = false;
    process(NITRO_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getNitroEngineForce() const
{
    float result;
    bool is_set = false;
    process(NITRO_ENGINE_FORCE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_ENGINE_FORCE).c_str());
    return result;
}

float AbstractCharacteristics::getNitroConsumption() const
{
    float result;
    bool is_set = false;
    process(NITRO_CONSUMPTION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_CONSUMPTION).c_str());
    return result;
}

float AbstractCharacteristics::getNitroSmallContainer() const
{
    float result;
    bool is_set = false;
    process(NITRO_SMALL_CONTAINER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_SMALL_CONTAINER).c_str());
    return result;
}

float AbstractCharacteristics::getNitroBigContainer() const
{
    float result;
    bool is_set = false;
    process(NITRO_BIG_CONTAINER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_BIG_CONTAINER).c_str());
    return result;
}

float AbstractCharacteristics::getNitroMaxSpeedIncrease() const
{
    float result;
    bool is_set = false;
    process(NITRO_MAX_SPEED_INCREASE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_MAX_SPEED_INCREASE).c_str());
    return result;
}

float AbstractCharacteristics::getNitroFadeOutTime() const
{
    float result;
    bool is_set = false;
    process(NITRO_FADE_OUT_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_FADE_OUT_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getNitroMax() const
{
    float result;
    bool is_set = false;
    process(NITRO_MAX, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(NITRO_MAX).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamDuration() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_DURATION, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_DURATION).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamLength() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_LENGTH, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_LENGTH).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamWidth() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_WIDTH, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_WIDTH).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamCollectTime() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_COLLECT_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_COLLECT_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamUseTime() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_USE_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_USE_TIME).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamAddPower() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_ADD_POWER, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_ADD_POWER).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamMinSpeed() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_MIN_SPEED, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_MIN_SPEED).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamMaxSpeedIncrease() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_MAX_SPEED_INCREASE, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_MAX_SPEED_INCREASE).c_str());
    return result;
}

float AbstractCharacteristics::getSlipstreamFadeOutTime() const
{
    float result;
    bool is_set = false;
    process(SLIPSTREAM_FADE_OUT_TIME, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic %s", getName(SLIPSTREAM_FADE_OUT_TIME).c_str());
    return result;
}

