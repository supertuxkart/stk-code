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

class Group:
    def __init__(self, baseName):
        self.baseName = baseName
        self.subNames = []

    def addName(self, name):
        self.subNames.append(name)

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
def joinSubName(g, subName, titleCase):
    words = toList(g.baseName) + toList(subName)
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

        # Search base name
        pos = line.find(":")
        if pos != -1:
            group = Group(line[:pos].strip())
            # Find all subnames
            while True:
                pos2 = line.find(",", pos + 1)
                # Are we already at the end?
                if pos2 == -1:
                    part = line[pos + 1:].strip()
                else:
                    part = line[pos + 1:pos2].strip()

                if len(part) != 0:
                    group.addName(part)

                # Find the next one
                pos, pos2 = pos2, pos

                # We are at the end, so stop searching
                if pos == -1:
                    break
            groups.append(group)
        else:
            # There is not base name, so use the whole line as one word
            group = Group(line.strip())
            group.addName("")
            groups.append(group)

    # Find longest name to align the function bodies
    nameLengthTitle = 0
    nameLengthUnderscore = 0
    for g in groups:
        for n in g.subNames:
            l = len(joinSubName(g, n, True))
            if l > nameLengthTitle:
                nameLengthTitle = l
            l = len(joinSubName(g, n, False))
            if l > nameLengthUnderscore:
                nameLengthUnderscore = l

    # Print the results
    print("Variables ****************************************")
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
    print("Getters ****************************************")

    for g in groups:
        print()
        for n in g.subNames:
            nameTitle = joinSubName(g, n, True)
            nameUnderscore = joinSubName(g, n, False)
            print("    float get{0}(){2} const {{ return m_{1};{3} }}".
                format(nameTitle, nameUnderscore, " " * (nameLengthTitle - len(nameTitle)),
                    " " * (nameLengthUnderscore - len(nameUnderscore))))

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
                    " " * (nameLengthUnderscore - len(nameUnderscore))))

if __name__ == '__main__':
    main()
