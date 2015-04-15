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

# This script takes the list from abstract_characteristics.hpp and creates the code
# An empty line is expected to end the input

class GroupMember:
    def __init__(self, name, typeC, typeStr):
        self.name = name
        self.typeC = typeC
        self.typeStr = typeStr

    """E.g. power(std::vector<float>/floatVector)
       or speed(InterpolationArray)
       The default type is float"""
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

class Group:
    def __init__(self, baseName):
        self.baseName = baseName
        self.members = []

    def addMember(self, content):
        self.members.append(GroupMember.parse(content))

    """E.g. engine: power, gears(std::vector<Gear>/Gears)
       or mass(float) or only mass"""
    def parse(content):
        pos = content.find(":")
        if pos == -1:
            group = Group("")
            group.addMember(content)
            return group
        else:
            group = Group(content[:pos].strip())
            for m in content[pos + 1:].split(","):
                group.addMember(m)
            return group

"""Creates a list of words from a titlecase string"""
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

"""titleCase: true  = result is titlecase
              false = result has underscores"""
def joinSubName(group, member, titleCase):
    words = toList(group.baseName) + toList(member.name)
    first = True
    if titleCase:
        words = map(lambda w: w.title(), words)
        return "".join(words)
    else:
        return "_".join(words)

def main():
    # All variables are saved here in titlecase
    groups = []
    # Read in lines
    while True:
        line = input()
        if len(line) == 0:
            break
        groups.append(Group.parse(line))

    # Find longest name to align the function bodies
    nameLengthTitle = 0
    nameLengthUnderscore = 0
    for g in groups:
        for m in g.members:
            l = len(joinSubName(g, m, True))
            if l > nameLengthTitle:
                nameLengthTitle = l
            l = len(joinSubName(g, m, False))
            if l > nameLengthUnderscore:
                nameLengthUnderscore = l

    # Print the results
    print("Enum ****************************************")
    for g in groups:
        print()
        print("        // {0}".format(g.baseName.title()))
        for m in g.members:
            print("        {0},".format(joinSubName(g, m, False).upper()))

    print()
    print()
    print("Getters ****************************************")

    for g in groups:
        print()
        for m in g.members:
            nameTitle = joinSubName(g, m, True)
            nameUnderscore = joinSubName(g, m, False)
            print("    {0} get{1}() const {{ return m_{2}; }}".
                format(m.typeC, nameTitle, nameUnderscore))

    # Commented out code
    """print("Variables ****************************************")
    for g in groups:
        print()
        print("    // {0}".format(g.baseName.title()))
        for n in g.subNames:
            print("    float m_{0};".format(joinSubName(g, n, False)))

    print()
    print()
    print("Constructor ****************************************")
    lineLength = 4;
    line = "    "
    for g in groups:
        for n in g.subNames:
            name = "m_{0} = ".format(joinSubName(g, n, False))
            l = len(name)
            if lineLength + l > 80 and lineLength > 4:
                print(line)
                line = "    " + name
                lineLength = l + 4
            else:
                line += name
                lineLength += l
    if lineLength > 4:
        line += "1;"
        print(line)

    print()
    print()
    print("Setters ****************************************")
    for g in groups:
        print()
        for n in g.subNames:
            nameTitle = joinSubName(g, n, True)
            nameUnderscore = joinSubName(g, n, False)
            print("    void set{0}(float value){2} {{ m_{1}{3} = value; }}".
                format(nameTitle, nameUnderscore, " " * (nameLengthTitle - len(nameTitle)),
                    " " * (nameLengthUnderscore - len(nameUnderscore))))"""

if __name__ == '__main__':
    main()
