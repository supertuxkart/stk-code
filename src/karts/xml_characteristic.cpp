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

#include "karts/xml_characteristic.hpp"

#include "utils/interpolation_array.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include "io/xml_node.hpp"

XmlCharacteristic::XmlCharacteristic(const XMLNode *node) :
    m_values(CHARACTERISTIC_COUNT)
{
    if (node)
        load(node);
}

void XmlCharacteristic::process(CharacteristicType type, Value value, bool *is_set) const
{
    if (m_values[type].empty())
        // That value was not changed in this configuration
        return;

    switch (getType(type))
    {
    case TYPE_FLOAT:
        processFloat(m_values[type], value.f, is_set);
        break;
    case TYPE_BOOL:
        processBool(m_values[type], value.b, is_set);
        break;
    case TYPE_FLOAT_VECTOR:
    {
        const std::vector<std::string> processors =
            StringUtils::split(m_values[type], ' ');
        // If the array should be completely replaced
        // That has to happen when the size is not the same or it is not yet set
        bool shouldReplace = false;
        if (*is_set)
        {
            if (processors.size() != value.fv->size())
                shouldReplace = true;
            else
            {
                std::vector<float>::iterator fit = value.fv->begin();
                for (std::vector<std::string>::const_iterator it = processors.begin();
                     it != processors.end(); it++, fit++)
                {
                    processFloat(*it, &*fit, is_set);
                    if (!*is_set)
                    {
                        Log::error("XmlCharacteristic::process", "Can't process %s",
                            it->c_str());
                        value.fv->clear();
                        break;
                    }
                }
            }
        }
        else
            shouldReplace = true;

        if (shouldReplace)
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
                    Log::error("XmlCharacteristic::process", "Can't process %s",
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
        // If the interpolation array should be completely replaced
        // That has to happen when the format is not the same
        bool shouldReplace = false;
        if (*is_set)
        {
            if (processors.size() != value.fv->size())
                shouldReplace = true;
            else
            {
                for (std::vector<std::string>::const_iterator it = processors.begin();
                     it != processors.end(); it++)
                {
                    std::vector<std::string> pair = StringUtils::split(*it, ':');
                    if (pair.size() != 2)
                        Log::error("XmlCharacteristic::process",
                            "Can't process %s: Wrong format", getName(type).c_str());
                    else
                    {
                        float x;
                        if (!StringUtils::fromString(pair[0], x))
                            Log::error("XmlCharacteristic::process",
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
                                if (!*is_set)
                                {
                                    Log::error("XmlCharacteristic::process", "Can't process %s",
                                        pair[1].c_str());
                                    value.fv->clear();
                                    break;
                                }
                            }
                            if (!found)
                            {
                                // The searched value was not found so we have
                                // a different format
                                shouldReplace = true;
                                break;
                            }
                        }
                    }
                }
            }
        } else
            // It's not yet set, so we will the current content
            shouldReplace = true;

        if (shouldReplace)
        {
            // Replace all values
            for (std::vector<std::string>::const_iterator it = processors.begin();
                 it != processors.end(); it++)
            {
                std::vector<std::string> pair = StringUtils::split(*it,':');
                if (pair.size() != 2)
                    Log::error("XmlCharacteristic::process",
                        "Can't process %s: Wrong format", getName(type).c_str());
                else
                {
                    float x;
                    if (!StringUtils::fromString(pair[0], x))
                        Log::error("XmlCharacteristic::process",
                            "Can't process %s: Not a float", getName(type).c_str());
                    else
                    {
                        float val;
                        *is_set = false;
                        processFloat(pair[1], &val, is_set);
                        if (!*is_set)
                        {
                            Log::error("XmlCharacteristic::process", "Can't process %s",
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
    default:
        Log::fatal("XmlCharacteristic::process", "Unknown type for %s", getName(type).c_str());
    }
}

void XmlCharacteristic::processFloat(const std::string &processor, float *value, bool *is_set)
{
    // Split the string by operators
    static const std::string operators = "*/+-";
    std::vector<std::string> parts;
    std::vector<std::string> operations;
    std::size_t pos = 0;
    std::size_t pos2;
    while ((pos2 = processor.find_first_of(operators, pos)) != std::string::npos)
    {
        parts.push_back(processor.substr(pos, pos2));
        operations.push_back(processor.substr(pos2, pos2 + 1));
        pos = pos2 + 1;
    }
    parts.push_back(processor.substr(pos));

    // Compute the result
    float x = *value;
    std::size_t index = 0;
    // If nothing preceeds the first operator, insert x
    if (parts[index].empty())
    {
        if (!*is_set)
        {
            Log::error("XmlCharacteristic::processFloat", "x is unknown");
            return;
        }
        // - is a special case: We don't take e.g. "-5" as relative, it
        // describes a negative number
        else if (operations[index] == "-")
            *value = 0;
        else
            *value = x;
    }
    else
    {
        float val;
    	if (!StringUtils::fromString(parts[index], val))
        {
            Log::fatal("XmlCharacteristic::processFloat",
                "Can't parse %s: Not a float", parts[index].c_str());
            return;
        }
        *value = val;
    }
    index++;
    for (; index < parts.size(); index++)
    {
        float val;
        if (parts[index] == "x" || parts[index] == "X")
            val = x;
        else if (!StringUtils::fromString(parts[index], val))
        {
            Log::fatal("XmlCharacteristic::processFloat",
                "Can't parse %s: Not a float", parts[index].c_str());
            return;
        }
        if (operations[index - 1] == "*")
            *value *= val;
        else if (operations[index - 1] == "/")
            *value /= val;
        else if (operations[index - 1] == "+")
            *value += val;
        else if (operations[index - 1] == "-")
            *value -= val;
        else
            Log::fatal("XmlCharacteristic::processFloat",
                "Unknown operator (%s)", operations[index - 1].c_str());
    }
    *is_set = true;
}

void XmlCharacteristic::processBool(const std::string &processor, bool *value, bool *is_set)
{
    if (processor == "true")
    {
        *value = true;
        *is_set = true;
    }
    else if (processor == "false")
    {
        *value = false;
        *is_set = true;
    }
    else
        Log::error("XmlCharacteristic::processBool", "Can't parse %s: Not a bool", processor.c_str());
}

void XmlCharacteristic::load(const XMLNode *node)
{
    // Script-generated content getXml
    if (const XMLNode *sub_node = node->getNode("suspension"))
    {
        sub_node->get("stiffness", &m_values[SUSPENSION_STIFFNESS]);
        sub_node->get("rest", &m_values[SUSPENSION_REST]);
        sub_node->get("travel-cm", &m_values[SUSPENSION_TRAVEL_CM]);
        sub_node->get("exp-spring-response", &m_values[SUSPENSION_EXP_SPRING_RESPONSE]);
        sub_node->get("max-force", &m_values[SUSPENSION_MAX_FORCE]);
    }

    if (const XMLNode *sub_node = node->getNode("stability"))
    {
        sub_node->get("roll-influence", &m_values[STABILITY_ROLL_INFLUENCE]);
        sub_node->get("chassis-linear-damping", &m_values[STABILITY_CHASSIS_LINEAR_DAMPING]);
        sub_node->get("chassis-angular-damping", &m_values[STABILITY_CHASSIS_ANGULAR_DAMPING]);
        sub_node->get("downward-impulse-factor", &m_values[STABILITY_DOWNWARD_IMPULSE_FACTOR]);
        sub_node->get("track-connection-accel", &m_values[STABILITY_TRACK_CONNECTION_ACCEL]);
        sub_node->get("smooth-flying-impulse", &m_values[STABILITY_SMOOTH_FLYING_IMPULSE]);
    }

    if (const XMLNode *sub_node = node->getNode("turn"))
    {
        sub_node->get("radius", &m_values[TURN_RADIUS]);
        sub_node->get("time-reset-steer", &m_values[TURN_TIME_RESET_STEER]);
        sub_node->get("time-full-steer", &m_values[TURN_TIME_FULL_STEER]);
    }

    if (const XMLNode *sub_node = node->getNode("engine"))
    {
        sub_node->get("power", &m_values[ENGINE_POWER]);
        sub_node->get("max-speed", &m_values[ENGINE_MAX_SPEED]);
        sub_node->get("brake-factor", &m_values[ENGINE_BRAKE_FACTOR]);
        sub_node->get("brake-time-increase", &m_values[ENGINE_BRAKE_TIME_INCREASE]);
        sub_node->get("max-speed-reverse-ratio", &m_values[ENGINE_MAX_SPEED_REVERSE_RATIO]);
    }

    if (const XMLNode *sub_node = node->getNode("gear"))
    {
        sub_node->get("switch-ratio", &m_values[GEAR_SWITCH_RATIO]);
        sub_node->get("power-increase", &m_values[GEAR_POWER_INCREASE]);
    }

    if (const XMLNode *sub_node = node->getNode("mass"))
    {
        sub_node->get("value", &m_values[MASS]);
    }

    if (const XMLNode *sub_node = node->getNode("wheels"))
    {
        sub_node->get("damping-relaxation", &m_values[WHEELS_DAMPING_RELAXATION]);
        sub_node->get("damping-compression", &m_values[WHEELS_DAMPING_COMPRESSION]);
        sub_node->get("radius", &m_values[WHEELS_RADIUS]);
        sub_node->get("position", &m_values[WHEELS_POSITION]);
    }

    if (const XMLNode *sub_node = node->getNode("camera"))
    {
        sub_node->get("distance", &m_values[CAMERA_DISTANCE]);
        sub_node->get("forward-up-angle", &m_values[CAMERA_FORWARD_UP_ANGLE]);
        sub_node->get("backward-up-angle", &m_values[CAMERA_BACKWARD_UP_ANGLE]);
    }

    if (const XMLNode *sub_node = node->getNode("jump"))
    {
        sub_node->get("animation-time", &m_values[JUMP_ANIMATION_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("lean"))
    {
        sub_node->get("max", &m_values[LEAN_MAX]);
        sub_node->get("speed", &m_values[LEAN_SPEED]);
    }

    if (const XMLNode *sub_node = node->getNode("anvil"))
    {
        sub_node->get("duration", &m_values[ANVIL_DURATION]);
        sub_node->get("weight", &m_values[ANVIL_WEIGHT]);
        sub_node->get("speed-factor", &m_values[ANVIL_SPEED_FACTOR]);
    }

    if (const XMLNode *sub_node = node->getNode("parachute"))
    {
        sub_node->get("friction", &m_values[PARACHUTE_FRICTION]);
        sub_node->get("duration", &m_values[PARACHUTE_DURATION]);
        sub_node->get("duration-other", &m_values[PARACHUTE_DURATION_OTHER]);
        sub_node->get("lbound-fraction", &m_values[PARACHUTE_LBOUND_FRACTION]);
        sub_node->get("ubound-fraction", &m_values[PARACHUTE_UBOUND_FRACTION]);
        sub_node->get("max-speed", &m_values[PARACHUTE_MAX_SPEED]);
    }

    if (const XMLNode *sub_node = node->getNode("bubblegum"))
    {
        sub_node->get("duration", &m_values[BUBBLEGUM_DURATION]);
        sub_node->get("speed-fraction", &m_values[BUBBLEGUM_SPEED_FRACTION]);
        sub_node->get("torque", &m_values[BUBBLEGUM_TORQUE]);
        sub_node->get("fade-in-time", &m_values[BUBBLEGUM_FADE_IN_TIME]);
        sub_node->get("shield-duration", &m_values[BUBBLEGUM_SHIELD_DURATION]);
    }

    if (const XMLNode *sub_node = node->getNode("zipper"))
    {
        sub_node->get("duration", &m_values[ZIPPER_DURATION]);
        sub_node->get("force", &m_values[ZIPPER_FORCE]);
        sub_node->get("speed-gain", &m_values[ZIPPER_SPEED_GAIN]);
        sub_node->get("max-speed-increase", &m_values[ZIPPER_MAX_SPEED_INCREASE]);
        sub_node->get("fade-out-time", &m_values[ZIPPER_FADE_OUT_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("swatter"))
    {
        sub_node->get("duration", &m_values[SWATTER_DURATION]);
        sub_node->get("distance", &m_values[SWATTER_DISTANCE]);
        sub_node->get("squash-duration", &m_values[SWATTER_SQUASH_DURATION]);
        sub_node->get("squash-slowdown", &m_values[SWATTER_SQUASH_SLOWDOWN]);
    }

    if (const XMLNode *sub_node = node->getNode("plunger"))
    {
        sub_node->get("band-max-length", &m_values[PLUNGER_BAND_MAX_LENGTH]);
        sub_node->get("band-force", &m_values[PLUNGER_BAND_FORCE]);
        sub_node->get("band-duration", &m_values[PLUNGER_BAND_DURATION]);
        sub_node->get("band-speed-increase", &m_values[PLUNGER_BAND_SPEED_INCREASE]);
        sub_node->get("band-fade-out-time", &m_values[PLUNGER_BAND_FADE_OUT_TIME]);
        sub_node->get("in-face-time", &m_values[PLUNGER_IN_FACE_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("startup"))
    {
        sub_node->get("time", &m_values[STARTUP_TIME]);
        sub_node->get("boost", &m_values[STARTUP_BOOST]);
    }

    if (const XMLNode *sub_node = node->getNode("rescue"))
    {
        sub_node->get("duration", &m_values[RESCUE_DURATION]);
        sub_node->get("vert-offset", &m_values[RESCUE_VERT_OFFSET]);
        sub_node->get("height", &m_values[RESCUE_HEIGHT]);
    }

    if (const XMLNode *sub_node = node->getNode("explosion"))
    {
        sub_node->get("duration", &m_values[EXPLOSION_DURATION]);
        sub_node->get("radius", &m_values[EXPLOSION_RADIUS]);
        sub_node->get("invulnerability-time", &m_values[EXPLOSION_INVULNERABILITY_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("nitro"))
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

    if (const XMLNode *sub_node = node->getNode("slipstream"))
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

