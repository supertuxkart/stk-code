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

#include "karts/xml_characteristics.hpp"

#include "utils/interpolation_array.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include "io/xml_node.hpp"

XmlCharacteristics::XmlCharacteristics(const XMLNode *node) :
    m_values(CHARACTERISTIC_COUNT),
    m_skidding(nullptr)
{
    if (node)
        load(node);
}

const SkiddingProperties* XmlCharacteristics::getSkiddingProperties() const
{
    return m_skidding;
}

void XmlCharacteristics::process(CharacteristicType type, Value value, bool *is_set) const
{
    switch (getType(type))
    {
    case TYPE_FLOAT:
        processFloat(m_values[type], value.f, is_set);
        break;
    case TYPE_FLOAT_VECTOR:
    {
        const std::vector<std::string> processors =
            StringUtils::split(m_values[type], ' ');
        if (*is_set)
        {
            if (processors.size() != value.fv->size())
            {
                Log::error("XmlCharacteristics::process",
                    "FloatVectors have different sizes for %s",
                    getName(type).c_str());
                break;
            }
            std::vector<float>::iterator fit = value.fv->begin();
            for (std::vector<std::string>::const_iterator it = processors.begin();
                 it != processors.end(); it++, fit++)
                processFloat(*it, &*fit, is_set);
        }
        else
        {
            value.fv->resize(processors.size());
            std::vector<float>::iterator fit = value.fv->begin();
            for (std::vector<std::string>::const_iterator it = processors.begin();
                 it != processors.end(); it++, fit++)
            {
                *is_set = false;
                processFloat(*it, &*fit, is_set);

                if (!*is_set)
                {
                    Log::error("XmlCharacteristics::process", "Can't process %s",
                        getName(type).c_str());
                    value.fv->clear();
                    break;
                }
            }
        }
        break;
    }
    case TYPE_INTERPOLATION_ARRAY:
    {
        const std::vector<std::string> processors =
            StringUtils::split(m_values[type], ' ');
        if (*is_set)
        {
            if (processors.size() != value.fv->size())
            {
                Log::error("XmlCharacteristics::process",
                    "InterpolationArrays have different sizes for %s",
                    getName(type).c_str());
                break;
            }
            else
            {
                for (std::vector<std::string>::const_iterator it = processors.begin();
                     it != processors.end(); it++)
                {
                    std::vector<std::string> pair = StringUtils::split(*it,':');
                    if (!pair.size() == 2)
                        Log::error("XmlCharacteristics::process",
                            "Can't process %s: Wrong format", getName(type).c_str());
                    else
                    {
                        float x;
                        if (!StringUtils::fromString(pair[0], x))
                            Log::error("XmlCharacteristics::process",
                                "Can't process %s: Not a float", getName(type).c_str());
                        else
                        {
                            // Search the index of this x value
                            bool found = false;
                            for (unsigned int i = 0; i < value.ia->size(); i++)
                            {
                                if (value.ia->getX(i) == x)
                                {
                                    float val;
                                    processFloat(pair[1], &val, is_set);
                                    value.ia->setY(i, val);
                                    break;
                                }
                            }
                            if (!found)
                                Log::error("XmlCharacteristics::process",
                                    "Can't find the %f in %s", x,
                                    getName(type).c_str());
                        }
                    }
                }
            }
        }
        else
        {
            for (std::vector<std::string>::const_iterator it = processors.begin();
                 it != processors.end(); it++)
            {
                std::vector<std::string> pair = StringUtils::split(*it,':');
                if (!pair.size() == 2)
                    Log::error("XmlCharacteristics::process",
                        "Can't process %s: Wrong format", getName(type).c_str());
                else
                {
                    float x;
                    if (!StringUtils::fromString(pair[0], x))
                        Log::error("XmlCharacteristics::process",
                            "Can't process %s: Not a float", getName(type).c_str());
                    else
                    {
                        float val;
                        *is_set = false;
                        processFloat(pair[1], &val, is_set);
                        if (!*is_set)
                        {
                            Log::error("XmlCharacteristics::process", "Can't process %s",
                                getName(type).c_str());
                            value.ia->clear();
                            break;
                        }
                        value.ia->push_back(x, val);
                    }
                }
            }
        }
        break;
    }
    }
}

void XmlCharacteristics::processFloat(const std::string &processor, float *value, bool *is_set)
{

}

