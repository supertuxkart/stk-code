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
}   // XmlCharacteristic constructor

// ----------------------------------------------------------------------------
/** Copies the characteristics from the specified other characteristic class
 *  into this class.
 */
void XmlCharacteristic::copyFrom(const AbstractCharacteristic *other)
{
    const XmlCharacteristic *xc = dynamic_cast<const XmlCharacteristic*>(other);
    assert(xc!=NULL);
    m_values = xc->m_values;
}   // operator=

// ----------------------------------------------------------------------------
/** process will execute the operation that is specified in the saved string.
 *  The format of the operations is specified in kart_characteristics.xml.
 */
void XmlCharacteristic::process(CharacteristicType type, Value value,
                                bool *is_set) const
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
                for (const std::string &processor : processors)
                {
                    processFloat(processor, &*fit, is_set);
                    if (!*is_set)
                    {
                        Log::error("XmlCharacteristic::process", "Can't process %s",
                            processor.c_str());
                        value.fv->clear();
                        break;
                    }
                    fit++;
                }
            }
        }
        else
            shouldReplace = true;

        if (shouldReplace)
        {
            value.fv->resize(processors.size());
            std::vector<float>::iterator fit = value.fv->begin();
            for (const std::string &processor : processors)
            {
                *is_set = false;
                processFloat(processor, &*fit, is_set);

                if (!*is_set)
                {
                    Log::error("XmlCharacteristic::process", "Can't process %s",
                        getName(type).c_str());
                    value.fv->clear();
                    break;
                }
                fit++;
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
                for (const std::string &processor : processors)
                {
                    std::vector<std::string> pair = StringUtils::split(processor, ':');
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
                                    found = true;
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
        }
        else
            // It's not yet set, so we will the current content
            shouldReplace = true;

        if (shouldReplace)
        {
            value.ia->clear();
            // Replace all values
            for (const std::string &processor : processors)
            {
                std::vector<std::string> pair = StringUtils::split(processor,':');
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
        Log::fatal("XmlCharacteristic::process", "Unknown type for %s",
                   getName(type).c_str());
    }
}   // process

// ----------------------------------------------------------------------------
/** Executes an operation on a float value. */
void XmlCharacteristic::processFloat(const std::string &processor, float *value,
                                     bool *is_set)
{
    // Split the string by operators
    static const std::string operators = "*/+-";
    std::vector<std::string> parts;
    std::vector<std::string> operations;
    std::size_t pos = 0;
    std::size_t pos2;
    while ((pos2 = processor.find_first_of(operators, pos)) != std::string::npos)
    {
        std::string s = processor.substr(pos, pos2);
        parts.push_back(processor.substr(pos, pos2-pos));
        operations.push_back(processor.substr(pos2, 1));
        pos = pos2 + 1;
    }
    parts.push_back(processor.substr(pos));

    // Compute the result
    float x = *value;
    std::size_t index = 0;
    // If nothing preceeds the first operator, insert x
    if (parts[index].empty())
    {
        // - is a special case: We don't take e.g. "-5" as relative, it
        // describes a negative number. So 
        if (!*is_set && operations[index] == "-")
            *value = 0;
        else if (!*is_set)
        {
            Log::error("XmlCharacteristic::processFloat", "x is unknown");
            return;
        }
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
}   // processFloat

// ----------------------------------------------------------------------------
/** Executes an operation on a bool value. */
void XmlCharacteristic::processBool(const std::string &processor, bool *value,
                                    bool *is_set)
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
        Log::error("XmlCharacteristic::processBool", "Can't parse %s: Not a bool",
                   processor.c_str());
}   // processBool

// ----------------------------------------------------------------------------
/** Loads all commands from a given xml file.
 *  Non-existing tags will be omitted.
 */
void XmlCharacteristic::load(const XMLNode *node)
{
    // Script-generated content generated by tools/create_kart_properties.py getXml
    // Please don't change the following tag. It will be automatically detected
    // by the script and replace the contained content.
    // To update the code, use tools/update_characteristics.py
    /* <characteristics-start getXml> */
    if (const XMLNode *sub_node = node->getNode("suspension"))
    {
        sub_node->get("stiffness",
            &m_values[SUSPENSION_STIFFNESS]);
        sub_node->get("rest",
            &m_values[SUSPENSION_REST]);
        sub_node->get("travel",
            &m_values[SUSPENSION_TRAVEL]);
        sub_node->get("exp-spring-response",
            &m_values[SUSPENSION_EXP_SPRING_RESPONSE]);
        sub_node->get("max-force",
            &m_values[SUSPENSION_MAX_FORCE]);
    }

    if (const XMLNode *sub_node = node->getNode("stability"))
    {
        sub_node->get("roll-influence",
            &m_values[STABILITY_ROLL_INFLUENCE]);
        sub_node->get("chassis-linear-damping",
            &m_values[STABILITY_CHASSIS_LINEAR_DAMPING]);
        sub_node->get("chassis-angular-damping",
            &m_values[STABILITY_CHASSIS_ANGULAR_DAMPING]);
        sub_node->get("downward-impulse-factor",
            &m_values[STABILITY_DOWNWARD_IMPULSE_FACTOR]);
        sub_node->get("track-connection-accel",
            &m_values[STABILITY_TRACK_CONNECTION_ACCEL]);
        sub_node->get("angular-factor",
            &m_values[STABILITY_ANGULAR_FACTOR]);
        sub_node->get("smooth-flying-impulse",
            &m_values[STABILITY_SMOOTH_FLYING_IMPULSE]);
    }

    if (const XMLNode *sub_node = node->getNode("turn"))
    {
        sub_node->get("radius",
            &m_values[TURN_RADIUS]);
        sub_node->get("time-reset-steer",
            &m_values[TURN_TIME_RESET_STEER]);
        sub_node->get("time-full-steer",
            &m_values[TURN_TIME_FULL_STEER]);
    }

    if (const XMLNode *sub_node = node->getNode("engine"))
    {
        sub_node->get("power",
            &m_values[ENGINE_POWER]);
        sub_node->get("max-speed",
            &m_values[ENGINE_MAX_SPEED]);
        sub_node->get("generic-max-speed",
            &m_values[ENGINE_GENERIC_MAX_SPEED]);
        sub_node->get("brake-factor",
            &m_values[ENGINE_BRAKE_FACTOR]);
        sub_node->get("brake-time-increase",
            &m_values[ENGINE_BRAKE_TIME_INCREASE]);
        sub_node->get("max-speed-reverse-ratio",
            &m_values[ENGINE_MAX_SPEED_REVERSE_RATIO]);
    }

    if (const XMLNode *sub_node = node->getNode("gear"))
    {
        sub_node->get("switch-ratio",
            &m_values[GEAR_SWITCH_RATIO]);
        sub_node->get("power-increase",
            &m_values[GEAR_POWER_INCREASE]);
    }

    if (const XMLNode *sub_node = node->getNode("mass"))
    {
        sub_node->get("value",
            &m_values[MASS]);
    }

    if (const XMLNode *sub_node = node->getNode("wheels"))
    {
        sub_node->get("damping-relaxation",
            &m_values[WHEELS_DAMPING_RELAXATION]);
        sub_node->get("damping-compression",
            &m_values[WHEELS_DAMPING_COMPRESSION]);
    }

    if (const XMLNode *sub_node = node->getNode("jump"))
    {
        sub_node->get("animation-time",
            &m_values[JUMP_ANIMATION_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("lean"))
    {
        sub_node->get("max",
            &m_values[LEAN_MAX]);
        sub_node->get("speed",
            &m_values[LEAN_SPEED]);
    }

    if (const XMLNode *sub_node = node->getNode("anvil"))
    {
        sub_node->get("duration",
            &m_values[ANVIL_DURATION]);
        sub_node->get("weight",
            &m_values[ANVIL_WEIGHT]);
        sub_node->get("speed-factor",
            &m_values[ANVIL_SPEED_FACTOR]);
    }

    if (const XMLNode *sub_node = node->getNode("parachute"))
    {
        sub_node->get("friction",
            &m_values[PARACHUTE_FRICTION]);
        sub_node->get("duration",
            &m_values[PARACHUTE_DURATION]);
        sub_node->get("duration-other",
            &m_values[PARACHUTE_DURATION_OTHER]);
        sub_node->get("duration-rank-mult",
            &m_values[PARACHUTE_DURATION_RANK_MULT]);
        sub_node->get("duration-speed-mult",
            &m_values[PARACHUTE_DURATION_SPEED_MULT]);
        sub_node->get("lbound-fraction",
            &m_values[PARACHUTE_LBOUND_FRACTION]);
        sub_node->get("ubound-fraction",
            &m_values[PARACHUTE_UBOUND_FRACTION]);
        sub_node->get("max-speed",
            &m_values[PARACHUTE_MAX_SPEED]);
    }

    if (const XMLNode *sub_node = node->getNode("friction"))
    {
        sub_node->get("kart-friction",
            &m_values[FRICTION_KART_FRICTION]);
    }

    if (const XMLNode *sub_node = node->getNode("bubblegum"))
    {
        sub_node->get("duration",
            &m_values[BUBBLEGUM_DURATION]);
        sub_node->get("speed-fraction",
            &m_values[BUBBLEGUM_SPEED_FRACTION]);
        sub_node->get("torque",
            &m_values[BUBBLEGUM_TORQUE]);
        sub_node->get("fade-in-time",
            &m_values[BUBBLEGUM_FADE_IN_TIME]);
        sub_node->get("shield-duration",
            &m_values[BUBBLEGUM_SHIELD_DURATION]);
    }

    if (const XMLNode *sub_node = node->getNode("zipper"))
    {
        sub_node->get("duration",
            &m_values[ZIPPER_DURATION]);
        sub_node->get("force",
            &m_values[ZIPPER_FORCE]);
        sub_node->get("speed-gain",
            &m_values[ZIPPER_SPEED_GAIN]);
        sub_node->get("max-speed-increase",
            &m_values[ZIPPER_MAX_SPEED_INCREASE]);
        sub_node->get("fade-out-time",
            &m_values[ZIPPER_FADE_OUT_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("swatter"))
    {
        sub_node->get("duration",
            &m_values[SWATTER_DURATION]);
        sub_node->get("distance",
            &m_values[SWATTER_DISTANCE]);
        sub_node->get("squash-duration",
            &m_values[SWATTER_SQUASH_DURATION]);
        sub_node->get("squash-slowdown",
            &m_values[SWATTER_SQUASH_SLOWDOWN]);
    }

    if (const XMLNode *sub_node = node->getNode("plunger"))
    {
        sub_node->get("band-max-length",
            &m_values[PLUNGER_BAND_MAX_LENGTH]);
        sub_node->get("band-force",
            &m_values[PLUNGER_BAND_FORCE]);
        sub_node->get("band-duration",
            &m_values[PLUNGER_BAND_DURATION]);
        sub_node->get("band-speed-increase",
            &m_values[PLUNGER_BAND_SPEED_INCREASE]);
        sub_node->get("band-fade-out-time",
            &m_values[PLUNGER_BAND_FADE_OUT_TIME]);
        sub_node->get("in-face-time",
            &m_values[PLUNGER_IN_FACE_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("startup"))
    {
        sub_node->get("time",
            &m_values[STARTUP_TIME]);
        sub_node->get("boost",
            &m_values[STARTUP_BOOST]);
    }

    if (const XMLNode *sub_node = node->getNode("rescue"))
    {
        sub_node->get("duration",
            &m_values[RESCUE_DURATION]);
        sub_node->get("vert-offset",
            &m_values[RESCUE_VERT_OFFSET]);
        sub_node->get("height",
            &m_values[RESCUE_HEIGHT]);
    }

    if (const XMLNode *sub_node = node->getNode("explosion"))
    {
        sub_node->get("duration",
            &m_values[EXPLOSION_DURATION]);
        sub_node->get("radius",
            &m_values[EXPLOSION_RADIUS]);
        sub_node->get("invulnerability-time",
            &m_values[EXPLOSION_INVULNERABILITY_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("nitro"))
    {
        sub_node->get("duration",
            &m_values[NITRO_DURATION]);
        sub_node->get("engine-force",
            &m_values[NITRO_ENGINE_FORCE]);
        sub_node->get("engine-mult",
            &m_values[NITRO_ENGINE_MULT]);
        sub_node->get("consumption",
            &m_values[NITRO_CONSUMPTION]);
        sub_node->get("small-container",
            &m_values[NITRO_SMALL_CONTAINER]);
        sub_node->get("big-container",
            &m_values[NITRO_BIG_CONTAINER]);
        sub_node->get("max-speed-increase",
            &m_values[NITRO_MAX_SPEED_INCREASE]);
        sub_node->get("fade-out-time",
            &m_values[NITRO_FADE_OUT_TIME]);
        sub_node->get("max",
            &m_values[NITRO_MAX]);
    }

    if (const XMLNode *sub_node = node->getNode("slipstream"))
    {
        sub_node->get("duration-factor",
            &m_values[SLIPSTREAM_DURATION_FACTOR]);
        sub_node->get("base-speed",
            &m_values[SLIPSTREAM_BASE_SPEED]);
        sub_node->get("length",
            &m_values[SLIPSTREAM_LENGTH]);
        sub_node->get("width",
            &m_values[SLIPSTREAM_WIDTH]);
        sub_node->get("inner-factor",
            &m_values[SLIPSTREAM_INNER_FACTOR]);
        sub_node->get("min-collect-time",
            &m_values[SLIPSTREAM_MIN_COLLECT_TIME]);
        sub_node->get("max-collect-time",
            &m_values[SLIPSTREAM_MAX_COLLECT_TIME]);
        sub_node->get("add-power",
            &m_values[SLIPSTREAM_ADD_POWER]);
        sub_node->get("min-speed",
            &m_values[SLIPSTREAM_MIN_SPEED]);
        sub_node->get("max-speed-increase",
            &m_values[SLIPSTREAM_MAX_SPEED_INCREASE]);
        sub_node->get("fade-out-time",
            &m_values[SLIPSTREAM_FADE_OUT_TIME]);
    }

    if (const XMLNode *sub_node = node->getNode("skid"))
    {
        sub_node->get("increase",
            &m_values[SKID_INCREASE]);
        sub_node->get("decrease",
            &m_values[SKID_DECREASE]);
        sub_node->get("max",
            &m_values[SKID_MAX]);
        sub_node->get("time-till-max",
            &m_values[SKID_TIME_TILL_MAX]);
        sub_node->get("visual",
            &m_values[SKID_VISUAL]);
        sub_node->get("visual-time",
            &m_values[SKID_VISUAL_TIME]);
        sub_node->get("revert-visual-time",
            &m_values[SKID_REVERT_VISUAL_TIME]);
        sub_node->get("min-speed",
            &m_values[SKID_MIN_SPEED]);
        sub_node->get("time-till-bonus",
            &m_values[SKID_TIME_TILL_BONUS]);
        sub_node->get("bonus-speed",
            &m_values[SKID_BONUS_SPEED]);
        sub_node->get("bonus-time",
            &m_values[SKID_BONUS_TIME]);
        sub_node->get("bonus-force",
            &m_values[SKID_BONUS_FORCE]);
        sub_node->get("physical-jump-time",
            &m_values[SKID_PHYSICAL_JUMP_TIME]);
        sub_node->get("graphical-jump-time",
            &m_values[SKID_GRAPHICAL_JUMP_TIME]);
        sub_node->get("post-skid-rotate-factor",
            &m_values[SKID_POST_SKID_ROTATE_FACTOR]);
        sub_node->get("reduce-turn-min",
            &m_values[SKID_REDUCE_TURN_MIN]);
        sub_node->get("reduce-turn-max",
            &m_values[SKID_REDUCE_TURN_MAX]);
        sub_node->get("enabled",
            &m_values[SKID_ENABLED]);
    }


    /* <characteristics-end getXml> */
}   // load
