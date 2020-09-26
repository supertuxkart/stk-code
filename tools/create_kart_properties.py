#!/usr/bin/env python3
#
#  SuperTuxKart - a fun racing game with go-kart
#  Copyright (C) 2006-2015 SuperTuxKart-Team
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 3
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# This script creates code for the characteristics.
# It takes an argument that specifies what the output of the script should be.
# The output options can be seen by running this script without arguments.
# A more convenient version to update the code is to run update_characteristics.py

import sys

# Input data
# Each line contains a topic and the attributes of that topic.
# This model is used for the xml file and to access the kart properties in the code.
characteristics = """Suspension: stiffness, rest, travel, expSpringResponse(bool), maxForce
Stability: rollInfluence, chassisLinearDamping, chassisAngularDamping, downwardImpulseFactor, trackConnectionAccel, angularFactor(std::vector<float>/floatVector), smoothFlyingImpulse
Turn: radius(InterpolationArray), timeResetSteer, timeFullSteer(InterpolationArray)
Engine: power, maxSpeed, genericMaxSpeed, brakeFactor, brakeTimeIncrease, maxSpeedReverseRatio
Gear: switchRatio(std::vector<float>/floatVector), powerIncrease(std::vector<float>/floatVector)
Mass
Wheels: dampingRelaxation, dampingCompression
Jump: animationTime
Lean: max, speed
Anvil: duration, weight, speedFactor
Parachute: friction, duration, durationOther, durationRankMult, durationSpeedMult, lboundFraction, uboundFraction, maxSpeed
Friction: kartFriction
Bubblegum: duration, speedFraction, torque, fadeInTime, shieldDuration
Zipper: duration, force, speedGain, maxSpeedIncrease, fadeOutTime
Swatter: duration, distance, squashDuration, squashSlowdown
Plunger: bandMaxLength, bandForce, bandDuration, bandSpeedIncrease, bandFadeOutTime, inFaceTime
Startup: time(std::vector<float>/floatVector), boost(std::vector<float>/floatVector)
Rescue: duration, vertOffset, height
Explosion: duration, radius, invulnerabilityTime
Nitro: duration, engineForce, engineMult, consumption, smallContainer, bigContainer, maxSpeedIncrease, fadeOutTime, max
Slipstream: durationFactor, baseSpeed, length, width, innerFactor, minCollectTime, maxCollectTime, addPower, minSpeed, maxSpeedIncrease, fadeOutTime
Skid: increase, decrease, max, timeTillMax, visual, visualTime, revertVisualTime, minSpeed, timeTillBonus(std::vector<float>/floatVector), bonusSpeed(std::vector<float>/floatVector), bonusTime(std::vector<float>/floatVector), bonusForce(std::vector<float>/floatVector), physicalJumpTime, graphicalJumpTime, postSkidRotateFactor, reduceTurnMin, reduceTurnMax, enabled(bool)"""

""" A GroupMember is an attribute of a group.
    In the xml files, a value will be assigned to it.
    If the name of the attribute is 'value', the getter method will only
    contain the group name and 'value' will be omitted (e.g. used for mass). """
class GroupMember:
    def __init__(self, name, typeC, typeStr):
        self.name = name
        if name == "value":
            self.getName = ""
        else:
            self.getName = name
        self.typeC = typeC
        self.typeStr = typeStr

    """ E.g. power(std::vector<float>/floatVector)
        or speed(InterpolationArray)
        The default type is float
        The name 'value' is special: Only the group name will be used to access
            the member but in the xml file it will be still value (because we
            need a name). """
    def parse(content):
        typeC = "float"
        typeStr = typeC
        name = content.strip()
        pos = content.find("(")
        end = content.find(")", pos)
        if pos != -1 and end != -1:
            name = content[:pos].strip()
            pos2 = content.find("/", pos, end)
            if pos2 != -1:
                typeC = content[pos + 1:pos2].strip()
                typeStr = content[pos2 + 1:end].strip()
            else:
                typeC = content[pos + 1:end].strip()
                typeStr = typeC

        return GroupMember(name, typeC, typeStr)

""" A Group has a base name and can contain GroupMembers.
    In the xml files, a group is a tag. """
class Group:
    def __init__(self, baseName):
        self.baseName = baseName
        self.members = []

    """ Parses and adds a member to this group """
    def addMember(self, content):
        self.members.append(GroupMember.parse(content))

    def getBaseName(self):
        return self.baseName

    """ E.g. engine: power, gears(std::vector<Gear>/Gears)
        or mass(float) or only mass """
    def parse(content):
        pos = content.find(":")
        if pos == -1:
            group = Group(content)
            group.addMember("value")
            return group
        else:
            group = Group(content[:pos].strip())
            for m in content[pos + 1:].split(","):
                group.addMember(m)
            return group

