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
    //FIXME is wheelPosition needed??
    /* The following lines are the input for a script that generates code for this class
    Suspension: stiffness, rest, travelCm, expSpringResponse, maxForce
    Stability: rollInfluence, chassisLinearDamping, chassisAngularDamping, downwardImpulseFactor, trackConnectionAccel, smoothFlyingImpulse
    Turn: radius(InterpolationArray), timeFullSteer, timeResetSteer, timeFullSteer(InterpolationArray)
    Engine: power, maxSpeed, brakeFactor, brakeTimeIncrease, maxSpeedReverseRatio
    Gear: switchRatio(std::vector<float>/floatVector), powerIncrease(std::vector<float>/floatVector)
    Mass
    Wheels: dampingRelaxation, dampingCompression, radius, position(std::vector<float>/floatVector)
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
    Startup: time(std::vector<float>/floatVector), boost(std::vector<float>/floatVector)
    Rescue: duration, vertOffset, height
    Explosion: duration, radius, invulnerabilityTime
    Nitro: duration, engineForce, consumption, smallContainer, bigContainer, maxSpeedIncrease, fadeOutTime, max
    Slipstream: duration, length, width, collectTime, useTime, addPower, minSpeed, maxSpeedIncrease, fadeOutTime
    */

public:
    enum CharacteristicType
    {
        // Script-generated content

        // Count
        CHARACTERISTIC_COUNT
    };
private:
    /** The skididing properties for this kart, as a separate object in order
     *  to reduce dependencies (and therefore compile time) when changing
     *  any skidding property. */
    //SkiddingProperties *m_skidding;

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
