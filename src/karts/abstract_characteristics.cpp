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

void AbstractCharacteristics::process(CharacteristicType type, Value value, bool *isSet) const
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

// Script-generated getter
float AbstractCharacteristics::getSuspensionStiffness() const
{
    float result;
    bool isSet = false;
    process(SUSPENSION_STIFFNESS, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SUSPENSION_STIFFNESS");
    return result;
}

float AbstractCharacteristics::getSuspensionRest() const
{
    float result;
    bool isSet = false;
    process(SUSPENSION_REST, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SUSPENSION_REST");
    return result;
}

float AbstractCharacteristics::getSuspensionTravelCm() const
{
    float result;
    bool isSet = false;
    process(SUSPENSION_TRAVEL_CM, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SUSPENSION_TRAVEL_CM");
    return result;
}

float AbstractCharacteristics::getSuspensionExpSpringResponse() const
{
    float result;
    bool isSet = false;
    process(SUSPENSION_EXP_SPRING_RESPONSE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SUSPENSION_EXP_SPRING_RESPONSE");
    return result;
}

float AbstractCharacteristics::getSuspensionMaxForce() const
{
    float result;
    bool isSet = false;
    process(SUSPENSION_MAX_FORCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SUSPENSION_MAX_FORCE");
    return result;
}

float AbstractCharacteristics::getStabilityRollInfluence() const
{
    float result;
    bool isSet = false;
    process(STABILITY_ROLL_INFLUENCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STABILITY_ROLL_INFLUENCE");
    return result;
}

float AbstractCharacteristics::getStabilityChassisLinearDamping() const
{
    float result;
    bool isSet = false;
    process(STABILITY_CHASSIS_LINEAR_DAMPING, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STABILITY_CHASSIS_LINEAR_DAMPING");
    return result;
}

float AbstractCharacteristics::getStabilityChassisAngularDamping() const
{
    float result;
    bool isSet = false;
    process(STABILITY_CHASSIS_ANGULAR_DAMPING, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STABILITY_CHASSIS_ANGULAR_DAMPING");
    return result;
}

float AbstractCharacteristics::getStabilityDownwardImpulseFactor() const
{
    float result;
    bool isSet = false;
    process(STABILITY_DOWNWARD_IMPULSE_FACTOR, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STABILITY_DOWNWARD_IMPULSE_FACTOR");
    return result;
}

float AbstractCharacteristics::getStabilityTrackConnectionAccel() const
{
    float result;
    bool isSet = false;
    process(STABILITY_TRACK_CONNECTION_ACCEL, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STABILITY_TRACK_CONNECTION_ACCEL");
    return result;
}

float AbstractCharacteristics::getStabilitySmoothFlyingImpulse() const
{
    float result;
    bool isSet = false;
    process(STABILITY_SMOOTH_FLYING_IMPULSE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STABILITY_SMOOTH_FLYING_IMPULSE");
    return result;
}

InterpolationArray&& AbstractCharacteristics::getTurnRadius() const
{
    InterpolationArray result;
    bool isSet = false;
    process(TURN_RADIUS, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic TURN_RADIUS");
    return std::move(result);
}

float AbstractCharacteristics::getTurnTimeResetSteer() const
{
    float result;
    bool isSet = false;
    process(TURN_TIME_RESET_STEER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic TURN_TIME_RESET_STEER");
    return result;
}

InterpolationArray&& AbstractCharacteristics::getTurnTimeFullSteer() const
{
    InterpolationArray result;
    bool isSet = false;
    process(TURN_TIME_FULL_STEER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic TURN_TIME_FULL_STEER");
    return std::move(result);
}

float AbstractCharacteristics::getEnginePower() const
{
    float result;
    bool isSet = false;
    process(ENGINE_POWER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ENGINE_POWER");
    return result;
}

float AbstractCharacteristics::getEngineMaxSpeed() const
{
    float result;
    bool isSet = false;
    process(ENGINE_MAX_SPEED, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ENGINE_MAX_SPEED");
    return result;
}

float AbstractCharacteristics::getEngineBrakeFactor() const
{
    float result;
    bool isSet = false;
    process(ENGINE_BRAKE_FACTOR, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ENGINE_BRAKE_FACTOR");
    return result;
}

float AbstractCharacteristics::getEngineBrakeTimeIncrease() const
{
    float result;
    bool isSet = false;
    process(ENGINE_BRAKE_TIME_INCREASE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ENGINE_BRAKE_TIME_INCREASE");
    return result;
}

float AbstractCharacteristics::getEngineMaxSpeedReverseRatio() const
{
    float result;
    bool isSet = false;
    process(ENGINE_MAX_SPEED_REVERSE_RATIO, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ENGINE_MAX_SPEED_REVERSE_RATIO");
    return result;
}

std::vector<float>&& AbstractCharacteristics::getGearSwitchRatio() const
{
    std::vector<float> result;
    bool isSet = false;
    process(GEAR_SWITCH_RATIO, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic GEAR_SWITCH_RATIO");
    return std::move(result);
}

std::vector<float>&& AbstractCharacteristics::getGearPowerIncrease() const
{
    std::vector<float> result;
    bool isSet = false;
    process(GEAR_POWER_INCREASE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic GEAR_POWER_INCREASE");
    return std::move(result);
}

float AbstractCharacteristics::getMass() const
{
    float result;
    bool isSet = false;
    process(MASS, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic MASS");
    return result;
}

float AbstractCharacteristics::getWheelsDampingRelaxation() const
{
    float result;
    bool isSet = false;
    process(WHEELS_DAMPING_RELAXATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic WHEELS_DAMPING_RELAXATION");
    return result;
}

float AbstractCharacteristics::getWheelsDampingCompression() const
{
    float result;
    bool isSet = false;
    process(WHEELS_DAMPING_COMPRESSION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic WHEELS_DAMPING_COMPRESSION");
    return result;
}

float AbstractCharacteristics::getWheelsRadius() const
{
    float result;
    bool isSet = false;
    process(WHEELS_RADIUS, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic WHEELS_RADIUS");
    return result;
}

std::vector<float>&& AbstractCharacteristics::getWheelsPosition() const
{
    std::vector<float> result;
    bool isSet = false;
    process(WHEELS_POSITION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic WHEELS_POSITION");
    return std::move(result);
}

float AbstractCharacteristics::getCameraDistance() const
{
    float result;
    bool isSet = false;
    process(CAMERA_DISTANCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic CAMERA_DISTANCE");
    return result;
}

float AbstractCharacteristics::getCameraForwardUpAngle() const
{
    float result;
    bool isSet = false;
    process(CAMERA_FORWARD_UP_ANGLE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic CAMERA_FORWARD_UP_ANGLE");
    return result;
}

float AbstractCharacteristics::getCameraBackwardUpAngle() const
{
    float result;
    bool isSet = false;
    process(CAMERA_BACKWARD_UP_ANGLE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic CAMERA_BACKWARD_UP_ANGLE");
    return result;
}

float AbstractCharacteristics::getJumpAnimationTime() const
{
    float result;
    bool isSet = false;
    process(JUMP_ANIMATION_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic JUMP_ANIMATION_TIME");
    return result;
}

float AbstractCharacteristics::getLeanMax() const
{
    float result;
    bool isSet = false;
    process(LEAN_MAX, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic LEAN_MAX");
    return result;
}

float AbstractCharacteristics::getLeanSpeed() const
{
    float result;
    bool isSet = false;
    process(LEAN_SPEED, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic LEAN_SPEED");
    return result;
}

float AbstractCharacteristics::getAnvilDuration() const
{
    float result;
    bool isSet = false;
    process(ANVIL_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ANVIL_DURATION");
    return result;
}

float AbstractCharacteristics::getAnvilWeight() const
{
    float result;
    bool isSet = false;
    process(ANVIL_WEIGHT, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ANVIL_WEIGHT");
    return result;
}

float AbstractCharacteristics::getAnvilSpeedFactor() const
{
    float result;
    bool isSet = false;
    process(ANVIL_SPEED_FACTOR, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ANVIL_SPEED_FACTOR");
    return result;
}

float AbstractCharacteristics::getParachuteFriction() const
{
    float result;
    bool isSet = false;
    process(PARACHUTE_FRICTION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PARACHUTE_FRICTION");
    return result;
}

float AbstractCharacteristics::getParachuteDuration() const
{
    float result;
    bool isSet = false;
    process(PARACHUTE_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PARACHUTE_DURATION");
    return result;
}

float AbstractCharacteristics::getParachuteDurationOther() const
{
    float result;
    bool isSet = false;
    process(PARACHUTE_DURATION_OTHER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PARACHUTE_DURATION_OTHER");
    return result;
}

float AbstractCharacteristics::getParachuteLboundFranction() const
{
    float result;
    bool isSet = false;
    process(PARACHUTE_LBOUND_FRANCTION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PARACHUTE_LBOUND_FRANCTION");
    return result;
}

float AbstractCharacteristics::getParachuteUboundFranction() const
{
    float result;
    bool isSet = false;
    process(PARACHUTE_UBOUND_FRANCTION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PARACHUTE_UBOUND_FRANCTION");
    return result;
}

float AbstractCharacteristics::getParachuteMaxSpeed() const
{
    float result;
    bool isSet = false;
    process(PARACHUTE_MAX_SPEED, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PARACHUTE_MAX_SPEED");
    return result;
}

float AbstractCharacteristics::getBubblegumDuration() const
{
    float result;
    bool isSet = false;
    process(BUBBLEGUM_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic BUBBLEGUM_DURATION");
    return result;
}

float AbstractCharacteristics::getBubblegumSpeedFraction() const
{
    float result;
    bool isSet = false;
    process(BUBBLEGUM_SPEED_FRACTION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic BUBBLEGUM_SPEED_FRACTION");
    return result;
}

float AbstractCharacteristics::getBubblegumTorque() const
{
    float result;
    bool isSet = false;
    process(BUBBLEGUM_TORQUE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic BUBBLEGUM_TORQUE");
    return result;
}

float AbstractCharacteristics::getBubblegumFadeInTime() const
{
    float result;
    bool isSet = false;
    process(BUBBLEGUM_FADE_IN_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic BUBBLEGUM_FADE_IN_TIME");
    return result;
}

float AbstractCharacteristics::getBubblegumShieldDuration() const
{
    float result;
    bool isSet = false;
    process(BUBBLEGUM_SHIELD_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic BUBBLEGUM_SHIELD_DURATION");
    return result;
}

float AbstractCharacteristics::getZipperDuration() const
{
    float result;
    bool isSet = false;
    process(ZIPPER_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ZIPPER_DURATION");
    return result;
}

float AbstractCharacteristics::getZipperForce() const
{
    float result;
    bool isSet = false;
    process(ZIPPER_FORCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ZIPPER_FORCE");
    return result;
}

float AbstractCharacteristics::getZipperSpeedGain() const
{
    float result;
    bool isSet = false;
    process(ZIPPER_SPEED_GAIN, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ZIPPER_SPEED_GAIN");
    return result;
}

float AbstractCharacteristics::getZipperSpeedIncrease() const
{
    float result;
    bool isSet = false;
    process(ZIPPER_SPEED_INCREASE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ZIPPER_SPEED_INCREASE");
    return result;
}

float AbstractCharacteristics::getZipperFadeOutTime() const
{
    float result;
    bool isSet = false;
    process(ZIPPER_FADE_OUT_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic ZIPPER_FADE_OUT_TIME");
    return result;
}

float AbstractCharacteristics::getSwatterDuration() const
{
    float result;
    bool isSet = false;
    process(SWATTER_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SWATTER_DURATION");
    return result;
}

float AbstractCharacteristics::getSwatterDistance() const
{
    float result;
    bool isSet = false;
    process(SWATTER_DISTANCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SWATTER_DISTANCE");
    return result;
}

float AbstractCharacteristics::getSwatterSquashDuration() const
{
    float result;
    bool isSet = false;
    process(SWATTER_SQUASH_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SWATTER_SQUASH_DURATION");
    return result;
}

float AbstractCharacteristics::getSwatterSquashSlowdown() const
{
    float result;
    bool isSet = false;
    process(SWATTER_SQUASH_SLOWDOWN, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SWATTER_SQUASH_SLOWDOWN");
    return result;
}

float AbstractCharacteristics::getPlungerMaxLength() const
{
    float result;
    bool isSet = false;
    process(PLUNGER_MAX_LENGTH, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PLUNGER_MAX_LENGTH");
    return result;
}

float AbstractCharacteristics::getPlungerForce() const
{
    float result;
    bool isSet = false;
    process(PLUNGER_FORCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PLUNGER_FORCE");
    return result;
}

float AbstractCharacteristics::getPlungerDuration() const
{
    float result;
    bool isSet = false;
    process(PLUNGER_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PLUNGER_DURATION");
    return result;
}

float AbstractCharacteristics::getPlungerSpeedIncrease() const
{
    float result;
    bool isSet = false;
    process(PLUNGER_SPEED_INCREASE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PLUNGER_SPEED_INCREASE");
    return result;
}

float AbstractCharacteristics::getPlungerFadeOutTime() const
{
    float result;
    bool isSet = false;
    process(PLUNGER_FADE_OUT_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PLUNGER_FADE_OUT_TIME");
    return result;
}

float AbstractCharacteristics::getPlungerInFaceTime() const
{
    float result;
    bool isSet = false;
    process(PLUNGER_IN_FACE_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic PLUNGER_IN_FACE_TIME");
    return result;
}

std::vector<float>&& AbstractCharacteristics::getStartupTime() const
{
    std::vector<float> result;
    bool isSet = false;
    process(STARTUP_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STARTUP_TIME");
    return std::move(result);
}

std::vector<float>&& AbstractCharacteristics::getStartupBoost() const
{
    std::vector<float> result;
    bool isSet = false;
    process(STARTUP_BOOST, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic STARTUP_BOOST");
    return std::move(result);
}

float AbstractCharacteristics::getRescueDuration() const
{
    float result;
    bool isSet = false;
    process(RESCUE_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic RESCUE_DURATION");
    return result;
}

float AbstractCharacteristics::getRescueVertOffset() const
{
    float result;
    bool isSet = false;
    process(RESCUE_VERT_OFFSET, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic RESCUE_VERT_OFFSET");
    return result;
}

float AbstractCharacteristics::getRescueHeight() const
{
    float result;
    bool isSet = false;
    process(RESCUE_HEIGHT, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic RESCUE_HEIGHT");
    return result;
}

float AbstractCharacteristics::getExplosionDuration() const
{
    float result;
    bool isSet = false;
    process(EXPLOSION_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic EXPLOSION_DURATION");
    return result;
}

float AbstractCharacteristics::getExplosionRadius() const
{
    float result;
    bool isSet = false;
    process(EXPLOSION_RADIUS, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic EXPLOSION_RADIUS");
    return result;
}

float AbstractCharacteristics::getExplosionInvulnerabilityTime() const
{
    float result;
    bool isSet = false;
    process(EXPLOSION_INVULNERABILITY_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic EXPLOSION_INVULNERABILITY_TIME");
    return result;
}

float AbstractCharacteristics::getNitroDuration() const
{
    float result;
    bool isSet = false;
    process(NITRO_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_DURATION");
    return result;
}

float AbstractCharacteristics::getNitroEngineForce() const
{
    float result;
    bool isSet = false;
    process(NITRO_ENGINE_FORCE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_ENGINE_FORCE");
    return result;
}

float AbstractCharacteristics::getNitroConsumption() const
{
    float result;
    bool isSet = false;
    process(NITRO_CONSUMPTION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_CONSUMPTION");
    return result;
}

float AbstractCharacteristics::getNitroSmallContainer() const
{
    float result;
    bool isSet = false;
    process(NITRO_SMALL_CONTAINER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_SMALL_CONTAINER");
    return result;
}

float AbstractCharacteristics::getNitroBigContainer() const
{
    float result;
    bool isSet = false;
    process(NITRO_BIG_CONTAINER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_BIG_CONTAINER");
    return result;
}

float AbstractCharacteristics::getNitroMaxSpeedIncrease() const
{
    float result;
    bool isSet = false;
    process(NITRO_MAX_SPEED_INCREASE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_MAX_SPEED_INCREASE");
    return result;
}

float AbstractCharacteristics::getNitroFadeOutTime() const
{
    float result;
    bool isSet = false;
    process(NITRO_FADE_OUT_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_FADE_OUT_TIME");
    return result;
}

float AbstractCharacteristics::getNitroMax() const
{
    float result;
    bool isSet = false;
    process(NITRO_MAX, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic NITRO_MAX");
    return result;
}

float AbstractCharacteristics::getSlipstreamDuration() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_DURATION, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_DURATION");
    return result;
}

float AbstractCharacteristics::getSlipstreamLength() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_LENGTH, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_LENGTH");
    return result;
}

float AbstractCharacteristics::getSlipstreamWidth() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_WIDTH, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_WIDTH");
    return result;
}

float AbstractCharacteristics::getSlipstreamCollectTime() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_COLLECT_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_COLLECT_TIME");
    return result;
}

float AbstractCharacteristics::getSlipstreamUseTime() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_USE_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_USE_TIME");
    return result;
}

float AbstractCharacteristics::getSlipstreamAddPower() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_ADD_POWER, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_ADD_POWER");
    return result;
}

float AbstractCharacteristics::getSlipstreamMinSpeed() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_MIN_SPEED, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_MIN_SPEED");
    return result;
}

float AbstractCharacteristics::getSlipstreamMaxSpeedIncrease() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_MAX_SPEED_INCREASE, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_MAX_SPEED_INCREASE");
    return result;
}

float AbstractCharacteristics::getSlipstreamFadeOutTime() const
{
    float result;
    bool isSet = false;
    process(SLIPSTREAM_FADE_OUT_TIME, &result, &isSet);
    if (!isSet)
        Log::fatal("AbstractCharacteristics", "Can't get characteristic SLIPSTREAM_FADE_OUT_TIME");
    return result;
}

