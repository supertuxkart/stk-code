#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#  2014, By konstin (http://github/konstin)
#
#  Removes all trailing whitespaces and replaces all tabs with four spacesin the
#  files with a given extension in a recursivly searched directory.
#  It can also count the number of code lines excluding comments and blank
#  lines.
#
#  Tested with python 2.7 and python 3

import os

def main():
    # -------------- config --------------
    extensions = ["cpp", "hpp", "c" , "h"]
    directory = "../src"
    # Counts files and lines if enabled
    statistics = True
    # ------------------------------------

    if statistics:
        lines_total  = 0
        lines_code   = 0
        file_counter = 0

    for dirpath, dirnames, filenames in os.walk(directory):
        for filename in filenames:
            if statistics:
                file_counter += 1

            if (filename.rfind(".") != -1 and
                             filename[ filename.rfind(".")+1 : ] in extensions):
                # reading
                src_file = open(dirpath + "/" + filename, "r")
                lines = src_file.readlines()

                if statistics:
                    lines_total += len(lines)

                modified = False
                for i in range(len(lines)):
                    oldLine = lines[i]
                    # replacing tabs with four spaces
                    lines[i] = lines[i].replace("\t", "    ")
                    
                    if lines[i].rstrip() != "": # don't de-indent empty lines
                        lines[i] = lines[i].rstrip() + "\n"
                        if statistics:
                            if lines[i].lstrip().startswith("//"):
                                lines_code += 1
                    if not modified and oldLine != lines[i]:
                        modified = True
                src_file.close()

                # writing back
                if modified:
                    src_file = open(dirpath + "/" + filename, "w")
                    src_file.write("".join(lines))
                    src_file.close()

    if statistics:
        print("Total number of files in " + directory + ": "
                                                            + str(file_counter))
        print("Lines in total in: " + str(lines_total))
        print("↳ excluding comments and blank lines: " + str(lines_code)+ "\n")

    print("Finished.")


if __name__ == '__main__':
    main()