""" Creates a list of words from a titlecase string """
def toList(name):
    result = []
    cur = ""
    for c in name:
        if c.isupper() and len(cur) != 0:
            result.append(cur)
            cur = ""
        cur += c.lower()
    if len(cur) != 0:
        result.append(cur)
    return result

""" titleCase: true  = result is titlecase
               false = result has underscores """
def joinSubName(group, member, titleCase):
    words = toList(group.baseName) + toList(member.getName)
    first = True
    if titleCase:
        words = [w.title() for w in words]
        return "".join(words)
    else:
        return "_".join(words)

# Functions to generate code

def createEnum(groups):
    for g in groups:
        print()
        print("        // {0}".format(g.getBaseName().title()))
        for m in g.members:
            print("        {0},".format(joinSubName(g, m, False).upper()))

def createAcDefs(groups):
    for g in groups:
        print()
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False)
            typeC = m.typeC

            print("    {0} get{1}() const;".
                format(typeC, nameTitle, nameUnderscore))

def createAcGetter(groups):
    for g in groups:
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False)
            typeC = m.typeC
            result = "result"

            print("""// ----------------------------------------------------------------------------
{3} AbstractCharacteristic::get{1}() const
{{
    {0} result;
    bool is_set = false;
    process({2}, &result, &is_set);
    if (!is_set)
        Log::fatal("AbstractCharacteristic", "Can't get characteristic %s",
                    getName({2}).c_str());
    return {4};
}}  // get{1}
""".format(m.typeC, nameTitle, nameUnderscore.upper(), typeC, result))

def createKpDefs(groups):
    for g in groups:
        print()
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False)
            typeC = m.typeC

            print("    {0} get{1}() const;".
                format(typeC, nameTitle, nameUnderscore))

def createKpGetter(groups):
    for g in groups:
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False)
            typeC = m.typeC
            result = "result"

            print("""// ----------------------------------------------------------------------------
{1} KartProperties::get{0}() const
{{
    return m_cached_characteristic->get{0}();
}}  // get{0}
""".format(nameTitle, typeC))

def createGetType(groups):
    for g in groups:
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False)
            print("    case {0}:\n        return TYPE_{1};".
                format(nameUnderscore.upper(), "_".join(toList(m.typeStr)).upper()))

def createGetName(groups):
    for g in groups:
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False).upper()
            print("    case {0}:\n        return \"{0}\";".
                format(nameUnderscore))

def createLoadXml(groups):
    for g in groups:
        print("    if (const XMLNode *sub_node = node->getNode(\"{0}\"))\n    {{".
            format(g.baseName.lower()))
        for m in g.members:
            nameUnderscore = joinSubName(g, m, False)
            nameMinus = "-".join(toList(m.name))
            print("""        sub_node->get(\"{0}\",
            &m_values[{1}]);""".
                format(nameMinus, nameUnderscore.upper()))
        print("    }\n")

# Dicionary that maps an argument string to a tupel of
# a generator function, a help string and a filename
functions = {
    "enum":     (createEnum,     "List the enum values for all characteristics",           "karts/abstract_characteristic.hpp"),
    "acdefs":   (createAcDefs,   "Create the header function definitions",                 "karts/abstract_characteristic.hpp"),
    "acgetter": (createAcGetter, "Implement the getters",                                  "karts/abstract_characteristic.cpp"),
    "getType":  (createGetType,  "Implement the getType function",                         "karts/abstract_characteristic.cpp"),
    "getName":  (createGetName,  "Implement the getName function",                         "karts/abstract_characteristic.cpp"),
    "kpdefs":   (createKpDefs,   "Create the header function definitions for the getters", "karts/kart_properties.hpp"),
    "kpgetter": (createKpGetter, "Implement the getters",                                  "karts/kart_properties.cpp"),
    "loadXml":  (createLoadXml,  "Code to load the characteristics from an xml file",      "karts/xml_characteristic.cpp"),
}

def main():
    # Find out what to do
    if len(sys.argv) != 2:
        print("""Usage: ./create_kart_properties.py <operation>
Operations:""")
        maxOperationLength = 0
        maxDescriptionLength = 0
        for o, f in functions.items():
            l = len(o)
            if l > maxOperationLength:
                maxOperationLength = l
            l = len(f[1])
            if l > maxDescriptionLength:
                maxDescriptionLength = l

        formatString = "    {{0:{0}}}    {{1:{1}}} in {{2}}".format(maxOperationLength, maxDescriptionLength)
        for o, f in functions.items():
            print(formatString.format(o, f[1], f[2]))
        return

    task = sys.argv[1]

    if task not in functions:
        print("The wanted operation was not found. Please call this script without arguments to list available arguments.")
        return

    # Parse properties
    groups = [Group.parse(line) for line in characteristics.split("\n")]

    # Create the wanted code
    functions[task][0](groups)

if __name__ == '__main__':
    main()