void XmlCharacteristics::load(const XMLNode *node)
{
    // Script-generated content
    if (const XMLNode *sub_node = node->getNode("Suspension"))
    {
        sub_node->get("stiffness", &m_values[SUSPENSION_STIFFNESS]);
        sub_node->get("rest", &m_values[SUSPENSION_REST]);
        sub_node->get("travel-cm", &m_values[SUSPENSION_TRAVEL_CM]);
        sub_node->get("exp-spring-response", &m_values[SUSPENSION_EXP_SPRING_RESPONSE]);
        sub_node->get("max-force", &m_values[SUSPENSION_MAX_FORCE]);
    }

    if (const XMLNode *sub_node = node->getNode("Stability"))
    {
        sub_node->get("roll-influence", &m_values[STABILITY_ROLL_INFLUENCE]);
        sub_node->get("chassis-linear-damping", &m_values[STABILITY_CHASSIS_LINEAR_DAMPING]);
        sub_node->get("chassis-angular-damping", &m_values[STABILITY_CHASSIS_ANGULAR_DAMPING]);
        sub_node->get("downward-impulse-factor", &m_values[STABILITY_DOWNWARD_IMPULSE_FACTOR]);
        sub_node->get("track-connection-accel", &m_values[STABILITY_TRACK_CONNECTION_ACCEL]);
        sub_node->get("smooth-flying-impulse", &m_values[STABILITY_SMOOTH_FLYING_IMPULSE]);
    }

    if (const XMLNode *sub_node = node->getNode("Turn"))
    {
        sub_node->get("radius", &m_values[TURN_RADIUS]);
        sub_node->get("time-reset-steer", &m_values[TURN_TIME_RESET_STEER]);
        sub_node->get("time-full-steer", &m_values[TURN_TIME_FULL_STEER]);
    }

    if (const XMLNode *sub_node = node->getNode("Engine"))
    {
        sub_node->get("power", &m_values[ENGINE_POWER]);
        sub_node->get("max-speed", &m_values[ENGINE_MAX_SPEED]);
        sub_node->get("brake-factor", &m_values[ENGINE_BRAKE_FACTOR]);
        sub_node->get("brake-time-increase", &m_values[ENGINE_BRAKE_TIME_INCREASE]);
        sub_node->get("max-speed-reverse-ratio", &m_values[ENGINE_MAX_SPEED_REVERSE_RATIO]);
    }

    if (const XMLNode *sub_node = node->getNode("Gear"))
    {
        sub_node->get("switch-ratio", &m_values[GEAR_SWITCH_RATIO]);
        sub_node->get("power-increase", &m_values[GEAR_POWER_INCREASE]);
    }

    if (const XMLNode *sub_node = node->getNode(""))
    {
        sub_node->get("mass", &m_values[MASS]);
    }

    if (const XMLNode *sub_node = node->getNode("Wheels"))
    {
        sub_node->get("damping-relaxation", &m_values[WHEELS_DAMPING_RELAXATION]);
        sub_node->get("damping-compression", &m_values[WHEELS_DAMPING_COMPRESSION]);
        sub_node->get("radius", &m_values[WHEELS_RADIUS]);
        sub_node->get("position", &m_values[WHEELS_POSITION]);
    }

    if (const XMLNode *sub_node = node->getNode("Camera"))
    {
        sub_node->get("distance", &m_values[CAMERA_DISTANCE]);
        sub_node->get("forward-up-angle", &m_values[CAMERA_FORWARD_UP_ANGLE]);
        sub_node->get("backward-up-angle", &m_values[CAMERA_BACKWARD_UP_ANGLE]);
    }

    if (const XMLNode *sub_node = node->getNode("Jump"))
    {
        sub_node->get("animation-time", &m_values[JUMP_ANIMATION_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("Lean"))
    {
        sub_node->get("max", &m_values[LEAN_MAX]);
        sub_node->get("speed", &m_values[LEAN_SPEED]);
    }

    if (const XMLNode *sub_node = node->getNode("Anvil"))
    {
        sub_node->get("duration", &m_values[ANVIL_DURATION]);
        sub_node->get("weight", &m_values[ANVIL_WEIGHT]);
        sub_node->get("speed-factor", &m_values[ANVIL_SPEED_FACTOR]);
    }

    if (const XMLNode *sub_node = node->getNode("Parachute"))
    {
        sub_node->get("friction", &m_values[PARACHUTE_FRICTION]);
        sub_node->get("duration", &m_values[PARACHUTE_DURATION]);
        sub_node->get("duration-other", &m_values[PARACHUTE_DURATION_OTHER]);
        sub_node->get("lbound-franction", &m_values[PARACHUTE_LBOUND_FRANCTION]);
        sub_node->get("ubound-franction", &m_values[PARACHUTE_UBOUND_FRANCTION]);
        sub_node->get("max-speed", &m_values[PARACHUTE_MAX_SPEED]);
    }

    if (const XMLNode *sub_node = node->getNode("Bubblegum"))
    {
        sub_node->get("duration", &m_values[BUBBLEGUM_DURATION]);
        sub_node->get("speed-fraction", &m_values[BUBBLEGUM_SPEED_FRACTION]);
        sub_node->get("torque", &m_values[BUBBLEGUM_TORQUE]);
        sub_node->get("fade-in-time", &m_values[BUBBLEGUM_FADE_IN_TIME]);
        sub_node->get("shield-duration", &m_values[BUBBLEGUM_SHIELD_DURATION]);
    }

    if (const XMLNode *sub_node = node->getNode("Zipper"))
    {
        sub_node->get("duration", &m_values[ZIPPER_DURATION]);
        sub_node->get("force", &m_values[ZIPPER_FORCE]);
        sub_node->get("speed-gain", &m_values[ZIPPER_SPEED_GAIN]);
        sub_node->get("speed-increase", &m_values[ZIPPER_SPEED_INCREASE]);
        sub_node->get("fade-out-time", &m_values[ZIPPER_FADE_OUT_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("Swatter"))
    {
        sub_node->get("duration", &m_values[SWATTER_DURATION]);
        sub_node->get("distance", &m_values[SWATTER_DISTANCE]);
        sub_node->get("squash-duration", &m_values[SWATTER_SQUASH_DURATION]);
        sub_node->get("squash-slowdown", &m_values[SWATTER_SQUASH_SLOWDOWN]);
    }

    if (const XMLNode *sub_node = node->getNode("Plunger"))
    {
        sub_node->get("max-length", &m_values[PLUNGER_MAX_LENGTH]);
        sub_node->get("force", &m_values[PLUNGER_FORCE]);
        sub_node->get("duration", &m_values[PLUNGER_DURATION]);
        sub_node->get("speed-increase", &m_values[PLUNGER_SPEED_INCREASE]);
        sub_node->get("fade-out-time", &m_values[PLUNGER_FADE_OUT_TIME]);
        sub_node->get("in-face-time", &m_values[PLUNGER_IN_FACE_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("Startup"))
    {
        sub_node->get("time", &m_values[STARTUP_TIME]);
        sub_node->get("boost", &m_values[STARTUP_BOOST]);
    }

    if (const XMLNode *sub_node = node->getNode("Rescue"))
    {
        sub_node->get("duration", &m_values[RESCUE_DURATION]);
        sub_node->get("vert-offset", &m_values[RESCUE_VERT_OFFSET]);
        sub_node->get("height", &m_values[RESCUE_HEIGHT]);
    }

    if (const XMLNode *sub_node = node->getNode("Explosion"))
    {
        sub_node->get("duration", &m_values[EXPLOSION_DURATION]);
        sub_node->get("radius", &m_values[EXPLOSION_RADIUS]);
        sub_node->get("invulnerability-time", &m_values[EXPLOSION_INVULNERABILITY_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("Nitro"))
    {
        sub_node->get("duration", &m_values[NITRO_DURATION]);
        sub_node->get("engine-force", &m_values[NITRO_ENGINE_FORCE]);
        sub_node->get("consumption", &m_values[NITRO_CONSUMPTION]);
        sub_node->get("small-container", &m_values[NITRO_SMALL_CONTAINER]);
        sub_node->get("big-container", &m_values[NITRO_BIG_CONTAINER]);
        sub_node->get("max-speed-increase", &m_values[NITRO_MAX_SPEED_INCREASE]);
        sub_node->get("fade-out-time", &m_values[NITRO_FADE_OUT_TIME]);
        sub_node->get("max", &m_values[NITRO_MAX]);
    }

    if (const XMLNode *sub_node = node->getNode("Slipstream"))
    {
        sub_node->get("duration", &m_values[SLIPSTREAM_DURATION]);
        sub_node->get("length", &m_values[SLIPSTREAM_LENGTH]);
        sub_node->get("width", &m_values[SLIPSTREAM_WIDTH]);
        sub_node->get("collect-time", &m_values[SLIPSTREAM_COLLECT_TIME]);
        sub_node->get("use-time", &m_values[SLIPSTREAM_USE_TIME]);
        sub_node->get("add-power", &m_values[SLIPSTREAM_ADD_POWER]);
        sub_node->get("min-speed", &m_values[SLIPSTREAM_MIN_SPEED]);
        sub_node->get("max-speed-increase", &m_values[SLIPSTREAM_MAX_SPEED_INCREASE]);
        sub_node->get("fade-out-time", &m_values[SLIPSTREAM_FADE_OUT_TIME]);
    }
}

