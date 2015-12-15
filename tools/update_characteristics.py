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

# This script uses create_kart_properties.py to create code and then replaces
# the code in the source files. The parts in the source are marked with tags, that
# contain the argument that has to be passed to create_kart_properties.py.
# The script has to be run from the root directory of this project.

import os
import re
import subprocess

from create_kart_properties import functions

def main():
    # Check, if it runs in the root directory
    if not os.path.isfile("tools/update_characteristics.py"):
        print("Please run this script in the root directory of the project.")
        exit(1)
    for operation, function in functions.items():
        result = subprocess.Popen("tools/create_kart_properties.py " +
            operation, shell = True,
            stdout = subprocess.PIPE).stdout.read().decode('UTF-8')
        with open("src/" + function[2], "r") as f:
            text = f.read()
        # Replace the text by using look behinds and look forwards
        text = re.sub("(?<=/\* \<characteristics-start " + operation +
            "\> \*/\\n)(.|\n)*(?=\\n\s*/\* <characteristics-end " + operation + "> \*/)", result, text)
        with open("src/" + function[2], "w") as f:
            f.write(text)

if __name__ == '__main__':
    main()

